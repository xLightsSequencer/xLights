#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/SubModel.h"
#include "outputs/OutputManager.h"

namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        mm.RecalcStartChannels();
        return m;
    }

    Model* Matrix(const std::string& subs) {
        return Add("<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"24\" StrandsPerString=\"4\">" + subs + "</model>");
    }
};

using Pt = std::pair<int, int>;

std::vector<Pt> Cells(const Model* m) {
    std::vector<Pt> pts;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        pts.push_back({ c.bufX, c.bufY });
    }
    return pts;
}

std::vector<int> Chans(const Model* m) {
    std::vector<int> r;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        r.push_back(m->NodeStartChannel(i) / 3);
    }
    return r;
}

std::string SubXml(const std::string& layout, const std::string& style, const std::string& lines) {
    return "<subModel name=\"s\" layout=\"" + layout + "\" type=\"ranges\" bufferstyle=\"" + style + "\" " + lines + "/>";
}

std::string SubBufferXml(const std::string& rect) {
    return "<subModel name=\"s\" layout=\"horizontal\" type=\"subbuffer\" subBuffer=\"" + rect + "\"/>";
}

SubModel* Sub(Model* m) {
    Model* s = m->GetSubModel("s");
    REQUIRE(s != nullptr);
    auto* sm = dynamic_cast<SubModel*>(s);
    REQUIRE(sm != nullptr);
    return sm;
}

} // namespace

TEST_CASE("Horizontal ranges submodel makes each line a row", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\" line1=\"7-9\""));
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() == 6);
    CHECK(s->GetDefaultBufferWi() == 3);
    CHECK(s->GetDefaultBufferHt() == 2);
    CHECK(Cells(s) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } });
    CHECK(Chans(s) == std::vector<int>{ 0, 1, 2, 6, 7, 8 });
}

TEST_CASE("Vertical ranges submodel makes each line a column", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("vertical", "Default", "line0=\"1-3\" line1=\"7-9\""));
    SubModel* s = Sub(m);
    CHECK(s->GetDefaultBufferWi() == 2);
    CHECK(s->GetDefaultBufferHt() == 3);
    CHECK(Cells(s) == std::vector<Pt>{ { 0, 0 }, { 0, 1 }, { 0, 2 }, { 1, 0 }, { 1, 1 }, { 1, 2 } });
    CHECK(Chans(s) == std::vector<int>{ 0, 1, 2, 6, 7, 8 });
}

TEST_CASE("Submodel reports its layout type and style", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("vertical", "Keep XY", "line0=\"1-3\""));
    SubModel* s = Sub(m);
    CHECK(s->GetSubModelLayout() == "vertical");
    CHECK(s->GetSubModelType() == "ranges");
    CHECK(s->GetSubModelBufferStyle() == "Keep XY");
    CHECK(s->GetSubModelLines() == "1-3");
    CHECK(s->IsXYBufferStyle());
    CHECK(s->GetDisplayAs() == DisplayAsType::SubModel);
    CHECK(s->GetParent() == m);
}

TEST_CASE("Keep XY submodel preserves the parent geometry", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Keep XY", "line0=\"1-3\" line1=\"13-15\""));
    SubModel* s = Sub(m);
    CHECK(s->GetDefaultBufferWi() == 3);
    CHECK(s->GetDefaultBufferHt() == 3);
    CHECK(Cells(s) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 2 }, { 1, 2 }, { 2, 2 } });
}

TEST_CASE("Default style packs lines even when the parent rows are far apart", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\" line1=\"13-15\""));
    SubModel* s = Sub(m);
    CHECK(s->GetDefaultBufferHt() == 2);
    CHECK(Cells(s) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } });
}

TEST_CASE("Zero cells in a line become gaps", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1,0,3\" line1=\"7\""));
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() == 3);
    CHECK(s->GetDefaultBufferWi() == 3);
    CHECK(s->GetDefaultBufferHt() == 2);
    CHECK(Cells(s) == std::vector<Pt>{ { 0, 0 }, { 2, 0 }, { 0, 1 } });
    CHECK(Chans(s) == std::vector<int>{ 0, 2, 6 });
}

TEST_CASE("Submodel line ranges can be listed in any order and mixed with single nodes", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"5,1-2,9\""));
    SubModel* s = Sub(m);
    CHECK(Chans(s) == std::vector<int>{ 4, 0, 1, 8 });
    CHECK(s->GetDefaultBufferWi() == 4);
    CHECK(s->GetDefaultBufferHt() == 1);
}

TEST_CASE("Submodel same line and cross line duplicates are reported one based", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1,1,3\" line1=\"3,4\""));
    SubModel* s = Sub(m);
    CHECK(s->GetSameLineDuplicates() == "1");
    CHECK(s->GetCrossLineDuplicates() == "3");
    CHECK(s->GetNodeCount() == 3);
}

TEST_CASE("Submodel without duplicates reports none", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-4\" line1=\"5-8\""));
    SubModel* s = Sub(m);
    CHECK(s->GetSameLineDuplicates().empty());
    CHECK(s->GetCrossLineDuplicates().empty());
}

TEST_CASE("Sub buffer rectangle selects the parent nodes inside it", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubBufferXml("0x0x50x50"));
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() == 12);
    CHECK(s->GetDefaultBufferWi() == 4);
    CHECK(s->GetDefaultBufferHt() == 3);
    for (const auto& c : Cells(s)) {
        CHECK(c.first >= 0);
        CHECK(c.first < 4);
        CHECK(c.second >= 0);
        CHECK(c.second < 3);
    }
}

TEST_CASE("Sub buffer in the upper right corner starts at its own origin", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubBufferXml("50x50x100x100"));
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() == 6);
    CHECK(s->GetDefaultBufferWi() == 3);
    CHECK(s->GetDefaultBufferHt() == 2);
    CHECK(Chans(s) == std::vector<int>{ 15, 16, 17, 18, 19, 20 });
    CHECK(Cells(s) == std::vector<Pt>{ { 0, 0 }, { 1, 0 }, { 2, 0 }, { 2, 1 }, { 1, 1 }, { 0, 1 } });
}

TEST_CASE("Sub buffer full width strip selects whole rows", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubBufferXml("0x0x100x25"));
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() == 12);
    CHECK(s->GetDefaultBufferWi() == 6);
    CHECK(s->GetDefaultBufferHt() == 2);
}

TEST_CASE("Unparseable sub buffer falls back to the whole parent", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubBufferXml("garbage"));
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() == 24);
    CHECK(s->GetDefaultBufferWi() == 6);
    CHECK(s->GetDefaultBufferHt() == 4);
}

TEST_CASE("Sub buffer with an empty rectangle selects nothing but stays valid", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubBufferXml("200x200x300x300"));
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() == 0);
    CHECK(s->GetDefaultBufferWi() >= 1);
    CHECK(s->GetDefaultBufferHt() >= 1);
}

TEST_CASE("Submodel lists the per strand buffer styles", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\""));
    const auto& styles = Sub(m)->GetBufferStyles();
    auto has = [&](const char* n) { return std::find(styles.begin(), styles.end(), n) != styles.end(); };
    CHECK(has("Default"));
    CHECK(has("Single Line"));
    CHECK(has("Horizontal Per Strand"));
    CHECK(has("Vertical Per Strand"));
    CHECK(SubModel::GetBufferStyleList().size() == 3);
}

TEST_CASE("Submodel Single Line style flattens every node", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\" line1=\"7-9\""));
    SubModel* s = Sub(m);
    int w = 0, h = 0;
    s->GetBufferSize("Single Line", "2D", "None", w, h, 0);
    CHECK(w == 6);
    CHECK(h == 1);
    std::vector<NodeBaseClassPtr> nodes;
    s->InitRenderBufferNodes("Single Line", "2D", "None", nodes, w, h, 0);
    REQUIRE(nodes.size() == 6);
    for (int i = 0; i < 6; ++i) {
        CHECK(nodes[i]->Coords[0].bufX == i);
        CHECK(nodes[i]->Coords[0].bufY == 0);
    }
}

TEST_CASE("Submodel rotated buffers swap width and height", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\" line1=\"7-9\""));
    SubModel* s = Sub(m);
    int w = 0, h = 0;
    s->GetBufferSize("Default", "2D", "Rotate CW 90", w, h, 0);
    CHECK(w == 2);
    CHECK(h == 3);
}

TEST_CASE("Vertical per strand on a horizontal submodel rotates the default", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\" line1=\"7-9\""));
    SubModel* s = Sub(m);
    int w = 0, h = 0;
    s->GetBufferSize("Vertical Per Strand", "2D", "None", w, h, 0);
    CHECK(w == 2);
    CHECK(h == 3);
    std::vector<NodeBaseClassPtr> nodes;
    int rw = 0, rh = 0;
    s->InitRenderBufferNodes("Vertical Per Strand", "2D", "None", nodes, rw, rh, 0);
    CHECK(rw == 2);
    CHECK(rh == 3);
    REQUIRE(nodes.size() == 6);
    std::set<Pt> seen;
    for (auto& n : nodes) {
        CHECK(n->Coords[0].bufX >= 0);
        CHECK(n->Coords[0].bufX < rw);
        CHECK(n->Coords[0].bufY >= 0);
        CHECK(n->Coords[0].bufY < rh);
        seen.insert({ n->Coords[0].bufX, n->Coords[0].bufY });
    }
    CHECK(seen.size() == 6);
}

TEST_CASE("Horizontal per strand on a vertical submodel rotates the default", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("vertical", "Default", "line0=\"1-3\" line1=\"7-9\""));
    SubModel* s = Sub(m);
    int w = 0, h = 0;
    s->GetBufferSize("Horizontal Per Strand", "2D", "None", w, h, 0);
    CHECK(w == 3);
    CHECK(h == 2);
    std::vector<NodeBaseClassPtr> nodes;
    int rw = 0, rh = 0;
    s->InitRenderBufferNodes("Horizontal Per Strand", "2D", "None", nodes, rw, rh, 0);
    CHECK(rw == 3);
    CHECK(rh == 2);
    std::set<Pt> seen;
    for (auto& n : nodes) {
        seen.insert({ n->Coords[0].bufX, n->Coords[0].bufY });
    }
    CHECK(seen.size() == 6);
}

TEST_CASE("Submodel per strand buffer size matches its render buffer", "[models2][submodel][!mayfail]") {
    for (const char* layout : { "horizontal", "vertical" }) {
        for (const char* style : { "Horizontal Per Strand", "Vertical Per Strand" }) {
            INFO(layout << " " << style);
            ShowFixture f;
            Model* m = f.Matrix(SubXml(layout, "Default", "line0=\"1-3\" line1=\"7-9\""));
            SubModel* s = Sub(m);
            int w = 0, h = 0;
            s->GetBufferSize(style, "2D", "None", w, h, 0);
            std::vector<NodeBaseClassPtr> nodes;
            int rw = 0, rh = 0;
            s->InitRenderBufferNodes(style, "2D", "None", nodes, rw, rh, 0);
            CHECK(w == rw);
            CHECK(h == rh);
        }
    }
}

TEST_CASE("Submodel render nodes copy the parent channels", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"5-6\" line1=\"1-2\""));
    SubModel* s = Sub(m);
    std::vector<NodeBaseClassPtr> nodes;
    int w = 0, h = 0;
    s->InitRenderBufferNodes("Default", "2D", "None", nodes, w, h, 0);
    REQUIRE(nodes.size() == 4);
    CHECK(nodes[0]->ActChan == (uint32_t)m->NodeStartChannel(4));
    CHECK(nodes[1]->ActChan == (uint32_t)m->NodeStartChannel(5));
    CHECK(nodes[2]->ActChan == (uint32_t)m->NodeStartChannel(0));
    CHECK(nodes[3]->ActChan == (uint32_t)m->NodeStartChannel(1));
}

TEST_CASE("Submodel shares its parent screen location and layout group", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\""));
    SubModel* s = Sub(m);
    CHECK(&s->GetModelScreenLocation() == &m->GetModelScreenLocation());
    CHECK(s->GetLayoutGroup() == m->GetLayoutGroup());
    CHECK(s->GetFullName() == "M/s");
}

TEST_CASE("Submodel active state follows an inactive parent", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Add("<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"24\" StrandsPerString=\"4\" Active=\"0\">" + SubXml("horizontal", "Default", "line0=\"1-3\"") + "</model>");
    SubModel* s = Sub(m);
    CHECK_FALSE(m->IsActive());
    CHECK_FALSE(s->IsActive());
}

TEST_CASE("Submodel inherits pixel properties from its parent", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Add("<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"24\" StrandsPerString=\"4\" PixelSize=\"7\" Transparency=\"40\">" + SubXml("horizontal", "Default", "line0=\"1-3\"") + "</model>");
    SubModel* s = Sub(m);
    CHECK(s->GetPixelSize() == 7);
    CHECK(s->GetTransparency() == 40);
}

TEST_CASE("Submodel shares face and state definitions with the parent", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\"") + "<stateInfo Name=\"S\" Type=\"NodeRange\" CustomColors=\"0\" s1=\"1-4\" s1-Name=\"Half\"/>");
    SubModel* s = Sub(m);
    CHECK(&s->GetStateInfo() == &m->GetStateInfo());
    CHECK(s->GetStateInfo().count("S") == 1);
    CHECK(&s->GetFaceInfo() == &m->GetFaceInfo());
}

TEST_CASE("Several submodels coexist and are found by name", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix("<subModel name=\"a\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-3\"/>"
                        "<subModel name=\"b\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"4-6\"/>"
                        "<subModel name=\"c\" layout=\"horizontal\" type=\"subbuffer\" subBuffer=\"0x0x100x25\"/>");
    CHECK(m->GetSubModels().size() == 3);
    CHECK(m->GetSubModel("a") != nullptr);
    CHECK(m->GetSubModel("b") != nullptr);
    CHECK(m->GetSubModel("c") != nullptr);
    CHECK(m->GetSubModel("d") == nullptr);
    CHECK(m->GetSubModel("a")->GetNodeCount() == 3);
    CHECK(m->GetSubModel("c")->GetNodeCount() == 12);
    CHECK(f.mm.GetModel("M/b") == m->GetSubModel("b"));
}

TEST_CASE("Submodel names are matched exactly", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\""));
    CHECK(m->GetSubModel("s") != nullptr);
    CHECK(m->GetSubModel("S") == nullptr);
    CHECK(m->GetSubModel("") == nullptr);
}

TEST_CASE("Rebuilding the parent rebuilds its submodels against the new geometry", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"1-3\""));
    CHECK(Chans(Sub(m)) == std::vector<int>{ 0, 1, 2 });
    m->SetStartChannel("101");
    f.mm.RecalcStartChannels();
    CHECK(Sub(m)->NodeStartChannel(0) == 100);
    CHECK(Sub(m)->NodeStartChannel(1) == 103);
    CHECK(Sub(m)->NodeStartChannel(2) == 106);
}

TEST_CASE("Submodel pointing at nodes past the end of the parent keeps only valid nodes", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"23-30\""));
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() == 2);
    CHECK(Chans(s) == std::vector<int>{ 22, 23 });
}

TEST_CASE("Submodel with only invalid nodes has a minimal buffer", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix(SubXml("horizontal", "Default", "line0=\"100-110\""));
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() == 0);
    CHECK(s->GetDefaultBufferWi() >= 1);
    CHECK(s->GetDefaultBufferHt() >= 1);
}

TEST_CASE("Submodel line attributes with unusual numbering are tolerated", "[models2][submodel]") {
    ShowFixture f;
    Model* m = f.Matrix("<subModel name=\"s\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\" line2=\"5-6\"/>");
    SubModel* s = Sub(m);
    CHECK(s->GetNodeCount() >= 2);
}
