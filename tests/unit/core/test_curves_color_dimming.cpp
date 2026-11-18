#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "render/ColorCurve.h"
#include "render/DimmingCurve.h"
#include "support/TempDir.h"

#include <memory>
#include <string>

using Catch::Matchers::WithinAbs;

namespace {
void CheckColor(const xlColor& c, int r, int g, int b, int tol = 1) {
    CHECK(std::abs((int)c.Red() - r) <= tol);
    CHECK(std::abs((int)c.Green() - g) <= tol);
    CHECK(std::abs((int)c.Blue() - b) <= tol);
}

ColorCurve TwoPoint(const std::string& type = "Gradient") {
    ColorCurve cc("ID_CC", type, xlRED);
    cc.SetValueAt(0.0f, xlRED);
    cc.SetValueAt(0.5f, xlBLACK);
    cc.DeletePoint(0.5f);
    cc.SetValueAt(1.0f, xlBLUE);
    cc.SetActive(true);
    return cc;
}
}

TEST_CASE("ColorCurve default is inactive with one black point", "[curves][colorcurve]") {
    ColorCurve cc;
    CHECK_FALSE(cc.IsOk());
    CHECK_FALSE(cc.IsActive());
    CHECK(cc.GetPointCount() == 1);
    CHECK(cc.GetType() == "Gradient");
    CHECK(cc.GetTimeCurve() == TC_TIME);
}

TEST_CASE("ColorCurve IsColorCurve recognises serialised form", "[curves][colorcurve]") {
    CHECK(ColorCurve::IsColorCurve("Active=TRUE|Id=ID_X|"));
    CHECK_FALSE(ColorCurve::IsColorCurve("#FF0000"));
    CHECK_FALSE(ColorCurve::IsColorCurve(""));
}

TEST_CASE("ColorCurve non curve string does not deserialise", "[curves][colorcurve]") {
    ColorCurve cc("#FF0000");
    CHECK_FALSE(cc.IsActive());
    CHECK(cc.GetPointCount() == 1);
}

TEST_CASE("ColorCurve inactive serialises as Active=FALSE", "[curves][colorcurve]") {
    ColorCurve cc("ID_CC", "Gradient", xlRED);
    CHECK(cc.Serialise() == "Active=FALSE|");
}

TEST_CASE("ColorCurve serialise and deserialise round trip", "[curves][colorcurve]") {
    auto cc = TwoPoint();
    std::string s = cc.Serialise();
    CHECK(s.find("Active=TRUE|Id=ID_CC|Values=") == 0);
    CHECK(s.find("Type=") == std::string::npos);

    ColorCurve back(s);
    CHECK(back.IsActive());
    CHECK(back.GetId() == "ID_CC");
    CHECK(back.GetPointCount() == 2);
    CHECK(back.Serialise() == s);
    CheckColor(back.GetValueAt(0.0f), 255, 0, 0);
    CheckColor(back.GetValueAt(1.0f), 0, 0, 255);
}

TEST_CASE("ColorCurve non-default type and timecurve are serialised", "[curves][colorcurve]") {
    auto cc = TwoPoint("None");
    cc.NextTimeCurve(true, true);
    std::string s = cc.Serialise();
    CHECK(s.find("Type=None|") != std::string::npos);
    CHECK(s.find("Timecurve=1|") != std::string::npos);
    ColorCurve back(s);
    CHECK(back.GetType() == "None");
    CHECK(back.GetTimeCurve() == TC_RIGHT);
}

TEST_CASE("ColorCurve gradient blends between points", "[curves][colorcurve]") {
    auto cc = TwoPoint();
    CheckColor(cc.GetValueAt(0.5f), 128, 0, 128);
    CheckColor(cc.GetValueAt(0.25f), 191, 0, 64);
    CheckColor(cc.GetValueAt(0.75f), 64, 0, 191);
}

TEST_CASE("ColorCurve None type steps between colours", "[curves][colorcurve]") {
    auto cc = TwoPoint("None");
    CheckColor(cc.GetValueAt(0.0f), 255, 0, 0, 0);
    CheckColor(cc.GetValueAt(0.49f), 255, 0, 0, 0);
    CheckColor(cc.GetValueAt(1.0f), 0, 0, 255, 0);
}

TEST_CASE("ColorCurve unknown type evaluates to black", "[curves][colorcurve]") {
    auto cc = TwoPoint("Bogus");
    CheckColor(cc.GetValueAt(0.5f), 0, 0, 0, 0);
}

TEST_CASE("ColorCurve Random type returns exact colours at points and is deterministic", "[curves][colorcurve]") {
    auto cc = TwoPoint("Random");
    CheckColor(cc.GetValueAt(0.0f), 255, 0, 0, 0);
    CheckColor(cc.GetValueAt(1.0f), 0, 0, 255, 0);
    auto a = cc.GetValueAt(0.4f);
    auto b = cc.GetValueAt(0.4f);
    CHECK(a.GetRGBA() == b.GetRGBA());
    CHECK(a.Red() >= 0);
    CHECK(a.Blue() <= 255);
}

TEST_CASE("ColorCurve points are kept sorted", "[curves][colorcurve]") {
    ColorCurve cc("ID_CC", "Gradient", xlRED);
    cc.SetValueAt(0.9f, xlBLUE);
    cc.SetValueAt(0.1f, xlGREEN);
    cc.SetValueAt(0.5f, xlWHITE);
    auto pts = cc.GetPoints();
    float last = -1.0f;
    for (const auto& p : pts) {
        CHECK(p.x > last);
        last = p.x;
    }
}

TEST_CASE("ColorCurve SetValueAt replaces rather than duplicates", "[curves][colorcurve]") {
    ColorCurve cc("ID_CC", "Gradient", xlRED);
    cc.SetValueAt(0.3f, xlGREEN);
    int n = cc.GetPointCount();
    cc.SetValueAt(0.3f, xlBLUE);
    CHECK(cc.GetPointCount() == n);
    CheckColor(cc.GetPointAt(0.3f)->color, 0, 0, 255, 0);
    cc.SetValueAt(0.301f, xlWHITE);
    CHECK(cc.GetPointCount() == n);
}

TEST_CASE("ColorCurve points snap to 1/100 grid", "[curves][colorcurve]") {
    CHECK_THAT(ccSortableColorPoint::Normalise(0.333f), WithinAbs(0.33, 1e-6));
    CHECK(ccSortableColorPoint::Normalise(-1.0f) == 0.0f);
    CHECK(ccSortableColorPoint::Normalise(7.0f) == 1.0f);
}

TEST_CASE("ColorCurve DeletePoint keeps the last point", "[curves][colorcurve]") {
    ColorCurve cc("ID_CC", "Gradient", xlRED);
    CHECK(cc.GetPointCount() == 1);
    cc.DeletePoint(0.5f);
    CHECK(cc.GetPointCount() == 1);
    cc.SetValueAt(0.2f, xlBLUE);
    CHECK(cc.GetPointCount() == 2);
    cc.DeletePoint(0.2f);
    CHECK(cc.GetPointCount() == 1);
    CHECK_FALSE(cc.IsSetPoint(0.2f));
}

TEST_CASE("ColorCurve DeletePoint snaps off-grid positions", "[curves][colorcurve]") {
    ColorCurve cc("ID_CC", "Gradient", xlRED);
    cc.SetValueAt(0.3f, xlBLUE);
    cc.DeletePoint(0.3004f);
    CHECK(cc.GetPointCount() == 1);
}

TEST_CASE("ColorCurve Flip mirrors point positions", "[curves][colorcurve]") {
    auto cc = TwoPoint();
    cc.SetValueAt(0.25f, xlGREEN);
    cc.Flip();
    REQUIRE(cc.GetPointAt(0.0f) != nullptr);
    REQUIRE(cc.GetPointAt(0.75f) != nullptr);
    REQUIRE(cc.GetPointAt(1.0f) != nullptr);
    CheckColor(cc.GetPointAt(0.0f)->color, 0, 0, 255, 0);
    CheckColor(cc.GetPointAt(0.75f)->color, 0, 255, 0, 0);
    CheckColor(cc.GetPointAt(1.0f)->color, 255, 0, 0, 0);
}

TEST_CASE("ColorCurve SetDefault only changes single-point curves", "[curves][colorcurve]") {
    ColorCurve one("ID_CC", "Gradient", xlRED);
    one.SetDefault(xlGREEN);
    CheckColor(one.GetPoints().front().color, 0, 255, 0, 0);

    auto two = TwoPoint();
    two.SetDefault(xlWHITE);
    CheckColor(two.GetPoints().front().color, 255, 0, 0, 0);
}

TEST_CASE("ColorCurve time curve cycling respects supported modes", "[curves][colorcurve]") {
    ColorCurve cc("ID_CC", "Gradient", xlRED);
    cc.NextTimeCurve(false, false);
    CHECK(cc.GetTimeCurve() == TC_TIME);

    cc.NextTimeCurve(true, false);
    CHECK(cc.GetTimeCurve() == TC_RIGHT);

    ColorCurve rad("ID_CC", "Gradient", xlRED);
    rad.NextTimeCurve(false, true);
    CHECK(rad.GetTimeCurve() == TC_RADIALIN);

    ColorCurve wrap("ID_CC", "Gradient", xlRED);
    for (int i = 0; i < 20; ++i) wrap.NextTimeCurve(true, true);
    CHECK(wrap.GetTimeCurve() >= TC_TIME);
    CHECK(wrap.GetTimeCurve() <= TC_CCW);
}

TEST_CASE("ColorCurve NearPoint and neighbour search", "[curves][colorcurve]") {
    ColorCurve cc("ID_CC", "Gradient", xlRED);
    cc.SetValueAt(0.2f, xlBLUE);
    cc.SetValueAt(0.8f, xlGREEN);
    CHECK(cc.NearPoint(0.2f));
    CHECK_FALSE(cc.NearPoint(0.4f));
    CHECK_THAT(cc.FindMinPointLessThan(0.5f), WithinAbs(0.23, 1e-3));
    CHECK_THAT(cc.FindMaxPointGreaterThan(0.5f), WithinAbs(0.78, 1e-3));
}

TEST_CASE("ColorCurve malformed values are tolerated", "[curves][colorcurve]") {
    SECTION("empty values list leaves a default black point") {
        ColorCurve cc("Active=TRUE|Id=ID_A|Values=|");
        CHECK(cc.GetPointCount() == 1);
    }
    SECTION("unknown keys ignored") {
        ColorCurve cc("Active=TRUE|Id=ID_A|Foo=bar|Values=x=0.000^c=#FF0000;x=1.000^c=#0000FF|");
        CHECK(cc.GetPointCount() == 2);
        CheckColor(cc.GetValueAt(1.0f), 0, 0, 255, 0);
    }
    SECTION("unsorted points are sorted on load") {
        ColorCurve cc("Active=TRUE|Id=ID_A|Values=x=1.000^c=#0000FF;x=0.000^c=#FF0000|");
        CHECK(cc.GetPoints().front().x == 0.0f);
    }
    SECTION("out of range x is clamped") {
        ColorCurve cc("Active=TRUE|Id=ID_A|Values=x=-3.000^c=#FF0000;x=9.000^c=#0000FF|");
        CHECK(cc.GetPoints().front().x == 0.0f);
        CHECK(cc.GetPoints().back().x == 1.0f);
    }
}

TEST_CASE("ColorCurve LoadXCC reads a colorcurve file", "[curves][colorcurve]") {
    TempDir td;
    auto f = td.Write("a.xcc", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<colorcurve data=\"Active=TRUE|Id=ID_FILE|Values=x=0.000^c=#FF0000;x=1.000^c=#00FF00|\" />");
    ColorCurve cc("ID_KEEP", "Gradient", xlBLACK);
    cc.LoadXCC(f);
    CHECK(cc.IsActive());
    CHECK(cc.GetId() == "ID_KEEP");
    CHECK(cc.GetPointCount() == 2);
    CheckColor(cc.GetValueAt(1.0f), 0, 255, 0, 0);
}

TEST_CASE("ColorCurve LoadXCC with a missing or wrong file leaves an inactive curve", "[curves][colorcurve]") {
    TempDir td;
    ColorCurve a("ID_KEEP", "Gradient", xlBLACK);
    a.LoadXCC(td.File("missing.xcc"));
    CHECK_FALSE(a.IsActive());
    CHECK(a.GetId() == "ID_KEEP");

    auto f = td.Write("bad.xcc", "<other data=\"x\"/>");
    ColorCurve b("ID_KEEP", "Gradient", xlBLACK);
    b.LoadXCC(f);
    CHECK_FALSE(b.IsActive());
}

TEST_CASE("ColorCurve GetColorCurveFolder creates the folder", "[curves][colorcurve]") {
    TempDir td;
    auto folder = ColorCurve::GetColorCurveFolder(td.path.string());
    CHECK_FALSE(folder.empty());
    CHECK(std::filesystem::is_directory(folder));
    CHECK(ColorCurve::GetColorCurveFolder("").empty());
}

TEST_CASE("DimmingCurve brightness gamma identity leaves colours unchanged", "[curves][dimming]") {
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createBrightnessGamma(0, 1.0f));
    for (int v : { 0, 1, 64, 128, 200, 255 }) {
        xlColor c(v, v, v);
        dc->apply(c);
        CHECK((int)c.Red() == v);
        CHECK((int)c.Green() == v);
        CHECK((int)c.Blue() == v);
    }
}

TEST_CASE("DimmingCurve gamma darkens midtones and preserves the endpoints", "[curves][dimming]") {
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createBrightnessGamma(0, 2.0f));
    xlColor black(0, 0, 0);
    xlColor white(255, 255, 255);
    xlColor mid(128, 128, 128);
    dc->apply(black);
    dc->apply(white);
    dc->apply(mid);
    CHECK((int)black.Red() == 0);
    CHECK((int)white.Red() == 255);
    CHECK(std::abs((int)mid.Red() - 64) <= 1);
}

TEST_CASE("DimmingCurve gamma below one brightens midtones", "[curves][dimming]") {
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createBrightnessGamma(0, 0.5f));
    xlColor mid(64, 64, 64);
    dc->apply(mid);
    CHECK((int)mid.Red() > 64);
}

TEST_CASE("DimmingCurve positive brightness scales and clamps to 255", "[curves][dimming]") {
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createBrightnessGamma(100, 1.0f));
    xlColor c(100, 100, 100);
    dc->apply(c);
    CHECK((int)c.Red() == 200);
    xlColor hi(200, 200, 200);
    dc->apply(hi);
    CHECK((int)hi.Red() == 255);
}

TEST_CASE("DimmingCurve brightness of -100 blacks out everything", "[curves][dimming]") {
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createBrightnessGamma(-100, 1.0f));
    xlColor c(255, 128, 3);
    dc->apply(c);
    CHECK((int)c.Red() == 0);
    CHECK((int)c.Green() == 0);
    CHECK((int)c.Blue() == 0);
}

TEST_CASE("DimmingCurve apply is monotonic", "[curves][dimming]") {
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createBrightnessGamma(0, 2.2f));
    int prev = -1;
    for (int v = 0; v < 256; ++v) {
        xlColor c(v, v, v);
        dc->apply(c);
        CHECK((int)c.Red() >= prev);
        prev = c.Red();
    }
}

TEST_CASE("DimmingCurve reverse approximately inverts apply for bright values", "[curves][dimming]") {
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createBrightnessGamma(0, 2.0f));
    for (int v : { 128, 160, 200, 255 }) {
        xlColor c(v, v, v);
        dc->apply(c);
        dc->reverse(c);
        CHECK(std::abs((int)c.Red() - v) <= 3);
    }
}

TEST_CASE("DimmingCurve applies independently per channel", "[curves][dimming]") {
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createBrightnessGamma(0, 2.0f));
    xlColor c(255, 128, 0);
    dc->apply(c);
    CHECK((int)c.Red() == 255);
    CHECK(std::abs((int)c.Green() - 64) <= 1);
    CHECK((int)c.Blue() == 0);
}

TEST_CASE("DimmingCurve createFromInfo returns null for identity settings", "[curves][dimming]") {
    std::map<std::string, std::map<std::string, std::string>> info;
    info["all"] = { { "gamma", "1.0" }, { "brightness", "0" } };
    CHECK(DimmingCurve::createFromInfo(info) == nullptr);
    CHECK(DimmingCurve::createFromInfo({}) == nullptr);
}

TEST_CASE("DimmingCurve createFromInfo all channels", "[curves][dimming]") {
    std::map<std::string, std::map<std::string, std::string>> info;
    info["all"] = { { "gamma", "2.0" }, { "brightness", "0" } };
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createFromInfo(info));
    REQUIRE(dc != nullptr);
    xlColor c(128, 128, 128);
    dc->apply(c);
    CHECK(std::abs((int)c.Red() - 64) <= 1);
    CHECK(std::abs((int)c.Blue() - 64) <= 1);
}

TEST_CASE("DimmingCurve createFromInfo single channel only touches that channel", "[curves][dimming]") {
    std::map<std::string, std::map<std::string, std::string>> info;
    info["green"] = { { "gamma", "2.0" }, { "brightness", "0" } };
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createFromInfo(info));
    REQUIRE(dc != nullptr);
    xlColor c(128, 128, 128);
    dc->apply(c);
    CHECK((int)c.Red() == 128);
    CHECK(std::abs((int)c.Green() - 64) <= 1);
    CHECK((int)c.Blue() == 128);
}

TEST_CASE("DimmingCurve createFromInfo ignores empty strings", "[curves][dimming]") {
    std::map<std::string, std::map<std::string, std::string>> info;
    info["all"] = { { "gamma", "" }, { "brightness", "" } };
    CHECK(DimmingCurve::createFromInfo(info) == nullptr);
}

TEST_CASE("DimmingCurve file curve maps each line to a level", "[curves][dimming]") {
    TempDir td;
    std::string body;
    for (int i = 0; i < 256; ++i) {
        body += std::to_string(i / 2) + "\n";
    }
    auto f = td.Write("curve.dim", body);
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createFromFile(f));
    REQUIRE(dc != nullptr);
    xlColor c(200, 100, 50);
    dc->apply(c);
    CHECK((int)c.Red() == 100);
    CHECK((int)c.Green() == 50);
    CHECK((int)c.Blue() == 25);
}

TEST_CASE("DimmingCurve missing file falls back to a usable curve", "[curves][dimming]") {
    TempDir td;
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createFromFile(td.File("nope.dim")));
    REQUIRE(dc != nullptr);
    xlColor c(10, 20, 30);
    dc->apply(c);
    CHECK((int)c.Red() <= 255);
}

TEST_CASE("DimmingCurve createFromInfo uses filename curves", "[curves][dimming]") {
    TempDir td;
    std::string body;
    for (int i = 0; i < 256; ++i) {
        body += std::to_string(255 - i) + "\n";
    }
    auto f = td.Write("inv.dim", body);
    std::map<std::string, std::map<std::string, std::string>> info;
    info["all"] = { { "filename", f } };
    std::unique_ptr<DimmingCurve> dc(DimmingCurve::createFromInfo(info));
    REQUIRE(dc != nullptr);
    xlColor c(0, 255, 100);
    dc->apply(c);
    CHECK((int)c.Red() == 255);
    CHECK((int)c.Green() == 0);
    CHECK((int)c.Blue() == 155);
}
