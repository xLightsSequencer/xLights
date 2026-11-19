// LayoutDelta and LayoutMerge: the layout as it travels in a live session,
// compared record by record.

#include <catch2/catch_test_macros.hpp>

#include "collab/CollabLayout.h"
#include "collab/CollabMessages.h"
#include "render/HeadlessRenderContext.h"
#include "utils/FileUtils.h"
#include "utils/ShowFileMerge.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializingVisitor.h"

#include <pugixml.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <functional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "support/TempDir.h"

using namespace Collab;
using nlohmann::json;

namespace {

const std::string kRgb = R"(<?xml version="1.0" encoding="UTF-8"?>
<xrgb>
  <models>
    <model name="Arch" xlId="id-arch" DisplayAs="Arches" StartChannel="1" NumStrings="1" NodesPerString="8" WorldPosX="10.0" Description="">
      <subModel name="Left" xlId="id-arch-left" layout="horizontal" type="ranges" line0="1-4"/>
    </model>
    <model name="Star" xlId="id-star" DisplayAs="Star" StartChannel="100" NumStrings="1" NodesPerString="10"/>
    <model name="Pole" DisplayAs="Single Line" StartChannel="200" NumStrings="1" NodesPerString="5"/>
  </models>
  <modelGroups>
    <modelGroup name="All" xlId="id-all" models="Arch,Star,Pole" layout="minimalGrid"/>
  </modelGroups>
  <view_objects>
    <view_object name="House" xlId="id-house" DisplayAs="Image" WorldPosX="1.0"/>
  </view_objects>
  <views>
    <view name="Front" models="Arch,Star"/>
    <view name="Back" models="Pole"/>
  </views>
  <settings>
    <previewWidth value="1280"/>
  </settings>
  <colors>
    <Grid value="#FF0000"/>
  </colors>
</xrgb>
)";

const std::string kNet = R"(<?xml version="1.0" encoding="UTF-8"?>
<Networks computer="Desk" BaseControllersSyncedTime="0">
  <Controller Name="C1" xlId="ctl-1" Id="1" Protocol="E131" IP="10.0.0.1">
    <network NetworkType="E131" MaxChannels="512" BaudRate="1"/>
  </Controller>
  <Controller Name="C2" xlId="ctl-2" Id="2" Protocol="DDP" IP="10.0.0.2">
    <network NetworkType="DDP" MaxChannels="3000"/>
  </Controller>
  <Controller Name="C3" xlId="ctl-3" Id="3" Protocol="DDP" IP="10.0.0.3">
    <network NetworkType="DDP" MaxChannels="3000"/>
  </Controller>
</Networks>
)";

LayoutFiles Base() {
    return { kRgb, kNet };
}

std::string TextReplace(std::string s, const std::string& a, const std::string& b) {
    auto at = s.find(a);
    REQUIRE(at != std::string::npos);
    s.replace(at, a.size(), b);
    return s;
}

LayoutFiles Rgb(const std::string& a, const std::string& b) {
    return { TextReplace(kRgb, a, b), kNet };
}

LayoutFiles Net(const std::string& a, const std::string& b) {
    return { kRgb, TextReplace(kNet, a, b) };
}

// Every record (and section) of two versions holds the same thing.
void RequireRecordEqual(const LayoutFiles& a, const LayoutFiles& b) {
    CHECK(LayoutDelta::Diff(a, b).is_null());
    CHECK(LayoutDelta::Diff(b, a).is_null());
    CHECK(LayoutDelta::Hash(a) == LayoutDelta::Hash(b));
}

// Apply(a, Diff(a, b)) holds b's records, and the delta only names what changed.
json RoundTrip(const LayoutFiles& a, const LayoutFiles& b) {
    json d = LayoutDelta::Diff(a, b);
    REQUIRE_FALSE(d.is_null());
    CHECK(LayoutDelta::Hash(a) != LayoutDelta::Hash(b));
    auto r = LayoutDelta::Apply(a, d);
    REQUIRE(r.has_value());
    RequireRecordEqual(*r, b);
    // The delta survives the wire.
    json wire = json::parse(d.dump());
    auto r2 = LayoutDelta::Apply(a, wire);
    REQUIRE(r2.has_value());
    CHECK(*r2 == *r);
    CHECK_FALSE(LayoutDelta::Describe(d).empty());
    return d;
}

bool Mentions(const json& d, const std::string& what) {
    for (const auto& s : LayoutDelta::Describe(d)) {
        if (s == what) {
            return true;
        }
    }
    return false;
}

std::string ReadFile(const std::filesystem::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::ostringstream os;
    os << in.rdbuf();
    return os.str();
}

// The xLights-test-data repository: XL_TEST_DATA, else a sibling of any
// folder above this source file.
std::string TestDataShow() {
    std::error_code ec;
    if (const char* env = std::getenv("XL_TEST_DATA")) {
        std::filesystem::path p = std::filesystem::path(env) / "shows" / "basic";
        if (std::filesystem::exists(p / "xlights_rgbeffects.xml", ec)) {
            return p.string();
        }
    }
    for (std::filesystem::path d = std::filesystem::path(__FILE__).parent_path(); !d.empty() && d != d.root_path(); d = d.parent_path()) {
        std::filesystem::path p = d / "xLights-test-data" / "shows" / "basic";
        if (std::filesystem::exists(p / "xlights_rgbeffects.xml", ec)) {
            return p.string();
        }
    }
    return {};
}

// Controllers log to "curl"; a bare test process has no such logger.
struct CurlLogger {
    CurlLogger() {
        if (!spdlog::get("curl")) {
            spdlog::register_logger(std::make_shared<spdlog::logger>("curl", std::make_shared<spdlog::sinks::null_sink_mt>()));
        }
    }
};

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

// The layout file as an app writes it from memory.
std::string Serialize(HeadlessRenderContext& ctx) {
    pugi::xml_document doc;
    doc.append_child(pugi::node_declaration).append_attribute("version") = "1.0";
    pugi::xml_node root = doc.append_child("xrgb");
    XmlSerializingVisitor visitor{ root };
    XmlSerializer::SerializeAllModels(ctx.AllModels, visitor);
    XmlSerializer::SerializeAllObjects(ctx.AllObjects, visitor);
    ctx._sequenceViewManager.Save(visitor);
    XmlSerializer::SerializeAllModelSets(ctx.AllModels.GetSetManager(), visitor);
    std::ostringstream out;
    doc.save(out, "  ");
    return out.str();
}

} // namespace

TEST_CASE("Layout delta: no change is a null delta", "[collab][layout]") {
    CHECK(LayoutDelta::Diff(Base(), Base()).is_null());
    CHECK(LayoutDelta::Describe(nullptr).empty());
    auto same = LayoutDelta::Apply(Base(), nullptr);
    REQUIRE(same);
    CHECK(*same == Base());
    CHECK(LayoutDelta::Hash(Base()) != 0);
}

TEST_CASE("Layout delta: ids attribute order whitespace numbers and the saving computer are not changes", "[collab][layout]") {
    LayoutFiles b = Base();
    b.rgbEffects = TextReplace(b.rgbEffects, "WorldPosX=\"10.0\"", "WorldPosX=\"10.000000\"");
    b.rgbEffects = TextReplace(b.rgbEffects, "StartChannel=\"100\" NumStrings=\"1\"", "NumStrings=\"1\"   StartChannel=\"100\"");
    b.rgbEffects = TextReplace(b.rgbEffects, "  <view_objects>\n", "<view_objects>");
    b.networks = TextReplace(b.networks, "computer=\"Desk\"", "computer=\"Laptop\"");
    RequireRecordEqual(Base(), b);
    CHECK(LayoutDelta::Equivalent(Base(), b));

    // Without an id a record is matched by name and holds the same thing,
    // but memory loaded from it can't stand in for the other.
    LayoutFiles noId = Rgb(" xlId=\"id-star\"", "");
    CHECK(LayoutDelta::Diff(Base(), noId).is_null());
    CHECK_FALSE(LayoutDelta::Equivalent(Base(), noId));
    // Another id is another record.
    LayoutFiles otherId = Rgb("xlId=\"id-star\"", "xlId=\"id-star-elsewhere\"");
    json d = LayoutDelta::Diff(Base(), otherId);
    REQUIRE_FALSE(d.is_null());
    CHECK(d.dump().find("\"del\"") != std::string::npos);
}

TEST_CASE("Layout delta: every kind of edit round-trips", "[collab][layout]") {
    SECTION("attribute") {
        json d = RoundTrip(Base(), Rgb("NodesPerString=\"10\"", "NodesPerString=\"12\""));
        CHECK(Mentions(d, "Model 'Star'"));
        // only the changed record travels
        CHECK(d.dump().find("Arch") == std::string::npos);
    }
    SECTION("child") {
        json d = RoundTrip(Base(), Rgb("line0=\"1-4\"", "line0=\"1-5\""));
        CHECK(Mentions(d, "Model 'Arch'"));
    }
    SECTION("add") {
        json d = RoundTrip(Base(), Rgb("  </models>", "  <model name=\"Tree\" xlId=\"id-tree\" DisplayAs=\"Tree 360\" StartChannel=\"300\"/>\n  </models>"));
        CHECK(Mentions(d, "Model 'Tree'"));
    }
    SECTION("remove") {
        json d = RoundTrip(Base(), Rgb("<model name=\"Star\" xlId=\"id-star\" DisplayAs=\"Star\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"10\"/>", ""));
        CHECK(Mentions(d, "Model 'Star'"));
    }
    SECTION("remove a record without an id") {
        RoundTrip(Base(), Rgb("<model name=\"Pole\" DisplayAs=\"Single Line\" StartChannel=\"200\" NumStrings=\"1\" NodesPerString=\"5\"/>", ""));
    }
    SECTION("rename") {
        LayoutFiles b = Rgb("name=\"Star\" xlId=\"id-star\"", "name=\"Big Star\" xlId=\"id-star\"");
        b.rgbEffects = TextReplace(b.rgbEffects, "models=\"Arch,Star,Pole\"", "models=\"Arch,Big Star,Pole\"");
        json d = RoundTrip(Base(), b);
        // a rename is a change of the record, not a delete and an add
        REQUIRE(d["rgb"]["s"].is_array());
        bool deleted = d.dump().find("\"del\"") != std::string::npos;
        CHECK_FALSE(deleted);
        CHECK(Mentions(d, "Model 'Big Star'"));
        CHECK(Mentions(d, "Group 'All'"));
    }
    SECTION("rename without an id") {
        RoundTrip(Base(), Rgb("name=\"Pole\" DisplayAs", "name=\"Pole 2\" DisplayAs"));
    }
    SECTION("group members and their order") {
        RoundTrip(Base(), Rgb("models=\"Arch,Star,Pole\"", "models=\"Pole,Arch\""));
    }
    SECTION("view objects") {
        RoundTrip(Base(), Rgb("WorldPosX=\"1.0\"", "WorldPosX=\"2.5\""));
    }
    SECTION("views and their order") {
        RoundTrip(Base(), Rgb("<view name=\"Front\" models=\"Arch,Star\"/>\n    <view name=\"Back\" models=\"Pole\"/>",
                              "<view name=\"Back\" models=\"Pole\"/>\n    <view name=\"Front\" models=\"Arch,Star\"/>\n    <view name=\"Side\" models=\"Arch\"/>"));
    }
    SECTION("settings and colours") {
        json d = RoundTrip(Base(), Rgb("value=\"1280\"", "value=\"1920\""));
        CHECK(Mentions(d, "settings"));
        RoundTrip(Base(), Rgb("#FF0000", "#00FF00"));
    }
    SECTION("a section added and one removed") {
        LayoutFiles b = Rgb("  <colors>\n    <Grid value=\"#FF0000\"/>\n  </colors>\n", "  <perspectives current=\"P\"><perspective name=\"P\"/></perspectives>\n");
        b.rgbEffects = TextReplace(b.rgbEffects, "  <view_objects>\n    <view_object name=\"House\" xlId=\"id-house\" DisplayAs=\"Image\" WorldPosX=\"1.0\"/>\n  </view_objects>\n", "");
        RoundTrip(Base(), b);
    }
    SECTION("controller edit") {
        json d = RoundTrip(Base(), Net("IP=\"10.0.0.2\"", "IP=\"10.0.0.22\""));
        CHECK(Mentions(d, "Controller 'C2'"));
        CHECK_FALSE(d.contains("rgb"));
    }
    SECTION("controller output child") {
        RoundTrip(Base(), Net("MaxChannels=\"512\"", "MaxChannels=\"1024\""));
    }
    SECTION("controller add rename and remove") {
        LayoutFiles b = Net("Name=\"C1\"", "Name=\"Front Yard\"");
        b.networks = TextReplace(b.networks, "  <Controller Name=\"C3\" xlId=\"ctl-3\" Id=\"3\" Protocol=\"DDP\" IP=\"10.0.0.3\">\n    <network NetworkType=\"DDP\" MaxChannels=\"3000\"/>\n  </Controller>\n", "");
        b.networks = TextReplace(b.networks, "</Networks>", "  <Controller Name=\"C4\" xlId=\"ctl-4\" Id=\"4\"/>\n</Networks>");
        json d = RoundTrip(Base(), b);
        CHECK(Mentions(d, "Controller 'Front Yard'"));
        CHECK(Mentions(d, "Controller 'C3'"));
        CHECK(Mentions(d, "Controller 'C4'"));
    }
    SECTION("controller order") {
        LayoutFiles b = Base();
        std::string c1 = "  <Controller Name=\"C1\" xlId=\"ctl-1\" Id=\"1\" Protocol=\"E131\" IP=\"10.0.0.1\">\n    <network NetworkType=\"E131\" MaxChannels=\"512\" BaudRate=\"1\"/>\n  </Controller>\n";
        b.networks = TextReplace(b.networks, c1, "");
        b.networks = TextReplace(b.networks, "</Networks>", c1 + "</Networks>");
        json d = RoundTrip(Base(), b);
        CHECK(d["net"]["s"][0].contains("order"));
        // the order is part of the hash, so a peer that lost it is found out
        auto r = LayoutDelta::Apply(Base(), d);
        REQUIRE(r);
        pugi::xml_document doc;
        REQUIRE(doc.load_string(r->networks.c_str()));
        CHECK(std::string(doc.document_element().first_child().attribute("Name").value()) == "C2");
    }
    SECTION("an added controller goes where it was put") {
        RoundTrip(Base(), Net("  <Controller Name=\"C2\"", "  <Controller Name=\"New\" xlId=\"ctl-new\" Id=\"9\"/>\n  <Controller Name=\"C2\""));
    }
    SECTION("networks file attributes") {
        RoundTrip(Base(), Net("BaseControllersSyncedTime=\"0\"", "BaseControllersSyncedTime=\"5\""));
    }
    SECTION("a file that appears or goes") {
        RoundTrip(Base(), { kRgb, "" });
        RoundTrip({ kRgb, "" }, Base());
    }
    SECTION("many edits at once") {
        LayoutFiles b = Rgb("NodesPerString=\"10\"", "NodesPerString=\"11\"");
        b.rgbEffects = TextReplace(b.rgbEffects, "line0=\"1-4\"", "line0=\"2-4\"");
        b.rgbEffects = TextReplace(b.rgbEffects, "#FF0000", "#0000FF");
        b.networks = TextReplace(b.networks, "IP=\"10.0.0.3\"", "IP=\"10.0.0.33\"");
        RoundTrip(Base(), b);
    }
}

TEST_CASE("Layout delta: a delta that does not fit gives nullopt", "[collab][layout]") {
    json d = LayoutDelta::Diff(Base(), Rgb("NodesPerString=\"10\"", "NodesPerString=\"12\""));
    REQUIRE_FALSE(d.is_null());
    // Star is gone: its change has nowhere to go.
    LayoutFiles without = Rgb("<model name=\"Star\" xlId=\"id-star\" DisplayAs=\"Star\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"10\"/>", "");
    CHECK_FALSE(LayoutDelta::Apply(without, d).has_value());
    // Adding a record that is already there.
    LayoutFiles withTree = Rgb("  </models>", "  <model name=\"Tree\" xlId=\"id-tree\"/>\n  </models>");
    json add = LayoutDelta::Diff(Base(), withTree);
    CHECK_FALSE(LayoutDelta::Apply(withTree, add).has_value());
}

TEST_CASE("Layout delta: malformed deltas give nullopt and never throw", "[collab][layout]") {
    const std::vector<std::string> bad = {
        "[]",
        "{}",
        "42",
        "{\"x\":{}}",
        "{\"rgb\":[]}",
        "{\"rgb\":{\"s\":{}}}",
        "{\"rgb\":{\"s\":[1]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"nope\"}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"models\",\"chg\":[{\"k\":\"i:id-star\"}]}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"models\",\"chg\":[{\"k\":\"i:id-star\",\"x\":\"<a><b/>\"}]}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"models\",\"chg\":[{\"k\":\"i:id-star\",\"x\":\"<a/><b/>\"}]}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"models\",\"chg\":[{\"k\":\"i:missing\",\"x\":\"<model name='X'/>\"}]}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"models\",\"del\":[{\"k\":5}]}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"models\",\"del\":\"all\"}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"models\",\"add\":[{\"x\":\"<model name='Arch' xlId='id-arch'/>\"}]}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"views\",\"order\":[\"n:view:Front\"]}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"views\",\"order\":[\"n:view:Front\",\"n:view:Front\"]}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"views\",\"order\":\"n:view:Front\"}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"models\",\"a\":{\"x\":1}}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"modelSets\",\"drop\":true}]}}",
        "{\"rgb\":{\"s\":[{\"s\":\"models\",\"new\":\"<views/>\"}]}}",
        "{\"rgb\":{\"root\":[\"a\"]}}",
        "{\"rgb\":{\"root\":{\"a\":null}}}",
        "{\"rgb\":{\"w\":[{\"k\":\"settings#0\"}]}}",
        "{\"rgb\":{\"w\":[{\"k\":\"settings#3\",\"del\":true}]}}",
        "{\"rgb\":{\"w\":[{\"k\":\"colors#0\",\"x\":\"<settings/>\"}]}}",
        "{\"rgb\":{\"w\":[{\"k\":\"models#0\",\"x\":\"<models/>\"}]}}",
        "{\"rgb\":{\"file\":7}}",
        "{\"rgb\":{\"file\":\"x\",\"root\":{}}}",
        "{\"rgb\":{\"zzz\":1}}",
        "{\"net\":{\"s\":[{\"add\":[{\"x\":\"<model name='X'/>\"}]}]}}",
        "{\"net\":{\"s\":[{\"s\":\"models\"}]}}",
        "{\"net\":{\"s\":[{\"order\":[\"i:ctl-1\"]}]}}",
        "{\"net\":{\"s\":[{\"chg\":[{\"k\":\"i:ctl-1\",\"x\":\"<Controller Name='C2' xlId='ctl-2'/>\"}]}]}}",
    };
    for (const auto& text : bad) {
        INFO(text);
        json d = json::parse(text, nullptr, false);
        REQUIRE_FALSE(d.is_discarded());
        std::optional<LayoutFiles> r;
        CHECK_NOTHROW(r = LayoutDelta::Apply(Base(), d));
        CHECK_FALSE(r.has_value());
        CHECK_NOTHROW(LayoutDelta::Describe(d));
    }
    // Files that don't parse: a delta against them doesn't fit; Diff and Hash still work.
    LayoutFiles broken{ "<xrgb><models>", "not xml" };
    CHECK_NOTHROW(LayoutDelta::Hash(broken));
    json toBroken = LayoutDelta::Diff(Base(), broken);
    auto r = LayoutDelta::Apply(Base(), toBroken);
    REQUIRE(r);
    CHECK(*r == broken);
    json d = LayoutDelta::Diff(Base(), Rgb("NodesPerString=\"10\"", "NodesPerString=\"12\""));
    CHECK_FALSE(LayoutDelta::Apply(broken, d).has_value());
}

TEST_CASE("Layout delta: every model in the test data show round-trips", "[collab][layout][testdata]") {
    std::string dir = TestDataShow();
    if (dir.empty()) {
        SKIP("xLights-test-data not found (set XL_TEST_DATA)");
    }
    LayoutFiles disk{ ReadFile(std::filesystem::path(dir) / "xlights_rgbeffects.xml"), ReadFile(std::filesystem::path(dir) / "xlights_networks.xml") };
    pugi::xml_document doc;
    REQUIRE(doc.load_string(disk.rgbEffects.c_str()));
    std::set<std::string> types;
    int models = 0;
    for (pugi::xml_node m : doc.document_element().child("models").children("model")) {
        std::string name = m.attribute("name").value();
        types.insert(m.attribute("DisplayAs").value());
        ++models;
        INFO(name << " (" << m.attribute("DisplayAs").value() << ")");
        auto edited = [&](const std::function<void(pugi::xml_node)>& edit) {
            pugi::xml_document d;
            REQUIRE(d.load_string(disk.rgbEffects.c_str()));
            for (pugi::xml_node n : d.document_element().child("models").children("model")) {
                if (name == n.attribute("name").value()) {
                    edit(n);
                    break;
                }
            }
            std::ostringstream os;
            d.save(os, "  ");
            return LayoutFiles{ os.str(), disk.networks };
        };
        RoundTrip(disk, edited([](pugi::xml_node n) { n.attribute("name").set_value((std::string(n.attribute("name").value()) + " renamed").c_str()); }));
        RoundTrip(disk, edited([](pugi::xml_node n) {
            pugi::xml_attribute a = n.attribute("Description");
            if (!a) {
                a = n.append_attribute("Description");
            }
            a.set_value("edited");
        }));
        RoundTrip(disk, edited([](pugi::xml_node n) { n.parent().remove_child(n); }));
        // and back again
        LayoutFiles gone = edited([](pugi::xml_node n) { n.parent().remove_child(n); });
        RoundTrip(gone, disk);
    }
    CHECK(models > 0);
    INFO(types.size() << " model types");

    // Memory's serialization of the show against the files on disk, both ways.
    CurlLogger curl;
    FixFileReset reset;
    TempDir tmp;
    tmp.Write("xlights_rgbeffects.xml", disk.rgbEffects);
    tmp.Write("xlights_networks.xml", disk.networks);
    HeadlessRenderContext ctx;
    REQUIRE(ctx.LoadShowFolder(tmp.path.string()));
    LayoutFiles memory{ Serialize(ctx), disk.networks };
    json d = LayoutDelta::Diff(disk, memory);
    if (!d.is_null()) {
        auto r = LayoutDelta::Apply(disk, d);
        REQUIRE(r);
        RequireRecordEqual(*r, memory);
    }
    json back = LayoutDelta::Diff(memory, disk);
    if (!back.is_null()) {
        auto r = LayoutDelta::Apply(memory, back);
        REQUIRE(r);
        RequireRecordEqual(*r, disk);
    }
}

TEST_CASE("Layout merge: the later proposal wins and keeps the other's changes", "[collab][layout]") {
    LayoutFiles base = Base();
    SECTION("different attributes of one model both survive") {
        LayoutFiles theirs = Rgb("NodesPerString=\"10\"", "NodesPerString=\"11\"");
        LayoutFiles mine = Rgb("StartChannel=\"100\"", "StartChannel=\"101\"");
        auto o = LayoutMerge::Merge(base, mine, theirs, LayoutMerge::Policy::Proposal);
        REQUIRE(o.ok);
        LayoutFiles both = Rgb("StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"10\"", "StartChannel=\"101\" NumStrings=\"1\" NodesPerString=\"11\"");
        RequireRecordEqual(o.result, both);
    }
    SECTION("the same attribute: mine") {
        LayoutFiles theirs = Rgb("NodesPerString=\"10\"", "NodesPerString=\"11\"");
        LayoutFiles mine = Rgb("NodesPerString=\"10\"", "NodesPerString=\"12\"");
        auto o = LayoutMerge::Merge(base, mine, theirs, LayoutMerge::Policy::Proposal);
        REQUIRE(o.ok);
        RequireRecordEqual(o.result, mine);
        auto t = LayoutMerge::Merge(base, mine, theirs, LayoutMerge::Policy::Theirs);
        REQUIRE(t.ok);
        RequireRecordEqual(t.result, theirs);
    }
    SECTION("they deleted what I changed: it stays deleted") {
        LayoutFiles theirs = Rgb("<model name=\"Star\" xlId=\"id-star\" DisplayAs=\"Star\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"10\"/>", "");
        LayoutFiles mine = Rgb("NodesPerString=\"10\"", "NodesPerString=\"12\"");
        auto o = LayoutMerge::Merge(base, mine, theirs, LayoutMerge::Policy::Proposal);
        REQUIRE(o.ok);
        RequireRecordEqual(o.result, theirs);
        CHECK(o.dropped == std::vector<std::string>{ "Star" });
    }
    SECTION("I deleted what they changed: it goes") {
        LayoutFiles theirs = Rgb("NodesPerString=\"10\"", "NodesPerString=\"12\"");
        LayoutFiles mine = Rgb("<model name=\"Star\" xlId=\"id-star\" DisplayAs=\"Star\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"10\"/>", "");
        auto o = LayoutMerge::Merge(base, mine, theirs, LayoutMerge::Policy::Proposal);
        REQUIRE(o.ok);
        RequireRecordEqual(o.result, mine);
        CHECK(o.dropped.empty());
    }
    SECTION("both added one name: the proposal is refused") {
        LayoutFiles theirs = Rgb("  </models>", "  <model name=\"Tree\" xlId=\"id-t1\" DisplayAs=\"Tree 360\"/>\n  </models>");
        LayoutFiles mine = Rgb("  </models>", "  <model name=\"Tree\" xlId=\"id-t2\" DisplayAs=\"Matrix\"/>\n  </models>");
        auto o = LayoutMerge::Merge(base, mine, theirs, LayoutMerge::Policy::Proposal);
        CHECK_FALSE(o.ok);
        CHECK(o.reason == Reason::NameTaken);
        CHECK(o.text.find("Tree") != std::string::npos);
        auto t = LayoutMerge::Merge(base, mine, theirs, LayoutMerge::Policy::Theirs);
        REQUIRE(t.ok);
        RequireRecordEqual(t.result, theirs);
    }
    SECTION("a controller renamed on one side, a model wired to it on the other") {
        LayoutFiles theirs = Net("Name=\"C1\"", "Name=\"Front\"");
        theirs.rgbEffects = TextReplace(theirs.rgbEffects, "name=\"Arch\" xlId=\"id-arch\"", "name=\"Arch\" xlId=\"id-arch\" Controller=\"Front\"");
        LayoutFiles mine = Rgb("name=\"Star\" xlId=\"id-star\"", "name=\"Star\" xlId=\"id-star\" Controller=\"C1\"");
        auto o = LayoutMerge::Merge(base, mine, theirs, LayoutMerge::Policy::Proposal);
        REQUIRE(o.ok);
        CHECK(o.result.rgbEffects.find("Controller=\"C1\"") == std::string::npos);
        CHECK(o.result.networks.find("Name=\"Front\"") != std::string::npos);
    }
}

TEST_CASE("Layout messages: live layout fields survive the wire and bad ones are refused", "[collab][layout][protocol]") {
    auto round = [](const Message& m) {
        auto d = DecodeMessage(EncodeMessage(m));
        REQUIRE(d.has_value());
        return *d;
    };
    Message hello;
    hello.type = MsgType::Hello;
    hello.liveLayout = true;
    CHECK(round(hello).liveLayout);

    Message welcome;
    welcome.type = MsgType::Welcome;
    welcome.participant = 3;
    welcome.liveLayout = true;
    welcome.layoutFiles = Base();
    welcome.layoutHash = LayoutDelta::Hash(Base());
    Message w = round(welcome);
    CHECK(w.liveLayout);
    CHECK(w.layoutFiles == Base());
    CHECK(w.layoutHash == welcome.layoutHash);

    Message commit;
    commit.type = MsgType::Commit;
    commit.rev = 7;
    commit.participant = 2;
    commit.layout = LayoutDelta::Diff(Base(), Rgb("NodesPerString=\"10\"", "NodesPerString=\"12\""));
    commit.layoutHash = 99;
    commit.text = "'Star' was deleted";
    Message c = round(commit);
    CHECK(c.layout == commit.layout);
    CHECK(c.command.is_null());
    CHECK(c.layoutHash == 99);
    CHECK(c.text == commit.text);

    Message presence;
    presence.type = MsgType::Presence;
    presence.participant = 2;
    presence.presence.layoutSelected = { "id-arch", "Star" };
    presence.presence.layoutLock = { "id-arch", "Arch", "submodels" };
    presence.lockOrder = 5;
    Message p = round(presence);
    CHECK(p.presence == presence.presence);
    CHECK(p.lockOrder == 5);

    Message notice;
    notice.type = MsgType::Notice;
    notice.reason = Reason::LockTaken;
    notice.text = "Ari started editing Arch first";
    Message n = round(notice);
    CHECK(n.reason == notice.reason);
    CHECK(n.text == notice.text);

    Message ping;
    ping.type = MsgType::Ping;
    ping.layoutHash = 1234;
    CHECK(round(ping).layoutHash == 1234);

    // A proposal is a command or a layout edit, never both or an empty one.
    for (const char* bad : {
             R"({"v":1,"t":"Propose","seq":1,"base":0,"kind":"edit","undoOf":0,"cmd":{"type":"Batch"},"layout":{"rgb":{}}})",
             R"({"v":1,"t":"Propose","seq":1,"base":0,"kind":"edit","undoOf":0,"layout":{}})",
             R"({"v":1,"t":"Propose","seq":1,"base":0,"kind":"edit","undoOf":0,"layout":[1]})",
             R"({"v":1,"t":"Commit","rev":1,"seq":1,"author":1,"hash":0,"kind":"edit","undoOf":0,"layout":{"rgb":{}},"lhash":"x"})",
             R"({"v":1,"t":"Presence","participant":2,"presence":{"lsel":[1]}})",
             R"({"v":1,"t":"Presence","participant":2,"presence":{"llock":"Arch"}})",
             R"({"v":1,"t":"Hello","live":"yes"})",
             R"({"v":1,"t":"Welcome","participant":2,"rev":0,"lrgb":5})",
         }) {
        INFO(bad);
        CHECK_FALSE(DecodeMessage(bad).has_value());
    }
}
