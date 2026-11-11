#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "graphics/xlMesh.h"
#include "support/TempDir.h"

using Catch::Matchers::WithinAbs;

namespace {

struct TestMesh : xlMesh {
    TestMesh(const std::string& f) : xlMesh(nullptr, f) {}
    size_t vertexFloats() const { return objects.GetAttrib().vertices.size(); }
    size_t normalFloats() const { return objects.GetAttrib().normals.size(); }
    size_t shapeCount() const { return objects.GetShapes().size(); }
    size_t triangleCount() const {
        size_t n = 0;
        for (auto const& s : objects.GetShapes()) {
            n += s.mesh.num_face_vertices.size();
        }
        return n;
    }
    size_t materialCount() const { return materials.size(); }
    bool resyncFlag() const { return materialsNeedResyncing; }
};

std::string ReadAll(const std::string& path) {
    std::ifstream f(path); // text mode: Windows writers emit CRLF
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

const char* kCube = R"(# unit cube
o Cube
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
v 0 0 1
v 1 0 1
v 1 1 1
v 0 1 1
f 1 2 3 4
f 5 8 7 6
f 1 5 6 2
f 2 6 7 3
f 3 7 8 4
f 5 1 4 8
)";

} // namespace

TEST_CASE("xlMesh loads a cube and reports its bounds", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("cube.obj", kCube));
    REQUIRE(m.IsLoaded());
    CHECK(m.HasGeometry());
    CHECK(m.vertexFloats() == 24);
    CHECK_THAT(m.GetXMin(), WithinAbs(0.0f, 1e-5));
    CHECK_THAT(m.GetXMax(), WithinAbs(1.0f, 1e-5));
    CHECK_THAT(m.GetYMin(), WithinAbs(0.0f, 1e-5));
    CHECK_THAT(m.GetYMax(), WithinAbs(1.0f, 1e-5));
    CHECK_THAT(m.GetZMin(), WithinAbs(0.0f, 1e-5));
    CHECK_THAT(m.GetZMax(), WithinAbs(1.0f, 1e-5));
}

TEST_CASE("xlMesh triangulates quads", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("cube.obj", kCube));
    REQUIRE(m.IsLoaded());
    CHECK(m.shapeCount() == 1);
    CHECK(m.triangleCount() == 12);
}

TEST_CASE("xlMesh loads a single triangle", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("tri.obj", "v 0 0 0\nv 2 0 0\nv 0 3 0\nf 1 2 3\n"));
    REQUIRE(m.IsLoaded());
    CHECK(m.triangleCount() == 1);
    CHECK_THAT(m.GetXMax(), WithinAbs(2.0f, 1e-5));
    CHECK_THAT(m.GetYMax(), WithinAbs(3.0f, 1e-5));
    CHECK_THAT(m.GetZMin(), WithinAbs(0.0f, 1e-5));
    CHECK_THAT(m.GetZMax(), WithinAbs(0.0f, 1e-5));
}

TEST_CASE("xlMesh fan-triangulates a pentagon", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("ngon.obj",
                       "v 0 0 0\nv 2 0 0\nv 3 1 0\nv 1 2 0\nv -1 1 0\nf 1 2 3 4 5\n"));
    REQUIRE(m.IsLoaded());
    CHECK(m.triangleCount() == 3);
    CHECK_THAT(m.GetXMin(), WithinAbs(-1.0f, 1e-5));
    CHECK_THAT(m.GetXMax(), WithinAbs(3.0f, 1e-5));
}

TEST_CASE("xlMesh handles faces without normals and with normals", "[misc][mesh]") {
    TempDir t;
    {
        TestMesh m(t.Write("nonormal.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"));
        REQUIRE(m.IsLoaded());
        CHECK(m.normalFloats() == 0);
        CHECK(m.triangleCount() == 1);
    }
    {
        TestMesh m(t.Write("normal.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nf 1//1 2//1 3//1\n"));
        REQUIRE(m.IsLoaded());
        CHECK(m.normalFloats() == 3);
        CHECK(m.triangleCount() == 1);
    }
}

TEST_CASE("xlMesh accepts negative relative indices", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("neg.obj", "v 0 0 0\nv 4 0 0\nv 0 5 0\nf -3 -2 -1\n"));
    REQUIRE(m.IsLoaded());
    CHECK(m.triangleCount() == 1);
    CHECK_THAT(m.GetXMax(), WithinAbs(4.0f, 1e-5));
    CHECK_THAT(m.GetYMax(), WithinAbs(5.0f, 1e-5));
}

TEST_CASE("xlMesh ignores comments blank lines and unknown statements", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("c.obj",
                       "# header\n\n   \ng group\ns off\nusemtl none\nv 1 1 1 # trailing\nv 2 1 1\nv 1 2 1\n\nf 1 2 3\n# end\n"));
    REQUIRE(m.IsLoaded());
    CHECK(m.triangleCount() == 1);
    CHECK_THAT(m.GetXMin(), WithinAbs(1.0f, 1e-5));
    CHECK_THAT(m.GetXMax(), WithinAbs(2.0f, 1e-5));
}

TEST_CASE("xlMesh handles CRLF line endings", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("crlf.obj", "v 0 0 0\r\nv 1 0 0\r\nv 0 1 0\r\nf 1 2 3\r\n"));
    REQUIRE(m.IsLoaded());
    CHECK(m.triangleCount() == 1);
    CHECK_THAT(m.GetXMax(), WithinAbs(1.0f, 1e-5));
}

TEST_CASE("xlMesh handles very large coordinates", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("big.obj", "v -1e6 -2e6 -3e6\nv 1e6 2e6 3e6\nv 0 0 0\nf 1 2 3\n"));
    REQUIRE(m.IsLoaded());
    CHECK_THAT(m.GetXMin(), WithinAbs(-1e6f, 1.0));
    CHECK_THAT(m.GetYMax(), WithinAbs(2e6f, 1.0));
    CHECK_THAT(m.GetZMin(), WithinAbs(-3e6f, 1.0));
    CHECK_THAT(m.GetZMax(), WithinAbs(3e6f, 1.0));
}

TEST_CASE("xlMesh with several objects creates several shapes", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("two.obj",
                       "o A\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\no B\nv 5 5 5\nv 6 5 5\nv 5 6 5\nf 4 5 6\n"));
    REQUIRE(m.IsLoaded());
    CHECK(m.shapeCount() == 2);
    CHECK_THAT(m.GetXMax(), WithinAbs(6.0f, 1e-5));
    CHECK_THAT(m.GetZMax(), WithinAbs(5.0f, 1e-5));
}

TEST_CASE("xlMesh fails cleanly for a missing file", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.File("nope.obj"));
    CHECK_FALSE(m.IsLoaded());
    CHECK_FALSE(m.HasGeometry());
}

TEST_CASE("xlMesh keeps default bounds for an empty file", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("empty.obj", ""));
    CHECK_FALSE(m.HasGeometry());
    CHECK(m.GetXMin() > m.GetXMax());
    CHECK(m.GetYMin() > m.GetYMax());
    CHECK(m.GetZMin() > m.GetZMax());
}

TEST_CASE("xlMesh survives malformed content", "[misc][mesh]") {
    TempDir t;
    const std::vector<std::string> bodies = {
        "v 0 0\nv 1 0 0\nf 1 2\n",
        "v 0 0 0\nf\n",
        std::string("\0\0\0\xff\xfe garbage", 14),
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1/ 2/ 3/\n",
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 0 1 2\n",
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nvn 0 0 1\nf 1//7 2//7 3//7\n",
    };
    int n = 0;
    for (auto const& b : bodies) {
        TestMesh m(t.Write("bad" + std::to_string(n++) + ".obj", b));
        INFO("body " << n);
        if (m.IsLoaded()) {
            (void)m.HasGeometry();
        }
        (void)m.GetXMin();
    }
    SUCCEED();
}

// tinyobj asserts (debug) or reads out of bounds (release) when a face names a vertex that does not exist,
// and xlMesh does not validate the file first.
TEST_CASE("xlMesh must not crash on faces that reference missing vertices", "[mesh]") {
    TempDir t;
    const std::vector<std::string> bodies = {
        "v a b c\nf 1 2 3\n",
        "f 1 2 3\n",
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 99\n",
        "v 0 0 0\nv 1 0 0\nv 0 1 0\nf -9 -2 -1\n",
    };
    int n = 0;
    for (auto const& b : bodies) {
        TestMesh m(t.Write("oob" + std::to_string(n++) + ".obj", b));
        (void)m.IsLoaded();
    }
    SUCCEED();
}

TEST_CASE("xlMesh reads materials from an mtl beside the obj", "[misc][mesh]") {
    TempDir t;
    t.Write("mat.mtl", "newmtl Red\nKd 1 0 0\nd 1\nnewmtl Half\nKd 0 0.5 0\nd 0.5\n");
    TestMesh m(t.Write("mat.obj", "mtllib mat.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\nusemtl Red\nf 1 2 3\n"));
    REQUIRE(m.IsLoaded());
    REQUIRE(m.materialCount() == 2);
    CHECK(m.GetMaterial(0).name == "Red");
    CHECK(m.GetMaterial(0).color.red == 255);
    CHECK(m.GetMaterial(0).color.green == 0);
    CHECK(m.GetMaterial(0).color.alpha == 255);
    CHECK(m.GetMaterial(1).name == "Half");
    CHECK(m.GetMaterial(1).color.green == 127);
    CHECK(m.GetMaterial(1).color.alpha == 127);
    CHECK(m.GetMaterial(0).texture == nullptr);
}

TEST_CASE("xlMesh SetMaterialColor forces and restores a colour", "[misc][mesh]") {
    TempDir t;
    t.Write("mat.mtl", "newmtl Red\nKd 1 0 0\nd 1\n");
    TestMesh m(t.Write("mat.obj", "mtllib mat.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\nusemtl Red\nf 1 2 3\n"));
    REQUIRE(m.materialCount() == 1);
    CHECK_FALSE(m.resyncFlag());

    xlColor blue(0, 0, 255);
    m.SetMaterialColor("Red", &blue);
    CHECK(m.GetMaterial(0).forceColor);
    CHECK(m.GetMaterial(0).color.blue == 255);
    CHECK(m.GetMaterial(0).color.red == 0);
    CHECK(m.resyncFlag());

    m.SetMaterialColor("Red", nullptr);
    CHECK_FALSE(m.GetMaterial(0).forceColor);
    CHECK(m.GetMaterial(0).color.red == 255);
    CHECK(m.GetMaterial(0).color.blue == 0);
}

TEST_CASE("xlMesh SetMaterialColor for an unknown material changes nothing", "[misc][mesh]") {
    TempDir t;
    t.Write("mat.mtl", "newmtl Red\nKd 1 0 0\nd 1\n");
    TestMesh m(t.Write("mat.obj", "mtllib mat.mtl\nv 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"));
    xlColor blue(0, 0, 255);
    m.SetMaterialColor("Nope", &blue);
    CHECK_FALSE(m.resyncFlag());
    CHECK_FALSE(m.GetMaterial(0).forceColor);
}

TEST_CASE("xlMesh name accessor", "[misc][mesh]") {
    TempDir t;
    TestMesh m(t.Write("tri.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"));
    CHECK(m.GetName().empty());
    m.SetName("tree");
    CHECK(m.GetName() == "tree");
}

TEST_CASE("InvalidMaterialsList flags two entries when one is not an mtl", "[misc][mesh]") {
    CHECK_FALSE(xlMesh::InvalidMaterialsList({}));
    CHECK_FALSE(xlMesh::InvalidMaterialsList({ "a.mtl" }));
    CHECK_FALSE(xlMesh::InvalidMaterialsList({ "a.mtl", "b.MTL" }));
    CHECK(xlMesh::InvalidMaterialsList({ "my", "materials.mtl" }));
    CHECK(xlMesh::InvalidMaterialsList({ "a.mtl", "b.txt" }));
    CHECK_FALSE(xlMesh::InvalidMaterialsList({ "a.mtl", "b.mtl", "c.txt" }));
}

TEST_CASE("GetMaterialFilenamesFromOBJ splits mtllib on spaces when strict", "[misc][mesh]") {
    TempDir t;
    auto obj = t.Write("a.obj", "# c\nmtllib one.mtl two.mtl\nv 0 0 0\n");
    auto strict = xlMesh::GetMaterialFilenamesFromOBJ(obj);
    REQUIRE(strict.size() == 2);
    CHECK(strict[0] == "one.mtl");
    CHECK(strict[1] == "two.mtl");

    auto loose = xlMesh::GetMaterialFilenamesFromOBJ(obj, false);
    REQUIRE(loose.size() == 1);
    CHECK(loose[0] == "one.mtl two.mtl");
}

TEST_CASE("GetMaterialFilenamesFromOBJ handles missing or empty input", "[misc][mesh]") {
    TempDir t;
    CHECK(xlMesh::GetMaterialFilenamesFromOBJ(t.File("none.obj")).empty());
    CHECK(xlMesh::GetMaterialFilenamesFromOBJ(t.Write("e.obj", "")).empty());
    CHECK(xlMesh::GetMaterialFilenamesFromOBJ(t.Write("n.obj", "v 0 0 0\n")).empty());
    CHECK(xlMesh::GetMaterialFilenamesFromOBJ(t.Write("b.obj", "mtllib \n")).empty());
}

TEST_CASE("GetMaterialFilenamesFromOBJ collects several mtllib lines", "[misc][mesh]") {
    TempDir t;
    auto obj = t.Write("m.obj", "mtllib a.mtl\nv 0 0 0\nmtllib b.mtl\n");
    auto r = xlMesh::GetMaterialFilenamesFromOBJ(obj);
    REQUIRE(r.size() == 2);
    CHECK(r[0] == "a.mtl");
    CHECK(r[1] == "b.mtl");
}

TEST_CASE("GetTextureFilenamesFromMTL finds map_Kd entries", "[misc][mesh]") {
    TempDir t;
    auto mtl = t.Write("a.mtl", "newmtl X\nKd 1 1 1\nmap_Kd wood.png\nnewmtl Y\nmap_Kd my tex.png\nmap_Ks spec.png\n");
    auto strict = xlMesh::GetTextureFilenamesFromMTL(mtl);
    REQUIRE(strict.size() == 3);
    CHECK(strict[0] == "wood.png");
    CHECK(strict[1] == "my");
    CHECK(strict[2] == "tex.png");

    auto loose = xlMesh::GetTextureFilenamesFromMTL(mtl, false);
    REQUIRE(loose.size() == 2);
    CHECK(loose[1] == "my tex.png");
}

TEST_CASE("GetTextureFilenamesFromMTL missing file gives nothing", "[misc][mesh]") {
    TempDir t;
    CHECK(xlMesh::GetTextureFilenamesFromMTL(t.File("none.mtl")).empty());
}

TEST_CASE("FixMaterialFilenamesInOBJ replaces spaces in the mtllib name and copies the mtl", "[misc][mesh]") {
    TempDir t;
    t.Write("my materials.mtl", "newmtl X\nKd 1 1 1\n");
    auto obj = t.Write("fix.obj", "mtllib my materials.mtl\nv 0 0 0\nf 1 1 1\n");
    xlMesh::FixMaterialFilenamesInOBJ(obj);

    CHECK(ReadAll(obj) == "mtllib my_materials.mtl\nv 0 0 0\nf 1 1 1\n");
    CHECK(ReadAll(obj + ".bak") == "mtllib my materials.mtl\nv 0 0 0\nf 1 1 1\n");
    CHECK(ReadAll(t.File("my_materials.mtl")) == "newmtl X\nKd 1 1 1\n");
}

TEST_CASE("FixMaterialFilenamesInOBJ leaves simple names alone", "[misc][mesh]") {
    TempDir t;
    auto obj = t.Write("ok.obj", "mtllib ok.mtl\nv 0 0 0\n");
    xlMesh::FixMaterialFilenamesInOBJ(obj);
    CHECK(ReadAll(obj) == "mtllib ok.mtl\nv 0 0 0\n");
}

TEST_CASE("FixMaterialFilenamesInOBJ on a missing file does not create it", "[misc][mesh]") {
    TempDir t;
    auto obj = t.File("missing.obj");
    xlMesh::FixMaterialFilenamesInOBJ(obj);
    CHECK_FALSE(std::filesystem::exists(obj));
}

TEST_CASE("xlMeshFilePrefetch reports local files ready", "[misc][mesh]") {
    TempDir t;
    auto obj = t.Write("p.obj", "v 0 0 0\n");
    xlMeshFilePrefetch p;
    bool ready = false;
    for (int i = 0; i < 100000 && !ready; ++i) {
        ready = p.Ready(obj);
        std::this_thread::yield();
    }
    CHECK(ready);
}
