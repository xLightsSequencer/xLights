#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>
#include <pugixml.hpp>

#include "utils/LayoutObjectId.h"
#include "utils/LayoutRecords.h"
#include "utils/LayoutRenameLog.h"
#include "utils/ShowFileMerge.h"

// Three-way merge of the show-wide files (utils/ShowFileMerge.h): base is the
// file as this process loaded it, mine what it wants to save, theirs what is
// on disk now. These cases build the three versions as XML/JSON text.
using namespace ShowFileMerge;

namespace {

const char* kA = "aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa";
const char* kB = "bbbbbbbb-bbbb-4bbb-8bbb-bbbbbbbbbbbb";
const char* kC = "cccccccc-cccc-4ccc-8ccc-cccccccccccc";
const char* kD = "dddddddd-dddd-4ddd-8ddd-dddddddddddd";
const char* kG = "99999999-9999-4999-8999-999999999999";

std::string Id(const std::string& id) {
    return id.empty() ? std::string() : " xlId=\"" + id + "\"";
}

std::string Model(const std::string& name, const std::string& id = "", const std::string& attrs = "", const std::string& children = "") {
    std::string start = attrs.find("StartChannel=") == std::string::npos ? " StartChannel=\"1\"" : "";
    return "<model name=\"" + name + "\" DisplayAs=\"Single Line\"" + start + " NodesPerString=\"10\"" + Id(id) + " " + attrs + ">" +
           children + "</model>";
}

std::string Sub(const std::string& name, const std::string& range = "1-5", const std::string& id = "") {
    return "<subModel name=\"" + name + "\" layout=\"horizontal\" type=\"ranges\" line0=\"" + range + "\"" + Id(id) + "/>";
}

std::string Group(const std::string& name, const std::string& members, const std::string& id = "", const std::string& attrs = "") {
    return "<modelGroup name=\"" + name + "\" models=\"" + members + "\" layout=\"minimalGrid\"" + Id(id) + " " + attrs + "/>";
}

std::string Rgb(const std::string& models, const std::string& groups = "", const std::string& extra = "") {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models type=\"rgb_effects\">" + models + "</models><modelGroups type=\"rgb_effects\">" +
           groups + "</modelGroups>" + extra + "</xrgb>";
}

std::string Controller(const std::string& name, const std::string& id = "", const std::string& attrs = "", const std::string& outputs = "") {
    std::string out = outputs.empty() ? "<network NetworkType=\"DDP\" MaxChannels=\"512\"/>" : outputs;
    return "<Controller Name=\"" + name + "\" Type=\"Ethernet\" IP=\"10.0.0.1\"" + Id(id) + " " + attrs + ">" + out + "</Controller>";
}

std::string Networks(const std::string& controllers, const std::string& attrs = "") {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks computer=\"one\" " + attrs + ">" + controllers + "</Networks>";
}

struct Doc {
    pugi::xml_document d;
    explicit Doc(const std::string& text) { REQUIRE(d.load_string(text.c_str())); }
    pugi::xml_node Root() const { return d.document_element(); }
    pugi::xml_node M(const std::string& name) const { return Root().child("models").find_child_by_attribute("model", "name", name.c_str()); }
    pugi::xml_node G(const std::string& name) const {
        return Root().child("modelGroups").find_child_by_attribute("modelGroup", "name", name.c_str());
    }
    pugi::xml_node C(const std::string& name) const { return Root().find_child_by_attribute("Controller", "Name", name.c_str()); }
    std::vector<std::string> ModelNames() const {
        std::vector<std::string> out;
        for (auto m : Root().child("models").children("model")) out.push_back(m.attribute("name").value());
        return out;
    }
    std::vector<std::string> ControllerNames() const {
        std::vector<std::string> out;
        for (auto c : Root().children("Controller")) out.push_back(c.attribute("Name").value());
        return out;
    }
};

Doc Result(const Merge& m) {
    REQUIRE(m.Ok());
    return Doc(m.ResultText());
}

Merge Rgbx(const std::string& b, const std::string& m, const std::string& t, const Options& o = {}) {
    Merge r = Merge::Run(FileKind::RgbEffects, b, m, t, o);
    REQUIRE(r.Ok());
    return r;
}

Conflict Only(const Merge& m) {
    REQUIRE(m.Conflicts().size() == 1);
    return m.Conflicts().front();
}

std::string Attr(pugi::xml_node n, const char* a) {
    return n.attribute(a).value();
}

} // namespace

// --- fast paths and what is not a change --------------------------------------

TEST_CASE("Merge: theirs unchanged gives mine byte for byte", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA));
    std::string mine = "<?xml version=\"1.0\"?>\n<xrgb>\n\t<models><model name=\"A\" xlId=\"" + std::string(kA) +
                       "\" DisplayAs=\"Single Line\" StartChannel=\"9\"/></models>\n</xrgb>\n";
    Merge m = Rgbx(base, mine, base);
    CHECK(m.TheirsUnchanged());
    CHECK(m.Conflicts().empty());
    CHECK(m.ResultText() == mine);
}

TEST_CASE("Merge: mine unchanged gives theirs byte for byte", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA));
    std::string theirs = Rgb(Model("A", kA, "Brightness=\"50\""), "", "<odd/>") + "\n";
    Merge m = Rgbx(base, base, theirs);
    CHECK(m.MineUnchanged());
    CHECK(m.ResultText() == theirs);
}

TEST_CASE("Merge: ids, attribute order, whitespace and number format are not changes", "[merge][showmerge]") {
    std::string base = "<xrgb><models><model name=\"A\" xlId=\"" + std::string(kA) + "\" X=\"6.0\" Y=\"2\"><subModel name=\"S\" xlId=\"" +
                       kB + "\" line0=\"1-2\"/></model></models></xrgb>";
    // an older desktop: no ids, its own formatting
    std::string theirs = "<xrgb>\n  <models>\n    <model Y=\"2\" X=\"6.000000\" name=\"A\">\n      <subModel line0=\"1-2\" name=\"S\"/>\n    </model>\n  "
                         "</models>\n</xrgb>";
    std::string mine = Rgb(Model("A", kA, "Brightness=\"10\""));
    Merge m = Rgbx(base, mine, theirs);
    CHECK(m.TheirsUnchanged());
    CHECK(m.ResultText() == mine);

    // a real number change is still a change
    std::string theirs2 = "<xrgb><models><model name=\"A\" X=\"6.5\" Y=\"2\"><subModel name=\"S\" line0=\"1-2\"/></model></models></xrgb>";
    CHECK_FALSE(Rgbx(base, mine, theirs2).TheirsUnchanged());

    // large integers (channel numbers) compare exactly; "007" is 7
    auto with = [](const std::string& v) { return "<xrgb><models><model name=\"A\" StartChannel=\"" + v + "\"/></models></xrgb>"; };
    CHECK_FALSE(Rgbx(with("1000001"), mine, with("1000002")).TheirsUnchanged());
    CHECK(Rgbx(with("7"), mine, with("007")).TheirsUnchanged());
}

TEST_CASE("Merge: a version that does not parse fails the merge", "[merge][showmerge]") {
    Merge m = Merge::Run(FileKind::RgbEffects, Rgb(""), "<xrgb><models>", Rgb(""));
    CHECK_FALSE(m.Ok());
    CHECK(m.Error().find("mine") != std::string::npos);
    Merge p = Merge::Run(FileKind::Presets, "{}", "{}", "not json");
    CHECK_FALSE(p.Ok());
}

TEST_CASE("Merge: the saving version and position rounding are not changes", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA, "SourceVersion=\"2026.01\" WorldPosX=\"10.0001\""));
    // another version saved it, writing positions to 4 decimals
    std::string theirs = Rgb(Model("A", kA, "SourceVersion=\"2026.10\" WorldPosX=\"10.00012\""));
    std::string mine = Rgb(Model("A", kA, "SourceVersion=\"2026.20\" WorldPosX=\"10.0001\" Brightness=\"5\""));
    Merge m = Rgbx(base, mine, theirs);
    CHECK(m.TheirsUnchanged());
    // a real move is still a change, and two saving versions are not a conflict
    std::string moved = Rgb(Model("A", kA, "SourceVersion=\"2026.10\" WorldPosX=\"11\""));
    Merge m2 = Rgbx(base, mine, moved);
    CHECK(m2.Conflicts().empty());
    Doc r = Result(m2);
    CHECK(Attr(r.M("A"), "WorldPosX") == "11");
    CHECK(Attr(r.M("A"), "Brightness") == "5");
}

// --- records from the base show folder ----------------------------------------------

TEST_CASE("Merge: two base syncs do not conflict on what the base owns", "[merge][showmerge][basemerge]") {
    auto doc = [](const std::string& stamp, const std::string& models) {
        return Rgb(models, "", "<settings><BaseRgbEffectsSyncedTime value=\"" + stamp + "\"/></settings>");
    };
    const std::string sub1 = Sub("Half", "1-5"), sub2 = Sub("Half", "1-6"), sub3 = Sub("Half", "1-7");
    std::string base = doc("100", Model("Arch", kA, "FromBase=\"1\" Description=\"v1\" Controller=\"Porch\"", sub1));
    // each synced a different version of the base show; mine is the newer sync
    std::string mine = doc("300", Model("Arch", kA, "FromBase=\"1\" Description=\"v3\" Controller=\"Porch\"", sub3) +
                                      Model("New", kB, "FromBase=\"1\" Brightness=\"3\""));
    std::string theirs = doc("200", Model("Arch", kA, "FromBase=\"1\" Description=\"v2\" Controller=\"Porch\"", sub2) +
                                        Model("New", kC, "FromBase=\"1\" Brightness=\"2\""));
    Merge m = Rgbx(base, mine, theirs);
    INFO(m.Conflicts().size());
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    CHECK(Attr(r.M("Arch"), "Description") == "v3");
    CHECK(Attr(r.M("Arch").child("subModel"), "line0") == "1-7");
    CHECK(Attr(r.M("New"), "Brightness") == "3");
    CHECK(Attr(r.Root().child("settings").child("BaseRgbEffectsSyncedTime"), "value") == "300");
    bool noted = false;
    for (const auto& i : m.ReferenceIssues()) noted |= i.message.find("newer base sync") != std::string::npos;
    CHECK(noted);

    // the other way round theirs is newer
    Merge older = Rgbx(base, theirs, mine);
    CHECK(older.Conflicts().empty());
    CHECK(Attr(Result(older).M("Arch"), "Description") == "v3");

    // what the show owns still conflicts
    std::string mineWired = doc("300", Model("Arch", kA, "FromBase=\"1\" Description=\"v1\" Controller=\"Garage\"", sub1));
    std::string theirsWired = doc("200", Model("Arch", kA, "FromBase=\"1\" Description=\"v1\" Controller=\"Shed\"", sub1));
    Merge wired = Rgbx(base, mineWired, theirsWired);
    REQUIRE(wired.Conflicts().size() == 1);
    CHECK(wired.Conflicts()[0].detail == "Controller");

    // control: a record not from the base conflicts as usual
    std::string localBase = doc("100", Model("Arch", kA, "Description=\"v1\""));
    Merge local = Rgbx(localBase, doc("300", Model("Arch", kA, "Description=\"v3\"")), doc("200", Model("Arch", kA, "Description=\"v2\"")));
    REQUIRE(local.Conflicts().size() == 1);
    CHECK(local.Conflicts()[0].detail == "Description");
}

TEST_CASE("Merge: the controllers' base sync time is not a conflict", "[merge][showmerge][basemerge]") {
    std::string base = Networks(Controller("A", kA), "BaseControllersSyncedTime=\"100\"");
    Merge m = Merge::Run(FileKind::Networks, base, Networks(Controller("A", kA, "Monitor=\"1\""), "BaseControllersSyncedTime=\"200\""),
                         Networks(Controller("A", kA), "BaseControllersSyncedTime=\"300\""));
    CHECK(m.Conflicts().empty());
    CHECK(Attr(Result(m).Root(), "BaseControllersSyncedTime") == "300");
}

// --- disjoint edits -------------------------------------------------------------

TEST_CASE("Merge: edits to different models both survive, in either order", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA) + Model("B", kB));
    std::string one = Rgb(Model("A", kA, "Brightness=\"40\"") + Model("B", kB));
    std::string two = Rgb(Model("A", kA) + Model("B", kB, "PixelSize=\"5\""));
    for (bool swap : { false, true }) {
        Merge m = Rgbx(base, swap ? two : one, swap ? one : two);
        CHECK(m.Conflicts().empty());
        Doc r = Result(m);
        CHECK(Attr(r.M("A"), "Brightness") == "40");
        CHECK(Attr(r.M("B"), "PixelSize") == "5");
        CHECK(Attr(r.M("A"), "xlId") == kA);
    }
}

TEST_CASE("Merge: edits to different attributes of one model both survive", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA, "Brightness=\"100\" PixelSize=\"2\" Gone=\"1\""));
    std::string one = Rgb(Model("A", kA, "Brightness=\"40\" PixelSize=\"2\" Gone=\"1\""));
    std::string two = Rgb(Model("A", kA, "Brightness=\"100\" PixelSize=\"7\" Added=\"x\""));
    for (bool swap : { false, true }) {
        Merge m = Rgbx(base, swap ? two : one, swap ? one : two);
        CHECK(m.Conflicts().empty());
        Doc r = Result(m);
        pugi::xml_node a = r.M("A");
        CHECK(Attr(a, "Brightness") == "40");
        CHECK(Attr(a, "PixelSize") == "7");
        CHECK(Attr(a, "Added") == "x");
        CHECK_FALSE(a.attribute("Gone"));
    }
}

TEST_CASE("Merge: different keyed children of one model both survive", "[merge][showmerge]") {
    std::string face = "<faceInfo Name=\"F\" Type=\"NodeRange\" Mouth-AI=\"1-2\"/>";
    std::string state = "<stateInfo Name=\"S\" Type=\"NodeRange\" s1=\"1-4\"/>";
    std::string conn = "<ControllerConnection Port=\"1\" Protocol=\"ws2811\"/>";
    std::string base = Rgb(Model("A", kA, "", Sub("L", "1-5") + Sub("R", "6-10") + conn));
    std::string one = Rgb(Model("A", kA, "", Sub("L", "1-4") + Sub("R", "6-10") + conn + face));
    std::string two = Rgb(Model("A", kA, "", Sub("L", "1-5") + Sub("R", "6-9") + "<ControllerConnection Port=\"2\" Protocol=\"ws2811\"/>" + state));
    for (bool swap : { false, true }) {
        Merge m = Rgbx(base, swap ? two : one, swap ? one : two);
        CHECK(m.Conflicts().empty());
        Doc r = Result(m);
        pugi::xml_node a = r.M("A");
        CHECK(Attr(a.find_child_by_attribute("subModel", "name", "L"), "line0") == "1-4");
        CHECK(Attr(a.find_child_by_attribute("subModel", "name", "R"), "line0") == "6-9");
        CHECK(Attr(a.child("ControllerConnection"), "Port") == "2");
        CHECK(a.find_child_by_attribute("faceInfo", "Name", "F"));
        CHECK(a.find_child_by_attribute("stateInfo", "Name", "S"));
    }
}

TEST_CASE("Merge: a model added on each side both survive", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA));
    std::string one = Rgb(Model("A", kA) + Model("B", kB));
    std::string two = Rgb(Model("A", kA) + Model("C", kC));
    for (bool swap : { false, true }) {
        Merge m = Rgbx(base, swap ? two : one, swap ? one : two);
        CHECK(m.Conflicts().empty());
        Doc r = Result(m);
        CHECK(r.M("B"));
        CHECK(r.M("C"));
        CHECK(r.ModelNames().size() == 3);
    }
}

TEST_CASE("Merge: a deleted model goes when the other side left it alone", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA) + Model("B", kB));
    std::string without = Rgb(Model("A", kA));
    std::string edited = Rgb(Model("A", kA, "Brightness=\"1\"") + Model("B", kB));
    for (bool swap : { false, true }) {
        Merge m = Rgbx(base, swap ? edited : without, swap ? without : edited);
        CHECK(m.Conflicts().empty());
        Doc r = Result(m);
        CHECK_FALSE(r.M("B"));
        CHECK(Attr(r.M("A"), "Brightness") == "1");
    }
}

// --- conflicts --------------------------------------------------------------------

TEST_CASE("Merge: the same attribute changed to different values conflicts", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA, "Brightness=\"100\" PixelSize=\"2\""));
    std::string mine = Rgb(Model("A", kA, "Brightness=\"40\" PixelSize=\"3\""));
    std::string theirs = Rgb(Model("A", kA, "Brightness=\"60\" PixelSize=\"2\""));
    Merge m = Rgbx(base, mine, theirs);
    const Conflict& c = Only(m);
    CHECK(c.kind == ConflictKind::AttributeChanged);
    CHECK(c.section == "models");
    CHECK(c.recordId == kA);
    CHECK(c.baseName == "A");
    CHECK(c.mineName == "A");
    CHECK(c.theirsName == "A");
    CHECK(c.detail == "Brightness");
    CHECK(c.baseValue == "100");
    CHECK(c.mineValue == "40");
    CHECK(c.theirsValue == "60");
    CHECK_FALSE(m.AllResolved());
    // unresolved: theirs, with the clean part of mine
    CHECK(Attr(Result(m).M("A"), "Brightness") == "60");
    CHECK(Attr(Result(m).M("A"), "PixelSize") == "3");

    REQUIRE(m.Resolve(c.key, Resolution::Mine));
    CHECK(m.AllResolved());
    CHECK(m.Conflicts().front().resolution == Resolution::Mine);
    CHECK(Attr(Result(m).M("A"), "Brightness") == "40");

    REQUIRE(m.Resolve(size_t(0), Resolution::Theirs));
    CHECK(Attr(Result(m).M("A"), "Brightness") == "60");

    CHECK_FALSE(m.Resolve("no such conflict", Resolution::Mine));
    CHECK_FALSE(m.Resolve(c.key, Resolution::KeepBoth));
    CHECK_FALSE(m.Resolve(size_t(5), Resolution::Mine));
}

TEST_CASE("Merge: the same change on both sides is not a conflict", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA, "Brightness=\"100\""));
    std::string both = Rgb(Model("A", kA, "Brightness=\"40\""));
    Merge m = Rgbx(base, both, both);
    CHECK(m.Conflicts().empty());
    CHECK(Attr(Result(m).M("A"), "Brightness") == "40");
}

TEST_CASE("Merge: the same submodel changed on both sides conflicts as a whole", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA, "", Sub("L", "1-5", kB)));
    std::string mine = Rgb(Model("A", kA, "", Sub("L", "1-4", kB)));
    std::string theirs = Rgb(Model("A", kA, "", Sub("L", "2-5", kB)));
    Merge m = Rgbx(base, mine, theirs);
    const Conflict& c = Only(m);
    CHECK(c.kind == ConflictKind::ChildChanged);
    CHECK(c.detail == "subModel[L]");
    CHECK(c.mineValue->find("1-4") != std::string::npos);
    CHECK(c.theirsValue->find("2-5") != std::string::npos);
    CHECK(Attr(Result(m).M("A").child("subModel"), "line0") == "2-5");
    m.ResolveAll(Resolution::Mine);
    CHECK(Attr(Result(m).M("A").child("subModel"), "line0") == "1-4");
}

TEST_CASE("Merge: a submodel renamed on one side and changed on the other is matched by id", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA, "", Sub("L", "1-5", kB)));
    std::string mine = Rgb(Model("A", kA, "", Sub("Left", "1-5", kB)));
    std::string theirs = Rgb(Model("A", kA, "", Sub("L", "2-5", kB)));
    Merge m = Rgbx(base, mine, theirs);
    // both changed the one child: a conflict about that child, not a delete + add
    const Conflict& c = Only(m);
    CHECK(c.kind == ConflictKind::ChildChanged);
    CHECK(c.recordId == kB);
}

TEST_CASE("Merge: deleting a model the other side changed conflicts", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA) + Model("B", kB));
    std::string without = Rgb(Model("A", kA));
    std::string edited = Rgb(Model("A", kA) + Model("B", kB, "Brightness=\"5\""));

    Merge mineDeleted = Rgbx(base, without, edited);
    const Conflict& c = Only(mineDeleted);
    CHECK(c.kind == ConflictKind::DeletedByMine);
    CHECK(c.mineName.empty());
    CHECK(c.theirsName == "B");
    CHECK_FALSE(c.mineValue);
    CHECK(Result(mineDeleted).M("B")); // unresolved keeps theirs
    mineDeleted.ResolveAll(Resolution::Mine);
    CHECK_FALSE(Result(mineDeleted).M("B"));

    Merge theirsDeleted = Rgbx(base, edited, without);
    CHECK(Only(theirsDeleted).kind == ConflictKind::DeletedByTheirs);
    CHECK_FALSE(Result(theirsDeleted).M("B"));
    theirsDeleted.ResolveAll(Resolution::Mine);
    CHECK(Attr(Result(theirsDeleted).M("B"), "Brightness") == "5");
}

TEST_CASE("Merge: both adding a model with one name conflicts and can keep both", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA));
    std::string mine = Rgb(Model("A", kA) + Model("New", kB, "Brightness=\"1\""));
    std::string theirs = Rgb(Model("A", kA) + Model("New", kC, "Brightness=\"2\""));
    Merge m = Rgbx(base, mine, theirs);
    const Conflict& c = Only(m);
    CHECK(c.kind == ConflictKind::BothAdded);
    CHECK(c.keepBothAllowed);
    CHECK_FALSE(c.baseValue);
    CHECK(Attr(Result(m).M("New"), "Brightness") == "2");

    REQUIRE(m.Resolve(c.key, Resolution::KeepBoth));
    Doc r = Result(m);
    CHECK(Attr(r.M("New"), "Brightness") == "2");
    CHECK(Attr(r.M("New-2"), "Brightness") == "1");
    CHECK(Attr(r.M("New-2"), "xlId") == kB);

    REQUIRE(m.Resolve(c.key, Resolution::Mine));
    CHECK(Attr(Result(m).M("New"), "Brightness") == "1");
    CHECK(Result(m).ModelNames().size() == 2);

    // the same addition on both sides is one model
    std::string same = Rgb(Model("A", kA) + Model("New", kB, "Brightness=\"1\""));
    std::string sameOther = Rgb(Model("A", kA) + Model("New", kC, "Brightness=\"1\""));
    Merge s = Rgbx(base, same, sameOther);
    CHECK(s.Conflicts().empty());
    CHECK(Result(s).ModelNames().size() == 2);
}

TEST_CASE("Merge: a whole-record section conflicts per record", "[merge][showmerge]") {
    auto views = [](const std::string& v1, const std::string& v2) {
        return Rgb(Model("A", kA) + Model("B", kB), "",
                   "<views><view name=\"V1\" models=\"" + v1 + "\"/><view name=\"V2\" models=\"" + v2 + "\"/></views>");
    };
    std::string base = views("A", "B");
    Merge clean = Rgbx(base, views("A,B", "B"), views("A", "B,A"));
    CHECK(clean.Conflicts().empty());
    Doc r = Result(clean);
    CHECK(Attr(r.Root().child("views").find_child_by_attribute("view", "name", "V1"), "models") == "A,B");
    CHECK(Attr(r.Root().child("views").find_child_by_attribute("view", "name", "V2"), "models") == "B,A");

    Merge both = Rgbx(base, views("A,B", "B"), views("B", "B"));
    const Conflict& c = Only(both);
    CHECK(c.kind == ConflictKind::RecordChanged);
    CHECK(c.section == "views");
    CHECK(c.baseName == "V1");
}

TEST_CASE("Merge: settings and colours merge per key", "[merge][showmerge]") {
    auto doc = [](const std::string& a, const std::string& b, const std::string& red) {
        return Rgb("", "", "<settings><a value=\"" + a + "\"/><b value=\"" + b + "\"/></settings><colors version=\"1\"><Timing1 Red=\"" + red +
                               "\" Green=\"0\" Blue=\"0\"/></colors>");
    };
    Merge m = Rgbx(doc("1", "1", "0"), doc("2", "1", "0"), doc("1", "3", "9"));
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    CHECK(Attr(r.Root().child("settings").child("a"), "value") == "2");
    CHECK(Attr(r.Root().child("settings").child("b"), "value") == "3");
    CHECK(Attr(r.Root().child("colors").child("Timing1"), "Red") == "9");

    Merge c = Rgbx(doc("1", "1", "0"), doc("2", "1", "0"), doc("3", "1", "0"));
    CHECK(Only(c).section == "settings");
    CHECK(Only(c).baseName == "a");
}

TEST_CASE("Merge: palettes are matched by content", "[merge][showmerge]") {
    auto doc = [](const std::string& palettes) { return Rgb("", "", "<palettes>" + palettes + "</palettes>"); };
    std::string p1 = "<palette data=\"#FF0000,#00FF00\"/>";
    std::string p2 = "<palette data=\"#0000FF\"/>";
    std::string p3 = "<palette data=\"#FFFFFF\"/>";
    Merge m = Rgbx(doc(p1), doc(p1 + p2), doc(p3));
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    std::vector<std::string> data;
    for (auto p : r.Root().child("palettes").children("palette")) data.push_back(Attr(p, "data"));
    CHECK(data == std::vector<std::string>{ "#FFFFFF", "#0000FF" });
}

TEST_CASE("Merge: a section attribute changed on both sides conflicts", "[merge][showmerge]") {
    auto doc = [](const std::string& current) {
        return Rgb("", "", "<perspectives current=\"" + current + "\"><perspective name=\"P\" settings=\"x\"/></perspectives>");
    };
    Merge m = Rgbx(doc("P"), doc("Q"), doc("R"));
    const Conflict& c = Only(m);
    CHECK(c.kind == ConflictKind::AttributeChanged);
    CHECK(c.detail == "current");
    CHECK(c.section == "perspectives");
}

// --- groups ---------------------------------------------------------------------------

TEST_CASE("Merge: group members merge as a set, order following theirs", "[merge][showmerge]") {
    std::string models = Model("A") + Model("B") + Model("C") + Model("D") + Model("E");
    std::string base = Rgb(models, Group("G", "A,B,C", kG));
    std::string one = Rgb(models, Group("G", "A,D,C", kG));   // drops B, adds D after A
    std::string two = Rgb(models, Group("G", "A,B,E", kG));   // drops C, adds E at the end
    Merge m = Rgbx(base, one, two);
    CHECK(m.Conflicts().empty());
    CHECK(Attr(Result(m).G("G"), "models") == "A,D,E");

    Merge swapped = Rgbx(base, two, one);
    CHECK(swapped.Conflicts().empty());
    CHECK(Attr(Result(swapped).G("G"), "models") == "A,D,E");

    // a reorder on one side is kept when the other only added
    Merge reorder = Rgbx(base, Rgb(models, Group("G", "C,B,A", kG)), Rgb(models, Group("G", "A,B,C,E", kG)));
    CHECK(reorder.Conflicts().empty());
    CHECK(Attr(Result(reorder).G("G"), "models") == "C,B,A,E");
}

TEST_CASE("Merge: group members and other attributes both merge", "[merge][showmerge]") {
    std::string models = Model("A") + Model("B") + Model("C");
    std::string base = Rgb(models, Group("G", "A", kG, "GridSize=\"400\""));
    Merge m = Rgbx(base, Rgb(models, Group("G", "A,B", kG, "GridSize=\"400\"")), Rgb(models, Group("G", "A,C", kG, "GridSize=\"800\"")));
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    pugi::xml_node g = r.G("G");
    CHECK(Attr(g, "models") == "A,C,B");
    CHECK(Attr(g, "GridSize") == "800");
}

TEST_CASE("Merge: deleting a group the other side changed conflicts", "[merge][showmerge]") {
    std::string models = Model("A") + Model("B");
    std::string base = Rgb(models, Group("G", "A", kG));
    Merge m = Rgbx(base, Rgb(models), Rgb(models, Group("G", "A,B", kG)));
    CHECK(Only(m).kind == ConflictKind::DeletedByMine);
    CHECK(Only(m).section == "modelGroups");
}

// --- renames and references -------------------------------------------------------------

TEST_CASE("Merge: a rename on one side and edits on the other both survive", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA, "Brightness=\"100\"") + Model("B", kB, "StartChannel=\">A:1\""), Group("G", "A,B", kG),
                           "<views><view name=\"V\" models=\"A\"/></views>");
    std::string mine = Rgb(Model("Arch", kA, "Brightness=\"100\"") + Model("B", kB, "StartChannel=\">Arch:1\""), Group("G", "Arch,B", kG),
                           "<views><view name=\"V\" models=\"Arch\"/></views>");
    // theirs edits the model under its old name and refers to it from new places
    std::string theirs = Rgb(Model("A", kA, "Brightness=\"50\"") + Model("B", kB, "StartChannel=\">A:1\"") +
                                 Model("C", kC, "StartChannel=\">A:51\" ShadowModelFor=\"A\""),
                             Group("G", "A,B", kG) + Group("H", "A/Sub,C", kD), "<views><view name=\"V\" models=\"A,C\"/></views>");
    for (bool swap : { false, true }) {
        Merge m = Rgbx(base, swap ? theirs : mine, swap ? mine : theirs);
        CHECK(m.Conflicts().empty());
        Doc r = Result(m);
        CHECK_FALSE(r.M("A"));
        REQUIRE(r.M("Arch"));
        CHECK(Attr(r.M("Arch"), "Brightness") == "50");
        CHECK(Attr(r.M("Arch"), "xlId") == kA);
        CHECK(Attr(r.M("B"), "StartChannel") == ">Arch:1");
        CHECK(Attr(r.M("C"), "StartChannel") == ">Arch:51");
        CHECK(Attr(r.M("C"), "ShadowModelFor") == "Arch");
        CHECK(Attr(r.G("G"), "models") == "Arch,B");
        CHECK(Attr(r.G("H"), "models") == "Arch/Sub,C");
        CHECK(Attr(r.Root().child("views").child("view"), "models") == "Arch,C");
        REQUIRE(m.Renames().size() == 1);
        CHECK(m.Renames()[0].baseName == "A");
        CHECK(m.Renames()[0].mergedName == "Arch");
        CHECK(m.Renames()[0].id == kA);
    }
}

TEST_CASE("Merge: a group rename follows into other groups", "[merge][showmerge]") {
    std::string models = Model("A", kA);
    std::string base = Rgb(models, Group("G", "A", kG) + Group("Outer", "G", kD));
    std::string mine = Rgb(models, Group("Front", "A", kG) + Group("Outer", "Front", kD));
    std::string theirs = Rgb(models, Group("G", "A", kG) + Group("Outer", "G", kD) + Group("New", "G,A", kC));
    Merge m = Rgbx(base, mine, theirs);
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    CHECK(Attr(r.G("Outer"), "models") == "Front");
    CHECK(Attr(r.G("New"), "models") == "Front,A");
}

TEST_CASE("Merge: renaming one model to two names conflicts", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA) + Model("B", kB, "StartChannel=\">A:1\""));
    std::string mine = Rgb(Model("Mine", kA) + Model("B", kB, "StartChannel=\">Mine:1\""));
    std::string theirs = Rgb(Model("Theirs", kA) + Model("B", kB, "StartChannel=\">Theirs:1\""));
    Merge m = Rgbx(base, mine, theirs);
    const Conflict& c = Only(m);
    CHECK(c.kind == ConflictKind::BothRenamed);
    CHECK(c.detail == "name");
    CHECK(c.mineValue == "Mine");
    CHECK(c.theirsValue == "Theirs");
    CHECK(Result(m).M("Theirs"));
    CHECK(Attr(Result(m).M("B"), "StartChannel") == ">Theirs:1");

    m.ResolveAll(Resolution::Mine);
    CHECK(m.AllResolved());
    Doc r = Result(m);
    CHECK(r.M("Mine"));
    CHECK_FALSE(r.M("Theirs"));
    // the references follow the chosen name
    CHECK(Attr(r.M("B"), "StartChannel") == ">Mine:1");

    // the same new name on both sides is not a conflict
    Merge same = Rgbx(base, mine, mine);
    CHECK(same.Conflicts().empty());
}

TEST_CASE("Merge: a version without ids is matched by name", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA) + Model("B", kB), Group("G", "A", kG));
    std::string mine = Rgb(Model("A", kA, "Brightness=\"10\"") + Model("B", kB), Group("G", "A", kG));
    // an older desktop dropped every id and edited B
    std::string theirs = Rgb(Model("A") + Model("B", "", "Brightness=\"20\""), Group("G", "A,B"));
    for (bool swap : { false, true }) {
        Merge m = Rgbx(base, swap ? theirs : mine, swap ? mine : theirs);
        CHECK(m.Conflicts().empty());
        Doc r = Result(m);
        CHECK(r.ModelNames().size() == 2);
        CHECK(Attr(r.M("A"), "Brightness") == "10");
        CHECK(Attr(r.M("B"), "Brightness") == "20");
        CHECK(Attr(r.G("G"), "models") == "A,B");
        // the ids survive from the side that has them
        CHECK(Attr(r.M("A"), "xlId") == kA);
        CHECK(Attr(r.M("B"), "xlId") == kB);
    }
}

TEST_CASE("Merge: two different ids are two records even with one name", "[merge][showmerge]") {
    // mine deleted A and made a new A; theirs left the old one alone
    std::string base = Rgb(Model("A", kA, "Brightness=\"1\""));
    std::string mine = Rgb(Model("A", kB, "Brightness=\"2\""));
    Merge m = Rgbx(base, mine, base + " ");
    CHECK(m.Conflicts().empty());
    // theirs == base: the fast path
    CHECK(Attr(Result(m).M("A"), "xlId") == kB);

    // theirs changed something else, so it really merges
    std::string theirs = Rgb(Model("A", kA, "Brightness=\"1\"") + Model("Z", kC));
    Merge m2 = Rgbx(base, mine, theirs);
    CHECK(m2.Conflicts().empty());
    Doc r = Result(m2);
    REQUIRE(r.ModelNames().size() == 2);
    CHECK(Attr(r.M("A"), "xlId") == kB);
    CHECK(Attr(r.M("A"), "Brightness") == "2");
}

TEST_CASE("Merge: the rename log matches a renamed record when ids are missing", "[merge][showmerge]") {
    // a file written before ids existed: no side has one
    std::string base = Rgb(Model("A") + Model("B"));
    std::string mine = Rgb(Model("Arch") + Model("B"));
    std::string theirs = Rgb(Model("A", "", "Brightness=\"50\"") + Model("B"));
    LayoutRenameLog log;
    log.Record("A", "Arch");
    Options o;
    o.modelRenames = &log;
    Merge m = Rgbx(base, mine, theirs, o);
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    CHECK_FALSE(r.M("A"));
    CHECK(Attr(r.M("Arch"), "Brightness") == "50");

    // control: without the log the rename is a delete of an edited model
    Merge noLog = Rgbx(base, mine, theirs);
    REQUIRE(noLog.Conflicts().size() == 1);
    CHECK(noLog.Conflicts()[0].kind == ConflictKind::DeletedByMine);
}

TEST_CASE("Merge: the rename log does not confuse a new record with the old name", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", "", "Brightness=\"1\""));
    // renamed A to Arch, then made a new A
    std::string mine = Rgb(Model("Arch", "", "Brightness=\"1\"") + Model("A", "", "Brightness=\"9\""));
    std::string theirs = Rgb(Model("A", "", "Brightness=\"1\"") + Model("Z"));
    LayoutRenameLog log;
    log.Record("A", "Arch");
    Options o;
    o.modelRenames = &log;
    Merge m = Rgbx(base, mine, theirs, o);
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    CHECK(Attr(r.M("Arch"), "Brightness") == "1");
    CHECK(Attr(r.M("A"), "Brightness") == "9");
    CHECK(r.M("Z"));
}

TEST_CASE("Merge: controller renames reach the models", "[merge][showmerge]") {
    std::string nb = Networks(Controller("C1", kA));
    std::string nm = Networks(Controller("Porch", kA));
    std::string nt = Networks(Controller("C1", kA) + Controller("C2", kB));
    Merge nets = Merge::Run(FileKind::Networks, nb, nm, nt);
    REQUIRE(nets.Ok());
    CHECK(nets.Conflicts().empty());
    NameMaps maps = nets.ControllerNameMaps();
    CHECK(maps.theirs.at("C1") == "Porch");

    std::string base = Rgb(Model("A", kA, "Controller=\"C1\" StartChannel=\"!C1:1\""));
    std::string mine = Rgb(Model("A", kA, "Controller=\"Porch\" StartChannel=\"!Porch:1\""));
    std::string theirs = Rgb(Model("A", kA, "Controller=\"C1\" StartChannel=\"!C1:1\"") + Model("B", kB, "Controller=\"C1\" StartChannel=\"!C1:301\""),
                             "", "<view_objects><view_object name=\"Box\" DisplayAs=\"Controller\" Controller=\"C1\"/></view_objects>");
    Options o;
    o.controllerNameMaps = maps;
    Merge m = Rgbx(base, mine, theirs, o);
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    CHECK(Attr(r.M("B"), "Controller") == "Porch");
    CHECK(Attr(r.M("B"), "StartChannel") == "!Porch:301");
    CHECK(Attr(r.Root().child("view_objects").child("view_object"), "Controller") == "Porch");

    // control: without the maps the new model keeps the old controller name
    Merge without = Rgbx(base, mine, theirs);
    CHECK(Attr(Result(without).M("B"), "Controller") == "C1");
}

// --- sections ----------------------------------------------------------------------------

TEST_CASE("Merge: a section one version does not write is kept", "[merge][showmerge]") {
    std::string unknown = "<futureThing a=\"1\"><x/></futureThing>";
    std::string base = Rgb(Model("A"), "", unknown + "<palettes><palette data=\"1\"/></palettes>");
    // mine was written by a version that writes neither section
    std::string mine = Rgb(Model("A", "", "Brightness=\"3\""));
    std::string theirs = Rgb(Model("A"), "", "<futureThing a=\"2\"><x/></futureThing><palettes><palette data=\"1\"/></palettes><newer/>");
    Merge m = Rgbx(base, mine, theirs);
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    CHECK(Attr(r.Root().child("futureThing"), "a") == "2");
    CHECK(r.Root().child("futureThing").child("x"));
    CHECK(r.Root().child("palettes").child("palette"));
    CHECK(r.Root().child("newer"));
    CHECK(Attr(r.M("A"), "Brightness") == "3");

    // theirs never had the section mine writes
    Merge other = Rgbx(Rgb(Model("A")), Rgb(Model("A"), "", "<views><view name=\"V\"/></views>"), Rgb(Model("A", "", "Brightness=\"3\"")));
    CHECK(Result(other).Root().child("views").child("view"));

    // an unknown section changed on both sides is replaced whole: a conflict
    Merge both = Rgbx(base, Rgb(Model("A"), "", "<futureThing a=\"3\"/>"), theirs);
    REQUIRE(both.Conflicts().size() == 1);
    CHECK(both.Conflicts()[0].section == "futureThing");
}

TEST_CASE("Merge: start channels changed on both sides are recalculated, not a conflict", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA, "StartChannel=\"!C:1\""));
    Merge m = Rgbx(base, Rgb(Model("A", kA, "StartChannel=\"!C:101\"")), Rgb(Model("A", kA, "StartChannel=\"!C:201\"")));
    CHECK(m.Conflicts().empty());
    CHECK(Attr(Result(m).M("A"), "StartChannel") == "!C:201");
    bool noted = false;
    for (const auto& i : m.ReferenceIssues()) {
        noted |= i.message.find("StartChannel") != std::string::npos;
    }
    CHECK(noted);
}

// --- reference checks -----------------------------------------------------------------

TEST_CASE("Merge: a clean merge can still leave references dangling, and they are reported", "[merge][showmerge]") {
    std::string base = Rgb(Model("A", kA) + Model("B", kB), Group("G", "A", kG));
    // mine chains a new model to A and adds B to G; theirs deletes A and B
    std::string mine = Rgb(Model("A", kA) + Model("B", kB) + Model("N", kC, "StartChannel=\">A:1\""), Group("G", "A,B", kG));
    std::string theirs = Rgb("", Group("G", "", kG));
    Merge m = Rgbx(base, mine, theirs);
    CHECK(m.Conflicts().empty());
    auto issues = m.ReferenceIssues();
    auto has = [&](const std::string& record, const std::string& text) {
        for (const auto& i : issues) {
            if (i.record == record && i.message.find(text) != std::string::npos) return true;
        }
        return false;
    };
    CHECK(has("N", "missing model 'A'"));
    CHECK(has("G", "'B' does not exist"));

    // control: a consistent layout has none
    CHECK(CheckReferences(Doc(mine).d, nullptr).empty());
}

TEST_CASE("Merge: reference checks see duplicate names and missing controllers", "[merge][showmerge]") {
    Doc rgb(Rgb(Model("A", "", "Controller=\"Gone\"") + Model("S", "", "ShadowModelFor=\"X\" ModelChain=\">Y\"") + Model("A"), Group("G", "S/Sub")));
    Doc nets(Networks(Controller("C") + Controller("C")));
    auto issues = CheckReferences(rgb.d, &nets.d);
    auto count = [&](const std::string& text) {
        int n = 0;
        for (const auto& i : issues) n += i.message.find(text) != std::string::npos;
        return n;
    };
    CHECK(count("more than one model or group is called 'A'") == 1);
    CHECK(count("missing controller 'Gone'") == 1);
    CHECK(count("shadows missing model 'X'") == 1);
    CHECK(count("chained to missing model 'Y'") == 1);
    CHECK(count("'S/Sub' does not exist") == 1);
    CHECK(count("more than one controller is called 'C'") == 1);
}

// --- networks --------------------------------------------------------------------------

TEST_CASE("Merge: controllers added on each side both survive, in order", "[merge][showmerge]") {
    std::string base = Networks(Controller("A", kA) + Controller("B", kB));
    std::string mine = Networks(Controller("A", kA) + Controller("M", kC) + Controller("B", kB));
    std::string theirs = Networks(Controller("A", kA) + Controller("B", kB) + Controller("T", kD));
    Merge m = Merge::Run(FileKind::Networks, base, mine, theirs);
    CHECK(m.Conflicts().empty());
    CHECK(Result(m).ControllerNames() == std::vector<std::string>{ "A", "M", "B", "T" });
    Merge swapped = Merge::Run(FileKind::Networks, base, theirs, mine);
    CHECK(Result(swapped).ControllerNames() == std::vector<std::string>{ "A", "M", "B", "T" });
}

TEST_CASE("Merge: controller attributes merge one by one, the output list as a whole", "[merge][showmerge]") {
    std::string base = Networks(Controller("A", kA, "Priority=\"100\" Monitor=\"0\""));
    std::string mine = Networks(Controller("A", kA, "Priority=\"50\" Monitor=\"0\""));
    std::string theirs = Networks(Controller("A", kA, "Priority=\"100\" Monitor=\"1\""), "computer=\"two\"");
    Merge m = Merge::Run(FileKind::Networks, base, mine, theirs);
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    pugi::xml_node c = r.C("A");
    CHECK(Attr(c, "Priority") == "50");
    CHECK(Attr(c, "Monitor") == "1");

    // the output list changed on both sides conflicts
    std::string outs1 = "<network NetworkType=\"E131\" BaudRate=\"1\" MaxChannels=\"512\"/><network NetworkType=\"E131\" BaudRate=\"2\" MaxChannels=\"512\"/>";
    std::string outs2 = "<network NetworkType=\"E131\" BaudRate=\"1\" MaxChannels=\"512\"/><network NetworkType=\"E131\" BaudRate=\"3\" MaxChannels=\"512\"/>";
    Merge outs = Merge::Run(FileKind::Networks, base, Networks(Controller("A", kA, "Priority=\"100\" Monitor=\"0\"", outs1)),
                            Networks(Controller("A", kA, "Priority=\"100\" Monitor=\"0\"", outs2)));
    REQUIRE(outs.Conflicts().size() == 1);
    CHECK(outs.Conflicts()[0].kind == ConflictKind::ChildChanged);
    CHECK(outs.Conflicts()[0].detail == "network");
    CHECK(outs.Conflicts()[0].section == "Controller");

    // an auto-sized channel count changed on both sides is recalculated, not a conflict
    Merge sized = Merge::Run(FileKind::Networks, base, Networks(Controller("A", kA, "Priority=\"100\" Monitor=\"0\"", "<network NetworkType=\"DDP\" MaxChannels=\"900\"/>")),
                             Networks(Controller("A", kA, "Priority=\"100\" Monitor=\"0\"", "<network NetworkType=\"DDP\" MaxChannels=\"700\"/>")));
    CHECK(sized.Conflicts().empty());
}

TEST_CASE("Merge: controller order merges as a list", "[merge][showmerge]") {
    std::string base = Networks(Controller("A", kA) + Controller("B", kB) + Controller("C", kC));
    std::string moved = Networks(Controller("C", kC) + Controller("A", kA) + Controller("B", kB));
    std::string added = Networks(Controller("A", kA) + Controller("B", kB) + Controller("C", kC) + Controller("D", kD));
    Merge m = Merge::Run(FileKind::Networks, base, moved, added);
    CHECK(m.Conflicts().empty());
    CHECK(Result(m).ControllerNames() == std::vector<std::string>{ "C", "A", "B", "D" });

    std::string other = Networks(Controller("B", kB) + Controller("C", kC) + Controller("A", kA));
    Merge both = Merge::Run(FileKind::Networks, base, moved, other);
    const Conflict& c = Only(both);
    CHECK(c.kind == ConflictKind::OrderChanged);
    CHECK(c.mineValue == "C, A, B");
    CHECK(c.theirsValue == "B, C, A");
    CHECK(Result(both).ControllerNames() == std::vector<std::string>{ "B", "C", "A" });
    both.ResolveAll(Resolution::Mine);
    CHECK(Result(both).ControllerNames() == std::vector<std::string>{ "C", "A", "B" });
}

TEST_CASE("Merge: a controller renamed on one side and edited on the other", "[merge][showmerge]") {
    std::string base = Networks(Controller("A", kA, "Priority=\"1\""));
    std::string mine = Networks(Controller("Garage", kA, "Priority=\"1\""));
    std::string theirs = Networks(Controller("A", kA, "Priority=\"2\""));
    Merge m = Merge::Run(FileKind::Networks, base, mine, theirs);
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    CHECK(r.ControllerNames() == std::vector<std::string>{ "Garage" });
    CHECK(Attr(r.C("Garage"), "Priority") == "2");
    CHECK(m.ControllerNameMaps().theirs.at("A") == "Garage");

    // id-less: the controller rename log
    std::string baseNoId = Networks(Controller("A", "", "Priority=\"1\""));
    std::string mineNoId = Networks(Controller("Garage", "", "Priority=\"1\""));
    std::string theirsNoId = Networks(Controller("A", "", "Priority=\"2\""));
    LayoutRenameLog log;
    log.Record("A", "Garage");
    Options o;
    o.controllerRenames = &log;
    Merge logged = Merge::Run(FileKind::Networks, baseNoId, mineNoId, theirsNoId, o);
    CHECK(logged.Conflicts().empty());
    CHECK(Result(logged).ControllerNames() == std::vector<std::string>{ "Garage" });
    CHECK(Attr(Result(logged).C("Garage"), "Priority") == "2");
}

TEST_CASE("Merge: the computer that saved the controllers is not a conflict", "[merge][showmerge]") {
    std::string base = Networks(Controller("A", kA));
    Merge m = Merge::Run(FileKind::Networks, base, Networks(Controller("A", kA, "Monitor=\"1\""), "computer=\"mine\""),
                         Networks(Controller("A", kA) + Controller("B", kB), "computer=\"theirs\""));
    CHECK(m.Conflicts().empty());
}

TEST_CASE("Merge: networks root children other than controllers are kept", "[merge][showmerge]") {
    std::string base = Networks(Controller("A", kA));
    std::string theirs = Networks("<e131sync universe=\"5\"/>" + Controller("A", kA) + "<testpreset name=\"T\"/>");
    Merge m = Merge::Run(FileKind::Networks, base, Networks(Controller("A", kA, "Monitor=\"1\"")), theirs);
    CHECK(m.Conflicts().empty());
    Doc r = Result(m);
    CHECK(Attr(r.Root().child("e131sync"), "universe") == "5");
    CHECK(r.Root().child("testpreset"));
    CHECK(Attr(r.C("A"), "Monitor") == "1");
}

// --- presets ----------------------------------------------------------------------------

namespace {

nlohmann::json Preset(const std::string& name, const std::string& settings) {
    return { { "type", "effect" }, { "name", name }, { "settings", settings }, { "version", "0007" }, { "xLightsVersion", "2026.1" } };
}

// Children go through an initializer_list: a braced `{ x }` passed straight to a nlohmann::json parameter is
// the array [x] on clang but a copy of x on MSVC.
nlohmann::json Children(std::initializer_list<nlohmann::json> items) {
    nlohmann::json a = nlohmann::json::array();
    for (const auto& i : items) a.push_back(i);
    return a;
}

nlohmann::json PresetGroup(const std::string& name, const nlohmann::json& children) {
    return { { "type", "group" }, { "name", name }, { "children", children } };
}

nlohmann::json PresetGroup(const std::string& name, std::initializer_list<nlohmann::json> children) {
    return PresetGroup(name, Children(children));
}

std::string Presets(const nlohmann::json& children) {
    nlohmann::json j = { { "version", "0007" }, { "children", children } };
    return j.dump(2);
}

std::string Presets(std::initializer_list<nlohmann::json> children) {
    return Presets(Children(children));
}

const nlohmann::json* Find(const nlohmann::json& j, const std::vector<std::string>& path) {
    const nlohmann::json* cur = &j;
    for (const auto& p : path) {
        const nlohmann::json* next = nullptr;
        for (const auto& c : (*cur)["children"]) {
            if (c["name"] == p) next = &c;
        }
        if (!next) return nullptr;
        cur = next;
    }
    return cur;
}

} // namespace

TEST_CASE("Merge: presets merge per preset path", "[merge][showmerge][presets]") {
    std::string base = Presets({ PresetGroup("G", { Preset("one", "E_A\t1") }), PresetGroup("H", nlohmann::json::array()) });
    std::string mine = Presets({ PresetGroup("G", { Preset("one", "E_A\t1"), Preset("two", "E_B\t1") }), PresetGroup("H", nlohmann::json::array()) });
    std::string theirs = Presets({ PresetGroup("G", { Preset("one", "E_A\t2") }), PresetGroup("H", { Preset("three", "E_C\t1") }) });
    for (bool swap : { false, true }) {
        Merge m = Merge::Run(FileKind::Presets, base, swap ? theirs : mine, swap ? mine : theirs);
        REQUIRE(m.Ok());
        CHECK(m.Conflicts().empty());
        nlohmann::json r = nlohmann::json::parse(m.ResultText());
        CHECK(r["version"] == "0007");
        REQUIRE(Find(r, { "G", "one" }));
        CHECK((*Find(r, { "G", "one" }))["settings"] == "E_A\t2");
        CHECK((*Find(r, { "G", "one" }))["type"] == "effect");
        CHECK(Find(r, { "G", "two" }));
        CHECK(Find(r, { "H", "three" }));
        CHECK((*Find(r, { "H" }))["type"] == "group");
    }
}

TEST_CASE("Merge: one preset changed on both sides conflicts", "[merge][showmerge][presets]") {
    std::string base = Presets({ Preset("one", "E_A\t1") });
    Merge m = Merge::Run(FileKind::Presets, base, Presets({ Preset("one", "E_A\t2") }), Presets({ Preset("one", "E_A\t3") }));
    const Conflict& c = Only(m);
    CHECK(c.kind == ConflictKind::RecordChanged);
    CHECK(c.section == "presets");
    CHECK(c.baseName == "one");
    m.ResolveAll(Resolution::Mine);
    nlohmann::json r = nlohmann::json::parse(m.ResultText());
    CHECK((*Find(r, { "one" }))["settings"] == "E_A\t2");
}

TEST_CASE("Merge: a preset group deleted on one side while the other added to it conflicts", "[merge][showmerge][presets]") {
    std::string base = Presets({ PresetGroup("G", { Preset("one", "E_A\t1") }) });
    Merge m = Merge::Run(FileKind::Presets, base, Presets({ PresetGroup("G", { Preset("one", "E_A\t1"), Preset("two", "E_B\t1") }) }),
                         Presets(nlohmann::json::array()));
    CHECK(Only(m).kind == ConflictKind::DeletedByTheirs);
    CHECK_FALSE(Find(nlohmann::json::parse(m.ResultText()), { "G" }));
    m.ResolveAll(Resolution::Mine);
    CHECK(Find(nlohmann::json::parse(m.ResultText()), { "G", "two" }));
}

TEST_CASE("Merge: presets fast path keeps mine's bytes", "[merge][showmerge][presets]") {
    std::string base = Presets({ Preset("one", "E_A\t1") });
    std::string mine = "{\"children\":[],\"version\":\"0007\"}";
    Merge m = Merge::Run(FileKind::Presets, base, mine, base);
    CHECK(m.TheirsUnchanged());
    CHECK(m.ResultText() == mine);
}

// --- rename log --------------------------------------------------------------------------

TEST_CASE("Rename log follows chains and ignores no-op renames", "[merge][renamelog]") {
    LayoutRenameLog log;
    CHECK(log.Empty());
    log.Record("A", "A");
    log.Record("", "B");
    CHECK(log.Empty());
    log.Record("A", "B");
    log.Record("B", "C");
    CHECK(log.CurrentName("A") == "C");
    CHECK(log.CurrentName("B") == "C");
    CHECK(log.CurrentName("X") == "X");
    // back again
    log.Record("C", "A");
    CHECK(log.CurrentName("A") == "A");
    CHECK(log.Entries().size() == 3);
    log.Clear();
    CHECK(log.Empty());
    CHECK(log.CurrentName("A") == "A");
}

TEST_CASE("Layout records: member list helpers", "[merge][layoutrecords]") {
    // the base show update's union keeps first's order, then second's extras, once each
    CHECK(LayoutRecords::UnionMembers("A,B,A", "B,C") == "A,B,C");
    CHECK(LayoutRecords::UnionMembers("", "B") == "B");
    CHECK(LayoutRecords::UnionMembers("A,,B", "C") == "A,,B,C");
    CHECK(LayoutRecords::SplitMembers("A,,B,") == std::vector<std::string>{ "A", "B" });
    std::string b = "A,B", m = "A,B,M", t = "T,A,B";
    CHECK(LayoutRecords::MergeMembers(&b, &m, &t) == "T,A,B,M");
    CHECK(LayoutRecords::MergeMembers(nullptr, &m, nullptr) == "A,B,M");
    CHECK(LayoutRecords::PositionsEqual("10.00001", "10.00002"));
    CHECK_FALSE(LayoutRecords::PositionsEqual("10.001", "10.0"));
    CHECK(LayoutRecords::IsBookkeepingAttribute("SourceVersion"));
    CHECK(LayoutRecords::IsBookkeepingAttribute("xlId"));
    CHECK(LayoutRecords::IsShowOwnedAttribute("model", "Controller"));
    CHECK_FALSE(LayoutRecords::IsShowOwnedAttribute("model", "Description"));
}
