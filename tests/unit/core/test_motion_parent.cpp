// Motion Parent: a model that follows a moving part of a servo model in the
// preview. The expected positions come from the servo models' own drawing
// (the matrices their meshes are drawn with, recorded by the fake context)
// and from BoxedScreenLocation::TranslatePoint, so the transform is checked
// against what the preview actually shows rather than against itself.

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "models/DMX/DmxGeneral.h"
#include "models/DMX/DmxImage.h"
#include "models/DMX/DmxServo.h"
#include "models/DMX/DmxServo3D.h"
#include "models/DMX/DmxSkull.h"
#include "models/DMX/Servo.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/ModelScreenLocation.h"
#include "XmlSerializer/XmlSerializer.h"

#include "support/TempDir.h"
#include "test_dmx2_support.h"

using Catch::Approx;
using namespace dmx2;

namespace {

struct CubeFiles {
    TempDir dir;
    std::string obj;
    CubeFiles() {
        dir.Write("cube.mtl", kCubeMtl);
        obj = dir.Write("cube.obj", kCubeObj);
    }
    std::string Mesh(const std::string& name, const std::string& extra = "") const {
        return "<" + name + " ObjFile=\"" + obj + "\" " + extra + "/>";
    }
};

glm::vec3 Apply(const glm::mat4& m, glm::vec3 p) {
    return glm::vec3(m * glm::vec4(p, 1.0f));
}

// A model-local point of `m` in world coordinates, the way the 2D preview
// places it.
glm::vec3 World(Model* m, glm::vec3 p) {
    const ModelScreenLocation& loc = m->GetModelScreenLocation();
    loc.PrepareToDraw(false, false);
    loc.TranslatePoint(p.x, p.y, p.z);
    return p;
}

void CheckNear(glm::vec3 got, glm::vec3 want) {
    INFO("got (" << got.x << ", " << got.y << ", " << got.z << ") want (" << want.x << ", " << want.y << ", " << want.z << ")");
    CHECK(got.x == Approx(want.x).margin(1e-3));
    CHECK(got.y == Approx(want.y).margin(1e-3));
    CHECK(got.z == Approx(want.z).margin(1e-3));
}

// For a local motion `delta` of the parent (model-local), the motion transform
// must move the world image of each local point v to the world image of delta*v.
void CheckFollows(Model* parent, const glm::mat4& delta, const glm::mat4& motion) {
    for (glm::vec3 v : { glm::vec3(0, 0, 0), glm::vec3(0.4f, -0.2f, 0), glm::vec3(-0.3f, 0.35f, 0.1f) }) {
        INFO("local point (" << v.x << ", " << v.y << ", " << v.z << ")");
        CheckNear(Apply(motion, World(parent, v)), World(parent, Apply(delta, v)));
    }
}

const std::string kPlace = "WorldPosX=\"100\" WorldPosY=\"50\" WorldPosZ=\"0\" ScaleX=\"40\" ScaleY=\"30\" ScaleZ=\"20\" RotateZ=\"30\"";

// One 8 bit Rotate Z servo on channel 1 with 90 degrees of travel, pivoting
// 0.1 units right of the image centre (2D pivot offsets are stored x100).
DmxServo* AddArm(Show& s) {
    return As<DmxServo>(s.Add(Dmx("DmxServo", "NumServos=\"1\" DmxChannelCount=\"1\" Bits16=\"0\" " + kPlace,
        "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"90\" ServoStyle=\"Rotate Z\" PivotOffsetX=\"10\" PivotOffsetY=\"0\"/>")));
}

Model* AddChild(Show& s, Model* parent, const std::string& part) {
    return s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"3\" MotionParent=\"" + parent->GetName() + "\" MotionParentPart=\"" + part + "\""));
}

} // namespace

TEST_CASE("Motion parent follows a 2D servo rotating about its pivot", "[dmx2][motion]") {
    Show s;
    auto* arm = AddArm(s);
    Model* child = AddChild(s, arm, "Servo 1");
    REQUIRE(child->GetMotionParent() == arm->GetName());
    REQUIRE(arm->GetMotionParts() == std::vector<std::string>{ "Servo 1" });

    glm::mat4 motion;
    // the minimum limit is the rest pose the layout shows
    Chan(arm, 1, 0);
    REQUIRE(child->GetMotionTransform(false, motion));
    CheckFollows(arm, glm::mat4(1.0f), motion);

    // end of travel is -90 degrees: the pivot stays put and a point half a
    // unit right of it swings to half a unit below it
    Chan(arm, 1, 255);
    REQUIRE(child->GetMotionTransform(false, motion));
    CheckNear(Apply(motion, World(arm, { 0.1f, 0, 0 })), World(arm, { 0.1f, 0, 0 }));
    CheckNear(Apply(motion, World(arm, { 0.6f, 0, 0 })), World(arm, { 0.1f, -0.5f, 0 }));
}

TEST_CASE("Motion parent follows the image's own offset and rotation", "[dmx2][motion]") {
    Show s;
    // the motion image is offset, turned and scaled; the servo moves it in
    // the image's own frame, as DmxImage::Draw does
    auto* arm = As<DmxServo>(s.Add(Dmx("DmxServo", "NumServos=\"1\" DmxChannelCount=\"1\" Bits16=\"0\" " + kPlace,
        "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"40\" ServoStyle=\"Translate X\"/>"
        "<MotionImage1 Image=\"\" OffsetX=\"20\" OffsetY=\"-10\" RotateZ=\"90\" ScaleX=\"0.5\" ScaleY=\"0.5\"/>")));
    Model* child = AddChild(s, arm, "Servo 1");
    Chan(arm, 1, 255);
    glm::mat4 motion;
    REQUIRE(child->GetMotionTransform(false, motion));
    glm::mat4 image = arm->GetMotionImage(0)->GetBaseMatrix();
    glm::mat4 servo(1.0f);
    arm->GetServo(0)->FillMotionMatrix(-40.0f, servo);
    CheckFollows(arm, image * servo * glm::inverse(image), motion);
    // the image's 90 degree turn makes a Translate X servo move along local Y
    glm::vec3 d = Apply(image * servo * glm::inverse(image), { 0, 0, 0 });
    CHECK(d.x == Approx(0.0f).margin(1e-4));
    // travel is 40 / ScaleX(40) = 1 unit in the image frame, scaled by 0.5
    CHECK(std::abs(d.y) == Approx(0.5f).margin(1e-4));
}

TEST_CASE("Motion parent matches the drawn motion of a Servo3d mesh chain", "[dmx2][motion]") {
    CubeFiles cube;
    for (bool showPivot : { false, true }) {
        INFO("show pivot " << showPivot);
        Show s;
        // mesh 2 rides on mesh 1; servo 1 rotates about a pivot, servo 2 slides
        auto* body = As<DmxServo3d>(s.Add(Dmx("DmxServo3d", "DmxChannelCount=\"2\" NumServos=\"2\" NumStatic=\"1\" NumMotion=\"2\" Bits16=\"0\" Mesh2Linkage=\"Mesh 1\" " + kPlace,
            "<Servo1 Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"90\" ServoStyle=\"Rotate Z\" PivotOffsetX=\"0.5\" PivotOffsetY=\"0.25\"/>"
            "<Servo2 Channel=\"2\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"10\" ServoStyle=\"Translate X\"/>" +
                cube.Mesh("StaticMesh1") + cube.Mesh("MotionMesh1", "OffsetX=\"0.3\" RotateY=\"20\"") + cube.Mesh("MotionMesh2", "OffsetY=\"0.2\" ScaleX=\"2\""))));
        body->SetShowPivot(showPivot);
        REQUIRE(body->GetMotionParts() == std::vector<std::string>{ "Mesh 1", "Mesh 2" });

        FakePreview rest;
        DrawLayout(body, rest, nullptr, true);
        REQUIRE(rest.ctx.solids.size() == 3);

        Chan(body, 1, 200);
        Chan(body, 2, 100);
        FakePreview moved;
        DrawEffect(body, moved);
        REQUIRE(moved.ctx.solids.size() == 3);

        for (int part = 0; part < 2; ++part) {
            INFO("mesh " << part + 1);
            Model* child = AddChild(s, body, "Mesh " + std::to_string(part + 1));
            glm::mat4 motion;
            REQUIRE(child->GetMotionTransform(false, motion));
            glm::mat4 delta = moved.ctx.solids[1 + part].matrix * glm::inverse(rest.ctx.solids[1 + part].matrix);
            CheckFollows(body, delta, motion);
        }
    }
}

TEST_CASE("Motion parent matches the drawn head and jaw of a skull", "[dmx2][motion]") {
    CubeFiles cube;
    Show s;
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\" Bits16=\"0\" HasTilt=\"0\" HasNod=\"0\" HasEyeUD=\"0\" HasEyeLR=\"0\" HasColor=\"0\" DmxPanOrient=\"0\" DmxJawOrient=\"0\" " + kPlace,
        "<JawServo Channel=\"1\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"20\" ServoStyle=\"Rotate X\"/>"
        "<PanServo Channel=\"3\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"180\" ServoStyle=\"Rotate Y\"/>" +
            cube.Mesh("HeadMesh") + cube.Mesh("JawMesh") + cube.Mesh("EyeMeshL") + cube.Mesh("EyeMeshR"))));
    REQUIRE(sk->GetMotionParts() == std::vector<std::string>{ "Head", "Jaw" });

    FakePreview rest;
    DrawLayout(sk, rest);
    REQUIRE(rest.ctx.solids.size() == 4);
    Chan(sk, 1, 255);
    Chan(sk, 3, 90);
    FakePreview moved;
    DrawEffect(sk, moved);
    REQUIRE(moved.ctx.solids.size() == 4);

    // DmxSkull::DisplayModelOnWindow draws the meshes scaled by 0.7 and
    // shifted inside the model box (z 0.5 in 2D)
    glm::mat4 place = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.7f)), glm::vec3(0.0f, -0.7f, 0.5f));
    const char* parts[] = { "Head", "Jaw" };
    for (int part = 0; part < 2; ++part) {
        INFO(parts[part]);
        Model* child = AddChild(s, sk, parts[part]);
        glm::mat4 motion;
        REQUIRE(child->GetMotionTransform(false, motion));
        glm::mat4 delta = moved.ctx.solids[part].matrix * glm::inverse(rest.ctx.solids[part].matrix);
        CheckFollows(sk, place * delta * glm::inverse(place), motion);
    }
}

TEST_CASE("Motion parent chains through a parent that follows another", "[dmx2][motion]") {
    CubeFiles cube;
    Show s;
    auto* arm = AddArm(s);
    // a skull riding on the arm, and eyes riding on the skull's head
    auto* sk = As<DmxSkull>(s.Add(Dmx("DmxSkull", "DmxChannelCount=\"26\" Bits16=\"0\" HasJaw=\"0\" HasTilt=\"0\" HasNod=\"0\" HasEyeUD=\"0\" HasEyeLR=\"0\" HasColor=\"0\" DmxPanOrient=\"0\" "
                                                  "MotionParent=\"" + arm->GetName() + "\" MotionParentPart=\"Servo 1\" " + kPlace,
        "<PanServo Channel=\"3\" MinLimit=\"0\" MaxLimit=\"255\" RangeOfMotion=\"180\" ServoStyle=\"Rotate Y\"/>" +
            cube.Mesh("HeadMesh") + cube.Mesh("JawMesh") + cube.Mesh("EyeMeshL") + cube.Mesh("EyeMeshR"))));
    Model* eyes = AddChild(s, sk, "Head");
    Chan(arm, 1, 180);
    Chan(sk, 3, 60);

    glm::mat4 armMotion, headMotion, motion;
    REQUIRE(arm->GetPartMotion("Servo 1", false, armMotion));
    REQUIRE(sk->GetPartMotion("Head", false, headMotion));
    REQUIRE(eyes->GetMotionTransform(false, motion));
    // the head turns in the skull's rest frame, then the arm carries both
    glm::mat4 want = armMotion * headMotion;
    for (glm::vec3 p : { glm::vec3(100, 50, 0), glm::vec3(120, 40, 5) }) {
        CheckNear(Apply(motion, p), Apply(want, p));
    }
    // with the head still, the eyes still follow the arm
    Chan(sk, 3, 0);
    REQUIRE(eyes->GetMotionTransform(false, motion));
    CheckNear(Apply(motion, { 110, 60, 0 }), Apply(armMotion, { 110, 60, 0 }));
}

TEST_CASE("Motion parent ignores missing parents, unknown parts and cycles", "[dmx2][motion]") {
    Show s;
    auto* arm = AddArm(s);
    Chan(arm, 1, 255);
    glm::mat4 motion;

    Model* orphan = s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"3\" MotionParent=\"NoSuchModel\" MotionParentPart=\"Servo 1\""));
    CHECK_FALSE(orphan->GetMotionTransform(false, motion));

    Model* badPart = AddChild(s, arm, "Servo 9");
    CHECK_FALSE(badPart->GetMotionTransform(false, motion));

    Model* none = s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"3\""));
    CHECK_FALSE(none->GetMotionTransform(false, motion));

    // a model can't follow itself
    none->SetMotionParent(none->GetName(), "Servo 1");
    CHECK(none->GetMotionParent().empty());

    // two servos following each other: the depth cap ends the walk
    auto* arm2 = AddArm(s);
    arm->SetMotionParent(arm2->GetName(), "Servo 1");
    arm2->SetMotionParent(arm->GetName(), "Servo 1");
    Chan(arm2, 1, 255);
    CHECK(arm->GetMotionTransform(false, motion));
}

TEST_CASE("Motion parent is applied when the preview plays, not in the layout", "[dmx2][motion]") {
    Show s;
    auto* arm = AddArm(s);
    Model* line = s.Add("<model name=\"Outline\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" NodesPerString=\"10\" "
                        "WorldPosX=\"90\" WorldPosY=\"40\" WorldPosZ=\"0\" X2=\"30\" Y2=\"5\" Z2=\"0\" "
                        "MotionParent=\"" + arm->GetName() + "\" MotionParentPart=\"Servo 1\"/>");
    Model* dmx = AddChild(s, arm, "Servo 1");
    Chan(arm, 1, 255);
    glm::mat4 motion;
    REQUIRE(line->GetMotionTransform(false, motion));
    REQUIRE(glm::length(Apply(motion, { 120, 45, 0 }) - glm::vec3(120, 45, 0)) > 1.0f);

    // a two point model: its placement matrix is pre-multiplied by the motion
    // the base model draw step reads the preview from the context
    FakePreview layout;
    layout.ctx.setContextualValue("modelPreview", &layout);
    DrawLayout(line, layout);
    REQUIRE_FALSE(layout.ctx.modelMatrices.empty());
    glm::mat4 placed = layout.ctx.modelMatrices.back();

    FakePreview house;
    house.ctx.setContextualValue("modelPreview", &house);
    line->DisplayModelOnWindow(&house, &house.ctx, &house.solid, &house.transparent, false, nullptr, false);
    house.Run();
    REQUIRE_FALSE(house.ctx.modelMatrices.empty());
    glm::mat4 got = house.ctx.modelMatrices.back();
    glm::mat4 want = motion * placed;
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            CHECK(got[c][r] == Approx(want[c][r]).margin(1e-3));
        }
    }

    // a boxed model: the motion is applied ahead of its own placement
    auto contains = [&](const std::vector<glm::mat4>& v) {
        for (const auto& m : v) {
            if (glm::length(Apply(m, { 120, 45, 0 }) - Apply(motion, { 120, 45, 0 })) < 1e-3f) {
                return true;
            }
        }
        return false;
    };
    FakePreview dmxLayout;
    DrawLayout(dmx, dmxLayout);
    CHECK_FALSE(contains(dmxLayout.ctx.applied));
    FakePreview dmxHouse;
    dmx->DisplayModelOnWindow(&dmxHouse, &dmxHouse.ctx, &dmxHouse.solid, &dmxHouse.transparent, false, nullptr, false);
    dmxHouse.Run();
    CHECK(contains(dmxHouse.ctx.applied));
}

TEST_CASE("Motion parent follows renames and is cleared when the parent is deleted", "[dmx2][motion]") {
    Show s;
    auto* arm = AddArm(s);
    Model* child = AddChild(s, arm, "Servo 1");
    const std::string childName = child->GetName();
    REQUIRE(s.mm.Rename(arm->GetName(), "Santa Arm"));
    child = s.mm.GetModel(childName);
    REQUIRE(child != nullptr);
    CHECK(child->GetMotionParent() == "Santa Arm");
    CHECK(child->GetMotionParentPart() == "Servo 1");

    REQUIRE(s.mm.Delete("Santa Arm", false));
    child = s.mm.GetModel(childName);
    REQUIRE(child != nullptr);
    CHECK(child->GetMotionParent().empty());
    CHECK(child->GetMotionParentPart().empty());
}

TEST_CASE("Models imported together under new names keep their motion parent links", "[dmx2][motion]") {
    Show s;
    auto* arm = AddArm(s);
    Model* child = AddChild(s, arm, "Servo 1");
    Model* other = s.Add(Dmx("DmxGeneral", "DmxChannelCount=\"3\""));
    Model* outsider = AddChild(s, other, "Servo 1");
    const std::string armName = arm->GetName();
    const std::string childName = child->GetName();
    const std::string otherName = other->GetName();

    // A multi-model .xmodel of the arm, its outline and a model whose parent
    // is not in the file, imported into the show that already has all three:
    // every import comes in under a generated name.
    XmlSerializer serializer;
    pugi::xml_document doc = serializer.SerializeModels({ arm, child, outsider }, false, true);
    pugi::xml_node root = doc.document_element();
    REQUIRE(std::string(root.name()) == "models");
    std::map<std::string, std::string> renames;
    for (pugi::xml_node n = root.first_child(); n; n = n.next_sibling()) {
        Model* m = serializer.DeserializeModel(n, s.mm, true);
        REQUIRE(m != nullptr);
        s.mm.AddModel(m);
        renames[n.attribute("name").as_string()] = m->GetName();
    }
    REQUIRE(renames.size() == 3);
    REQUIRE(renames[armName] != armName);
    REQUIRE(renames[childName] != childName);

    s.mm.RemapMotionParents(renames);

    Model* newChild = s.mm.GetModel(renames[childName]);
    REQUIRE(newChild != nullptr);
    CHECK(newChild->GetMotionParent() == renames[armName]);
    CHECK(newChild->GetMotionParentPart() == "Servo 1");

    Model* newOutsider = s.mm.GetModel(renames[outsider->GetName()]);
    REQUIRE(newOutsider != nullptr);
    CHECK(newOutsider->GetMotionParent() == otherName);

    CHECK(s.mm.GetModel(childName)->GetMotionParent() == armName);
}
