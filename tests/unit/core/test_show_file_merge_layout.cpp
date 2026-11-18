#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "diagnostics/CheckSequenceReport.h"
#include "diagnostics/SequenceChecker.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"
#include "outputs/Controller.h"
#include "outputs/OutputManager.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceViewManager.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/LayoutObjectId.h"
#include "utils/LayoutRenameLog.h"
#include "utils/ShowFileMerge.h"
#include "XmlSerializer/StringSerializingVisitor.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializingVisitor.h"

// The show-file merge (utils/ShowFileMerge.h) over real layouts: every model
// type built by the model code, loaded into a show context, edited through the
// model API on two sides, serialized the way the desktop saves, merged, and
// loaded again.
using namespace ShowFileMerge;

namespace {

const std::vector<std::string>& AllTypes() {
    static const std::vector<std::string> types = { "Star", "Arches", "Candy Canes", "Channel Block", "Circle", "DmxMovingHead", "DmxGeneral",
                                                    "DmxMovingHeadAdv", "DmxFloodlight", "DmxFloodArea", "DmxSkull", "DmxServo", "DmxServo3d",
                                                    "DmxServo3Axis", "Image", "Label", "Window Frame", "Wreath", "Sphere", "Single Line", "Poly Line",
                                                    "MultiPoint", "Cube", "Custom", "Tree 360", "Tree Flat", "Matrix", "Spinner", "Icicles" };
    return types;
}

std::string Name(size_t i) { return "M" + std::to_string(i); }

const std::string kExtras = "<subModel name=\"top\" layout=\"horizontal\" type=\"ranges\" line0=\"1-3\"/>"
                            "<subModel name=\"bottom\" layout=\"horizontal\" type=\"ranges\" line0=\"4-6\"/>"
                            "<faceInfo Name=\"F\" Type=\"NodeRange\" CustomColors=\"0\" Eyes-Open=\"1-2\"/>"
                            "<stateInfo Name=\"S\" Type=\"NodeRange\" CustomColors=\"0\" s1=\"1-4\" s1-Name=\"Half\" s1-Color=\"#FF0000\"/>";

std::string Print(pugi::xml_node n) {
    std::ostringstream os;
    n.print(os, "", pugi::format_raw);
    return os.str();
}

// One model of each type, as the model code writes a new one, chained one
// after the other, with submodels, a face and a state; groups, a view, a
// gridlines object and the sections the desktop writes from its own state.
std::string SeedRgb() {
    OutputManager om;
    ModelManager mm(&om);
    std::string models, prev, all, even;
    for (size_t i = 0; i < AllTypes().size(); ++i) {
        Model* m = mm.CreateDefaultModel(AllTypes()[i], "1");
        REQUIRE(m != nullptr);
        StringSerializingVisitor v(false, false);
        m->Accept(v);
        delete m;
        pugi::xml_document d;
        REQUIRE(d.load_string(v.GetResult().c_str()));
        pugi::xml_node n = d.document_element();
        LayoutObjectId::StripFromXml(n);
        n.attribute("name").set_value(Name(i).c_str());
        if (!n.attribute("StartChannel")) n.append_attribute("StartChannel");
        n.attribute("StartChannel").set_value(prev.empty() ? "1" : (">" + prev + ":1").c_str());
        pugi::xml_document extras;
        REQUIRE(extras.load_string(("<r>" + kExtras + "</r>").c_str()));
        for (pugi::xml_node c : extras.document_element().children()) n.append_copy(c);
        models += Print(n);
        prev = Name(i);
        all += (all.empty() ? "" : ",") + Name(i);
        if (i % 2 == 0) even += (even.empty() ? "" : ",") + Name(i);
    }
    std::string groups = "<modelGroup name=\"G_All\" models=\"" + all + "\" layout=\"minimalGrid\" GridSize=\"400\"/>" +
                         "<modelGroup name=\"G_Even\" models=\"" + even + "\" layout=\"minimalGrid\" GridSize=\"400\"/>" +
                         "<modelGroup name=\"G_Outer\" models=\"G_Even,M1,M3/top\" layout=\"minimalGrid\" GridSize=\"400\"/>";
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models type=\"rgb_effects\">" + models + "</models><modelGroups type=\"rgb_effects\">" +
           groups +
           "</modelGroups><view_objects><view_object name=\"Grid\" DisplayAs=\"Gridlines\" LayoutGroup=\"Default\" GridLineSpacing=\"50\" "
           "GridWidth=\"100\" GridHeight=\"100\"/></view_objects><views><view name=\"Front\" models=\"M0,M1,G_All\"/></views></xrgb>";
}

std::string SeedNetworks() {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks computer=\"one\">"
           "<Controller Type=\"Null\" Id=\"64001\" Name=\"Bulk\"><network NetworkType=\"NULL\" MaxChannels=\"500000\"/></Controller>"
           "<Controller Type=\"Null\" Id=\"64002\" Name=\"Spare\"><network NetworkType=\"NULL\" MaxChannels=\"512\"/></Controller></Networks>";
}

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

// Loading an Ethernet controller pings it, and that logs through the named
// "curl" logger the app registers at startup.
struct CurlLogger {
    CurlLogger() {
        if (!spdlog::get("curl")) {
            spdlog::register_logger(std::make_shared<spdlog::logger>("curl", std::make_shared<spdlog::sinks::null_sink_mt>()));
        }
    }
};

struct ShowFolder {
    CurlLogger curl;
    TempDir dir;
    FixFileReset reset;
    HeadlessRenderContext ctx;

    void Load(const std::string& rgb, const std::string& nets = SeedNetworks()) {
        dir.Write("xlights_networks.xml", nets);
        dir.Write("xlights_rgbeffects.xml", rgb);
        REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    }
    Model* M(const std::string& name) {
        Model* m = ctx.AllModels.GetModel(name);
        REQUIRE(m != nullptr);
        return m;
    }
    ModelGroup* G(const std::string& name) {
        ModelGroup* g = dynamic_cast<ModelGroup*>(ctx.AllModels.GetModel(name));
        REQUIRE(g != nullptr);
        return g;
    }
    // The parts of the layout file the desktop rebuilds from memory.
    std::string Rgb() {
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
    std::string Nets() { return ctx._outputManager.SaveToString(); }
    Model* AddModel(const std::string& type, const std::string& name, const std::string& start) {
        Model* m = ctx.AllModels.CreateDefaultModel(type, start);
        REQUIRE(m != nullptr);
        StringSerializingVisitor v(false, false);
        m->Accept(v);
        delete m;
        pugi::xml_document d;
        REQUIRE(d.load_string(v.GetResult().c_str()));
        pugi::xml_node n = d.document_element();
        LayoutObjectId::StripFromXml(n);
        n.attribute("name").set_value(name.c_str());
        Model* added = ctx.AllModels.createAndAddModel(n, 100, 100);
        REQUIRE(added != nullptr);
        return added;
    }
};

// The layout as text with ids removed (fresh models get random ids, so two
// shows built the same way differ only there).
std::string NoIds(const std::string& text) {
    pugi::xml_document d;
    REQUIRE(d.load_string(text.c_str()));
    LayoutObjectId::StripFromXml(d.document_element());
    return Print(d.document_element());
}

// Which records differ between two layouts (for failure messages).
std::string DiffRecords(const std::string& a, const std::string& b) {
    pugi::xml_document da, db;
    da.load_string(a.c_str());
    db.load_string(b.c_str());
    std::map<std::string, std::string> ra, rb;
    for (pugi::xml_document* d : { &da, &db }) {
        auto& r = d == &da ? ra : rb;
        for (pugi::xml_node section : d->document_element().children()) {
            for (pugi::xml_node rec : section.children()) {
                r[std::string(section.name()) + "/" + rec.attribute("name").value()] = Print(rec);
            }
        }
    }
    std::string out;
    for (const auto& [k, v] : ra) {
        auto f = rb.find(k);
        if (f == rb.end()) out += "only in result: " + k + "\n";
        else if (f->second != v) out += "differs: " + k + "\n  result:   " + v + "\n  expected: " + f->second + "\n";
    }
    for (const auto& [k, v] : rb) {
        if (!ra.count(k)) out += "only expected: " + k + "\n";
    }
    return out;
}

using Edit = std::function<void(ShowFolder&)>;

std::string Edited(const std::string& base, const std::vector<Edit>& edits) {
    ShowFolder f;
    f.Load(base);
    for (const auto& e : edits) e(f);
    return f.Rgb();
}

// The layout after loading `text` and saving it again.
std::string Reloaded(const std::string& text) {
    ShowFolder f;
    f.Load(text);
    return f.Rgb();
}

const std::string& Base() {
    static const std::string base = [] {
        ShowFolder f;
        f.Load(SeedRgb());
        return f.Rgb();
    }();
    return base;
}

std::string ConflictList(const Merge& m) {
    std::string s;
    for (const auto& c : m.Conflicts()) s += c.key + " (" + ToString(c.kind) + ")\n";
    return s;
}

// merge(base, one, two), loaded and saved again, must equal applying one's
// edits and then two's to base - in either order.
void CheckDisjoint(const std::string& base, const std::vector<Edit>& one, const std::vector<Edit>& two, const std::vector<Edit>& both) {
    std::string a = Edited(base, one);
    std::string b = Edited(base, two);
    std::string expected = NoIds(Reloaded(Edited(base, both)));
    for (bool swap : { false, true }) {
        INFO("swapped: " << swap);
        Merge m = Merge::Run(FileKind::RgbEffects, base, swap ? b : a, swap ? a : b);
        REQUIRE(m.Ok());
        INFO(ConflictList(m));
        CHECK(m.Conflicts().empty());
        std::string got = NoIds(Reloaded(m.ResultText()));
        INFO(DiffRecords(got, expected));
        CHECK(got == expected);
    }
}

bool ReportHas(const CheckSequenceReport& report, const std::string& a, const std::string& b) {
    for (const auto& s : report.GetSections()) {
        for (const auto& i : s.issues) {
            if (i.message.find(a) != std::string::npos && i.message.find(b) != std::string::npos) return true;
        }
    }
    return false;
}

std::string TestDataShow() {
    if (const char* env = std::getenv("XL_TEST_DATA")) {
        std::filesystem::path p = std::filesystem::path(env) / "shows" / "basic";
        if (std::filesystem::exists(p / "xlights_rgbeffects.xml")) return p.string();
    }
    std::filesystem::path p = std::filesystem::path(__FILE__).parent_path() / ".." / ".." / ".." / ".." / "xLights-test-data" / "shows" / "basic";
    std::error_code ec;
    if (std::filesystem::exists(p / "xlights_rgbeffects.xml", ec)) return p.string();
    return {};
}

std::string ReadFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream os;
    os << in.rdbuf();
    return os.str();
}

} // namespace

TEST_CASE("Layout merge: the seed layout has every model type with its children", "[merge][showmerge][layout]") {
    ShowFolder f;
    f.Load(Base());
    for (size_t i = 0; i < AllTypes().size(); ++i) {
        INFO(AllTypes()[i]);
        Model* m = f.M(Name(i));
        CHECK_FALSE(m->GetObjectId().empty());
    }
    pugi::xml_document d;
    REQUIRE(d.load_string(Base().c_str()));
    int withSubmodels = 0;
    for (auto m : d.document_element().child("models").children("model")) withSubmodels += m.child("subModel") ? 1 : 0;
    CHECK(withSubmodels > 20);
}

TEST_CASE("Layout merge: theirs unchanged gives mine, for every model type", "[merge][showmerge][layout]") {
    for (size_t i = 0; i < AllTypes().size(); ++i) {
        INFO(AllTypes()[i]);
        std::string mine = Edited(Base(), { [i](ShowFolder& f) {
                                      f.M(Name(i))->SetDescription("mine");
                                      f.M(Name(i))->RemoveSubModel("top");
                                  } });
        Merge m = Merge::Run(FileKind::RgbEffects, Base(), mine, Base());
        REQUIRE(m.Ok());
        CHECK(m.TheirsUnchanged());
        CHECK(m.ResultText() == mine);
    }
}

TEST_CASE("Layout merge: edits to one model of each type and to its neighbour both survive", "[merge][showmerge][layout]") {
    const size_t n = AllTypes().size();
    for (size_t i = 0; i < n; ++i) {
        INFO(AllTypes()[i]);
        std::string me = Name(i), next = Name((i + 1) % n);
        Edit one = [me](ShowFolder& f) {
            f.M(me)->SetDescription("mine");
            f.M(me)->RemoveSubModel("top");
        };
        // the same model: another attribute and another submodel
        Edit two = [me, next](ShowFolder& f) {
            f.M(next)->SetTransparency(33);
            f.M(me)->SetPixelSize(6);
            f.M(me)->RemoveSubModel("bottom");
        };
        CheckDisjoint(Base(), { one }, { two }, { one, two });
    }
}

TEST_CASE("Layout merge: a mixed set of disjoint edits survives in either order", "[merge][showmerge][layout]") {
    std::vector<Edit> mine = {
        [](ShowFolder& f) { f.ctx.AllModels.Rename("M3", "Renamed"); },
        [](ShowFolder& f) { f.AddModel("Arches", "NewMine", "1"); },
        [](ShowFolder& f) { REQUIRE(f.ctx.AllModels.Delete("M5")); },
        [](ShowFolder& f) { f.G("G_Even")->AddModel("M1"); },
        [](ShowFolder& f) { f.M("M8")->SetPixelSize(9); },
    };
    auto theirs = [](const std::string& m3) {
        return std::vector<Edit>{
            [m3](ShowFolder& f) { f.M(m3)->SetDescription("from theirs"); },
            [](ShowFolder& f) { f.AddModel("Star", "NewTheirs", "1"); },
            [](ShowFolder& f) { f.M("M7")->RemoveSubModel("bottom"); },
            [](ShowFolder& f) { f.M("M9")->SetTransparency(40); },
            [](ShowFolder& f) {
                std::vector<std::string> members;
                for (const auto& s : f.G("G_All")->ModelNames()) {
                    if (s != "M11") members.push_back(s);
                }
                f.G("G_All")->SetModels(members);
            },
        };
    };
    std::vector<Edit> both = mine;
    for (const auto& e : theirs("Renamed")) both.push_back(e);
    CheckDisjoint(Base(), mine, theirs("M3"), both);
}

TEST_CASE("Layout merge: deleting a model the other side edited is a conflict either way it resolves", "[merge][showmerge][layout]") {
    std::string mine = Edited(Base(), { [](ShowFolder& f) { REQUIRE(f.ctx.AllModels.Delete("M12")); } });
    std::string theirs = Edited(Base(), { [](ShowFolder& f) { f.M("M12")->SetDescription("kept"); } });
    Merge m = Merge::Run(FileKind::RgbEffects, Base(), mine, theirs);
    REQUIRE(m.Conflicts().size() == 1);
    CHECK(m.Conflicts()[0].kind == ConflictKind::DeletedByMine);
    {
        ShowFolder r;
        r.Load(m.ResultText());
        CHECK(r.M("M12")->GetDescription() == "kept");
    }
    m.ResolveAll(Resolution::Mine);
    ShowFolder r;
    r.Load(m.ResultText());
    CHECK(r.ctx.AllModels.GetModel("M12") == nullptr);
}

TEST_CASE("Layout merge: the model rename log covers a layout saved without ids", "[merge][showmerge][layout]") {
    // the file on disk came from a version that writes no ids
    std::string base = NoIds(Base());
    ShowFolder mineShow;
    mineShow.Load(base);
    mineShow.ctx.AllModels.Rename("M2", "Arch");
    CHECK(mineShow.ctx.AllModels.GetRenameLog().CurrentName("M2") == "Arch");
    std::string mine = mineShow.Rgb();
    // theirs, also without ids, edits the model under its old name
    pugi::xml_document t;
    REQUIRE(t.load_string(base.c_str()));
    t.document_element().child("models").find_child_by_attribute("model", "name", "M2").append_attribute("Description") = "theirs";
    std::string theirs = Print(t.document_element());

    Options o;
    o.modelRenames = &mineShow.ctx.AllModels.GetRenameLog();
    Merge m = Merge::Run(FileKind::RgbEffects, base, mine, theirs, o);
    INFO(ConflictList(m));
    CHECK(m.Conflicts().empty());
    ShowFolder r;
    r.Load(m.ResultText());
    CHECK(r.ctx.AllModels.GetModel("M2") == nullptr);
    CHECK(r.M("Arch")->GetDescription() == "theirs");
    // the model chained to it follows the rename
    CHECK(r.M("M3")->ModelStartChannel == ">Arch:1");

    // control: without the log it is a delete of an edited model
    Merge noLog = Merge::Run(FileKind::RgbEffects, base, mine, theirs);
    CHECK_FALSE(noLog.Conflicts().empty());
}

TEST_CASE("Layout merge: a model added on each side on the same channels shows up in the model checks", "[merge][showmerge][layout]") {
    std::string mine = Edited(Base(), { [](ShowFolder& f) { f.AddModel("Single Line", "MineLine", "400001"); } });
    std::string theirs = Edited(Base(), { [](ShowFolder& f) { f.AddModel("Single Line", "TheirLine", "400001"); } });
    Merge m = Merge::Run(FileKind::RgbEffects, Base(), mine, theirs);
    CHECK(m.Conflicts().empty());

    auto check = [](const std::string& rgb) {
        ShowFolder f;
        f.Load(rgb);
        CheckSequenceReport report;
        for (const auto& s : CheckSequenceReport::REPORT_SECTIONS) report.AddSection(s);
        SequenceChecker checker(f.ctx._sequenceElements, f.ctx.AllModels, f.ctx._outputManager, nullptr, f.dir.path.string());
        checker.RunModelChecks(report);
        return ReportHas(report, "'MineLine'", "'TheirLine'");
    };
    CHECK(check(m.ResultText()));
    // control: neither side alone overlaps
    CHECK_FALSE(check(mine));
    CHECK_FALSE(check(theirs));
}

TEST_CASE("Layout merge: a chain to a model the other side deleted is reported", "[merge][showmerge][layout]") {
    std::string mine = Edited(Base(), { [](ShowFolder& f) { f.AddModel("Single Line", "Chained", ">M20:1"); } });
    std::string theirs = Edited(Base(), { [](ShowFolder& f) { REQUIRE(f.ctx.AllModels.Delete("M20")); } });
    Merge m = Merge::Run(FileKind::RgbEffects, Base(), mine, theirs);
    CHECK(m.Conflicts().empty());
    bool reported = false;
    for (const auto& i : m.ReferenceIssues()) reported |= i.record == "Chained" && i.message.find("'M20'") != std::string::npos;
    CHECK(reported);

    ShowFolder f;
    f.Load(m.ResultText());
    CheckSequenceReport report;
    for (const auto& s : CheckSequenceReport::REPORT_SECTIONS) report.AddSection(s);
    SequenceChecker checker(f.ctx._sequenceElements, f.ctx.AllModels, f.ctx._outputManager, nullptr, f.dir.path.string());
    checker.RunModelChecks(report);
    CHECK(ReportHas(report, "'Chained'", "non existent model 'M20'"));
}

TEST_CASE("Layout merge: controllers saved by the output manager merge", "[merge][showmerge][layout]") {
    ShowFolder baseShow;
    baseShow.Load(Base());
    std::string base = baseShow.Nets();

    ShowFolder mineShow;
    mineShow.Load(Base(), base);
    mineShow.ctx._outputManager.GetController("Bulk")->SetName("Garage");
    std::string mine = mineShow.Nets();

    ShowFolder theirsShow;
    theirsShow.Load(Base(), base);
    Controller* spare = theirsShow.ctx._outputManager.GetController("Spare");
    Controller* copy = spare->Copy(&theirsShow.ctx._outputManager);
    copy->SetName("Added");
    theirsShow.ctx._outputManager.AddController(copy);
    std::string theirs = theirsShow.Nets();

    Merge m = Merge::Run(FileKind::Networks, base, mine, theirs);
    INFO(ConflictList(m));
    CHECK(m.Conflicts().empty());
    TempDir dir;
    dir.Write("xlights_networks.xml", m.ResultText());
    OutputManager om;
    REQUIRE(om.Load(dir.path.string()));
    std::vector<std::string> names;
    for (const Controller* c : om.GetControllers()) names.push_back(c->GetName());
    CHECK(names == std::vector<std::string>{ "Garage", "Spare", "Added" });
    CHECK(om.GetController("Garage")->GetObjectId() == baseShow.ctx._outputManager.GetController("Bulk")->GetObjectId());
    CHECK(m.ControllerNameMaps().theirs.at("Bulk") == "Garage");
}

TEST_CASE("Layout merge: the show in the test data repository merges", "[merge][showmerge][layout][testdata]") {
    std::string dir = TestDataShow();
    if (dir.empty()) {
        SKIP("xLights-test-data not found (set XL_TEST_DATA)");
    }
    std::string rgb = ReadFile(dir + "/xlights_rgbeffects.xml");
    std::string nets = ReadFile(dir + "/xlights_networks.xml");
    ShowFolder baseShow;
    baseShow.Load(rgb, nets);
    std::string base = baseShow.Rgb();
    std::vector<std::string> names;
    for (const auto& [name, model] : baseShow.ctx.AllModels) names.push_back(name);
    REQUIRE(names.size() >= 2);

    auto edited = [&](const std::vector<Edit>& edits) {
        ShowFolder f;
        f.Load(base, nets);
        for (const auto& e : edits) e(f);
        return f.Rgb();
    };
    Edit one = [&](ShowFolder& f) { f.M(names[0])->SetDescription("one"); };
    Edit two = [&](ShowFolder& f) {
        f.M(names[1])->SetPixelSize(4);
        f.AddModel("Star", "Added", "1");
    };
    std::string a = edited({ one }), b = edited({ two });
    std::string expected = [&] {
        ShowFolder f;
        f.Load(edited({ one, two }), nets);
        return NoIds(f.Rgb());
    }();
    for (bool swap : { false, true }) {
        Merge m = Merge::Run(FileKind::RgbEffects, base, swap ? b : a, swap ? a : b);
        CHECK(m.Conflicts().empty());
        ShowFolder r;
        r.Load(m.ResultText(), nets);
        CHECK(NoIds(r.Rgb()) == expected);
    }
    // theirs == base: mine exactly
    CHECK(Merge::Run(FileKind::RgbEffects, rgb, a, rgb).ResultText() == a);

    // the controllers: one renamed, one added. The file has no ids yet, so
    // the rename is matched through this side's rename log.
    ShowFolder mineShow;
    mineShow.Load(base, nets);
    mineShow.ctx._outputManager.GetControllers().front()->SetName("Renamed");
    std::string netsMine = mineShow.Nets();
    std::string netsTheirs = [&] {
        ShowFolder f;
        f.Load(base, nets);
        Controller* c = f.ctx._outputManager.GetControllers().front()->Copy(&f.ctx._outputManager);
        c->SetName("Second");
        f.ctx._outputManager.AddController(c);
        return f.Nets();
    }();
    Options o;
    o.controllerRenames = &mineShow.ctx._outputManager.GetRenameLog();
    Merge nm = Merge::Run(FileKind::Networks, nets, netsMine, netsTheirs, o);
    INFO(ConflictList(nm));
    CHECK(nm.Conflicts().empty());
    TempDir out;
    out.Write("xlights_networks.xml", nm.ResultText());
    OutputManager om;
    REQUIRE(om.Load(out.path.string()));
    CHECK(om.GetController("Renamed") != nullptr);
    CHECK(om.GetController("Second") != nullptr);
    CHECK(om.GetControllers().size() == 2);

    // control: without the log the rename reads as a delete of a changed record
    Merge noLog = Merge::Run(FileKind::Networks, nets, netsMine, netsTheirs);
    CHECK_FALSE(noLog.Conflicts().empty());
}

// --- the rename log chokepoints -------------------------------------------------------------

TEST_CASE("Rename log: model and group renames are recorded by the model manager", "[merge][renamelog]") {
    ShowFolder f;
    f.Load(Base());
    LayoutRenameLog& log = f.ctx.AllModels.GetRenameLog();
    CHECK(log.Empty());
    REQUIRE(f.ctx.AllModels.Rename("M1", "First"));
    f.ctx.AllModels.Rename("First", "Second");
    f.ctx.AllModels.Rename("G_All", "Everything");
    CHECK(log.CurrentName("M1") == "Second");
    CHECK(log.CurrentName("G_All") == "Everything");
    // a failed rename is not recorded
    f.ctx.AllModels.Rename("NoSuchModel", "X");
    CHECK(log.CurrentName("NoSuchModel") == "NoSuchModel");
    f.ctx.AllModels.clear();
    CHECK(log.Empty());
}

TEST_CASE("Rename log: view object renames are recorded by the view object manager", "[merge][renamelog]") {
    ShowFolder f;
    f.Load(Base());
    LayoutRenameLog& log = f.ctx.AllObjects.GetRenameLog();
    CHECK(log.Empty());
    f.ctx.AllObjects.Rename("Grid", "Floor");
    CHECK(log.CurrentName("Grid") == "Floor");
    f.ctx.AllObjects.clear();
    CHECK(log.Empty());
}

TEST_CASE("Rename log: controller renames are recorded for controllers the manager holds", "[merge][renamelog]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", SeedNetworks());
    OutputManager om;
    REQUIRE(om.Load(dir.path.string()));
    CHECK(om.GetRenameLog().Empty());
    om.GetController("Bulk")->SetName("Garage");
    CHECK(om.GetRenameLog().CurrentName("Bulk") == "Garage");

    // a copy being named before it is added is not a rename
    Controller* copy = om.GetController("Spare")->Copy(&om);
    copy->SetName("Copy");
    CHECK(om.GetRenameLog().CurrentName("Spare") == "Spare");
    om.AddController(copy);
    CHECK(om.GetRenameLog().Entries().size() == 1);

    REQUIRE(om.Load(dir.path.string()));
    CHECK(om.GetRenameLog().Empty());
}
