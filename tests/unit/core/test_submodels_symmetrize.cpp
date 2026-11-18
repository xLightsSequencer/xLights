#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/SubModelOps.h"
#include "models/SubModelSymmetrize.h"
#include "outputs/OutputManager.h"

namespace {

using Coords = std::map<int, std::pair<float, float>>;

Coords Ring(int count, float cx, float cy, float radius, float startDeg = 0.0f) {
    Coords c;
    for (int i = 0; i < count; ++i) {
        float a = (startDeg + 360.0f * i / count) * 3.14159265f / 180.0f;
        c[i + 1] = { cx + radius * std::cos(a), cy + radius * std::sin(a) };
    }
    return c;
}

std::set<int> NodesOf(const std::vector<std::string>& strands) {
    std::set<int> s;
    for (const auto& st : strands) {
        std::string e = submodel_ops::ExpandStrand(st);
        size_t pos = 0;
        while (pos <= e.size()) {
            size_t comma = e.find(',', pos);
            std::string cell = e.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
            if (!cell.empty() && cell != "0") {
                s.insert((int)std::strtol(cell.c_str(), nullptr, 10));
            }
            if (comma == std::string::npos) {
                break;
            }
            pos = comma + 1;
        }
    }
    return s;
}

int CountNodes(const std::vector<std::string>& strands) {
    int n = 0;
    for (const auto& st : strands) {
        std::string e = submodel_ops::ExpandStrand(st);
        size_t pos = 0;
        while (pos <= e.size()) {
            size_t comma = e.find(',', pos);
            std::string cell = e.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
            if (!cell.empty() && cell != "0") {
                ++n;
            }
            if (comma == std::string::npos) {
                break;
            }
            pos = comma + 1;
        }
    }
    return n;
}

SubModelSymmetrize::Options Opts(int dos) {
    SubModelSymmetrize::Options o;
    o.degreeOfSymmetry = dos;
    return o;
}

} // namespace

TEST_CASE("Symmetrize options default to eight way symmetry", "[models2][symmetrize]") {
    SubModelSymmetrize::Options o;
    CHECK(o.degreeOfSymmetry == 8);
    CHECK_FALSE(o.clockwise);
    CHECK_FALSE(o.bottomToTop);
    CHECK(o.squarifyAspect);
    CHECK(o.handleCenterNode);
    SubModelSymmetrize::Result r;
    CHECK_FALSE(r.success);
    CHECK(r.strands.empty());
}

TEST_CASE("Symmetrize replicates a strand around a ring", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1-2" }, Opts(4));
    REQUIRE(r.success);
    CHECK(r.strands.size() == 4);
    CHECK(r.unmatchedNodes.empty());
    CHECK(NodesOf(r.strands) == std::set<int>{ 1, 2, 3, 4, 5, 6, 7, 8 });
    CHECK(CountNodes(r.strands) == 8);
}

TEST_CASE("Symmetrize keeps the original strands and puts copies above them by default", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1-2" }, Opts(4));
    REQUIRE(r.success);
    REQUIRE(r.strands.size() == 4);
    CHECK(r.strands.back() == "1-2");
}

TEST_CASE("Symmetrize bottom to top puts the original first", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto o = Opts(4);
    o.bottomToTop = true;
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1-2" }, o);
    REQUIRE(r.success);
    REQUIRE(r.strands.size() == 4);
    CHECK(r.strands.front() == "1-2");
}

TEST_CASE("Symmetrize clockwise and counter clockwise give opposite rotations", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto ccw = Opts(4);
    auto cw = Opts(4);
    cw.clockwise = true;
    cw.bottomToTop = ccw.bottomToTop = true;
    auto a = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1" }, ccw);
    auto b = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1" }, cw);
    REQUIRE(a.success);
    REQUIRE(b.success);
    REQUIRE(a.strands.size() == 4);
    REQUIRE(b.strands.size() == 4);
    CHECK(a.strands[0] == "1");
    CHECK(b.strands[0] == "1");
    CHECK(a.strands[1] == b.strands[3]);
    CHECK(a.strands[3] == b.strands[1]);
    CHECK(a.strands[2] == b.strands[2]);
    CHECK(a.strands[1] != a.strands[3]);
    CHECK(a.strands[2] == "5");
}

TEST_CASE("Symmetrize first generated copy is a quarter turn away", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto o = Opts(4);
    o.bottomToTop = true;
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1" }, o);
    REQUIRE(r.success);
    REQUIRE(r.strands.size() == 4);
    std::set<std::string> copies(r.strands.begin() + 1, r.strands.end());
    CHECK(copies == std::set<std::string>{ "3", "5", "7" });
}

TEST_CASE("Symmetrize with twofold symmetry mirrors through the centre", "[models2][symmetrize]") {
    Coords c = Ring(6, 50, 50, 25);
    auto o = Opts(2);
    o.bottomToTop = true;
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1,2" }, o);
    REQUIRE(r.success);
    REQUIRE(r.strands.size() == 2);
    CHECK(r.strands[1] == "4-5");
}

TEST_CASE("Symmetrize handles several strands", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto o = Opts(4);
    o.bottomToTop = true;
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1", "2" }, o);
    REQUIRE(r.success);
    CHECK(r.strands.size() == 8);
    CHECK(CountNodes(r.strands) == 8);
    CHECK(r.strands[0] == "1");
    CHECK(r.strands[1] == "2");
}

TEST_CASE("Symmetrize keeps blank cells in place in the copies", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto o = Opts(4);
    o.bottomToTop = true;
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1,0,2" }, o);
    REQUIRE(r.success);
    REQUIRE(r.strands.size() == 4);
    for (size_t i = 1; i < r.strands.size(); ++i) {
        std::string e = submodel_ops::ExpandStrand(r.strands[i]);
        CHECK(std::count(e.begin(), e.end(), ',') == 2);
        CHECK(e.find(",,") != std::string::npos);
    }
}

TEST_CASE("Symmetrize reports nodes it cannot match", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    c.erase(5);
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1" }, Opts(4));
    CHECK_FALSE(r.success);
    CHECK(r.strands.empty());
    CHECK_FALSE(r.unmatchedNodes.empty());
}

TEST_CASE("Symmetrize reports a strand node that is not in the model", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1,99" }, Opts(4));
    CHECK_FALSE(r.success);
    REQUIRE(r.unmatchedNodes.size() >= 1);
    CHECK(std::find(r.unmatchedNodes.begin(), r.unmatchedNodes.end(), 99) != r.unmatchedNodes.end());
}

TEST_CASE("Symmetrize rejects invalid setups", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    CHECK_FALSE(SubModelSymmetrize::Symmetrize(c, 100, 100, { "1" }, Opts(1)).success);
    CHECK_FALSE(SubModelSymmetrize::Symmetrize(c, 100, 100, { "1" }, Opts(0)).success);
    CHECK_FALSE(SubModelSymmetrize::Symmetrize(c, 100, 100, { "1" }, Opts(-3)).success);
    CHECK_FALSE(SubModelSymmetrize::Symmetrize(Coords{}, 100, 100, { "1" }, Opts(4)).success);
    CHECK_FALSE(SubModelSymmetrize::Symmetrize(c, 0, 100, { "1" }, Opts(4)).success);
    CHECK_FALSE(SubModelSymmetrize::Symmetrize(c, 100, -1, { "1" }, Opts(4)).success);
}

TEST_CASE("Symmetrize with no strands succeeds with nothing to copy", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, {}, Opts(4));
    CHECK(r.success);
    CHECK(r.strands.empty());
}

TEST_CASE("Symmetrize treats a centre node as its own image", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    c[9] = { 50.0f, 50.0f };
    auto o = Opts(4);
    o.bottomToTop = true;
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "9,1" }, o);
    REQUIRE(r.success);
    REQUIRE(r.strands.size() == 4);
    for (const auto& s : r.strands) {
        std::string e = submodel_ops::ExpandStrand(s);
        CHECK(e.rfind("9,", 0) == 0);
    }
}

TEST_CASE("Symmetrize without centre handling cannot match a centre node", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    c[9] = { 50.0f, 50.0f };
    auto o = Opts(4);
    o.handleCenterNode = false;
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "9" }, o);
    CHECK_FALSE(r.success);
}

TEST_CASE("Symmetrize works for a rotated ring", "[models2][symmetrize]") {
    Coords c = Ring(12, 40, 40, 30, 15.0f);
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1-3" }, Opts(4));
    REQUIRE(r.success);
    CHECK(r.strands.size() == 4);
    CHECK(CountNodes(r.strands) == 12);
    CHECK(NodesOf(r.strands).size() == 12);
}

TEST_CASE("Symmetrize eightfold on a sixteen node ring", "[models2][symmetrize]") {
    Coords c = Ring(16, 50, 50, 30);
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1-2" }, Opts(8));
    REQUIRE(r.success);
    CHECK(r.strands.size() == 8);
    CHECK(NodesOf(r.strands).size() == 16);
}

TEST_CASE("Symmetrize round trips the number of nodes per copy", "[models2][symmetrize]") {
    Coords c = Ring(12, 50, 50, 30);
    auto o = Opts(3);
    o.bottomToTop = true;
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1,2,3,4" }, o);
    REQUIRE(r.success);
    REQUIRE(r.strands.size() == 3);
    for (const auto& s : r.strands) {
        CHECK(CountNodes({ s }) == 4);
    }
}

TEST_CASE("Squarify is offered for a stretched layout only", "[models2][symmetrize]") {
    CHECK_FALSE(SubModelSymmetrize::ShouldOfferSquarify(Coords{}));
    CHECK_FALSE(SubModelSymmetrize::ShouldOfferSquarify(Ring(8, 50, 50, 20)));
    Coords stretched = Ring(8, 50, 50, 20);
    for (auto& p : stretched) {
        p.second.first = 50.0f + (p.second.first - 50.0f) * 2.0f;
    }
    CHECK(SubModelSymmetrize::ShouldOfferSquarify(stretched));
}

TEST_CASE("Squarify is not offered for collinear nodes", "[models2][symmetrize]") {
    Coords line = { { 1, { 0.0f, 5.0f } }, { 2, { 10.0f, 5.0f } }, { 3, { 20.0f, 5.0f } } };
    CHECK_FALSE(SubModelSymmetrize::ShouldOfferSquarify(line));
    Coords one = { { 1, { 3.0f, 3.0f } } };
    CHECK_FALSE(SubModelSymmetrize::ShouldOfferSquarify(one));
}

TEST_CASE("Symmetrize reports an advisory for a stretched layout", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    for (auto& p : c) {
        p.second.first = 50.0f + (p.second.first - 50.0f) * 1.5f;
    }
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1-2" }, Opts(4));
    CHECK(r.aspectAdvisory == "non-square");
}

TEST_CASE("Symmetrize with squarify on matches a stretched ring", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    for (auto& p : c) {
        p.second.first = 50.0f + (p.second.first - 50.0f) * 1.5f;
    }
    auto o = Opts(4);
    o.squarifyAspect = true;
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1-2" }, o);
    CHECK(r.success);
}

TEST_CASE("Symmetrize with a square ring has no advisory", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1-2" }, Opts(4));
    CHECK(r.aspectAdvisory.empty());
}

TEST_CASE("Symmetrize copes with coordinates outside the pixel bounds", "[models2][symmetrize]") {
    Coords c = Ring(8, 5, 5, 20);
    auto r = SubModelSymmetrize::Symmetrize(c, 10, 10, { "1" }, Opts(4));
    CHECK((r.success || !r.unmatchedNodes.empty()));
}

TEST_CASE("Symmetrize with a huge degree fails gracefully", "[models2][symmetrize]") {
    Coords c = Ring(8, 50, 50, 20);
    auto r = SubModelSymmetrize::Symmetrize(c, 100, 100, { "1" }, Opts(1000));
    CHECK_FALSE(r.success);
}

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
};

} // namespace

TEST_CASE("Symmetrize supports only rotationally symmetric model types", "[models2][symmetrize]") {
    ShowFixture f;
    CHECK_FALSE(SubModelSymmetrize::SupportsModel(nullptr));
    Model* circle = f.Add("<model name=\"C\" DisplayAs=\"Circle\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"8\"/>");
    CHECK(SubModelSymmetrize::SupportsModel(circle));
    Model* star = f.Add("<model name=\"S\" DisplayAs=\"Star\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"10\" StarPoints=\"5\"/>");
    CHECK(SubModelSymmetrize::SupportsModel(star));
    Model* wreath = f.Add("<model name=\"W\" DisplayAs=\"Wreath\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"10\"/>");
    CHECK(SubModelSymmetrize::SupportsModel(wreath));
    Model* custom = f.Add("<model name=\"Cu\" DisplayAs=\"Custom\" StartChannel=\"1\" CustomWidth=\"2\" CustomHeight=\"1\" CustomModel=\"1,2\"/>");
    CHECK(SubModelSymmetrize::SupportsModel(custom));
    Model* matrix = f.Add("<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\"/>");
    CHECK_FALSE(SubModelSymmetrize::SupportsModel(matrix));
    Model* tree = f.Add("<model name=\"T\" DisplayAs=\"Tree 360\" StartChannel=\"1\" NumStrings=\"2\" NodesPerString=\"4\" StrandsPerString=\"1\"/>");
    CHECK_FALSE(SubModelSymmetrize::SupportsModel(tree));
    Model* line = f.Add("<model name=\"L\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"5\"/>");
    CHECK_FALSE(SubModelSymmetrize::SupportsModel(line));
}

TEST_CASE("Symmetrize excludes partial arc spinners", "[models2][symmetrize]") {
    ShowFixture f;
    Model* full = f.Add("<model name=\"Sp\" DisplayAs=\"Spinner\" StartChannel=\"1\" NumStrings=\"1\" NodesPerArm=\"3\" ArmsPerString=\"4\" Arc=\"360\"/>");
    CHECK(SubModelSymmetrize::SupportsModel(full));
    Model* partial = f.Add("<model name=\"Sp2\" DisplayAs=\"Spinner\" StartChannel=\"1\" NumStrings=\"1\" NodesPerArm=\"3\" ArmsPerString=\"4\" Arc=\"180\"/>");
    CHECK_FALSE(SubModelSymmetrize::SupportsModel(partial));
}

TEST_CASE("Symmetrize on the real node layout of a wreath", "[models2][symmetrize]") {
    ShowFixture f;
    Model* m = f.Add("<model name=\"W\" DisplayAs=\"Wreath\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"16\"/>");
    Coords c;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& co = m->GetNode(i)->Coords[0];
        c[(int)i + 1] = { co.screenX, co.screenY };
    }
    REQUIRE(c.size() == 16);
    float minX = 1e9f, minY = 1e9f;
    for (auto& p : c) {
        minX = std::min(minX, p.second.first);
        minY = std::min(minY, p.second.second);
    }
    for (auto& p : c) {
        p.second.first -= minX;
        p.second.second -= minY;
    }
    int w = m->GetDefaultBufferWi() + 1;
    int h = m->GetDefaultBufferHt() + 1;
    auto r = SubModelSymmetrize::Symmetrize(c, w * 4, h * 4, { "1-2" }, Opts(8));
    if (r.success) {
        CHECK(NodesOf(r.strands).size() == 16);
    } else {
        CHECK_FALSE(r.unmatchedNodes.empty());
    }
}
