#include "test_roundtrip_common.h"

#include "models/GridlinesObject.h"
#include "models/MeshObject.h"
#include "models/ModelGroup.h"
#include "models/RulerObject.h"
#include "models/TerrainObject.h"
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"

using namespace rt;

namespace {

struct ObjFixture {
    ViewObjectManager vom{ nullptr };

    ViewObject* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        XmlSerializer s;
        ViewObject* o = s.DeserializeObject(doc.first_child(), vom, false);
        if (o != nullptr) {
            vom.AddViewObject(o);
        }
        return o;
    }
};

std::string SerObj(const ViewObject* o) {
    StringSerializingVisitor v(false, false);
    o->Accept(v);
    std::string s = v.GetResult();
    auto p = s.find("?>");
    return p == std::string::npos ? s : s.substr(p + 2);
}

std::string NoLayoutGroup(std::string s) {
    auto p = s.find(" LayoutGroup=\"");
    if (p != std::string::npos) {
        auto e = s.find('"', p + 14);
        s.erase(p, e - p + 1);
    }
    return s;
}

std::string ObjXml(const std::string& type, const std::string& name, const Attrs& attrs) {
    std::string x = "<view_object name=\"" + Esc(name) + "\" DisplayAs=\"" + type + "\"";
    for (auto& a : attrs) x += " " + a.first + "=\"" + Esc(a.second) + "\"";
    return x + "/>";
}

const Attrs kBoxed = { { "WorldPosX", "10.5" }, { "WorldPosY", "-20.25" }, { "WorldPosZ", "30.75" }, { "ScaleX", "2.5" }, { "ScaleY", "3.5" }, { "ScaleZ", "4.5" }, { "RotateX", "11" }, { "RotateY", "22" }, { "RotateZ", "33" } };

Attrs With(Attrs a, const Attrs& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

void CheckObject(const std::string& type, const Attrs& attrs, const std::set<std::string>& na = {}) {
    ObjFixture a;
    ViewObject* o = a.Add(ObjXml(type, "Obj", attrs));
    REQUIRE(o != nullptr);
    std::string s1 = SerObj(o);
    ObjFixture b;
    ViewObject* o2 = b.Add(s1);
    REQUIRE(o2 != nullptr);
    std::string s2 = SerObj(o2);
    INFO("save1 vs save2:" << DiffAttrs(s1, s2));
    CHECK(s2 == s1);
    ObjFixture c;
    ViewObject* o3 = c.Add(s2);
    REQUIRE(o3 != nullptr);
    CHECK(SerObj(o3) == s2);
    pugi::xml_document d;
    REQUIRE(d.load_string(s1.c_str()));
    auto am = AttrMap(d.first_child());
    for (auto& e : attrs) {
        if (na.count(e.first)) continue;
        INFO(e.first << " = " << e.second);
        REQUIRE(am.count(e.first) == 1);
        CHECK(Same(am[e.first], e.second));
    }
}

void CheckObjectDefault(const std::string& type) {
    ObjFixture a;
    ViewObject* o = a.vom.CreateAndAddObject(type);
    REQUIRE(o != nullptr);
    std::string s1 = NoLayoutGroup(SerObj(o));
    ObjFixture b;
    ViewObject* o2 = b.Add(s1);
    REQUIRE(o2 != nullptr);
    std::string s2 = NoLayoutGroup(SerObj(o2));
    INFO("save1 vs save2:" << DiffAttrs(s1, s2));
    CHECK(s2 == s1);
}

} // namespace

// KNOWN BUG: a freshly created view object writes LayoutGroup="" but one loaded from that text writes "Default".
TEST_CASE("Roundtrip default view object LayoutGroup is stable", "[roundtrip][objects][!mayfail]") {
    ObjFixture a;
    ViewObject* o = a.vom.CreateAndAddObject("Gridlines");
    REQUIRE(o != nullptr);
    std::string s1 = SerObj(o);
    ObjFixture b;
    CHECK(SerObj(b.Add(s1)) == s1);
}
// KNOWN BUG: DeserializeBaseObjectAttributes forces LayoutGroup to "Default", discarding the saved value.
TEST_CASE("Roundtrip view object keeps its LayoutGroup", "[roundtrip][objects][!mayfail]") {
    ObjFixture a;
    ViewObject* o = a.Add(ObjXml("Gridlines", "G", { { "LayoutGroup", "Back Yard" } }));
    REQUIRE(o != nullptr);
    CHECK(o->GetLayoutGroup() == "Back Yard");
}

TEST_CASE("Roundtrip gridlines object keeps every attribute", "[roundtrip][objects]") {
    CheckObject("Gridlines", With(kBoxed, { { "GridLineSpacing", "75" }, { "GridWidth", "1500" }, { "GridHeight", "800" }, { "GridColor", "#ff00aa" }, { "GridAxis", "1" }, { "PointToFront", "1" } }));
}
TEST_CASE("Roundtrip terrain object keeps every attribute", "[roundtrip][objects]") {
    CheckObject("Terrain", With(kBoxed, { { "TerrianLineSpacing", "100" }, { "TerrianWidth", "800" }, { "TerrianDepth", "600" }, { "gridColor", "#112233" }, { "HideGrid", "1" }, { "HideImage", "1" }, { "Transparency", "40" }, { "Brightness", "60" } }));
}
TEST_CASE("Roundtrip ruler object keeps every attribute", "[roundtrip][objects]") {
    CheckObject("Ruler", { { "X2", "50.5" }, { "Y2", "6.5" }, { "Z2", "7.5" }, { "WorldPosX", "10.5" }, { "WorldPosY", "20.5" }, { "WorldPosZ", "30.5" }, { "Units", "3" }, { "Length", "12.5" } });
}
TEST_CASE("Roundtrip mesh object keeps every attribute", "[roundtrip][objects]") {
    CheckObject("Mesh", With(kBoxed, { { "MeshOnly", "1" }, { "Brightness", "55" } }));
}
TEST_CASE("Roundtrip image object keeps every attribute", "[roundtrip][objects]") {
    CheckObject("Image", With(kBoxed, { { "Transparency", "33" }, { "Brightness", "77" } }));
}
TEST_CASE("Roundtrip default view objects are stable", "[roundtrip][objects]") {
    for (const char* t : { "Gridlines", "Terrain", "Ruler", "Mesh", "Image" }) {
        INFO(t);
        CheckObjectDefault(t);
    }
}
TEST_CASE("Roundtrip gridlines default flags are stable", "[roundtrip][objects]") {
    ObjFixture a;
    ViewObject* o = a.Add(ObjXml("Gridlines", "G", {}));
    REQUIRE(o != nullptr);
    std::string s1 = SerObj(o);
    ObjFixture b;
    CHECK(SerObj(b.Add(s1)) == s1);
}
TEST_CASE("Roundtrip view object name whitespace is trimmed once", "[roundtrip][objects]") {
    ObjFixture a;
    ViewObject* o = a.Add(ObjXml("Gridlines", "  padded name \t", {}));
    REQUIRE(o != nullptr);
    CHECK(o->GetName() == "padded name");
    std::string s1 = SerObj(o);
    ObjFixture b;
    CHECK(SerObj(b.Add(s1)) == s1);
}
TEST_CASE("Roundtrip view object name with specials", "[roundtrip][objects]") {
    ObjFixture a;
    ViewObject* o = a.Add(ObjXml("Gridlines", "A&B <\"x\"> '\xC3\xA9'", {}));
    REQUIRE(o != nullptr);
    std::string s1 = SerObj(o);
    ObjFixture b;
    ViewObject* o2 = b.Add(s1);
    REQUIRE(o2 != nullptr);
    CHECK(o2->GetName() == o->GetName());
    CHECK(SerObj(o2) == s1);
}
TEST_CASE("Roundtrip view object extreme numbers are stable after one cycle", "[roundtrip][objects]") {
    for (const char* v : { "0", "-1", "-100000", "999999999", "1e30", "abc", "" }) {
        INFO(v);
        ObjFixture a;
        ViewObject* o = a.Add(ObjXml("Gridlines", "G", { { "GridLineSpacing", v }, { "GridWidth", v }, { "GridHeight", v }, { "ScaleX", v }, { "WorldPosX", v } }));
        REQUIRE(o != nullptr);
        std::string s1 = SerObj(o);
        ObjFixture b;
        ViewObject* o2 = b.Add(s1);
        REQUIRE(o2 != nullptr);
        std::string s2 = SerObj(o2);
        ObjFixture c;
        CHECK(SerObj(c.Add(s2)) == s2);
    }
}
TEST_CASE("Roundtrip terrain extreme spacing and size are stable", "[roundtrip][objects]") {
    for (const char* v : { "0", "-5", "1", "100000", "3" }) {
        INFO(v);
        ObjFixture a;
        ViewObject* o = a.Add(ObjXml("Terrain", "T", { { "TerrianLineSpacing", v }, { "TerrianWidth", v }, { "TerrianDepth", v } }));
        REQUIRE(o != nullptr);
        std::string s1 = SerObj(o);
        ObjFixture b;
        ViewObject* o2 = b.Add(s1);
        REQUIRE(o2 != nullptr);
        std::string s2 = SerObj(o2);
        ObjFixture c;
        CHECK(SerObj(c.Add(s2)) == s2);
    }
}
TEST_CASE("Roundtrip whole view object collection is stable", "[roundtrip][objects]") {
    ObjFixture a;
    a.Add(ObjXml("Gridlines", "G1", kBoxed));
    a.Add(ObjXml("Ruler", "R1", { { "X2", "5" } }));
    a.Add(ObjXml("Terrain", "T1", kBoxed));
    StringSerializingVisitor v1(false, false);
    XmlSerializer::SerializeAllObjects(a.vom, v1);
    pugi::xml_document d;
    REQUIRE(d.load_string(v1.GetResult().c_str()));
    ObjFixture b;
    for (auto n : d.child("view_objects").children("view_object")) {
        std::ostringstream os;
        n.print(os, "", pugi::format_raw);
        REQUIRE(b.Add(os.str()) != nullptr);
    }
    StringSerializingVisitor v2(false, false);
    XmlSerializer::SerializeAllObjects(b.vom, v2);
    CHECK(v2.GetResult() == v1.GetResult());
}
TEST_CASE("Roundtrip unknown view object type is rejected safely", "[roundtrip][objects]") {
    ObjFixture a;
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<view_object name=\"x\" DisplayAs=\"Bogus\"/>"));
    XmlSerializer s;
    CHECK_THROWS(s.DeserializeObject(doc.first_child(), a.vom, false));
}

// Model groups ---------------------------------------------------------------------------

namespace {

struct GroupFixture {
    ShowFixture f;
    GroupFixture() {
        f.Add(ModelXml("A", "Single Line", { { "NumStrings", "1" }, { "NodesPerString", "5" } }));
        f.Add(ModelXml("B", "Single Line", { { "NumStrings", "1" }, { "NodesPerString", "5" } }));
        f.Add(ModelXml("C,comma", "Single Line", { { "NumStrings", "1" }, { "NodesPerString", "5" } }));
    }
    Model* Group(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = f.mm.createAndAddModel(doc.first_child(), 100, 100);
        return m;
    }
};

std::string G(const std::string& attrs, const std::string& children = "") {
    return "<modelGroup name=\"Grp\" " + attrs + ">" + children + "</modelGroup>";
}

void CheckGroup(const std::string& xml, const std::vector<std::string>& expectAttrs = {}) {
    GroupFixture a;
    Model* g = a.Group(xml);
    REQUIRE(g != nullptr);
    std::string s1 = Ser(g);
    GroupFixture b;
    Model* g2 = b.Group(s1);
    REQUIRE(g2 != nullptr);
    std::string s2 = Ser(g2);
    INFO("save1 vs save2:" << DiffAttrs(s1, s2));
    CHECK(s2 == s1);
    GroupFixture c;
    CHECK(Ser(c.Group(s2)) == s2);
    pugi::xml_document d, in;
    REQUIRE(d.load_string(s1.c_str()));
    REQUIRE(in.load_string(xml.c_str()));
    auto am = AttrMap(d.first_child());
    for (auto& kv : AttrMap(in.first_child())) {
        INFO(kv.first << " = " << kv.second);
        REQUIRE(am.count(kv.first) == 1);
        CHECK(Same(am[kv.first], kv.second));
    }
}

} // namespace

TEST_CASE("Roundtrip model group minimal grid", "[roundtrip][groups]") {
    CheckGroup(G("models=\"A,B\" layout=\"minimalGrid\" GridSize=\"123\" DefaultCamera=\"3D\""));
}
TEST_CASE("Roundtrip model group horizontal layouts", "[roundtrip][groups]") {
    for (const char* l : { "horizontal", "vertical", "Horizontal Per Model", "Vertical Per Model", "Single Line", "Overlay - Scaled", "Overlay - Centered", "Horizontal Stack", "Vertical Stack", "Grid as per preview", "minimalGrid", "Grid as per preview", "Horizontal Per Model/Strand" }) {
        INFO(l);
        CheckGroup(G(std::string("models=\"A,B\" layout=\"") + l + "\" GridSize=\"400\" DefaultCamera=\"2D\""));
    }
}
TEST_CASE("Roundtrip model group centre defined", "[roundtrip][groups]") {
    CheckGroup(G("models=\"A,B\" layout=\"minimalGrid\" GridSize=\"400\" DefaultCamera=\"2D\" centreDefined=\"1\" centrex=\"12.5\" centrey=\"-7.25\" centreMinx=\"-3\" centreMiny=\"-4\" centreMaxx=\"50\" centreMaxy=\"60\""));
}
TEST_CASE("Roundtrip model group integer centre offsets", "[roundtrip][groups]") {
    CheckGroup(G("models=\"A\" layout=\"minimalGrid\" GridSize=\"400\" DefaultCamera=\"2D\" XCentreOffset=\"12\" YCentreOffset=\"-9\""));
}
TEST_CASE("Roundtrip model group fractional centre offsets", "[roundtrip][groups]") {
    CheckGroup(G("models=\"A\" layout=\"minimalGrid\" GridSize=\"400\" DefaultCamera=\"2D\" XCentreOffset=\"12.5\" YCentreOffset=\"-9.75\""));
}
TEST_CASE("Roundtrip model group tag colour and aliases", "[roundtrip][groups]") {
    CheckGroup(G("models=\"A,B\" layout=\"minimalGrid\" GridSize=\"400\" DefaultCamera=\"2D\" TagColour=\"#aabbcc\"", "<Aliases><alias name=\"old1\"/><alias name=\"old2\"/></Aliases>"));
}
TEST_CASE("Roundtrip model group empty", "[roundtrip][groups]") {
    CheckGroup(G("layout=\"minimalGrid\" GridSize=\"400\" DefaultCamera=\"2D\" models=\"\""));
}
TEST_CASE("Roundtrip model group nested in group", "[roundtrip][groups]") {
    GroupFixture a;
    a.Group("<modelGroup name=\"Inner\" models=\"A,B\" layout=\"minimalGrid\" GridSize=\"400\"/>");
    Model* g = a.Group("<modelGroup name=\"Outer\" models=\"Inner,C\" layout=\"minimalGrid\" GridSize=\"400\"/>");
    REQUIRE(g != nullptr);
    std::string s1 = Ser(g);
    GroupFixture b;
    b.Group(Ser(a.f.mm.GetModel("Inner")));
    CHECK(Ser(b.Group(s1)) == s1);
}
TEST_CASE("Roundtrip model group extreme grid sizes", "[roundtrip][groups]") {
    for (const char* v : { "0", "-1", "1", "99999999", "abc", "" }) {
        INFO(v);
        GroupFixture a;
        Model* g = a.Group(G(std::string("models=\"A\" GridSize=\"") + v + "\""));
        if (g == nullptr) continue;
        std::string s1 = Ser(g);
        GroupFixture b;
        Model* g2 = b.Group(s1);
        REQUIRE(g2 != nullptr);
        std::string s2 = Ser(g2);
        GroupFixture c;
        CHECK(Ser(c.Group(s2)) == s2);
    }
}
TEST_CASE("Roundtrip model group name with specials", "[roundtrip][groups]") {
    GroupFixture a;
    Model* g = a.Group("<modelGroup name=\"G&amp;&lt;1&gt; &quot;q&quot; \xC3\xA9\" models=\"A\" layout=\"minimalGrid\" GridSize=\"400\"/>");
    REQUIRE(g != nullptr);
    std::string s1 = Ser(g);
    GroupFixture b;
    Model* g2 = b.Group(s1);
    REQUIRE(g2 != nullptr);
    CHECK(g2->GetName() == g->GetName());
    CHECK(Ser(g2) == s1);
}
TEST_CASE("Roundtrip model group member order is preserved", "[roundtrip][groups]") {
    GroupFixture a;
    Model* g = a.Group(G("models=\"B,A\" layout=\"minimalGrid\" GridSize=\"400\""));
    REQUIRE(g != nullptr);
    GroupFixture b;
    Model* g2 = b.Group(Ser(g));
    REQUIRE(g2 != nullptr);
    pugi::xml_document d;
    REQUIRE(d.load_string(Ser(g2).c_str()));
    CHECK(std::string(d.first_child().attribute("models").value()) == "B,A");
}
TEST_CASE("Roundtrip model group duplicate members are stable", "[roundtrip][groups]") {
    GroupFixture a;
    Model* g = a.Group(G("models=\"A,A,B\" layout=\"minimalGrid\" GridSize=\"400\""));
    REQUIRE(g != nullptr);
    std::string s1 = Ser(g);
    GroupFixture b;
    Model* g2 = b.Group(s1);
    REQUIRE(g2 != nullptr);
    CHECK(Ser(g2) == s1);
}
TEST_CASE("Roundtrip model group with missing member is stable", "[roundtrip][groups]") {
    GroupFixture a;
    Model* g = a.Group(G("models=\"A,Missing\" layout=\"minimalGrid\" GridSize=\"400\""));
    REQUIRE(g != nullptr);
    std::string s1 = Ser(g);
    GroupFixture b;
    Model* g2 = b.Group(s1);
    REQUIRE(g2 != nullptr);
    CHECK(Ser(g2) == s1);
}
TEST_CASE("Roundtrip all models and groups collection", "[roundtrip][groups]") {
    GroupFixture a;
    a.Group(G("models=\"A,B\" layout=\"horizontal\" GridSize=\"200\""));
    StringSerializingVisitor v1(false, false);
    XmlSerializer::SerializeAllModels(a.f.mm, v1);
    pugi::xml_document d;
    REQUIRE(d.load_string(v1.GetResult().c_str()));
    ShowFixture b;
    for (auto n : d.child("models").children("model")) b.Add(Canon(n));
    for (auto n : d.child("modelGroups").children("modelGroup")) b.Add(Canon(n));
    StringSerializingVisitor v2(false, false);
    XmlSerializer::SerializeAllModels(b.mm, v2);
    CHECK(v2.GetResult() == v1.GetResult());
}
