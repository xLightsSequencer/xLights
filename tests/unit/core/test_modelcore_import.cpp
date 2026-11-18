#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/BoxedScreenLocation.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/PolyLineModel.h"
#include "outputs/OutputManager.h"
#include "XmlSerializer/XmlSerializer.h"

#include "test_modelcore_fakegfx.h"

using Catch::Approx;
using fakegfx::FakePreview;

namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 1000, 1000);
        REQUIRE(m != nullptr);
        mm.RecalcStartChannels();
        return m;
    }

    // Import a saved model file's root element over a placeholder model, the
    // way the layout's "import model" does.
    Model* Import(const std::string& xml, bool& cancelled) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* base = mm.CreateDefaultModel("Custom", "17");
        REQUIRE(base != nullptr);
        base->SetLayoutGroup("Back Yard");
        base->SetHcenterPos(300);
        base->SetVcenterPos(200);
        cancelled = false;
        return base->CreateDefaultModelFromSavedModelNode(base, doc.first_child(), mm, cancelled);
    }
};

} // namespace

TEST_CASE("Saved models of every kind import over the placeholder", "[modelcore][import]") {
    struct Case {
        const char* element;
        const char* attrs;
        const char* displayAs;
        uint32_t nodes;
        bool keepsPosition;
    };
    const Case cases[] = {
        { "custommodel", "CustomWidth=\"2\" CustomHeight=\"1\" CustomModel=\"1,2\"", "Custom", 2, false },
        { "polylinemodel", "DisplayAs=\"Poly Line\" NodesPerString=\"4\" NumPoints=\"2\" PointData=\"0,0,0,1,0,0\"", "Poly Line", 4, false },
        { "multipointmodel", "DisplayAs=\"MultiPoint\" NodesPerString=\"3\" NumPoints=\"3\" PointData=\"0,0,0,1,0,0,2,0,0\"", "MultiPoint", 3, false },
        { "matrixmodel", "DisplayAs=\"Matrix\" NumStrings=\"1\" NodesPerString=\"4\" StrandsPerString=\"2\"", "Matrix", 4, false },
        { "starmodel", "DisplayAs=\"Star\" NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\"", "Star", 10, false },
        { "treemodel", "DisplayAs=\"Tree 360\" NumStrings=\"2\" NodesPerString=\"4\" StrandsPerString=\"1\"", "Tree", 8, false },
        { "circlemodel", "DisplayAs=\"Circle\" NumStrings=\"1\" NodesPerString=\"6\"", "Circle", 6, false },
        { "spheremodel", "DisplayAs=\"Sphere\" NumStrings=\"1\" NodesPerString=\"6\" StrandsPerString=\"2\"", "Sphere", 6, false },
        { "iciclemodel", "DisplayAs=\"Icicles\" NumStrings=\"1\" NodesPerString=\"6\" DropPattern=\"3,2\"", "Icicles", 6, false },
        { "Cubemodel", "DisplayAs=\"Cube\" CubeWidth=\"2\" CubeHeight=\"2\" CubeDepth=\"2\"", "Cube", 8, true },
        { "archesmodel", "DisplayAs=\"Arches\" NumArches=\"2\" NodesPerArch=\"5\"", "Arches", 10, true },
        { "singlelinemodel", "DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"7\"", "Single Line", 7, true },
    };
    for (const auto& c : cases) {
        INFO(c.element);
        ShowFixture f;
        bool cancelled = true;
        Model* m = f.Import(std::string("<") + c.element + " name=\"Imported\" " + c.attrs + "/>", cancelled);
        REQUIRE(m != nullptr);
        CHECK_FALSE(cancelled);
        CHECK(m->GetNodeCount() == c.nodes);
        CHECK(DisplayAsTypeToString(m->GetDisplayAs()).find(c.displayAs) != std::string::npos);
        if (c.keepsPosition) {
            // the placeholder's start channel, place and layout group carry over
            CHECK(m->GetModelStartChannel() == "17");
            CHECK(m->GetHcenterPos() == Approx(300.0f).margin(0.5));
            CHECK(m->GetVcenterPos() == Approx(200.0f).margin(0.5));
            CHECK(m->GetLayoutGroup() == "Back Yard");
            CHECK(m->Selected());
        }
        delete m;
    }
}

TEST_CASE("Saved DMX models import with the placeholder's scale", "[modelcore][import]") {
    for (const char* el : { "dmxmodel", "dmxgeneral", "dmxservo", "dmxservo3d" }) {
        INFO(el);
        ShowFixture f;
        bool cancelled = true;
        std::string type = std::string(el) == "dmxmodel" ? "DmxGeneral" : (std::string(el) == "dmxgeneral" ? "DmxGeneral" : (std::string(el) == "dmxservo" ? "DmxServo" : "DmxServo3d"));
        Model* m = f.Import(std::string("<") + el + " name=\"D\" DisplayAs=\"" + type + "\" DmxChannelCount=\"4\"/>", cancelled);
        REQUIRE(m != nullptr);
        CHECK_FALSE(cancelled);
        CHECK(m->IsDMXModel());
        delete m;
    }
}

TEST_CASE("Saved model files may wrap the model in a models element", "[modelcore][import]") {
    ShowFixture f;
    bool cancelled = true;
    Model* m = f.Import(R"(<models><model name="W" DisplayAs="Single Line" NumStrings="1" NodesPerString="3"/></models>)", cancelled);
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeCount() == 3);
    delete m;

    // an empty wrapper leaves the placeholder
    ShowFixture g;
    Model* base = g.mm.CreateDefaultModel("Custom", "1");
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<models/>"));
    bool c2 = false;
    CHECK(base->CreateDefaultModelFromSavedModelNode(base, doc.first_child(), g.mm, c2) == base);
    delete base;
}

TEST_CASE("Unknown saved model types cancel the import", "[modelcore][import]") {
    ShowFixture f;
    bool cancelled = false;
    Model* m = nullptr;
    try {
        m = f.Import(R"(<teapotmodel name="T" DisplayAs="Teapot"/>)", cancelled);
    } catch (...) {
        cancelled = true;
    }
    CHECK(cancelled);
    CHECK(m == nullptr);
}

TEST_CASE("Shadow models in a saved model are imported beside it", "[modelcore][import]") {
    ShowFixture f;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<custommodel name="Face" CustomWidth="2" CustomHeight="1" CustomModel="1,2" WorldPosX="400" WorldPosY="300">
        <shadowmodels>
          <model name="Spot" DisplayAs="Single Line" NumStrings="1" NodesPerString="2" StartChannel="1"/>
        </shadowmodels>
        <associatedmodels>
          <model name="Halo" DisplayAs="Circle" NumStrings="1" NodesPerString="4" StartChannel="1"/>
        </associatedmodels>
    </custommodel>)"));
    XmlSerializer s;
    Model* face = s.DeserializeModel(doc.first_child(), f.mm, true);
    REQUIRE(face != nullptr);
    Model* spot = f.mm.GetModel("Spot");
    Model* halo = f.mm.GetModel("Halo");
    REQUIRE(spot != nullptr);
    REQUIRE(halo != nullptr);
    CHECK(spot->GetLayoutGroup() == "Unassigned");
    CHECK(halo->GetLayoutGroup() == face->GetLayoutGroup());
    // they are not on a controller until the user wires them
    CHECK(spot->GetControllerName() == NO_CONTROLLER);
    // placed to the right of the model, 20 apart
    CHECK(spot->GetHcenterPos() == Approx(face->GetHcenterPos() + 20).margin(1.0));
    CHECK(halo->GetHcenterPos() == Approx(face->GetHcenterPos() + 20).margin(1.0));
    delete face;
}

TEST_CASE("Base model strand mapping", "[modelcore][import]") {
    ShowFixture f;
    Model* line = f.Add(R"(<model name="L" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="6"/>)");
    CHECK(line->MapToNodeIndex(0, 4) == 4);
    CHECK(line->GetStrandLength(0) == 6);
    Model* tree = f.Add(R"(<model name="T" DisplayAs="Tree 360" StartChannel="1" NumStrings="2" NodesPerString="6" StrandsPerString="2"/>)");
    CHECK(tree->GetNumStrands() == 4);
    CHECK(tree->MapToNodeIndex(3, 1) == 3 * 6 / 2 + 1);
    Model* single = f.Add(R"(<model name="S" DisplayAs="Tree 360" StartChannel="1" NumStrings="3" NodesPerString="4" StrandsPerString="1" StringType="Single Color Red"/>)");
    CHECK(single->MapToNodeIndex(2, 3) == 2);
}

TEST_CASE("Effect preview of a dropping poly line uses buffer coordinates", "[modelcore][import][preview]") {
    ShowFixture f;
    auto* p = dynamic_cast<PolyLineModel*>(f.Add(R"(<model name="P" DisplayAs="Poly Line" StartChannel="1" NodesPerString="6" NumPoints="2" PointData="0,0,0,10,0,0" DropPattern="3"/>)"));
    REQUIRE(p != nullptr);
    REQUIRE(p->UsesBufCoordsForModelPreview());
    FakePreview pv(200, 200, false);
    p->DisplayEffectOnWindow(&pv, 2.0);
    REQUIRE(pv.ctx.ivas.size() == 1);
    auto* v = pv.ctx.ivas.back();
    REQUIRE(v->v.size() == 6);
    for (auto& e : v->v) {
        CHECK(e.x == (float)p->GetNode(e.idx)->Coords[0].bufX);
        CHECK(e.y == (float)p->GetNode(e.idx)->Coords[0].bufY);
    }
    pv.Run();
    CHECK(pv.ctx.points == 1);
}

TEST_CASE("Group effect preview switches back from circles to points", "[modelcore][import][preview]") {
    ShowFixture f;
    Model* a = f.Add(R"(<model name="A" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="3" WorldPosX="100" WorldPosY="100"/>)");
    Model* b = f.Add(R"(<model name="B" DisplayAs="Single Line" StartChannel="1" NumStrings="1" NodesPerString="3" WorldPosX="300" WorldPosY="100"/>)");
    a->SetPixelStyle(Model::PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE);
    b->SetPixelStyle(Model::PIXEL_STYLE::PIXEL_STYLE_SQUARE);
    Model* g = f.Add(R"(<modelGroup name="G" models="A,B" layout="minimalGrid" GridSize="400"/>)");
    g->Reinitialize();
    FakePreview pv(400, 400, false);
    g->DisplayEffectOnWindow(&pv, 2.0);
    pv.Run();
    CHECK(pv.ctx.triangles == 1);
    CHECK(pv.ctx.points == 1);
}
