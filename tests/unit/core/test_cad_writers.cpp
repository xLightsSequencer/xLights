#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "cad/CADModel.h"
#include "cad/CADPoint.h"
#include "cad/CADWriter.h"
#include "cad/DXFWriter.h"
#include "cad/ModelToCAD.h"
#include "cad/STLWriter.h"
#include "cad/VRMLWriter.h"
#include "support/TempDir.h"

using Catch::Matchers::WithinAbs;

namespace {

std::string ReadAll(const std::string& path) {
    std::ifstream f(path); // text mode: Windows writers emit CRLF
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::vector<std::string> Lines(const std::string& s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string l;
    while (std::getline(ss, l)) {
        out.push_back(l);
    }
    return out;
}

size_t Count(const std::string& hay, const std::string& needle) {
    size_t n = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + needle.size())) {
        ++n;
    }
    return n;
}

struct DXFEntity {
    std::string type;
    std::vector<std::pair<int, std::string>> codes;
    std::string Get(int code) const {
        for (auto const& c : codes) {
            if (c.first == code) {
                return c.second;
            }
        }
        return "";
    }
    bool Has(int code) const { return !Get(code).empty(); }
};

// Reads the group-code/value pairs of a DXF ENTITIES section the way a CAD importer does.
std::vector<DXFEntity> ParseDXF(const std::string& text, bool& sawEof, bool& wellFormed) {
    std::vector<std::string> l = Lines(text);
    std::vector<DXFEntity> ents;
    sawEof = false;
    wellFormed = (l.size() % 2) == 0;
    for (size_t i = 0; i + 1 < l.size(); i += 2) {
        int code = (int)std::strtol(l[i].c_str(), nullptr, 10);
        std::string val = l[i + 1];
        if (code == 0) {
            if (val == "EOF") {
                sawEof = true;
            } else if (val != "SECTION" && val != "ENDSEC") {
                ents.push_back({ val, {} });
            }
        } else if (!ents.empty()) {
            ents.back().codes.push_back({ code, val });
        }
    }
    return ents;
}

struct Recorder : CADWriter {
    std::vector<std::string> log;
    std::vector<std::string> texts;
    std::vector<uint32_t> colors;
    explicit Recorder(std::string f = "") : CADWriter(std::move(f)) {}
    void WriteText(CADPoint const& loc, std::string const& text, float, uint32_t color, float) override {
        log.push_back("text");
        texts.push_back(text);
        colors.push_back(color);
    }
    void WriteWire(CADPoint const&, CADPoint const&, uint32_t) override { log.push_back("wire"); }
    void WriteNode(CADPoint const&, uint32_t color) override {
        log.push_back("node");
        colors.push_back(color);
    }
    void WriteNodeHeader() override { log.push_back("nodehdr"); }
    void WriteNodeFooter() override { log.push_back("nodeftr"); }
    void WriteWireHeader() override { log.push_back("wirehdr"); }
    void WriteWireFooter() override { log.push_back("wireftr"); }
    void WriteTextHeader() override { log.push_back("texthdr"); }
    void WriteTextFooter() override { log.push_back("textftr"); }
};

} // namespace

TEST_CASE("CADPoint and CADText constructors", "[misc][cad]") {
    CADPoint p(1.5f, -2.0f, 3.0f);
    CHECK(p.X == 1.5f);
    CHECK(p.Y == -2.0f);
    CHECK(p.Z == 3.0f);

    CADText t(1, 2, 3, "hello");
    CHECK(t.Loc.X == 1);
    CHECK(t.Text == "hello");
    CHECK(t.Size == 1.0f);
    CHECK(t.Rotation == 0.0f);

    CADText t2(0, 0, 0, "x", 4.0f, 90.0f);
    CHECK(t2.Size == 4.0f);
    CHECK(t2.Rotation == 90.0f);
}

TEST_CASE("ModelToCAD::Create maps the file type to a writer", "[misc][cad]") {
    TempDir t;
    CHECK(dynamic_cast<DXFWriter*>(ModelToCAD::Create(t.File("a.dxf"), "dxf").get()) != nullptr);
    CHECK(dynamic_cast<STLWriter*>(ModelToCAD::Create(t.File("a.stl"), "stl").get()) != nullptr);
    CHECK(dynamic_cast<VRMLWriter*>(ModelToCAD::Create(t.File("a.wrl"), "wrl").get()) != nullptr);
    CHECK(ModelToCAD::Create(t.File("a.x"), "obj") == nullptr);
    CHECK(ModelToCAD::Create(t.File("a.x"), "") == nullptr);
    CHECK(ModelToCAD::Create(t.File("a.x"), "DXF") == nullptr);
}

TEST_CASE("CADWriter MapToScreen differs between formats", "[misc][cad]") {
    TempDir t;
    CHECK(ModelToCAD::Create(t.File("a"), "dxf")->MapToScreen());
    CHECK_FALSE(ModelToCAD::Create(t.File("b"), "stl")->MapToScreen());
    CHECK_FALSE(ModelToCAD::Create(t.File("c"), "wrl")->MapToScreen());
}

TEST_CASE("CADWriter Open fails for an unwritable path", "[misc][cad]") {
    TempDir t;
    DXFWriter w(t.File("no_such_dir/out.dxf"));
    CHECK_FALSE(w.Open());
    DXFWriter ok(t.File("out.dxf"));
    CHECK(ok.Open());
}

TEST_CASE("ModelToCAD ExportCAD of no model writes an empty drawing", "[misc][cad]") {
    TempDir t;
    auto path = t.File("empty.dxf");
    CHECK(ModelToCAD::ExportCAD((Model*)nullptr, path, "dxf"));
    bool eof = false, ok = false;
    auto ents = ParseDXF(ReadAll(path), eof, ok);
    CHECK(eof);
    CHECK(ents.empty());
    CHECK(ReadAll(path).find("ENTITIES") != std::string::npos);
}

TEST_CASE("ModelToCAD ExportCAD rejects unknown types and bad paths", "[misc][cad]") {
    TempDir t;
    CHECK_FALSE(ModelToCAD::ExportCAD((Model*)nullptr, t.File("x.obj"), "obj"));
    CHECK_FALSE(std::filesystem::exists(t.File("x.obj")));
    CHECK_FALSE(ModelToCAD::ExportCAD((Model*)nullptr, t.File("nodir/x.dxf"), "dxf"));
}

TEST_CASE("DXFWriter header and footer frame the entities section", "[misc][cad]") {
    TempDir t;
    auto path = t.File("h.dxf");
    {
        DXFWriter w(path);
        REQUIRE(w.Open());
        w.WriteHeader();
        w.WriteEndOfFile();
    }
    CHECK(ReadAll(path) == "0\nSECTION\n2\nENTITIES\n0\nENDSEC\n0\nEOF\n");
}

TEST_CASE("DXFWriter writes a node as a small circle", "[misc][cad]") {
    TempDir t;
    auto path = t.File("n.dxf");
    {
        DXFWriter w(path);
        REQUIRE(w.Open());
        w.WriteHeader();
        w.WriteNode(CADPoint(1.5f, 2.5f, 3.5f), 0u);
        w.WriteEndOfFile();
    }
    bool eof, ok;
    auto ents = ParseDXF(ReadAll(path), eof, ok);
    CHECK(ok);
    CHECK(eof);
    REQUIRE(ents.size() == 1);
    CHECK(ents[0].type == "CIRCLE");
    CHECK(ents[0].Get(8) == "0");
    CHECK_THAT(std::strtod(ents[0].Get(10).c_str(), nullptr), WithinAbs(1.5, 1e-5));
    CHECK_THAT(std::strtod(ents[0].Get(20).c_str(), nullptr), WithinAbs(2.5, 1e-5));
    CHECK_THAT(std::strtod(ents[0].Get(30).c_str(), nullptr), WithinAbs(3.5, 1e-5));
    CHECK_THAT(std::strtod(ents[0].Get(40).c_str(), nullptr), WithinAbs(0.2, 1e-5));
    CHECK_FALSE(ents[0].Has(420));
    CHECK_FALSE(ents[0].Has(39));
}

TEST_CASE("DXFWriter writes a wire as a LINE on layer 2", "[misc][cad]") {
    TempDir t;
    auto path = t.File("w.dxf");
    {
        DXFWriter w(path);
        REQUIRE(w.Open());
        w.WriteWire(CADPoint(0, 1, 2), CADPoint(3, 4, 5), 0xFF0000);
    }
    bool eof, ok;
    auto ents = ParseDXF(ReadAll(path), eof, ok);
    REQUIRE(ents.size() == 1);
    CHECK(ents[0].type == "LINE");
    CHECK(ents[0].Get(8) == "2");
    CHECK_THAT(std::strtod(ents[0].Get(11).c_str(), nullptr), WithinAbs(3.0, 1e-5));
    CHECK_THAT(std::strtod(ents[0].Get(21).c_str(), nullptr), WithinAbs(4.0, 1e-5));
    CHECK_THAT(std::strtod(ents[0].Get(31).c_str(), nullptr), WithinAbs(5.0, 1e-5));
    CHECK(ents[0].Get(420) == std::to_string(0xFF0000));
}

TEST_CASE("DXFWriter writes text with height rotation and colour only when set", "[misc][cad]") {
    TempDir t;
    auto path = t.File("t.dxf");
    {
        DXFWriter w(path);
        REQUIRE(w.Open());
        w.WriteText(CADPoint(1, 2, 3), "Plain", 2.0f, 0, 0.0f);
        w.WriteText(CADPoint(1, 2, 3), "Rotated", 3.0f, 255, 45.0f);
    }
    bool eof, ok;
    auto ents = ParseDXF(ReadAll(path), eof, ok);
    REQUIRE(ents.size() == 2);
    CHECK(ents[0].type == "TEXT");
    CHECK(ents[0].Get(1) == "Plain");
    CHECK(ents[0].Get(8) == "1");
    CHECK_THAT(std::strtod(ents[0].Get(40).c_str(), nullptr), WithinAbs(2.0, 1e-5));
    CHECK_FALSE(ents[0].Has(50));
    CHECK_FALSE(ents[0].Has(420));
    CHECK(ents[1].Get(1) == "Rotated");
    CHECK_THAT(std::strtod(ents[1].Get(50).c_str(), nullptr), WithinAbs(45.0, 1e-5));
    CHECK(ents[1].Get(420) == "255");
}

TEST_CASE("DXFWriter keeps text with odd characters on one value line", "[misc][cad]") {
    TempDir t;
    auto path = t.File("odd.dxf");
    {
        DXFWriter w(path);
        REQUIRE(w.Open());
        w.WriteText(CADPoint(0, 0, 0), "Tree \"A\" & <B> 100%", 1.0f, 0);
        w.WriteText(CADPoint(0, 0, 0), "", 1.0f, 0);
        w.WriteText(CADPoint(0, 0, 0), std::string(5000, 'x'), 1.0f, 0);
    }
    bool eof, ok;
    auto ents = ParseDXF(ReadAll(path), eof, ok);
    REQUIRE(ents.size() == 3);
    CHECK(ents[0].Get(1) == "Tree \"A\" & <B> 100%");
    CHECK(ents[2].Get(1).size() == 5000);
}

TEST_CASE("DXFWriter handles extreme coordinates without breaking the format", "[misc][cad]") {
    TempDir t;
    auto path = t.File("ext.dxf");
    {
        DXFWriter w(path);
        REQUIRE(w.Open());
        w.WriteNode(CADPoint(1e30f, -1e30f, 0), 0u);
        w.WriteNode(CADPoint(0, 0, 0), 0u);
    }
    bool eof, ok;
    auto ents = ParseDXF(ReadAll(path), eof, ok);
    CHECK(ok);
    CHECK(ents.size() == 2);
}

TEST_CASE("DXFWriter pairs every group code with a value", "[misc][cad]") {
    TempDir t;
    auto path = t.File("pairs.dxf");
    {
        DXFWriter w(path);
        REQUIRE(w.Open());
        w.WriteHeader();
        for (int i = 0; i < 50; ++i) {
            w.WriteNode(CADPoint((float)i, (float)i * 2, 0), i % 2 ? 0 : 0x112233);
            w.WriteWire(CADPoint(0, 0, 0), CADPoint((float)i, 1, 1), 0);
            w.WriteText(CADPoint(0, 0, 0), "n" + std::to_string(i), 1.0f, 0);
        }
        w.WriteEndOfFile();
    }
    bool eof, ok;
    auto ents = ParseDXF(ReadAll(path), eof, ok);
    CHECK(ok);
    CHECK(eof);
    CHECK(ents.size() == 150);
    int circles = 0, lines = 0, texts = 0;
    for (auto const& e : ents) {
        circles += e.type == "CIRCLE";
        lines += e.type == "LINE";
        texts += e.type == "TEXT";
    }
    CHECK(circles == 50);
    CHECK(lines == 50);
    CHECK(texts == 50);
}

TEST_CASE("STLWriter writes a cube per node with 12 facets", "[misc][cad]") {
    TempDir t;
    auto path = t.File("n.stl");
    {
        STLWriter w(path);
        REQUIRE(w.Open());
        w.WriteHeader();
        w.WriteNode(CADPoint(0, 0, 0), 0u);
        w.WriteEndOfFile();
    }
    std::string s = ReadAll(path);
    CHECK(s.rfind("solid xLights\n", 0) == 0);
    CHECK(s.find("endsolid xLights") != std::string::npos);
    CHECK(Count(s, "facet normal") == 12);
    CHECK(Count(s, "endfacet") == 12);
    CHECK(Count(s, "outer loop") == 12);
    CHECK(Count(s, "vertex ") == 36);
}

TEST_CASE("STLWriter first node is a larger cube than a normal node", "[misc][cad]") {
    TempDir t;
    auto extent = [&](bool first) {
        auto path = t.File(first ? "f.stl" : "n.stl");
        {
            STLWriter w(path);
            REQUIRE(w.Open());
            w.WriteHeader();
            if (first) {
                w.WriteFirstNode(CADPoint(0, 0, 0), 0);
            } else {
                w.WriteNode(CADPoint(0, 0, 0), 0u);
            }
            w.WriteEndOfFile();
        }
        double mx = 0;
        for (auto const& l : Lines(ReadAll(path))) {
            auto p = l.find("vertex ");
            if (p == std::string::npos) {
                continue;
            }
            const char* c = l.c_str() + p + 7;
            char* e;
            double x = std::strtod(c, &e);
            mx = std::max(mx, std::fabs(x));
        }
        return mx;
    };
    CHECK_THAT(extent(false), WithinAbs(0.2, 1e-5));
    CHECK_THAT(extent(true), WithinAbs(0.4, 1e-5));
}

TEST_CASE("STLWriter swaps Y and Z and negates Z for the CAD axes", "[misc][cad]") {
    TempDir t;
    auto path = t.File("axes.stl");
    {
        STLWriter w(path);
        REQUIRE(w.Open());
        w.WriteHeader();
        w.WriteNode(CADPoint(10, 20, 30), 0u);
        w.WriteEndOfFile();
    }
    double minx = 1e9, maxx = -1e9, miny = 1e9, maxy = -1e9, minz = 1e9, maxz = -1e9;
    for (auto const& l : Lines(ReadAll(path))) {
        auto p = l.find("vertex ");
        if (p == std::string::npos) {
            continue;
        }
        double x, y, z;
        REQUIRE(std::sscanf(l.c_str() + p + 7, "%lf %lf %lf", &x, &y, &z) == 3);
        minx = std::min(minx, x);
        maxx = std::max(maxx, x);
        miny = std::min(miny, y);
        maxy = std::max(maxy, y);
        minz = std::min(minz, z);
        maxz = std::max(maxz, z);
    }
    CHECK_THAT((minx + maxx) / 2, WithinAbs(10.0, 1e-4));
    CHECK_THAT((miny + maxy) / 2, WithinAbs(-30.0, 1e-4));
    CHECK_THAT((minz + maxz) / 2, WithinAbs(20.0, 1e-4));
}

TEST_CASE("STLWriter ignores text and wires", "[misc][cad]") {
    TempDir t;
    auto path = t.File("tw.stl");
    {
        STLWriter w(path);
        REQUIRE(w.Open());
        w.WriteHeader();
        w.WriteText(CADPoint(0, 0, 0), "hi", 1.0f, 0);
        w.WriteWire(CADPoint(0, 0, 0), CADPoint(1, 1, 1), 0);
        w.WriteEndOfFile();
    }
    CHECK(ReadAll(path) == "solid xLights\nendsolid xLights\n");
}

TEST_CASE("STLWriter many nodes scale linearly", "[misc][cad]") {
    TempDir t;
    auto path = t.File("many.stl");
    {
        STLWriter w(path);
        REQUIRE(w.Open());
        w.WriteHeader();
        for (int i = 0; i < 200; ++i) {
            w.WriteNode(CADPoint((float)i, 0, 0), 0u);
        }
        w.WriteEndOfFile();
    }
    CHECK(Count(ReadAll(path), "facet normal") == 200 * 12);
}

TEST_CASE("VRMLWriter header names the format and application", "[misc][cad]") {
    TempDir t;
    auto path = t.File("h.wrl");
    {
        VRMLWriter w(path);
        REQUIRE(w.Open());
        w.WriteHeader();
    }
    std::string s = ReadAll(path);
    CHECK(s.rfind("#VRML V2.0 utf8\n", 0) == 0);
    CHECK(s.find("WorldInfo {") != std::string::npos);
    CHECK(s.find("Created by xLights ") != std::string::npos);
}

TEST_CASE("VRMLWriter node colours distinguish first normal and last", "[misc][cad]") {
    TempDir t;
    auto path = t.File("c.wrl");
    {
        VRMLWriter w(path);
        REQUIRE(w.Open());
        w.WriteFirstNode(CADPoint(1, 2, 3), 0);
        w.WriteNode(CADPoint(1, 2, 3), 0u);
        w.WriteLastNode(CADPoint(1, 2, 3), 0);
    }
    std::string s = ReadAll(path);
    CHECK(Count(s, "Transform {") == 3);
    CHECK(Count(s, "IndexedFaceSet") == 3);
    auto first = s.find("diffuseColor 0.000 0.000 1.000");
    auto normal = s.find("diffuseColor 0.800 0.800 0.800");
    auto last = s.find("diffuseColor 0.000 1.000 0.000");
    REQUIRE(first != std::string::npos);
    REQUIRE(normal != std::string::npos);
    REQUIRE(last != std::string::npos);
    CHECK(first < normal);
    CHECK(normal < last);
}

TEST_CASE("VRMLWriter swaps Y and Z in the translation", "[misc][cad]") {
    TempDir t;
    auto path = t.File("tr.wrl");
    {
        VRMLWriter w(path);
        REQUIRE(w.Open());
        w.WriteNode(CADPoint(1, 2, 3), 0u);
    }
    CHECK(ReadAll(path).find("translation 1 -3 2\n") != std::string::npos);
}

TEST_CASE("VRMLWriter text carries the string and its length", "[misc][cad]") {
    TempDir t;
    auto path = t.File("tx.wrl");
    {
        VRMLWriter w(path);
        REQUIRE(w.Open());
        w.WriteText(CADPoint(0, 0, 0), "Arch 1", 1.0f, 0);
        w.WriteWire(CADPoint(0, 0, 0), CADPoint(1, 1, 1), 0);
    }
    std::string s = ReadAll(path);
    CHECK(s.find("string[\"Arch 1\"]") != std::string::npos);
    CHECK(s.find("length[6]") != std::string::npos);
    CHECK(Count(s, "Transform {") == 1);
}

TEST_CASE("VRMLWriter has balanced brackets for a mixed drawing", "[misc][cad]") {
    TempDir t;
    auto path = t.File("bal.wrl");
    {
        VRMLWriter w(path);
        REQUIRE(w.Open());
        w.WriteHeader();
        w.WriteFirstNode(CADPoint(0, 0, 0), 0);
        for (int i = 1; i < 5; ++i) {
            w.WriteNode(CADPoint((float)i, 0, 0), 0u);
        }
        w.WriteLastNode(CADPoint(5, 0, 0), 0);
        w.WriteText(CADPoint(0, 0, 0), "x", 1.0f, 0);
    }
    std::string s = ReadAll(path);
    CHECK(Count(s, "{") == Count(s, "}"));
    CHECK(Count(s, "[") == Count(s, "]"));
}

TEST_CASE("CADModel with nothing loaded writes only the section markers", "[misc][cad]") {
    CADModel m;
    Recorder r;
    m.WriteToCAD(&r);
    std::vector<std::string> expect = { "nodehdr", "nodeftr", "wirehdr", "wireftr", "texthdr", "textftr" };
    CHECK(r.log == expect);
}

TEST_CASE("CADModel LoadModel of a null model is ignored", "[misc][cad]") {
    CADModel m;
    m.LoadModel(nullptr, true, true, true);
    m.ShiftModel(1, 2, 3);
    Recorder r;
    m.WriteToCAD(&r);
    CHECK(r.log.size() == 6);
}

TEST_CASE("CAD writer default first and last node callbacks fall back to WriteNode", "[misc][cad]") {
    struct Plain : CADWriter {
        int nodes = 0;
        Plain() : CADWriter("") {}
        void WriteText(CADPoint const&, std::string const&, float, uint32_t, float) override {}
        void WriteWire(CADPoint const&, CADPoint const&, uint32_t) override {}
        void WriteNode(CADPoint const&, uint32_t) override { ++nodes; }
    } p;
    p.WriteFirstNode(CADPoint(), 0);
    p.WriteLastNode(CADPoint(), 0);
    p.WriteNode(CADPoint(), 0);
    CHECK(p.nodes == 3);
    CHECK(p.MapToScreen());
}
