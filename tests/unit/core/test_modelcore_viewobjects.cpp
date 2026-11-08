#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <set>
#include <thread>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/ControllerObject.h"
#include "models/MeshObject.h"
#include "models/RulerObject.h"
#include "models/ViewObjectManager.h"
#include "XmlSerializer/StringSerializingVisitor.h"
#include "XmlSerializer/XmlSerializer.h"

#include "utils/FileUtils.h"

#include "support/TempDir.h"
#include "test_modelcore_fakegfx.h"

using Catch::Approx;
using fakegfx::FakePreview;

namespace {

ViewObject* AddObj(ViewObjectManager& vom, const std::string& xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    XmlSerializer s;
    ViewObject* o = s.DeserializeObject(doc.first_child(), vom, false);
    if (o != nullptr) {
        vom.AddViewObject(o);
    }
    return o;
}

std::string Ser(const ViewObject* o) {
    StringSerializingVisitor v(false, false);
    o->Accept(v);
    std::string s = v.GetResult();
    auto p = s.find("?>");
    return p == std::string::npos ? s : s.substr(p + 2);
}

// A ruler 10 units long along x, standing for `length` real units.
RulerObject* MakeRuler(ViewObjectManager& vom, float length, int units) {
    auto* r = dynamic_cast<RulerObject*>(vom.CreateAndAddObject("Ruler"));
    REQUIRE(r != nullptr);
    auto& loc = static_cast<TwoPointScreenLocation&>(r->GetObjectScreenLocation());
    loc.SetX2(10.0f);
    loc.SetY2(0.0f);
    loc.SetZ2(0.0f);
    r->SetLength(length);
    r->SetUnits(units);
    return r;
}

} // namespace

// ---------------------------------------------------------------------------
// RulerObject
// ---------------------------------------------------------------------------

TEST_CASE("Ruler static helpers are inert without a ruler", "[modelcore][ruler]") {
    REQUIRE(RulerObject::GetRuler() == nullptr);
    CHECK(RulerObject::GetUnits() == RULER_UNITS_M);
    CHECK(RulerObject::GetUnitDescription() == "m");
    CHECK(RulerObject::Measure(5.0f) == 0.0f);
    CHECK(RulerObject::UnMeasure(5.0f) == 0.0f);
    glm::vec3 a(0, 0, 0), b(3, 4, 0);
    CHECK(RulerObject::Measure(a, b) == 0.0f);
    CHECK(RulerObject::MeasureWidth(a, b) == 0.0f);
    CHECK(RulerObject::MeasureHeight(a, b) == 0.0f);
    CHECK(RulerObject::MeasureDepth(a, b) == 0.0f);
    CHECK(RulerObject::MeasureDescription(1.0f).empty());
    CHECK(RulerObject::PrescaledMeasureDescription(1.0f).empty());
    CHECK(RulerObject::MeasureLengthDescription(a, b).empty());
    CHECK(RulerObject::MeasureWidthDescription(a, b).empty());
    CHECK(RulerObject::MeasureHeightDescription(a, b).empty());
    CHECK(RulerObject::MeasureDepthDescription(a, b).empty());
}

TEST_CASE("Ruler measures distances in its real units", "[modelcore][ruler]") {
    ViewObjectManager vom(nullptr);
    RulerObject* r = MakeRuler(vom, 5.0f, RULER_UNITS_FEET); // 10 model units = 5 ft
    REQUIRE(RulerObject::GetRuler() == r);
    CHECK(RulerObject::GetUnits() == RULER_UNITS_FEET);
    CHECK(RulerObject::GetUnitDescription() == "ft");

    CHECK(RulerObject::Measure(4.0f) == Approx(2.0f));
    CHECK(RulerObject::UnMeasure(2.0f) == Approx(4.0f));
    glm::vec3 a(0, 0, 0), b(6, 8, 0), c(1, -2, 7);
    CHECK(RulerObject::Measure(a, b) == Approx(5.0f));
    CHECK(RulerObject::MeasureWidth(a, b) == Approx(3.0f));
    CHECK(RulerObject::MeasureHeight(a, b) == Approx(4.0f));
    CHECK(RulerObject::MeasureDepth(a, c) == Approx(3.5f));
    CHECK(RulerObject::MeasureLengthDescription(a, b) == "5.00ft");
    CHECK(RulerObject::MeasureWidthDescription(a, b) == "3.00ft");
    CHECK(RulerObject::MeasureHeightDescription(a, b) == "4.00ft");
    CHECK(RulerObject::MeasureDepthDescription(a, c) == "3.50ft");
    CHECK(RulerObject::MeasureDescription(3.0f) == "1.50ft");
    CHECK(RulerObject::PrescaledMeasureDescription(3.0f) == "3.00ft");

    struct U {
        int u;
        const char* d;
    };
    for (auto u : { U{ RULER_UNITS_INCHES, "in" }, U{ RULER_UNITS_YARDS, "yds" }, U{ RULER_UNITS_MM, "mm" }, U{ RULER_UNITS_CM, "cm" }, U{ RULER_UNITS_M, "m" }, U{ 42, "m" } }) {
        r->SetUnits(u.u);
        CHECK(RulerObject::GetUnitDescription() == u.d);
    }
}

TEST_CASE("Ruler with no length or no extent measures one to one", "[modelcore][ruler]") {
    ViewObjectManager vom(nullptr);
    RulerObject* r = MakeRuler(vom, 0.0f, RULER_UNITS_M);
    CHECK(RulerObject::Measure(7.0f) == Approx(7.0f));
    r->SetLength(3.0f);
    auto& loc = static_cast<TwoPointScreenLocation&>(r->GetObjectScreenLocation());
    loc.SetX2(0.0f);
    CHECK(RulerObject::Measure(7.0f) == Approx(7.0f));
}

TEST_CASE("Ruler is unregistered when it is deleted", "[modelcore][ruler]") {
    {
        ViewObjectManager vom(nullptr);
        MakeRuler(vom, 1.0f, RULER_UNITS_CM);
        CHECK(RulerObject::GetRuler() != nullptr);
    }
    CHECK(RulerObject::GetRuler() == nullptr);
}

TEST_CASE("Ruler unit conversion between named units", "[modelcore][ruler]") {
    const char* units[] = { "m", "cm", "mm", "i", "f", "y" };
    const float inMetres[] = { 1.0f, 0.01f, 0.001f, 0.0254f, 0.3048f, 0.9144f };
    for (int a = 0; a < 6; ++a) {
        for (int b = 0; b < 6; ++b) {
            INFO(units[a] << " -> " << units[b]);
            float expect = 2.0f * inMetres[a] / inMetres[b];
            CHECK(RulerObject::Convert(units[a], units[b], 2.0f) == Approx(expect).epsilon(1e-4));
        }
    }
    CHECK(RulerObject::Convert("furlong", "m", 2.0f) == 0.0f);
    CHECK(RulerObject::Convert("m", "furlong", 2.0f) == 0.0f);
    CHECK(RulerObject::Convert("furlong", "furlong", 2.0f) == 2.0f);

    // the integer-unit overloads map RULER_UNITS_* to the same names
    const int ids[] = { RULER_UNITS_M, RULER_UNITS_CM, RULER_UNITS_MM, RULER_UNITS_INCHES, RULER_UNITS_FEET, RULER_UNITS_YARDS };
    for (int a = 0; a < 6; ++a) {
        INFO(units[a]);
        CHECK(RulerObject::Convert(units[a], ids[(a + 1) % 6], 3.0f) == Approx(RulerObject::Convert(units[a], units[(a + 1) % 6], 3.0f)));
        CHECK(RulerObject::Convert(ids[a], units[(a + 2) % 6], 3.0f) == Approx(RulerObject::Convert(units[a], units[(a + 2) % 6], 3.0f)));
    }
    // an unknown unit id converts to nothing
    CHECK(RulerObject::Convert("m", 99, 3.0f) == 0.0f);
    CHECK(RulerObject::Convert(99, "m", 3.0f) == 0.0f);
}

TEST_CASE("Ruler ConvertDimension converts into the ruler's units", "[modelcore][ruler]") {
    ViewObjectManager vom(nullptr);
    RulerObject* r = MakeRuler(vom, 1.0f, RULER_UNITS_M);
    const char* units[] = { "m", "cm", "mm", "i", "f", "y" };
    const float inMetres[] = { 1.0f, 0.01f, 0.001f, 0.0254f, 0.3048f, 0.9144f };
    const int ids[] = { RULER_UNITS_M, RULER_UNITS_CM, RULER_UNITS_MM, RULER_UNITS_INCHES, RULER_UNITS_FEET, RULER_UNITS_YARDS };
    for (int to = 0; to < 6; ++to) {
        r->SetUnits(ids[to]);
        for (int from = 0; from < 6; ++from) {
            INFO(units[from] << " -> " << units[to]);
            CHECK(r->ConvertDimension(units[from], 4.0f) == Approx(4.0f * inMetres[from] / inMetres[to]).epsilon(1e-4));
        }
    }
    // unknown units pass through
    CHECK(r->ConvertDimension("parsec", 4.0f) == 4.0f);
}

TEST_CASE("Ruler InitModel sizes to its end point and Draw adds one line", "[modelcore][ruler]") {
    ViewObjectManager vom(nullptr);
    RulerObject* r = MakeRuler(vom, 1.0f, RULER_UNITS_M);
    auto& loc = static_cast<TwoPointScreenLocation&>(r->GetObjectScreenLocation());
    loc.SetWorldPos(100.0f, 200.0f, 0.0f);
    loc.SetX2(-30.0f);
    loc.SetY2(40.0f);
    r->InitModel();
    CHECK(loc.GetRenderWi() == Approx(30.0f));
    CHECK(loc.GetRenderHt() == Approx(40.0f));

    FakePreview p(1000, 1000, true);
    CHECK(r->Draw(&p, &p.ctx, p.solid.get(), p.transparent.get(), false));
    auto& tv = p.TransparentVerts();
    REQUIRE(tv.v.size() == 2);
    CHECK(tv.v[0].x == Approx(100.0f));
    CHECK(tv.v[0].y == Approx(200.0f));
    CHECK(tv.v[1].x == Approx(70.0f));
    CHECK(tv.v[1].y == Approx(240.0f));
    CHECK(tv.v[0].c == xlColor(255, 0, 255));
    p.Run();
    CHECK(p.ctx.lines == 1);
    CHECK(p.ctx.vertsDrawn == 2);

    // selected: handles are drawn too
    r->Selected(true);
    FakePreview p2(1000, 1000, true);
    CHECK(r->Draw(&p2, &p2.ctx, p2.solid.get(), p2.transparent.get(), true));
    CHECK(p2.SolidVerts().v.size() > 0);

    // inactive rulers draw nothing
    r->SetActive(false);
    FakePreview p3(1000, 1000, true);
    CHECK(r->Draw(&p3, &p3.ctx, p3.solid.get(), p3.transparent.get(), false));
    CHECK(p3.TransparentVerts().v.empty());
}

// ---------------------------------------------------------------------------
// ControllerObject
// ---------------------------------------------------------------------------

TEST_CASE("Controller object visibility strings round trip", "[modelcore][controllerobject]") {
    using V = ControllerObject::Visibility;
    for (V v : { V::Off, V::ControllerTab, V::LayoutPanel, V::Always }) {
        CHECK(ControllerObject::VisibilityFromString(ControllerObject::VisibilityToString(v)) == v);
    }
    CHECK(ControllerObject::VisibilityToString(V::Always) == "Always");
    CHECK(ControllerObject::VisibilityFromString("") == V::Off);
    CHECK(ControllerObject::VisibilityFromString("always") == V::Off);
    CHECK(ControllerObject::ObjectNameFor("F16") == "Controller: F16");
}

TEST_CASE("Controller object visibility decides which previews draw it", "[modelcore][controllerobject]") {
    using V = ControllerObject::Visibility;
    using C = ControllerObjectContext;
    struct Row {
        V v;
        bool none, editor, tab;
    };
    const Row rows[] = {
        { V::Off, false, false, false },
        { V::ControllerTab, false, false, true },
        { V::LayoutPanel, false, true, true },
        { V::Always, true, true, true },
    };
    for (const auto& r : rows) {
        INFO(ControllerObject::VisibilityToString(r.v));
        CHECK(ControllerObject::ShouldDraw(r.v, C::None) == r.none);
        CHECK(ControllerObject::ShouldDraw(r.v, C::LayoutEditor) == r.editor);
        CHECK(ControllerObject::ShouldDraw(r.v, C::LayoutEditorControllerTab) == r.tab);
    }
    ViewObjectManager vom(nullptr);
    auto* co = vom.CreateControllerObject("A");
    co->SetVisibility(V::LayoutPanel);
    CHECK(co->ShouldDrawIn(C::LayoutEditor));
    CHECK_FALSE(co->ShouldDrawIn(C::None));
}

TEST_CASE("Controller object properties and label size clamp", "[modelcore][controllerobject]") {
    ViewObjectManager vom(nullptr);
    auto* co = vom.CreateControllerObject("Ctl");
    REQUIRE(co != nullptr);
    CHECK(co->GetName() == "Controller: Ctl");
    CHECK(co->GetLayoutGroup() == "Default");
    CHECK(co->GetVisibility() == ControllerObject::Visibility::Off);
    CHECK_FALSE(co->GetShowLabel());
    CHECK(co->GetLabelSize() == 100);
    co->SetLabelSize(1);
    CHECK(co->GetLabelSize() == ControllerObject::MIN_LABEL_SIZE);
    co->SetLabelSize(10000);
    CHECK(co->GetLabelSize() == ControllerObject::MAX_LABEL_SIZE);
    co->SetStatusColor(xlColor(1, 2, 3));
    CHECK(co->GetStatusColor() == xlColor(1, 2, 3));

    co->SetControllerName("Other");
    CHECK(co->GetControllerName() == "Other");

    co->InitModel();
    CHECK(co->GetObjectScreenLocation().GetRenderWi() == Approx(100.0f));
    CHECK(co->GetObjectScreenLocation().GetRenderHt() == Approx(60.0f));
}

TEST_CASE("View object manager binds one object per controller", "[modelcore][controllerobject]") {
    ViewObjectManager vom(nullptr);
    CHECK(vom.CreateControllerObject("") == nullptr);
    auto* a = vom.CreateControllerObject("A");
    auto* b = vom.CreateControllerObject("B");
    CHECK(vom.CreateControllerObject("A") == a);
    CHECK(vom.size() == 2);
    CHECK(vom.GetControllerObject("A") == a);
    CHECK(vom.GetControllerObject("Z") == nullptr);
    CHECK(vom["Controller: B"] == b);
    CHECK(vom.GetObject("Controller: B") == b);

    CHECK(vom.RenameController("A", "A2"));
    CHECK_FALSE(vom.RenameController("nope", "x"));
    CHECK(vom.GetViewObject("Controller: A") == nullptr);
    CHECK(vom.GetViewObject("Controller: A2") == a);
    CHECK(a->GetControllerName() == "A2");

    auto orphans = vom.GetOrphanedControllerObjects([](const std::string& n) { return n == "B"; });
    CHECK(orphans == std::vector<std::string>{ "A2" });

    CHECK(vom.DeleteControllerObject("A2"));
    CHECK_FALSE(vom.DeleteControllerObject("A2"));
    CHECK(vom.size() == 1);
    CHECK(vom.GetOrphanedControllerObjects([](const std::string&) { return true; }).empty());
}

TEST_CASE("Controller object serialises its binding and display options", "[modelcore][controllerobject]") {
    ViewObjectManager vom(nullptr);
    auto* o = AddObj(vom, R"(<view_object name="ignored" DisplayAs="Controller" Controller="Front" Visibility="LayoutPanel" ShowLabel="1" LabelSize="250" WorldPosX="10" WorldPosY="20" WorldPosZ="0"/>)");
    auto* co = dynamic_cast<ControllerObject*>(o);
    REQUIRE(co != nullptr);
    // the name always derives from the controller
    CHECK(co->GetName() == "Controller: Front");
    CHECK(co->GetControllerName() == "Front");
    CHECK(co->GetVisibility() == ControllerObject::Visibility::LayoutPanel);
    CHECK(co->GetShowLabel());
    CHECK(co->GetLabelSize() == 250);
    std::string s = Ser(co);
    CHECK(s.find("Controller=\"Front\"") != std::string::npos);
    CHECK(s.find("Visibility=\"LayoutPanel\"") != std::string::npos);
    CHECK(s.find("ShowLabel=\"1\"") != std::string::npos);
    CHECK(s.find("LabelSize=\"250\"") != std::string::npos);

    ViewObjectManager vom2(nullptr);
    auto* co2 = dynamic_cast<ControllerObject*>(AddObj(vom2, s));
    REQUIRE(co2 != nullptr);
    CHECK(Ser(co2) == s);
}

TEST_CASE("Controller object draws a flat footprint in 2D", "[modelcore][controllerobject]") {
    ViewObjectManager vom(nullptr);
    auto* co = vom.CreateControllerObject("Ctl");
    co->GetObjectScreenLocation().SetWorldPos(500.0f, 400.0f, 77.0f);
    co->SetStatusColor(xlColor(0, 200, 0));
    co->InitModel();

    FakePreview p(1000, 1000, false);
    CHECK(co->Draw(&p, &p.ctx, p.solid.get(), p.transparent.get(), false));
    auto& v = p.SolidVerts().v;
    // body (6) + outline (8) + status lamp (6); label is off by default
    REQUIRE(v.size() == 20);
    float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
    for (auto& x : v) {
        CHECK(x.z == 0.0f); // flattened onto the canvas
        minX = std::min(minX, x.x);
        maxX = std::max(maxX, x.x);
        minY = std::min(minY, x.y);
        maxY = std::max(maxY, x.y);
    }
    CHECK(maxX - minX == Approx(100.0f));
    CHECK(maxY - minY == Approx(60.0f));
    CHECK((minX + maxX) / 2 == Approx(500.0f));
    CHECK((minY + maxY) / 2 == Approx(400.0f));
    // lamp uses the status colour
    CHECK(v.back().c == xlColor(0, 200, 0));
    p.Run();
    CHECK(p.ctx.triangles == 2);
    CHECK(p.ctx.lines == 1);
    CHECK(p.ctx.vertsDrawn == 20);
}

TEST_CASE("Controller object draws a shaded box in 3D", "[modelcore][controllerobject]") {
    ViewObjectManager vom(nullptr);
    auto* co = vom.CreateControllerObject("Ctl");
    co->InitModel();
    co->SetStatusColor(xlColor(100, 100, 100));

    FakePreview p(1000, 1000, true);
    CHECK(co->Draw(&p, &p.ctx, p.solid.get(), p.transparent.get(), false));
    auto& v = p.SolidVerts().v;
    // fallback box (36) + status lamp (6)
    REQUIRE(v.size() == 42);
    std::set<float> zs;
    for (size_t i = 0; i < 36; ++i) {
        zs.insert(v[i].z);
    }
    CHECK(zs.size() == 2);
    CHECK(*zs.rbegin() - *zs.begin() == Approx(25.0f));
    // front face is full brightness, back face darker
    CHECK(v[0].c == xlColor(100, 100, 100));
    CHECK(v[6].c.red < 100);

    // a highlighted controller tints the box yellow
    p.highlightedController = "Ctl";
    FakePreview p2(1000, 1000, true);
    p2.highlightedController = "Ctl";
    CHECK(co->Draw(&p2, &p2.ctx, p2.solid.get(), p2.transparent.get(), false));
    auto& v2 = p2.SolidVerts().v;
    REQUIRE(v2.size() == 42);
    CHECK(v2[0].c == xlYELLOW);
}

TEST_CASE("Controller object label is drawn only when enabled", "[modelcore][controllerobject]") {
    ViewObjectManager vom(nullptr);
    auto* co = vom.CreateControllerObject("Controller A");
    co->InitModel();
    co->SetShowLabel(true);
    co->Selected(true);

    for (bool is3d : { false, true }) {
        INFO("3d " << is3d);
        FakePreview p(1000, 1000, is3d);
        CHECK(co->Draw(&p, &p.ctx, p.solid.get(), p.transparent.get(), true));
        p.Run();
        CHECK(p.ctx.textures == 1);
    }

    co->SetShowLabel(false);
    FakePreview p(1000, 1000, false);
    CHECK(co->Draw(&p, &p.ctx, p.solid.get(), p.transparent.get(), false));
    p.Run();
    CHECK(p.ctx.textures == 0);

    co->SetActive(false);
    FakePreview p2(1000, 1000, false);
    CHECK(co->Draw(&p2, &p2.ctx, p2.solid.get(), p2.transparent.get(), false));
    CHECK(p2.SolidVerts().v.empty());
}

TEST_CASE("Controller object uses the enclosure mesh when it is installed", "[modelcore][controllerobject]") {
    TempDir res;
    std::filesystem::create_directories(res.path / "meshobjects" / "controller");
    std::ofstream(res.path / "meshobjects" / "controller" / "controller.obj") << "v -1 -1 0\nv 1 -1 0\nv 0 1 0\nf 1 2 3\n";
    const std::string prev = FileUtils::GetResourcesDir();
    FileUtils::SetResourcesDir(res.path.string());

    ViewObjectManager vom(nullptr);
    auto* co = vom.CreateControllerObject("Ctl");
    co->InitModel();
    FakePreview p(1000, 1000, true);
    p.ctx.provideMeshes = true;
    CHECK(co->Draw(&p, &p.ctx, p.solid.get(), p.transparent.get(), false));
    // the mesh replaces the fallback box: only the status lamp is geometry
    CHECK(p.SolidVerts().v.size() == 6);
    p.Run();
    CHECK(p.ctx.meshes == 1);
    CHECK(p.ctx.matrixDepth == 0);

    // a highlighted controller draws the tintable box instead
    FakePreview h(1000, 1000, true);
    h.highlightedController = "Ctl";
    CHECK(co->Draw(&h, &h.ctx, h.solid.get(), h.transparent.get(), false));
    CHECK(h.SolidVerts().v.size() == 42);

    FileUtils::SetResourcesDir(prev);
}

// ---------------------------------------------------------------------------
// ViewObjectManager
// ---------------------------------------------------------------------------

TEST_CASE("View object manager creates every type with unique names", "[modelcore][viewobjects]") {
    ViewObjectManager vom(nullptr);
    std::set<std::string> names;
    for (const char* t : { "Gridlines", "Ruler", "Image", "Mesh", "Terrain", "Controller", "Gridlines" }) {
        ViewObject* o = vom.CreateAndAddObject(t);
        REQUIRE(o != nullptr);
        names.insert(o->GetName());
    }
    CHECK(names.size() == 7);
    CHECK(vom.size() == 7);
    CHECK(vom.CreateAndAddObject("Teapot") == nullptr);
    CHECK(vom.size() == 7);

    int n = 0;
    for (auto it = vom.begin(); it != vom.end(); ++it) {
        ++n;
    }
    CHECK(n == 7);

    vom.clear();
    CHECK(vom.size() == 0);
}

TEST_CASE("View object manager renames and deletes objects", "[modelcore][viewobjects]") {
    ViewObjectManager vom(nullptr);
    ViewObject* g = vom.CreateAndAddObject("Gridlines");
    std::string old = g->GetName();
    CHECK_FALSE(vom.Rename("missing", "x"));
    vom.Rename(old, "Grid A");
    CHECK(vom.GetViewObject(old) == nullptr);
    CHECK(vom.GetViewObject("Grid A") == g);
    CHECK(g->GetName() == "Grid A");

    vom.Delete("not there");
    CHECK(vom.size() == 1);
    vom.Delete("Grid A");
    CHECK(vom.size() == 0);
}

TEST_CASE("Adding a view object with an existing name replaces it", "[modelcore][viewobjects]") {
    ViewObjectManager vom(nullptr);
    AddObj(vom, R"(<view_object name="G" DisplayAs="Gridlines" GridLineSpacing="10"/>)");
    ViewObject* second = AddObj(vom, R"(<view_object name="G" DisplayAs="Gridlines" GridLineSpacing="20"/>)");
    CHECK(vom.size() == 1);
    CHECK(vom.GetViewObject("G") == second);
    vom.AddViewObject(nullptr);
    CHECK(vom.size() == 1);
}

TEST_CASE("View object manager loads only named view objects", "[modelcore][viewobjects]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<view_objects>
        <view_object name="G" DisplayAs="Gridlines"/>
        <view_object name="" DisplayAs="Gridlines"/>
        <other name="X" DisplayAs="Gridlines"/>
        <view_object name="R" DisplayAs="Ruler" Units="2" Length="3"/>
    </view_objects>)"));
    ViewObjectManager vom(nullptr);
    vom.CreateAndAddObject("Terrain"); // replaced by the load
    vom.LoadViewObjects(doc.first_child());
    CHECK(vom.size() == 2);
    CHECK(vom.GetViewObject("G") != nullptr);
    auto* r = dynamic_cast<RulerObject*>(vom.GetViewObject("R"));
    REQUIRE(r != nullptr);
    CHECK(r->GetLength() == 3.0f);
    CHECK(RulerObject::GetUnits() == RULER_UNITS_MM);
}

namespace {
const char* kBaseShow = R"(<?xml version="1.0" encoding="UTF-8"?>
<xrgb>
  <view_objects>
    <view_object name="BaseGrid" DisplayAs="Gridlines" GridLineSpacing="40"/>
    <view_object name="Shared" DisplayAs="Gridlines" GridLineSpacing="70"/>
    <view_object name="" DisplayAs="Gridlines"/>
  </view_objects>
</xrgb>
)";
}

TEST_CASE("Base show view objects merge into the XML", "[modelcore][viewobjects][basemerge]") {
    TempDir base;
    base.Write("xlights_rgbeffects.xml", kBaseShow);
    ViewObjectManager vom(nullptr);

    pugi::xml_document local;
    REQUIRE(local.load_string(R"(<view_objects>
        <view_object name="Shared" DisplayAs="Gridlines" GridLineSpacing="10"/>
    </view_objects>)"));
    bool changed = false;
    CHECK(vom.MergeBaseXml(base.path.string(), local.first_child(), &changed));
    CHECK(changed);
    auto vo = local.first_child();
    // a local object not from base keeps its own values
    auto shared = vo.find_child_by_attribute("view_object", "name", "Shared");
    CHECK(std::string(shared.attribute("GridLineSpacing").as_string()) == "10");
    auto added = vo.find_child_by_attribute("view_object", "name", "BaseGrid");
    REQUIRE(added);
    CHECK(std::string(added.attribute("FromBase").as_string()) == "1");

    // a FromBase object that differs from base is replaced, a matching one is left
    pugi::xml_document local2;
    REQUIRE(local2.load_string(R"(<view_objects>
        <view_object name="Shared" DisplayAs="Gridlines" GridLineSpacing="10" FromBase="1"/>
        <view_object name="BaseGrid" DisplayAs="Gridlines" GridLineSpacing="40" FromBase="1"/>
    </view_objects>)"));
    bool changed2 = false;
    CHECK(vom.MergeBaseXml(base.path.string(), local2.first_child(), &changed2));
    CHECK(changed2);
    auto s2 = local2.first_child().find_child_by_attribute("view_object", "name", "Shared");
    CHECK(std::string(s2.attribute("GridLineSpacing").as_string()) == "70");

    pugi::xml_document local3;
    REQUIRE(local3.load_string(R"(<view_objects>
        <view_object name="Shared" DisplayAs="Gridlines" GridLineSpacing="70" FromBase="1"/>
        <view_object name="BaseGrid" DisplayAs="Gridlines" GridLineSpacing="40" FromBase="1"/>
    </view_objects>)"));
    bool changed3 = false;
    CHECK(vom.MergeBaseXml(base.path.string(), local3.first_child(), &changed3));
    CHECK_FALSE(changed3);

    TempDir empty;
    CHECK_FALSE(vom.MergeBaseXml(empty.path.string(), local3.first_child(), nullptr));
}

TEST_CASE("Base show view objects merge into the manager", "[modelcore][viewobjects][basemerge]") {
    TempDir base;
    base.Write("xlights_rgbeffects.xml", kBaseShow);
    ViewObjectManager vom(nullptr);
    AddObj(vom, R"(<view_object name="Shared" DisplayAs="Gridlines" GridLineSpacing="10"/>)");
    AddObj(vom, R"(<view_object name="Mine" DisplayAs="Gridlines" GridLineSpacing="15"/>)");

    bool acceptAll = false, rejectAll = false, changed = false;
    CHECK(vom.MergeFromBase(base.path.string(), true, acceptAll, rejectAll, &changed));
    CHECK(changed);
    CHECK(vom.size() == 3);
    ViewObject* bg = vom.GetViewObject("BaseGrid");
    REQUIRE(bg != nullptr);
    CHECK(bg->IsFromBase());
    // without a UI to ask, the clashing local object is kept
    CHECK_FALSE(vom.GetViewObject("Shared")->IsFromBase());
    CHECK(Ser(vom.GetViewObject("Shared")).find("GridLineSpacing=\"10\"") != std::string::npos);

    // a second merge is a no-op
    bool changed2 = true;
    CHECK(vom.MergeFromBase(base.path.string(), false, acceptAll, rejectAll, &changed2));
    CHECK_FALSE(changed2);

    // once an object is marked as from base, the base version replaces it
    vom.GetViewObject("Shared")->SetFromBase(true);
    bool changed3 = false;
    CHECK(vom.MergeFromBase(base.path.string(), false, acceptAll, rejectAll, &changed3));
    CHECK(changed3);
    CHECK(Ser(vom.GetViewObject("Shared")).find("GridLineSpacing=\"70\"") != std::string::npos);

    TempDir none;
    CHECK_FALSE(vom.MergeFromBase(none.path.string(), false, acceptAll, rejectAll, nullptr));
    TempDir noObjects;
    noObjects.Write("xlights_rgbeffects.xml", "<xrgb><models/></xrgb>");
    CHECK(vom.MergeFromBase(noObjects.path.string(), false, acceptAll, rejectAll, nullptr));
}

// ---------------------------------------------------------------------------
// MeshObject
// ---------------------------------------------------------------------------

namespace {
const char* kTriObj = R"(mtllib tri.mtl
v -2 -1 0
v 2 -1 0
v 0 3 0.5
usemtl red
f 1 2 3
)";
const char* kTriMtl = R"(newmtl red
Kd 1 0 0
map_Kd missing_tex.png
)";
} // namespace

TEST_CASE("Mesh object clamps brightness and reports a missing obj file", "[modelcore][mesh]") {
    ViewObjectManager vom(nullptr);
    auto* m = dynamic_cast<MeshObject*>(AddObj(vom, R"(<view_object name="M" DisplayAs="Mesh" Brightness="250"/>)"));
    REQUIRE(m != nullptr);
    m->InitModel();
    CHECK(m->GetBrightness() == 100);
    m->SetBrightness(-5);
    m->InitModel();
    CHECK(m->GetBrightness() == 0);

    auto errs = m->CheckModelSettings();
    REQUIRE_FALSE(errs.empty());
    CHECK(errs.front().find("ERR: Mesh object 'M' cant find obj file") != std::string::npos);
    CHECK(m->GetFileReferences().empty());
}

TEST_CASE("Mesh object finds its material and reports a missing texture", "[modelcore][mesh]") {
    TempDir d;
    std::string obj = d.Write("tri.obj", kTriObj);
    std::string mtl = d.Write("tri.mtl", kTriMtl);
    ViewObjectManager vom(nullptr);
    auto* m = dynamic_cast<MeshObject*>(vom.CreateAndAddObject("Mesh"));
    REQUIRE(m != nullptr);
    m->SetObjectFile(obj);
    CHECK(m->GetObjFile() == obj);

    auto refs = m->GetFileReferences();
    CHECK(std::find(refs.begin(), refs.end(), obj) != refs.end());
    CHECK(std::find(refs.begin(), refs.end(), mtl) != refs.end());

    auto errs = m->CheckModelSettings();
    bool missingTex = false;
    for (auto& e : errs) {
        if (e.find("cant find texture file") != std::string::npos && e.find("missing_tex.png") != std::string::npos) missingTex = true;
        CHECK(e.find("cant find obj file") == std::string::npos);
    }
    CHECK(missingTex);
}

TEST_CASE("Mesh object warns when its material file is missing", "[modelcore][mesh]") {
    TempDir d;
    std::string obj = d.Write("tri.obj", kTriObj);
    ViewObjectManager vom(nullptr);
    auto* m = dynamic_cast<MeshObject*>(vom.CreateAndAddObject("Mesh"));
    m->SetObjectFile(obj);
    auto errs = m->CheckModelSettings();
    bool missingMtl = false;
    for (auto& e : errs) {
        if (e.find("is missing material file") != std::string::npos) missingMtl = true;
    }
    CHECK(missingMtl);

    TempDir d2;
    std::string bare = d2.Write("bare.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n");
    m->SetObjectFile(bare);
    bool noMtl = false;
    for (auto& e : m->CheckModelSettings()) {
        if (e.find("does not have a material file") != std::string::npos) noMtl = true;
    }
    CHECK(noMtl);
}

TEST_CASE("Mesh object draws its mesh once it has loaded", "[modelcore][mesh]") {
    TempDir d;
    std::string obj = d.Write("tri.obj", kTriObj);
    d.Write("tri.mtl", "newmtl red\nKd 1 0 0\n");
    ViewObjectManager vom(nullptr);
    auto* m = dynamic_cast<MeshObject*>(vom.CreateAndAddObject("Mesh"));
    m->SetObjectFile(obj);

    FakePreview p(1000, 1000, true);
    p.ctx.provideMeshes = true;
    // the obj file is prefetched in the background; keep drawing until it is in
    for (int i = 0; i < 500 && p.solid->getAccumulator() && p.ctx.meshes == 0; ++i) {
        FakePreview step(1000, 1000, true);
        step.ctx.provideMeshes = true;
        CHECK_FALSE(m->Draw(&step, &step.ctx, step.solid.get(), step.transparent.get(), false));
        step.Run();
        p.ctx.meshes += step.ctx.meshes;
        if (step.ctx.meshes == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    CHECK(p.ctx.meshes == 2); // solids + transparents
    // the render size is twice the largest extent on each axis
    CHECK(m->GetObjectScreenLocation().GetRenderWi() == Approx(4.0f));
    CHECK(m->GetObjectScreenLocation().GetRenderHt() == Approx(6.0f));

    // wireframe only draws once
    m->SetMeshOnly(true);
    FakePreview w(1000, 1000, true);
    m->Selected(true);
    m->Draw(&w, &w.ctx, w.solid.get(), w.transparent.get(), true);
    w.Run();
    CHECK(w.ctx.meshes == 1);
    CHECK(w.SolidVerts().v.size() > 0); // handles

    m->SetActive(false);
    FakePreview off(1000, 1000, true);
    CHECK(m->Draw(&off, &off.ctx, off.solid.get(), off.transparent.get(), false));
}
