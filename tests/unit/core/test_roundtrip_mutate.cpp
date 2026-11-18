#include <set>

#include <catch2/catch_approx.hpp>

#include "test_roundtrip_common.h"

using namespace rt;

namespace {

// Rewrites every numeric attribute of a serialized default model (and of its child elements) to a
// different, non-default value, then checks the loader/writer pair preserves it.
std::string MutateValue(const std::string& name, const std::string& v) {
    if (v.empty()) return v;
    char* end = nullptr;
    double d = std::strtod(v.c_str(), &end);
    if (*end != 0) return v;
    bool isInt = v.find('.') == std::string::npos && v.find('e') == std::string::npos;
    if (isInt) {
        long l = (long)d;
        if (name == "versionNumber" || name == "StartChannel") return v;
        return std::to_string(l == 0 ? 1 : l + 1);
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f", d + 0.5);
    return buf;
}

int MutateNode(pugi::xml_node n, const std::set<std::string>& skip) {
    int changed = 0;
    for (auto a = n.first_attribute(); a; a = a.next_attribute()) {
        std::string nm = a.name();
        if (skip.count(nm)) continue;
        std::string nv = MutateValue(nm, a.value());
        if (nv != a.value()) {
            a.set_value(nv.c_str());
            ++changed;
        }
    }
    for (auto c : n.children()) {
        changed += MutateNode(c, skip);
    }
    return changed;
}

void Walk(const pugi::xml_node& in, const pugi::xml_node& out, const std::string& path, std::vector<std::string>& bad, const std::set<std::string>& skip) {
    for (auto a = in.first_attribute(); a; a = a.next_attribute()) {
        if (skip.count(a.name())) continue;
        auto o = out.attribute(a.name());
        if (!o) {
            bad.push_back(path + "@" + a.name() + " dropped (was " + a.value() + ")");
        } else if (!Same(o.value(), a.value())) {
            bad.push_back(path + "@" + a.name() + " " + a.value() + " -> " + o.value());
        }
    }
    for (auto c : in.children()) {
        auto oc = out.child(c.name());
        if (!oc) {
            bad.push_back(path + "/" + c.name() + " dropped");
            continue;
        }
        Walk(c, oc, path + "/" + c.name(), bad, skip);
    }
}

void CheckMutated(const char* type, const std::set<std::string>& skip = {}) {
    ShowFixture a;
    Model* m = a.mm.CreateDefaultModel(type, "1");
    REQUIRE(m != nullptr);
    pugi::xml_document base;
    REQUIRE(base.load_string(Ser(m).c_str()));
    delete m;
    // Skipped: identity fields, values the loader legitimately clamps/derives (flags, brightness, mesh sizes from the OBJ file,
    // point counts tied to PointData...) and TagColour/PivotOffset which have their own known-bug tests below.
    std::set<std::string> alwaysSkip = { "name", "DisplayAs", "versionNumber", "SourceVersion", "StartChannel", "Controller", "TagColour",
        "Bits16", "HasColor", "HasEyeLR", "HasEyeUD", "HasJaw", "HasNod", "HasPan", "HasTilt", "DmxColorType", "Shear", "LayerSizes", "NumPoints",
        "PointData", "Width", "Height", "Depth", "Brightness", "TreeDegrees", "MaxLimit", "PivotOffsetX", "PivotOffsetY", "PivotOffsetZ" };
    alwaysSkip.insert(skip.begin(), skip.end());
    int changed = MutateNode(base.first_child(), alwaysSkip);
    CHECK(changed > 3);
    std::string mutated = Canon(base.first_child());

    ShowFixture b;
    Model* m1 = b.Add(mutated);
    REQUIRE(m1 != nullptr);
    std::string s1 = Ser(m1);
    pugi::xml_document d1;
    REQUIRE(d1.load_string(s1.c_str()));
    std::vector<std::string> bad;
    Walk(base.first_child(), d1.first_child(), "model", bad, alwaysSkip);
    for (auto& s : bad) {
        INFO(s);
        CHECK(s.empty());
    }

    ShowFixture c;
    Model* m2 = c.Add(s1);
    REQUIRE(m2 != nullptr);
    std::string s2 = Ser(m2);
    INFO("save1 vs save2:" << DiffAttrs(s1, s2));
    CHECK(s2 == s1);
    ShowFixture d;
    Model* m3 = d.Add(s2);
    REQUIRE(m3 != nullptr);
    CHECK(Ser(m3) == s2);
}

} // namespace

TEST_CASE("Roundtrip servo pivot offset does not drift", "[roundtrip][mutate]") {
    ShowFixture a;
    Model* m = a.mm.CreateDefaultModel("DmxServo", "1");
    REQUIRE(m != nullptr);
    pugi::xml_document base;
    REQUIRE(base.load_string(Ser(m).c_str()));
    delete m;
    base.first_child().child("Servo1").attribute("PivotOffsetX").set_value("5.0");
    std::string cur = Canon(base.first_child());
    for (int i = 0; i < 3; ++i) {
        ShowFixture f;
        Model* mm = f.Add(cur);
        REQUIRE(mm != nullptr);
        cur = Ser(mm);
        pugi::xml_document d;
        REQUIRE(d.load_string(cur.c_str()));
        INFO("cycle " << i);
        CHECK(d.first_child().child("Servo1").attribute("PivotOffsetX").as_float() == Catch::Approx(5.0f));
    }
}

#define MUTATE_CASE(NAME, TYPE, ...) \
    TEST_CASE("Roundtrip every attribute survives mutation: " NAME, "[roundtrip][mutate]") { CheckMutated(TYPE, { __VA_ARGS__ }); }

MUTATE_CASE("Star", "Star")
MUTATE_CASE("Arches", "Arches")
MUTATE_CASE("Candy Canes", "Candy Canes")
MUTATE_CASE("Channel Block", "Channel Block")
MUTATE_CASE("Circle", "Circle")
MUTATE_CASE("DmxMovingHead", "DmxMovingHead")
MUTATE_CASE("DmxGeneral", "DmxGeneral")
MUTATE_CASE("DmxMovingHeadAdv", "DmxMovingHeadAdv")
MUTATE_CASE("DmxFloodlight", "DmxFloodlight")
MUTATE_CASE("DmxFloodArea", "DmxFloodArea")
MUTATE_CASE("DmxSkull", "DmxSkull")
MUTATE_CASE("DmxServo", "DmxServo", "DmxChannelCount")
MUTATE_CASE("DmxServo3d", "DmxServo3d", "DmxChannelCount")
MUTATE_CASE("Image", "Image")
MUTATE_CASE("Label", "Label")
MUTATE_CASE("Window Frame", "Window Frame")
MUTATE_CASE("Wreath", "Wreath")
MUTATE_CASE("Sphere", "Sphere")
MUTATE_CASE("Single Line", "Single Line")
MUTATE_CASE("Poly Line", "Poly Line", "NodesPerString")
MUTATE_CASE("MultiPoint", "MultiPoint")
MUTATE_CASE("Cube", "Cube")
MUTATE_CASE("Custom", "Custom")
MUTATE_CASE("Tree 360", "Tree 360")
MUTATE_CASE("Matrix", "Matrix")
MUTATE_CASE("Spinner", "Spinner")
MUTATE_CASE("Icicles", "Icicles")
