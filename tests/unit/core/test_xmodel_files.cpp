#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include "models/DMX/DmxImage.h"
#include "models/DMX/DmxServo.h"
#include "models/DMX/DmxServo3D.h"
#include "models/DMX/Mesh.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "XmlSerializer/StringSerializingVisitor.h"
#include "XmlSerializer/XModelFiles.h"
#include "XmlSerializer/XmlSerializer.h"

#include "support/TempDir.h"
#include "test_dmx2_support.h"

using namespace dmx2;
namespace fs = std::filesystem;

namespace {

// Binary content, so a text-mode read or write anywhere would show.
std::string Bytes(const std::string& tag) {
    return std::string("\x89PNG\r\n\x1a\n\0\xff", 10) + tag;
}

std::string Put(const TempDir& dir, const std::string& rel, const std::string& body) {
    const fs::path p = dir.path / rel;
    fs::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary) << body;
    return p.string();
}

std::string Slurp(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

int CountEmbedded(pugi::xml_node n) {
    int c = 0;
    for (pugi::xml_node k : n.children()) {
        c += std::string_view(k.name()) == XModelFiles::NodeName ? 1 : CountEmbedded(k);
    }
    return c;
}

pugi::xml_node ModelNode(pugi::xml_document& doc, const std::string& name) {
    return doc.document_element().find_child_by_attribute("model", "name", name.c_str());
}

// A servo with two images, a Servo 3D whose two meshes share one .obj (with
// its material and texture), and a matrix with an image face.
struct Prop {
    TempDir src;
    Show s;
    std::string body, arm, obj, mtl, tex, face;
    DmxServo* servo = nullptr;
    DmxServo3d* servo3d = nullptr;
    Model* matrix = nullptr;

    Prop() {
        body = Put(src, "art/body.png", Bytes("body"));
        arm = Put(src, "art/arm.png", Bytes("arm"));
        obj = Put(src, "mesh/arm.obj", "mtllib arm.mtl\nv 0 0 0\n");
        mtl = Put(src, "mesh/arm.mtl", "newmtl skin\nmap_Kd tex/skin.png\n");
        tex = Put(src, "mesh/tex/skin.png", Bytes("skin"));
        face = Put(src, "faces/open.png", Bytes("face"));
        servo = As<DmxServo>(s.Add("<model name=\"Snow Man\" DisplayAs=\"DmxServo\" NumServos=\"1\" DmxChannelCount=\"1\" Bits16=\"0\" StartChannel=\"1\" StringType=\"Single Color White\">"
                                   "<Servo1 Channel=\"1\"/><StaticImage1 Image=\"" + body + "\"/><MotionImage1 Image=\"" + arm + "\"/></model>"));
        servo3d = As<DmxServo3d>(s.Add("<model name=\"Arm 3D\" DisplayAs=\"DmxServo3d\" NumServos=\"1\" NumStatic=\"1\" NumMotion=\"1\" DmxChannelCount=\"1\" Bits16=\"0\" StartChannel=\"1\" StringType=\"Single Color White\">"
                                       "<Servo1 Channel=\"1\"/><StaticMesh1 ObjFile=\"" + obj + "\"/><MotionMesh1 ObjFile=\"" + obj + "\"/></model>"));
        matrix = s.Add("<model name=\"Face\" DisplayAs=\"Horiz Matrix\" NumStrings=\"1\" NodesPerString=\"16\" StrandsPerString=\"4\" StringType=\"RGB Nodes\" StartChannel=\"1\">"
                       "<faceInfo Name=\"Talk\" Type=\"Matrix\" Mouth-AI-EyesOpen=\"" + face + "\" Mouth-E-EyesOpen=\"" + face + "\"/></model>");
    }

    pugi::xml_document Export(bool embed) const {
        XmlSerializer serializer;
        return serializer.SerializeModels({ servo, servo3d, matrix }, false, true, embed);
    }
};

std::string ToText(const pugi::xml_document& doc) {
    std::ostringstream out;
    doc.save(out);
    return out.str();
}

} // namespace

TEST_CASE("An exported model carries the files it refers to", "[xmodel]") {
    Prop p;
    pugi::xml_document doc = p.Export(true);
    pugi::xml_node servo = ModelNode(doc, "Snow Man");
    pugi::xml_node servo3d = ModelNode(doc, "Arm 3D");
    pugi::xml_node face = ModelNode(doc, "Face").child("faceInfo");

    // references name the files' place in the package, under the first model's name
    CHECK(std::string(servo.child("StaticImage1").attribute("Image").value()) == "modeldownload/Snow Man/body.png");
    CHECK(std::string(servo.child("MotionImage1").attribute("Image").value()) == "modeldownload/Snow Man/arm.png");
    CHECK(std::string(servo3d.child("StaticMesh1").attribute("ObjFile").value()) == "modeldownload/Snow Man/arm/arm.obj");
    CHECK(std::string(servo3d.child("MotionMesh1").attribute("ObjFile").value()) == "modeldownload/Snow Man/arm/arm.obj");
    CHECK(std::string(face.attribute("Mouth-AI-EyesOpen").value()) == "modeldownload/Snow Man/open.png");
    CHECK(std::string(face.attribute("Mouth-E-EyesOpen").value()) == "modeldownload/Snow Man/open.png");

    // each file is carried once, by the first model that refers to it; the
    // mesh brings its material and texture with it
    CHECK(CountEmbedded(servo) == 2);
    CHECK(CountEmbedded(servo3d) == 3);
    CHECK(CountEmbedded(ModelNode(doc, "Face")) == 1);
    CHECK(servo3d.find_child_by_attribute(XModelFiles::NodeName, XModelFiles::PathAttribute, "modeldownload/Snow Man/arm/arm.mtl"));
    CHECK(servo3d.find_child_by_attribute(XModelFiles::NodeName, XModelFiles::PathAttribute, "modeldownload/Snow Man/arm/tex/skin.png"));
}

TEST_CASE("Importing a packed model places its files and points the model at them", "[xmodel]") {
    Prop p;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(ToText(p.Export(true)).c_str()));

    TempDir show;
    CHECK(XModelFiles::Unpack(doc.document_element(), show.path.string()) == 6);
    CHECK(CountEmbedded(doc.document_element()) == 0);

    const fs::path dir = show.path / "modeldownload" / "Snow Man";
    CHECK(Slurp((dir / "body.png").string()) == Bytes("body"));
    CHECK(Slurp((dir / "arm.png").string()) == Bytes("arm"));
    CHECK(Slurp((dir / "arm" / "arm.obj").string()) == Slurp(p.obj));
    CHECK(Slurp((dir / "arm" / "arm.mtl").string()) == Slurp(p.mtl));
    CHECK(Slurp((dir / "arm" / "tex" / "skin.png").string()) == Bytes("skin"));
    CHECK(Slurp((dir / "open.png").string()) == Bytes("face"));

    // the imported models use the placed files
    Show s2;
    auto* servo = As<DmxServo>(s2.mm.createAndAddModel(ModelNode(doc, "Snow Man"), 100, 100));
    CHECK(servo->GetStaticImage(0)->GetImageFile() == (dir / "body.png").string());
    CHECK(servo->GetMotionImage(0)->GetImageFile() == (dir / "arm.png").string());
    auto* servo3d = As<DmxServo3d>(s2.mm.createAndAddModel(ModelNode(doc, "Arm 3D"), 100, 100));
    CHECK(servo3d->GetMotionMesh(0)->GetObjFile() == (dir / "arm" / "arm.obj").string());
    Model* matrix = s2.mm.createAndAddModel(ModelNode(doc, "Face"), 100, 100);
    REQUIRE(matrix != nullptr);
    CHECK(matrix->GetFaceInfo().at("Talk").at("Mouth-AI-EyesOpen") == (dir / "open.png").string());
}

TEST_CASE("Importing a packed model again reuses its files, and different files get their own folder", "[xmodel]") {
    Prop p;
    const std::string text = ToText(p.Export(true));
    TempDir show;
    const fs::path dir = show.path / "modeldownload" / "Snow Man";
    auto import = [&]() {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(text.c_str()));
        REQUIRE(XModelFiles::Unpack(doc.document_element(), show.path.string()) == 6);
        return std::string(ModelNode(doc, "Snow Man").child("StaticImage1").attribute("Image").value());
    };

    CHECK(import() == (dir / "body.png").string());
    CHECK(import() == (dir / "body.png").string());
    CHECK_FALSE(fs::exists(show.path / "modeldownload" / "Snow Man-2"));

    // someone else's file of the same name: the whole prop moves over, so
    // the mesh still finds its material beside it
    std::ofstream(dir / "arm" / "arm.mtl", std::ios::binary) << "different";
    const fs::path dir2 = show.path / "modeldownload" / "Snow Man-2";
    CHECK(import() == (dir2 / "body.png").string());
    CHECK(Slurp((dir2 / "arm" / "arm.mtl").string()) == Slurp(p.mtl));
    CHECK(Slurp((dir / "arm" / "arm.mtl").string()) == "different");
}

TEST_CASE("Embedded files can only land inside the show's modeldownload folder", "[xmodel]") {
    TempDir show;
    pugi::xml_document doc;
    const std::string data = "aGVsbG8="; // "hello"
    REQUIRE(doc.load_string(("<models><model name=\"M\" Image=\"modeldownload/../../evil.png\">"
                             "<xmodelFile path=\"modeldownload/../../evil.png\">" + data + "</xmodelFile>"
                             "<xmodelFile path=\"modeldownload/p/../../evil.png\">" + data + "</xmodelFile>"
                             "<xmodelFile path=\"/tmp/evil.png\">" + data + "</xmodelFile>"
                             "<xmodelFile path=\"other/evil.png\">" + data + "</xmodelFile>"
                             "<xmodelFile path=\"modeldownload/evil.png\">" + data + "</xmodelFile>"
                             "<xmodelFile path=\"modeldownload/p/ok.png\">" + data + "</xmodelFile>"
                             "</model></models>").c_str()));
    CHECK(XModelFiles::Unpack(doc.document_element(), show.path.string()) == 1);
    CHECK(CountEmbedded(doc.document_element()) == 0);
    CHECK(Slurp((show.path / "modeldownload" / "p" / "ok.png").string()) == "hello");
    std::vector<std::string> written;
    for (const auto& e : fs::recursive_directory_iterator(show.path)) {
        if (e.is_regular_file()) written.push_back(e.path().string());
    }
    CHECK(written == std::vector<std::string>{ (show.path / "modeldownload" / "p" / "ok.png").string() });
    // a reference to a file that wasn't placed is left alone
    CHECK(std::string(doc.document_element().child("model").attribute("Image").value()) == "modeldownload/../../evil.png");
}

TEST_CASE("Exports that don't pack files keep their paths", "[xmodel]") {
    Prop p;
    pugi::xml_document doc = p.Export(false);
    CHECK(std::string(ModelNode(doc, "Snow Man").child("StaticImage1").attribute("Image").value()) == p.body);
    CHECK(std::string(ModelNode(doc, "Face").child("faceInfo").attribute("Mouth-AI-EyesOpen").value()) == p.face);
    CHECK(CountEmbedded(doc.document_element()) == 0);

    // a file that can't be read stays a reference
    fs::remove(p.arm);
    pugi::xml_document packed = p.Export(true);
    CHECK(std::string(ModelNode(packed, "Snow Man").child("MotionImage1").attribute("Image").value()) == p.arm);
    CHECK(CountEmbedded(ModelNode(packed, "Snow Man")) == 1);
}

TEST_CASE("A packed model still loads where the files aren't unpacked", "[xmodel]") {
    // what a version without embedded file support does with the file
    Prop p;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(ToText(p.Export(true)).c_str()));
    Show s2;
    auto* servo = As<DmxServo>(s2.mm.createAndAddModel(ModelNode(doc, "Snow Man"), 100, 100));
    CHECK(servo->GetNumServos() == 1);
    As<DmxServo3d>(s2.mm.createAndAddModel(ModelNode(doc, "Arm 3D"), 100, 100));
    Model* matrix = s2.mm.createAndAddModel(ModelNode(doc, "Face"), 100, 100);
    REQUIRE(matrix != nullptr);
    CHECK(matrix->GetFaceInfo().count("Talk") == 1);
}

TEST_CASE("Export as custom model carries matrix face images", "[xmodel]") {
    Prop p;
    StringSerializingVisitor visitor(/*exporting*/ true);
    XModelFiles::Packer packer(p.matrix->GetName());
    visitor.SetFilePacker(&packer);
    p.matrix->ExportAsCustomXModel(visitor);

    pugi::xml_document doc;
    REQUIRE(doc.load_string(visitor.GetResult().c_str()));
    pugi::xml_node custom = doc.document_element();
    CHECK(std::string(custom.child("faceInfo").attribute("Mouth-AI-EyesOpen").value()) == "modeldownload/Face/open.png");
    CHECK(CountEmbedded(custom) == 1);

    TempDir show;
    CHECK(XModelFiles::Unpack(custom, show.path.string()) == 1);
    CHECK(Slurp((show.path / "modeldownload" / "Face" / "open.png").string()) == Bytes("face"));
}
