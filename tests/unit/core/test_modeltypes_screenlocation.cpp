#include <cmath>
#include <string>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/BoxedScreenLocation.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/PolyPointScreenLocation.h"
#include "models/ThreePointScreenLocation.h"
#include "models/TwoPointScreenLocation.h"
#include "outputs/OutputManager.h"
#include "utils/FloatChecks.h"

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

struct P3 {
    float x, y, z;
};

P3 Map(const ModelScreenLocation& l, float x, float y, float z) {
    l.TranslatePoint(x, y, z);
    return { x, y, z };
}

void CheckPt(const P3& p, float x, float y, float z) {
    CHECK(p.x == Catch::Approx(x).margin(1e-3));
    CHECK(p.y == Catch::Approx(y).margin(1e-3));
    CHECK(p.z == Catch::Approx(z).margin(1e-3));
}

std::string MatrixXml(const std::string& attrs) {
    return "<model name=\"M\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\" " + attrs + "/>";
}

std::string LineXml(const std::string& attrs) {
    return "<model name=\"S\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"10\" " + attrs + "/>";
}

std::string ArchXml(const std::string& attrs) {
    return "<model name=\"A\" DisplayAs=\"Arches\" StartChannel=\"1\" NumArches=\"1\" NodesPerArch=\"10\" " + attrs + "/>";
}

BoxedScreenLocation& Boxed(Model* m) {
    auto* b = dynamic_cast<BoxedScreenLocation*>(&m->GetModelScreenLocation());
    REQUIRE(b != nullptr);
    return *b;
}

TwoPointScreenLocation& TwoPt(Model* m) {
    auto* b = dynamic_cast<TwoPointScreenLocation*>(&m->GetModelScreenLocation());
    REQUIRE(b != nullptr);
    return *b;
}

ThreePointScreenLocation& ThreePt(Model* m) {
    auto* b = dynamic_cast<ThreePointScreenLocation*>(&m->GetModelScreenLocation());
    REQUIRE(b != nullptr);
    return *b;
}

} // namespace

TEST_CASE("Model types use the expected screen location kinds", "[models2][screenloc]") {
    ShowFixture f;
    Model* matrix = f.Add(MatrixXml(""));
    CHECK(dynamic_cast<BoxedScreenLocation*>(&matrix->GetModelScreenLocation()) != nullptr);
    Model* line = f.Add(LineXml(""));
    CHECK(dynamic_cast<TwoPointScreenLocation*>(&line->GetModelScreenLocation()) != nullptr);
    Model* arch = f.Add(ArchXml(""));
    CHECK(dynamic_cast<ThreePointScreenLocation*>(&arch->GetModelScreenLocation()) != nullptr);
    Model* poly = f.Add("<model name=\"P\" DisplayAs=\"Poly Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"6\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\"/>");
    CHECK(dynamic_cast<PolyPointScreenLocation*>(&poly->GetModelScreenLocation()) != nullptr);
}

TEST_CASE("Boxed location combines scale then rotation then translation", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("WorldPosX=\"10\" WorldPosY=\"20\" WorldPosZ=\"5\" ScaleX=\"2\" ScaleY=\"3\" ScaleZ=\"1\" RotateZ=\"90\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    CheckPt(Map(l, 1, 1, 0), 7, 22, 5);
    CheckPt(Map(l, 0, 0, 0), 10, 20, 5);
    CheckPt(Map(l, 2, 0, 0), 10, 24, 5);
}

TEST_CASE("Boxed location with default settings only translates", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("WorldPosX=\"3\" WorldPosY=\"-4\" WorldPosZ=\"7\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    CheckPt(Map(l, 1, 2, 3), 4, -2, 10);
}

TEST_CASE("Boxed rotation applies X then Y then Z", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("RotateX=\"90\" RotateY=\"90\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    CheckPt(Map(l, 0, 1, 0), 1, 0, 0);
    CheckPt(Map(l, 1, 0, 0), 0, 0, -1);
}

TEST_CASE("Boxed X rotation turns up into forward", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("RotateX=\"90\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    CheckPt(Map(l, 0, 1, 0), 0, 0, 1);
    CheckPt(Map(l, 1, 0, 0), 1, 0, 0);
}

TEST_CASE("Boxed rotation setters keep the transform in sync", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml(""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    CheckPt(Map(l, 1, 0, 0), 1, 0, 0);
    l.SetRotateZ(90.0f);
    CHECK(l.GetRotateZ() == Catch::Approx(90.0f));
    CheckPt(Map(l, 1, 0, 0), 0, 1, 0);
    l.SetRotate(0.0f, 90.0f, 0.0f);
    CHECK(l.GetRotateY() == Catch::Approx(90.0f));
    CHECK(l.GetRotateZ() == Catch::Approx(0.0f));
    CheckPt(Map(l, 0, 0, 1), 1, 0, 0);
    l.SetRotateX(180.0f);
    CHECK(l.GetRotateX() == Catch::Approx(180.0f));
}

TEST_CASE("Boxed Rotate composes with the current rotation", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml(""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    CHECK(l.Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, 90.0f));
    CheckPt(Map(l, 1, 0, 0), 0, 1, 0);
    CHECK(l.Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, 90.0f));
    CheckPt(Map(l, 1, 0, 0), -1, 0, 0);
    CHECK(l.Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, -180.0f));
    CheckPt(Map(l, 1, 0, 0), 1, 0, 0);
}

TEST_CASE("Boxed scale multiplies each axis", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("ScaleX=\"2\" ScaleY=\"3\" ScaleZ=\"4\""));
    auto& b = Boxed(m);
    CHECK(b.Scale(glm::vec3(2.0f, 0.5f, 3.0f)));
    CHECK(b.GetScaleX() == Catch::Approx(4.0f));
    CHECK(b.GetScaleY() == Catch::Approx(1.5f));
    CHECK(b.GetScaleZ() == Catch::Approx(12.0f));
}

TEST_CASE("Locked boxed location ignores rotate scale and offset", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("WorldPosX=\"5\" ScaleX=\"2\""));
    auto& b = Boxed(m);
    b.Lock();
    CHECK(b.IsLocked());
    CHECK_FALSE(b.Scale(glm::vec3(3.0f, 3.0f, 3.0f)));
    CHECK_FALSE(b.Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, 45.0f));
    b.AddOffset(1.0f, 1.0f, 1.0f);
    b.RotateAboutPoint(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 30.0f));
    CHECK(b.GetScaleX() == Catch::Approx(2.0f));
    CHECK(b.GetHcenterPos() == Catch::Approx(5.0f));
    CHECK(b.GetVcenterPos() == Catch::Approx(0.0f));
    b.Lock(false);
    CHECK_FALSE(b.IsLocked());
    CHECK(b.Scale(glm::vec3(2.0f, 1.0f, 1.0f)));
}

TEST_CASE("Boxed bounds follow position size and scale", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("WorldPosX=\"10\" WorldPosY=\"20\" ScaleX=\"2\" ScaleY=\"3\""));
    auto& l = m->GetModelScreenLocation();
    CHECK(l.GetRenderWi() == Catch::Approx(4.0f));
    CHECK(l.GetRenderHt() == Catch::Approx(3.0f));
    CHECK(l.GetLeft() == Catch::Approx(6.0f));
    CHECK(l.GetRight() == Catch::Approx(14.0f));
    CHECK(l.GetTop() == Catch::Approx(24.5f));
    CHECK(l.GetBottom() == Catch::Approx(15.5f));
    CHECK(l.GetMWidth() == Catch::Approx(8.0f));
    CHECK(l.GetMHeight() == Catch::Approx(9.0f));
    CHECK(l.GetRight() - l.GetLeft() == Catch::Approx(l.GetMWidth()));
    CHECK(l.GetTop() - l.GetBottom() == Catch::Approx(l.GetMHeight()));
    CHECK(l.GetHcenterPos() == Catch::Approx((l.GetLeft() + l.GetRight()) / 2.0f));
    CHECK(l.GetVcenterPos() == Catch::Approx((l.GetTop() + l.GetBottom()) / 2.0f));
}

TEST_CASE("Boxed edge setters move the model so the edge lands where asked", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("WorldPosX=\"10\" WorldPosY=\"20\" ScaleX=\"2\" ScaleY=\"3\""));
    auto& l = m->GetModelScreenLocation();
    float w = l.GetMWidth();
    float h = l.GetMHeight();
    l.SetLeft(100.0f);
    CHECK(l.GetLeft() == Catch::Approx(100.0f));
    CHECK(l.GetRight() == Catch::Approx(100.0f + w));
    l.SetRight(50.0f);
    CHECK(l.GetRight() == Catch::Approx(50.0f));
    CHECK(l.GetLeft() == Catch::Approx(50.0f - w));
    l.SetTop(0.0f);
    CHECK(l.GetTop() == Catch::Approx(0.0f));
    CHECK(l.GetBottom() == Catch::Approx(-h));
    l.SetBottom(30.0f);
    CHECK(l.GetBottom() == Catch::Approx(30.0f));
    CHECK(l.GetTop() == Catch::Approx(30.0f + h));
}

TEST_CASE("Boxed size setters round trip through the restorable size", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml(""));
    auto& l = m->GetModelScreenLocation();
    l.SetMWidth(30.0f);
    CHECK(l.GetRestorableMWidth() == Catch::Approx(30.0f));
    CHECK(Boxed(m).GetScaleX() == Catch::Approx(30.0f / (l.GetRenderWi() - 1.0f)));
    l.SetMHeight(10.0f);
    CHECK(l.GetRestorableMHeight() == Catch::Approx(10.0f));
    l.SetMHeight(0.0f);
    CHECK(Boxed(m).GetScaleY() == Catch::Approx(1.0f));
}

TEST_CASE("Boxed position setters and offsets", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml(""));
    auto& l = m->GetModelScreenLocation();
    l.SetPosition(4.0f, 5.0f);
    CHECK(l.GetHcenterPos() == Catch::Approx(4.0f));
    CHECK(l.GetVcenterPos() == Catch::Approx(5.0f));
    l.SetDcenterPos(6.0f);
    CHECK(l.GetWorldPos_Z() == Catch::Approx(6.0f));
    l.AddOffset(1.0f, -2.0f, 3.0f);
    CHECK(l.GetWorldPos_X() == Catch::Approx(5.0f));
    CHECK(l.GetWorldPos_Y() == Catch::Approx(3.0f));
    CHECK(l.GetWorldPos_Z() == Catch::Approx(9.0f));
    auto c = l.GetCenterPosition();
    CHECK(c.x == Catch::Approx(5.0f));
    CHECK(c.y == Catch::Approx(3.0f));
    CHECK(c.z == Catch::Approx(9.0f));
}

TEST_CASE("Boxed rotate about a point swings the model around it", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("WorldPosX=\"10\" WorldPosY=\"0\""));
    auto& l = m->GetModelScreenLocation();
    l.RotateAboutPoint(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 90.0f));
    CHECK(std::fabs(l.GetHcenterPos()) < 1e-3);
    CHECK(std::fabs(l.GetVcenterPos()) == Catch::Approx(10.0f).margin(1e-3));
}

TEST_CASE("Boxed rotate about a point through its own centre only turns it", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("WorldPosX=\"10\" WorldPosY=\"20\""));
    auto& l = m->GetModelScreenLocation();
    l.RotateAboutPoint(glm::vec3(10.0f, 20.0f, 0.0f), glm::vec3(0.0f, 0.0f, 90.0f));
    CHECK(l.GetHcenterPos() == Catch::Approx(10.0f).margin(1e-3));
    CHECK(l.GetVcenterPos() == Catch::Approx(20.0f).margin(1e-3));
    CheckPt(Map(l, 1, 0, 0), 10, 21, 0);
}

TEST_CASE("Boxed perspective rotates a 2D point about the X axis", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml(""));
    auto& b = Boxed(m);
    b.SetPerspective2D(0.5f);
    b.PrepareToDraw(false, false);
    CheckPt(Map(b, 0, 1, 0), 0, std::cos(0.5f), std::sin(0.5f));
    b.PrepareToDraw(true, false);
    CheckPt(Map(b, 0, 1, 0), 0, 1, 0);
}

TEST_CASE("Boxed location repairs out of range values on load", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("ScaleX=\"-3\" ScaleY=\"-1\" ScaleZ=\"-2\" RotateX=\"270\" RotateY=\"-200\" RotateZ=\"181\""));
    auto& b = Boxed(m);
    CHECK(b.GetScaleX() > 0.0f);
    CHECK(b.GetScaleY() > 0.0f);
    CHECK(b.GetScaleZ() > 0.0f);
    CHECK(b.GetRotateX() == Catch::Approx(0.0f));
    CHECK(b.GetRotateY() == Catch::Approx(0.0f));
    CHECK(b.GetRotateZ() == Catch::Approx(0.0f));
}

TEST_CASE("Boxed location keeps legal rotations on load", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("RotateX=\"-180\" RotateY=\"179\" RotateZ=\"45.5\""));
    auto& b = Boxed(m);
    CHECK(b.GetRotateX() == Catch::Approx(-180.0f));
    CHECK(b.GetRotateY() == Catch::Approx(179.0f));
    CHECK(b.GetRotateZ() == Catch::Approx(45.5f));
}

TEST_CASE("Boxed location repairs non finite positions", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("WorldPosX=\"nan\" WorldPosY=\"inf\" WorldPosZ=\"-inf\""));
    auto& l = m->GetModelScreenLocation();
    CHECK(l.GetWorldPos_X() == 0.0f);
    CHECK(l.GetWorldPos_Y() == 0.0f);
    CHECK(l.GetWorldPos_Z() == 0.0f);
}

TEST_CASE("Boxed Z extent is centred on the model depth", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("WorldPosZ=\"10\" ScaleZ=\"2\""));
    auto& l = m->GetModelScreenLocation();
    CHECK(l.GetFront() > l.GetBack());
    CHECK((l.GetFront() + l.GetBack()) / 2.0f == Catch::Approx(10.0f).margin(1e-2));
    float depth = l.GetFront() - l.GetBack();
    l.SetFront(30.0f);
    CHECK(l.GetFront() == Catch::Approx(30.0f));
    CHECK(l.GetFront() - l.GetBack() == Catch::Approx(depth));
    l.SetBack(0.0f);
    CHECK(l.GetBack() == Catch::Approx(0.0f));
}

TEST_CASE("Two point location maps the model ends onto the end points", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"1\" WorldPosY=\"2\" WorldPosZ=\"3\" X2=\"3\" Y2=\"4\" Z2=\"0\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    float wi = l.GetRenderWi();
    CHECK(wi == Catch::Approx(10.0f));
    CheckPt(Map(l, 0, 0, 0), 1, 2, 3);
    CheckPt(Map(l, wi, 0, 0), 4, 6, 3);
    CheckPt(Map(l, wi / 2.0f, 0, 0), 2.5f, 4, 3);
}

TEST_CASE("Two point location scales by length over render width", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("X2=\"3\" Y2=\"4\" Z2=\"0\""));
    auto& t = TwoPt(m);
    t.PrepareToDraw(true, false);
    float scale = 5.0f / t.GetRenderWi();
    CheckPt(Map(t, 1, 0, 0), 0.6f * scale, 0.8f * scale, 0);
    CheckPt(Map(t, 0, 1, 0), -0.8f * scale, 0.6f * scale, 0);
    CheckPt(Map(t, 0, 0, 1), 0, 0, scale);
}

TEST_CASE("Two point location works in all three dimensions", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"0\" WorldPosY=\"0\" WorldPosZ=\"0\" X2=\"0\" Y2=\"0\" Z2=\"-8\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    CheckPt(Map(l, l.GetRenderWi(), 0, 0), 0, 0, -8);
    CHECK(TwoPt(m).GetPoint2().z == Catch::Approx(-8.0f));
    CHECK(TwoPt(m).GetPoint1().x == Catch::Approx(0.0f));
}

TEST_CASE("Two point location with identical end points stays finite", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"5\" WorldPosY=\"5\" WorldPosZ=\"5\" X2=\"0\" Y2=\"0\" Z2=\"0\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, true);
    P3 p = Map(l, l.GetRenderWi(), 0, 0);
    CHECK(std::fabs(p.x - 5.0f) < 0.01f);
    CHECK(std::fabs(p.y - 5.0f) < 0.01f);
    CHECK(std::fabs(p.z - 5.0f) < 0.01f);
}

TEST_CASE("Two point bounds come from the end points", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"1\" WorldPosY=\"2\" WorldPosZ=\"3\" X2=\"3\" Y2=\"4\" Z2=\"0\""));
    auto& l = m->GetModelScreenLocation();
    CHECK(l.GetLeft() == Catch::Approx(1.0f));
    CHECK(l.GetRight() == Catch::Approx(4.0f));
    CHECK(l.GetBottom() == Catch::Approx(2.0f));
    CHECK(l.GetTop() == Catch::Approx(6.0f));
    CHECK(l.GetMWidth() == Catch::Approx(3.0f));
    CHECK(l.GetMHeight() == Catch::Approx(4.0f));
    CHECK(l.GetHcenterPos() == Catch::Approx(2.5f));
    CHECK(l.GetVcenterPos() == Catch::Approx(4.0f));
    CHECK_FALSE(l.IsCenterBased());
}

TEST_CASE("Two point bounds handle end points to the left and below", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"10\" WorldPosY=\"10\" X2=\"-4\" Y2=\"-6\" Z2=\"0\""));
    auto& l = m->GetModelScreenLocation();
    CHECK(l.GetLeft() == Catch::Approx(6.0f));
    CHECK(l.GetRight() == Catch::Approx(10.0f));
    CHECK(l.GetBottom() == Catch::Approx(4.0f));
    CHECK(l.GetTop() == Catch::Approx(10.0f));
}

TEST_CASE("Two point swap exchanges the end points", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"1\" WorldPosY=\"2\" WorldPosZ=\"3\" X2=\"3\" Y2=\"4\" Z2=\"5\""));
    auto& t = TwoPt(m);
    t.SwapStartEnd();
    CHECK(t.GetWorldPos_X() == Catch::Approx(4.0f));
    CHECK(t.GetWorldPos_Y() == Catch::Approx(6.0f));
    CHECK(t.GetWorldPos_Z() == Catch::Approx(8.0f));
    CHECK(t.GetX2() == Catch::Approx(-3.0f));
    CHECK(t.GetY2() == Catch::Approx(-4.0f));
    CHECK(t.GetZ2() == Catch::Approx(-5.0f));
    t.SwapStartEnd();
    CHECK(t.GetWorldPos_X() == Catch::Approx(1.0f));
    CHECK(t.GetX2() == Catch::Approx(3.0f));
}

TEST_CASE("Two point rotation turns the line about its midpoint", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"0\" WorldPosY=\"0\" WorldPosZ=\"0\" X2=\"10\" Y2=\"0\" Z2=\"0\""));
    auto& t = TwoPt(m);
    CHECK(t.Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, 90.0f));
    CHECK(t.GetWorldPos_X() == Catch::Approx(5.0f).margin(1e-3));
    CHECK(t.GetWorldPos_Y() == Catch::Approx(-5.0f).margin(1e-3));
    CHECK(t.GetX2() == Catch::Approx(0.0f).margin(1e-3));
    CHECK(t.GetY2() == Catch::Approx(10.0f).margin(1e-3));
    CHECK(t.GetHcenterPos() == Catch::Approx(5.0f).margin(1e-3));
    CHECK(t.GetVcenterPos() == Catch::Approx(0.0f).margin(1e-3));
}

TEST_CASE("Two point scale grows the line about its midpoint", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"0\" WorldPosY=\"0\" WorldPosZ=\"0\" X2=\"10\" Y2=\"0\" Z2=\"0\""));
    auto& t = TwoPt(m);
    t.PrepareToDraw(true, true);
    CHECK(t.Scale(glm::vec3(2.0f, 2.0f, 2.0f)));
    CHECK(t.GetWorldPos_X() == Catch::Approx(-5.0f).margin(1e-3));
    CHECK(t.GetX2() == Catch::Approx(20.0f).margin(1e-3));
    CHECK(t.GetHcenterPos() == Catch::Approx(5.0f).margin(1e-3));
}

TEST_CASE("Locked two point location ignores changes", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("X2=\"10\" Y2=\"0\" Z2=\"0\""));
    auto& t = TwoPt(m);
    t.Lock();
    CHECK_FALSE(t.Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, 90.0f));
    CHECK_FALSE(t.Scale(glm::vec3(2.0f, 2.0f, 2.0f)));
    t.SwapStartEnd();
    t.AddOffset(5.0f, 5.0f, 5.0f);
    CHECK(t.GetX2() == Catch::Approx(10.0f));
    CHECK(t.GetWorldPos_X() == Catch::Approx(0.0f));
}

TEST_CASE("Two point flip direction follows the dominant axis", "[models2][screenloc]") {
    using FD = TwoPointScreenLocation::FlipDirection;
    ShowFixture f;
    auto dir = [&](const std::string& xyz) {
        Model* m = f.Add(LineXml(xyz));
        return TwoPt(m).GetFlipDirection();
    };
    CHECK(dir("X2=\"100\" Y2=\"1\" Z2=\"0\"") == FD::None);
    CHECK(dir("X2=\"-100\" Y2=\"1\" Z2=\"0\"") == FD::LeftRight);
    CHECK(dir("X2=\"1\" Y2=\"-100\" Z2=\"0\"") == FD::TopBottom);
    CHECK(dir("X2=\"1\" Y2=\"100\" Z2=\"0\"") == FD::None);
    CHECK(dir("X2=\"-10\" Y2=\"0\" Z2=\"0\"") == FD::None);
    CHECK(dir("X2=\"-1\" Y2=\"-2\" Z2=\"-500\"") == FD::None);
}

TEST_CASE("Two point end point setters move only that end", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"1\" WorldPosY=\"1\" X2=\"4\" Y2=\"4\" Z2=\"0\""));
    auto& t = TwoPt(m);
    t.SetX2(10.0f);
    t.SetY2(-2.0f);
    t.SetZ2(3.0f);
    t.PrepareToDraw(true, false);
    CHECK(t.GetPoint1().x == Catch::Approx(1.0f));
    CHECK(t.GetPoint2().x == Catch::Approx(11.0f));
    CHECK(t.GetPoint2().y == Catch::Approx(-1.0f));
    CHECK(t.GetPoint2().z == Catch::Approx(3.0f));
}

TEST_CASE("Three point location maps the model ends onto the end points", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(ArchXml("WorldPosX=\"1\" WorldPosY=\"2\" WorldPosZ=\"3\" X2=\"6\" Y2=\"0\" Z2=\"0\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    float wi = l.GetRenderWi();
    CheckPt(Map(l, 0, 0, 0), 1, 2, 3);
    CheckPt(Map(l, wi, 0, 0), 7, 2, 3);
}

TEST_CASE("Three point location with a negative X2 still lands on both end points", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(ArchXml("WorldPosX=\"1\" WorldPosY=\"2\" WorldPosZ=\"3\" X2=\"-6\" Y2=\"0\" Z2=\"0\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    float wi = l.GetRenderWi();
    CheckPt(Map(l, 0, 0, 0), 1, 2, 3);
    CheckPt(Map(l, wi, 0, 0), -5, 2, 3);
    CheckPt(Map(l, 0, 1, 0), 1, 2 + 6.0f / wi, 3);
}

TEST_CASE("Three point shear slants points above the baseline", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(ArchXml("WorldPosX=\"0\" WorldPosY=\"0\" WorldPosZ=\"0\" X2=\"10\" Y2=\"0\" Z2=\"0\""));
    auto& t = ThreePt(m);
    t.SetSupportsShear(true);
    t.SetYShear(0.0f);
    t.PrepareToDraw(true, false);
    P3 flat = Map(t, 0, 1, 0);
    t.SetYShear(0.5f);
    t.PrepareToDraw(true, false);
    P3 sheared = Map(t, 0, 1, 0);
    P3 base = Map(t, 3, 0, 0);
    CHECK(sheared.y == Catch::Approx(flat.y).margin(1e-3));
    float scale = 10.0f / t.GetRenderWi();
    CHECK(std::fabs(sheared.x - flat.x) == Catch::Approx(0.5f * scale).margin(1e-3));
    P3 baseFlat = P3{ 3.0f * scale, 0, 0 };
    CHECK(base.x == Catch::Approx(baseFlat.x).margin(1e-3));
    CHECK(base.y == Catch::Approx(0.0f).margin(1e-3));
}

TEST_CASE("Three point shear is ignored unless supported", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(ArchXml("X2=\"10\" Y2=\"0\" Z2=\"0\""));
    auto& t = ThreePt(m);
    t.SetSupportsShear(false);
    t.SetYShear(0.7f);
    CHECK(t.GetYShear() == Catch::Approx(0.0f));
    t.SetSupportsShear(true);
    CHECK(t.GetYShear() == Catch::Approx(0.7f));
    CHECK(t.GetShear() == Catch::Approx(0.7f));
}

TEST_CASE("Three point height scales the model height", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(ArchXml("X2=\"10\" Y2=\"0\" Z2=\"0\""));
    auto& t = ThreePt(m);
    t.SetHeight(3.0f);
    CHECK(t.GetHeight() == Catch::Approx(3.0f));
    CHECK(t.GetMHeight() == Catch::Approx(3.0f));
    t.SetMHeight(1.5f);
    CHECK(t.GetHeight() == Catch::Approx(1.5f));
}

TEST_CASE("Poly line location spans its points", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add("<model name=\"P\" DisplayAs=\"Poly Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"6\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\" WorldPosX=\"1\" WorldPosY=\"2\" WorldPosZ=\"3\"/>");
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, false);
    CHECK(l.GetNumHandles() == 3);
    CHECK(l.GetLeft() == Catch::Approx(1.0f));
    CHECK(l.GetRight() == Catch::Approx(11.0f));
    CHECK(l.GetBottom() == Catch::Approx(2.0f));
    CHECK(l.GetTop() == Catch::Approx(7.0f));
}

TEST_CASE("Poly line nodes are spread along the polyline", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add("<model name=\"P\" DisplayAs=\"Poly Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"5\" NumPoints=\"3\" PointData=\"0.0,0.0,0.0,5.0,5.0,0.0,10.0,0.0,0.0\"/>");
    REQUIRE(m->GetNodeCount() == 5);
    float lastX = -1.0f;
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        float x = m->GetNode(i)->Coords[0].screenX;
        CHECK(x > lastX);
        lastX = x;
        CHECK(x >= 0.0f);
        CHECK(x <= 1.0f);
    }
    CHECK(m->GetNode(0)->Coords[0].screenX == Catch::Approx(0.0f).margin(1e-3));
    CHECK(m->GetNode(0)->Coords[0].screenY == Catch::Approx(0.0f).margin(1e-3));
    CHECK(m->GetNode(2)->Coords[0].screenY == Catch::Approx(0.8f).margin(1e-3));
    CHECK(m->GetNode(1)->Coords[0].screenY - m->GetNode(0)->Coords[0].screenY == Catch::Approx(2.0f * (m->GetNode(1)->Coords[0].screenX - m->GetNode(0)->Coords[0].screenX)).margin(1e-3));
}

TEST_CASE("Poly line with identical points does not produce invalid node positions", "[models2][screenloc][!mayfail]") {
    ShowFixture f;
    Model* m = f.Add("<model name=\"P\" DisplayAs=\"Poly Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"4\" NumPoints=\"2\" PointData=\"0.0,0.0,0.0,0.0,0.0,0.0\"/>");
    REQUIRE(m->GetNodeCount() == 4);
    for (uint32_t i = 0; i < m->GetNodeCount(); ++i) {
        const auto& c = m->GetNode(i)->Coords[0];
        CHECK(xl::isfinite(c.screenX));
        CHECK(xl::isfinite(c.screenY));
    }
}

TEST_CASE("Screen location values survive repeated PrepareToDraw calls", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(LineXml("WorldPosX=\"1\" WorldPosY=\"2\" WorldPosZ=\"3\" X2=\"3\" Y2=\"4\" Z2=\"0\""));
    auto& l = m->GetModelScreenLocation();
    l.PrepareToDraw(true, true);
    P3 a = Map(l, 5, 0, 0);
    l.PrepareToDraw(false, true);
    l.PrepareToDraw(true, false);
    P3 b = Map(l, 5, 0, 0);
    CHECK(a.x == Catch::Approx(b.x));
    CHECK(a.y == Catch::Approx(b.y));
    CHECK(a.z == Catch::Approx(b.z));
}

TEST_CASE("Render size setters feed the bounds", "[models2][screenloc]") {
    ShowFixture f;
    Model* m = f.Add(MatrixXml("ScaleX=\"1\" ScaleY=\"1\""));
    auto& l = m->GetModelScreenLocation();
    l.SetRenderSize(20.0f, 10.0f, 2.0f);
    CHECK(l.GetRenderWi() == Catch::Approx(20.0f));
    CHECK(l.GetRenderHt() == Catch::Approx(10.0f));
    CHECK(l.GetRenderDp() == Catch::Approx(2.0f));
    CHECK(l.GetMWidth() == Catch::Approx(20.0f));
    CHECK(l.GetMHeight() == Catch::Approx(10.0f));
    CHECK(l.GetRight() - l.GetLeft() == Catch::Approx(20.0f));
}
