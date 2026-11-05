#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/Node.h"
#include "outputs/OutputManager.h"

// GetBufferSize answers "how big is this buffer" without building it; the
// render buffer sizes sub-buffers and value curves from it. It must agree with
// the buffer InitRenderBufferNodes actually builds for the same style.
namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        return m;
    }

    // A small show with one of most model shapes, spread out in the preview.
    void Populate() {
        Add(R"(<model name="Line" DisplayAs="Single Line" NumStrings="1" NodesPerString="7" StartChannel="1" WorldPosX="10" WorldPosY="10" X2="60" Y2="0"/>)");
        Add(R"(<model name="Matrix" DisplayAs="Horiz Matrix" NumStrings="2" NodesPerString="20" StrandsPerString="4" StartChannel="1" WorldPosX="300" WorldPosY="200" ScaleX="2" ScaleY="1"/>)");
        Add(R"(<model name="Tree" DisplayAs="Tree 360" NumStrings="6" NodesPerString="10" StrandsPerString="1" StartChannel="1" WorldPosX="500" WorldPosY="100"/>)");
        Add(R"(<model name="Star" DisplayAs="Star" NumStrings="1" NodesPerString="25" StrandsPerString="5" StartChannel="1" WorldPosX="150" WorldPosY="300"/>)");
        Add(R"(<model name="Arches" DisplayAs="Arches" NumStrings="3" NodesPerString="8" LightsPerNode="1" StartChannel="1" WorldPosX="200" WorldPosY="50" X2="100" Y2="0"/>)");
        Add(R"(<model name="Circle" DisplayAs="Circle" NumStrings="1" NodesPerString="16" StartChannel="1" WorldPosX="400" WorldPosY="400"/>)");
        Add(R"(<model name="Custom" DisplayAs="Custom" CustomWidth="3" CustomHeight="2" CustomModel="1,,2;3,4,5" StartChannel="1" WorldPosX="50" WorldPosY="450"/>)");
        // One white channel lighting several cells, as a dumb string drawn as a shape
        Add(R"(<model name="CustomSingle" DisplayAs="Custom" CustomWidth="3" CustomHeight="2" CustomModel="1,1,1;1,,1" StringType="Single Color White" StartChannel="1" WorldPosX="80" WorldPosY="450"/>)");
        Add(R"(<model name="Custom3D" DisplayAs="Custom" CustomWidth="3" CustomHeight="2" Depth="2" CMDepth="2" CustomModel="1,,2;3,4,5|6,7,;,8," StartChannel="1" WorldPosX="120" WorldPosY="450"/>)");
        mm.RecalcStartChannels();
    }

    ModelGroup* Group(const std::string& name, const std::string& members, const std::string& extra = "") {
        auto* g = dynamic_cast<ModelGroup*>(Add("<modelGroup name=\"" + name + "\" models=\"" + members + "\" " + extra + "/>"));
        REQUIRE(g != nullptr);
        return g;
    }
};

struct Size {
    int w = 0;
    int h = 0;
    bool operator==(const Size& o) const { return w == o.w && h == o.h; }
};

Size Reported(const Model* m, const std::string& style, const std::string& transform, int stagger = 0) {
    Size s;
    m->GetBufferSize(style, "2D", transform, s.w, s.h, stagger);
    return s;
}

Size Built(Model* m, const std::string& style, const std::string& transform, int stagger = 0) {
    std::vector<NodeBaseClassPtr> nodes;
    Size s;
    m->InitRenderBufferNodes(style, "2D", transform, nodes, s.w, s.h, stagger);
    return s;
}

// Every node of a built buffer must land inside it.
bool NodesInside(Model* m, const std::string& style, const std::string& transform, int stagger = 0) {
    std::vector<NodeBaseClassPtr> nodes;
    int w = 0, h = 0;
    m->InitRenderBufferNodes(style, "2D", transform, nodes, w, h, stagger);
    for (const auto& n : nodes) {
        for (const auto& c : n->Coords) {
            if (c.bufX < 0 || c.bufY < 0 || c.bufX >= w || c.bufY >= h) {
                return false;
            }
        }
    }
    return true;
}

bool IsPerModel(const std::string& style) {
    return style.compare(0, 9, "Per Model") == 0;
}

// The transforms the buffer panel offers (resources/effectmetadata/shared/Buffer.json)
const char* kTransforms[] = { "None", "Rotate CC 90", "Rotate CW 90", "Rotate 180", "Flip Vertical", "Flip Horizontal",
                              "Rotate CC 90 Flip Horizontal", "Rotate CW 90 Flip Horizontal" };

} // namespace

TEST_CASE("Model buffer size matches the buffer built for every style", "[models][buffersize]") {
    ShowFixture f;
    f.Populate();
    for (const char* name : { "Line", "Matrix", "Tree", "Star", "Arches", "Circle", "Custom", "CustomSingle", "Custom3D" }) {
        Model* m = f.mm.GetModel(name);
        REQUIRE(m != nullptr);
        for (const auto& style : m->GetBufferStyles()) {
            for (const char* t : kTransforms) {
                INFO(name << " / " << style << " / " << t);
                Size built = Built(m, style, t);
                Size reported = Reported(m, style, t);
                CHECK(reported.w == built.w);
                CHECK(reported.h == built.h);
                CHECK(NodesInside(m, style, t));
            }
        }
    }
}

TEST_CASE("Group buffer size matches the buffer built for every style", "[models][buffersize][group]") {
    ShowFixture f;
    f.Populate();
    ModelGroup* g = f.Group("All", "Line,Matrix,Tree,Star,Arches,Circle,Custom", "layout=\"minimalGrid\" GridSize=\"400\"");
    for (const auto& style : g->GetBufferStyles()) {
        if (IsPerModel(style)) {
            // Rendered model by model, not as one group buffer.
            continue;
        }
        for (const char* t : kTransforms) {
            INFO(style << " / " << t);
            Size built = Built(g, style, t);
            Size reported = Reported(g, style, t);
            CHECK(reported.w == built.w);
            CHECK(reported.h == built.h);
            CHECK(NodesInside(g, style, t));
        }
    }
}

TEST_CASE("Group buffer size matches the buffer built with a stagger", "[models][buffersize][group]") {
    ShowFixture f;
    f.Populate();
    ModelGroup* g = f.Group("All", "Line,Matrix,Star", "layout=\"minimalGrid\" GridSize=\"400\"");
    for (const char* style : { "Horizontal Stack", "Vertical Stack" }) {
        for (int stagger : { 0, 2, 5 }) {
            INFO(style << " stagger " << stagger);
            CHECK(Reported(g, style, "None", stagger) == Built(g, style, "None", stagger));
            CHECK(NodesInside(g, style, "None", stagger));
        }
    }
}

TEST_CASE("A group of groups reports the buffer it builds", "[models][buffersize][group]") {
    ShowFixture f;
    f.Populate();
    f.Group("Inner", "Line,Star", "layout=\"minimalGrid\" GridSize=\"400\"");
    ModelGroup* outer = f.Group("Outer", "Inner,Matrix", "layout=\"minimalGrid\" GridSize=\"400\"");
    for (const auto& style : outer->GetBufferStyles()) {
        if (IsPerModel(style)) {
            continue;
        }
        INFO(style);
        CHECK(Reported(outer, style, "None") == Built(outer, style, "None"));
    }
}

TEST_CASE("Group stack and per model styles size by the members", "[models][buffersize][group]") {
    ShowFixture f;
    f.Populate();
    ModelGroup* g = f.Group("G", "Line,Matrix", "layout=\"minimalGrid\" GridSize=\"400\"");
    Model* line = f.mm.GetModel("Line");
    Model* matrix = f.mm.GetModel("Matrix");
    // One column per model, as tall as the model with the most nodes
    Size hpm = Reported(g, "Horizontal Per Model", "None");
    CHECK(hpm.w == 2);
    CHECK(hpm.h == (int)std::max(line->GetNodeCount(), matrix->GetNodeCount()));
    Size vpm = Reported(g, "Vertical Per Model", "None");
    CHECK(vpm.h == 2);
    CHECK(vpm.w == hpm.h);
    // Every node in one row
    Size sl = Reported(g, "Single Line", "None");
    CHECK(sl.h == 1);
    CHECK(sl.w == (int)(line->GetNodeCount() + matrix->GetNodeCount()));
    // One pixel per model
    Size slap = Reported(g, "Single Line Model As A Pixel", "None");
    CHECK(slap.w == 2);
    CHECK(slap.h == 1);
}

TEST_CASE("Group buffer ignores inactive members", "[models][buffersize][group]") {
    ShowFixture f;
    f.Populate();
    ModelGroup* g = f.Group("G", "Line,Matrix", "layout=\"minimalGrid\" GridSize=\"400\"");
    Size before = Reported(g, "Horizontal Per Model", "None");
    f.mm.GetModel("Matrix")->SetActive(false);
    Size after = Reported(g, "Horizontal Per Model", "None");
    CHECK(before.w == 2);
    CHECK(after.w == 1);
    CHECK(after.h == (int)f.mm.GetModel("Line")->GetNodeCount());
}

TEST_CASE("An empty group reports a one by one buffer", "[models][buffersize][group]") {
    ShowFixture f;
    ModelGroup* g = f.Group("Empty", "");
    for (const auto& style : g->GetBufferStyles()) {
        INFO(style);
        Size s = Reported(g, style, "None");
        CHECK(s.w >= 1);
        CHECK(s.h >= 1);
    }
}
