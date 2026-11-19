#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/ViewObjectManager.h"
#include "outputs/OutputManager.h"
#include "render/HeadlessRenderContext.h"
#include "render/Element.h"
#include "render/EffectLayer.h"
#include "render/SequenceElements.h"
#include "render/SequenceMutationGuard.h"
#include "render/ShowMergeApply.h"
#include "render/UndoManager.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/LayoutObjectId.h"
#include "utils/LayoutRenameLog.h"
#include "utils/ShowAutosave.h"
#include "utils/ShowFileGuard.h"
#include "utils/ShowSaveMerge.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializingVisitor.h"

// Saving the shared show files through a merge (utils/ShowSaveMerge.h) and
// bringing memory up to the merged result (render/ShowMergeApply.h).
using namespace ShowSaveMerge;
using ShowFileMerge::Resolution;

namespace {

const char* kA = "aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa";
const char* kB = "bbbbbbbb-bbbb-4bbb-8bbb-bbbbbbbbbbbb";
const char* kC = "cccccccc-cccc-4ccc-8ccc-cccccccccccc";

std::string Id(const std::string& id) {
    return id.empty() ? std::string() : " xlId=\"" + id + "\"";
}

std::string ModelXml(const std::string& name, const std::string& id = "", const std::string& attrs = "") {
    std::string start = attrs.find("StartChannel=") == std::string::npos ? " StartChannel=\"1\"" : "";
    return "<model name=\"" + name + "\" DisplayAs=\"Single Line\"" + start + " NodesPerString=\"10\"" + Id(id) + " " + attrs + "/>";
}

std::string Rgb(const std::string& models, const std::string& groups = "", const std::string& extra = "") {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models type=\"rgb_effects\">" + models + "</models><modelGroups type=\"rgb_effects\">" +
           groups + "</modelGroups>" + extra + "</xrgb>";
}

std::string ControllerXml(const std::string& name, const std::string& id = "") {
    return "<Controller Name=\"" + name + "\" Type=\"Ethernet\" IP=\"10.0.0.1\"" + Id(id) + "><network NetworkType=\"DDP\" MaxChannels=\"512\"/></Controller>";
}

std::string Networks(const std::string& controllers) {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks computer=\"one\">" + controllers + "</Networks>";
}

std::string Read(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::string Attr(const std::string& xml, const std::string& model, const char* attr) {
    pugi::xml_document d;
    REQUIRE(d.load_string(xml.c_str()));
    return d.document_element().child("models").find_child_by_attribute("model", "name", model.c_str()).attribute(attr).value();
}

bool HasModel(const std::string& xml, const std::string& model) {
    pugi::xml_document d;
    REQUIRE(d.load_string(xml.c_str()));
    return (bool)d.document_element().child("models").find_child_by_attribute("model", "name", model.c_str());
}

FileVersions Changed(ShowFileMerge::FileKind kind, const std::string& path, const std::string& base, const std::string& mine,
                     const std::string& theirs) {
    FileVersions v;
    v.kind = kind;
    v.path = path;
    v.base = base;
    v.mine = mine;
    v.theirs = theirs;
    v.theirsFingerprint = ShowFileGuard::FingerprintOf(theirs);
    v.changedOnDisk = base != theirs;
    return v;
}

Plan One(FileVersions v, const ShowFileMerge::Options& o = {}) {
    std::vector<FileVersions> files;
    files.push_back(std::move(v));
    return Plan::FromVersions(std::move(files), o);
}

// A show folder on disk whose layout file this process loaded (recorded in
// its own guard), then someone else may rewrite.
struct SharedFolder {
    TempDir dir;
    ShowFileGuard guard;
    std::string rgb;
    explicit SharedFolder(const std::string& loaded) {
        rgb = dir.Write("xlights_rgbeffects.xml", loaded);
        guard.RecordFromDisk(rgb);
    }
    void TheyWrite(const std::string& text) { dir.Write("xlights_rgbeffects.xml", text); }
    Plan Prepare(const std::string& mine) {
        return Plan::Prepare({ { ShowFileMerge::FileKind::RgbEffects, rgb, mine } }, {}, guard);
    }
};

} // namespace

// --- the guard keeps the base ---------------------------------------------------------------

TEST_CASE("Save merge: a held layout leaves the presets to save", "[showsavemerge]") {
    using ShowSaveMerge::FileKind;
    using ShowSaveMerge::FileToSave;
    CHECK(ShowSaveMerge::HeldWithLayout(FileKind::RgbEffects));
    CHECK(ShowSaveMerge::HeldWithLayout(FileKind::Networks));
    CHECK_FALSE(ShowSaveMerge::HeldWithLayout(FileKind::Presets));
    std::vector<FileToSave> files{ { FileKind::RgbEffects, "rgb", "r", true },
                                   { FileKind::Networks, "nets", "n", false },
                                   { FileKind::Presets, "presets", "p", true } };
    auto split = ShowSaveMerge::SplitHeldWithLayout(files);
    REQUIRE(split.writable.size() == 1);
    CHECK(split.writable[0].kind == FileKind::Presets);
    CHECK(split.writable[0].path == "presets");
    CHECK(split.writable[0].mine == "p");
    CHECK(split.writable[0].write);
    // Only what would have been written counts as held.
    CHECK(split.held == std::vector<std::string>{ "rgb" });
    auto layoutOnly = ShowSaveMerge::SplitHeldWithLayout({ { FileKind::RgbEffects, "rgb", "r", true } });
    CHECK(layoutOnly.writable.empty());
    CHECK(layoutOnly.held == std::vector<std::string>{ "rgb" });
}

TEST_CASE("Save merge: the guard keeps the loaded text of the shared show files only", "[showsavemerge]") {
    TempDir dir;
    ShowFileGuard guard;
    const std::string rgb = dir.Write("xlights_rgbeffects.xml", Rgb(ModelXml("A")));
    const std::string nets = dir.Write("xlights_networks.xml", Networks(ControllerXml("C1")));
    const std::string presets = dir.Write("xlights_effectpresets.json", "{\"version\":\"0007\",\"children\":[]}");
    const std::string seq = dir.Write("Song.xsq", "<xsequence/>");
    const std::string missing = dir.File("xlights_rgbeffects_other.xml");
    for (const auto& p : { rgb, nets, presets, seq }) guard.RecordFromDisk(p);

    CHECK(guard.RecordedText(rgb) == Rgb(ModelXml("A")));
    CHECK(guard.RecordedText(nets) == Networks(ControllerXml("C1")));
    CHECK(guard.RecordedText(presets).has_value());
    // control: a sequence is tracked but its text is not kept
    CHECK(guard.IsTracked(seq));
    CHECK_FALSE(guard.RecordedText(seq).has_value());
    CHECK_FALSE(guard.RecordedText(missing).has_value());

    // a save records the new base; a missing file's base is empty
    REQUIRE(guard.SaveFile(rgb, Rgb(ModelXml("B"))));
    CHECK(guard.RecordedText(rgb) == Rgb(ModelXml("B")));
    TempDir other;
    const std::string absent = other.File("xlights_networks.xml");
    guard.RecordFromDisk(absent);
    CHECK(guard.RecordedText(absent) == std::string());
    guard.Forget(rgb);
    CHECK_FALSE(guard.RecordedText(rgb).has_value());
}

// --- plain saves ----------------------------------------------------------------------------

TEST_CASE("Save merge: a file nobody else changed saves as it always did", "[showsavemerge]") {
    SharedFolder f(Rgb(ModelXml("A")));
    const std::string mine = Rgb(ModelXml("A", "", "Brightness=\"40\""));
    Plan plan = f.Prepare(mine);
    CHECK_FALSE(plan.ExternalChanges());
    CHECK(plan.Status(0) == FileStatus::Unchanged);
    CHECK_FALSE(plan.HasConflicts());
    CHECK(CanSaveUnattended(plan));
    REQUIRE(plan.Write(f.guard) == WriteResult::Written);
    CHECK(Read(f.rgb) == mine);
    CHECK(f.guard.RecordedText(f.rgb) == mine);
    CHECK_FALSE(f.guard.HasChangedOnDisk(f.rgb));
    CHECK_FALSE(plan.Report().HasSomethingToSay());
}

// --- merging --------------------------------------------------------------------------------

TEST_CASE("Save merge: disjoint changes both survive", "[showsavemerge]") {
    SharedFolder f(Rgb(ModelXml("A", kA) + ModelXml("B", kB)));
    f.TheyWrite(Rgb(ModelXml("A", kA) + ModelXml("B", kB, "Brightness=\"20\"") + ModelXml("C", kC)));
    Plan plan = f.Prepare(Rgb(ModelXml("A", kA, "Brightness=\"70\"") + ModelXml("B", kB)));
    REQUIRE(plan.ExternalChanges());
    REQUIRE(plan.Status(0) == FileStatus::Merged);
    CHECK(plan.CanMerge());
    CHECK_FALSE(plan.HasConflicts());
    CHECK(CanSaveUnattended(plan));
    CHECK(plan.ResultDiffersFromMine(0));
    REQUIRE(plan.Write(f.guard) == WriteResult::Written);
    const std::string onDisk = Read(f.rgb);
    CHECK(Attr(onDisk, "A", "Brightness") == "70");
    CHECK(Attr(onDisk, "B", "Brightness") == "20");
    CHECK(HasModel(onDisk, "C"));
    // the merged file is the new base, so the next save compares against it
    CHECK(f.guard.RecordedText(f.rgb) == onDisk);
    CHECK_FALSE(f.guard.HasChangedOnDisk(f.rgb));
    auto report = plan.Report();
    CHECK(report.mergedFiles == std::vector<std::string>{ "xlights_rgbeffects.xml" });
}

TEST_CASE("Save merge: conflicts block the save until every one is resolved", "[showsavemerge]") {
    SharedFolder f(Rgb(ModelXml("A", kA) + ModelXml("B", kB)));
    const std::string theirs = Rgb(ModelXml("A", kA, "Brightness=\"20\"") + ModelXml("B", kB, "Description=\"theirs\""));
    f.TheyWrite(theirs);
    Plan plan = f.Prepare(Rgb(ModelXml("A", kA, "Brightness=\"70\"") + ModelXml("B", kB, "Description=\"mine\"")));
    REQUIRE(plan.Conflicts().size() == 2);
    CHECK_FALSE(plan.AllResolved());
    CHECK_FALSE(CanSaveUnattended(plan));
    std::string error;
    CHECK(plan.Write(f.guard, &error) == WriteResult::Refused);
    CHECK_FALSE(error.empty());
    CHECK(Read(f.rgb) == theirs);

    auto refs = plan.Conflicts();
    ConflictText t = plan.Describe(refs[0]);
    CHECK(t.record.find("Model '") == 0);
    CHECK(t.what.find("both changed") != std::string::npos);

    REQUIRE(plan.Resolve(refs[0], Resolution::Mine));
    CHECK_FALSE(plan.AllResolved());
    CHECK(plan.Write(f.guard) == WriteResult::Refused);
    CHECK(Read(f.rgb) == theirs);
    REQUIRE(plan.Resolve(refs[1], Resolution::Theirs));
    REQUIRE(plan.AllResolved());
    REQUIRE(plan.Write(f.guard) == WriteResult::Written);
    const std::string onDisk = Read(f.rgb);
    const std::string brightness = Attr(onDisk, "A", "Brightness");
    const std::string description = Attr(onDisk, "B", "Description");
    // the first conflict is A's (mine), the second B's (theirs)
    CHECK(brightness == "70");
    CHECK(description == "theirs");
}

TEST_CASE("Save merge: all mine and all theirs", "[showsavemerge]") {
    const std::string base = Rgb(ModelXml("A", kA) + ModelXml("B", kB));
    const std::string mine = Rgb(ModelXml("A", kA, "Brightness=\"70\"") + ModelXml("B", kB, "Description=\"mine\""));
    const std::string theirs = Rgb(ModelXml("A", kA, "Brightness=\"20\"") + ModelXml("B", kB, "Description=\"theirs\""));
    Plan plan = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", base, mine, theirs));
    plan.ResolveAll(Resolution::Mine);
    REQUIRE(plan.AllResolved());
    CHECK(Attr(plan.Result(0), "A", "Brightness") == "70");
    CHECK(Attr(plan.Result(0), "B", "Description") == "mine");
    plan.ResolveAll(Resolution::Theirs);
    CHECK(Attr(plan.Result(0), "A", "Brightness") == "20");
    CHECK(Attr(plan.Result(0), "B", "Description") == "theirs");
}

TEST_CASE("Save merge: a file whose base is unknown is not merged", "[showsavemerge]") {
    FileVersions v = Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", Rgb(ModelXml("A")), Rgb(ModelXml("A", "", "Brightness=\"1\"")),
                             Rgb(ModelXml("B")));
    v.base.reset();
    Plan plan = One(std::move(v));
    CHECK(plan.ExternalChanges());
    CHECK(plan.Status(0) == FileStatus::NoBase);
    CHECK_FALSE(plan.CanMerge());
    CHECK_FALSE(CanSaveUnattended(plan));
    ShowFileGuard guard;
    CHECK(plan.Write(guard) == WriteResult::Refused);
}

TEST_CASE("Save merge: a rename on one side and an edit on the other both land", "[showsavemerge]") {
    // with ids
    {
        const std::string base = Rgb(ModelXml("Arch", kA) + ModelXml("B", kB), "<modelGroup name=\"G\" models=\"Arch,B\" layout=\"grid\"/>");
        const std::string mine = Rgb(ModelXml("Arch Left", kA) + ModelXml("B", kB), "<modelGroup name=\"G\" models=\"Arch Left,B\" layout=\"grid\"/>");
        const std::string theirs = Rgb(ModelXml("Arch", kA, "Brightness=\"30\"") + ModelXml("B", kB));
        Plan plan = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", base, mine, theirs));
        REQUIRE_FALSE(plan.HasConflicts());
        const std::string r = plan.Result(0);
        CHECK_FALSE(HasModel(r, "Arch"));
        CHECK(Attr(r, "Arch Left", "Brightness") == "30");
    }
    // without ids (an older version wrote theirs): the rename log matches it
    {
        const std::string base = Rgb(ModelXml("Arch") + ModelXml("B"));
        const std::string mine = Rgb(ModelXml("Arch Left") + ModelXml("B"));
        const std::string theirs = Rgb(ModelXml("Arch", "", "Brightness=\"30\"") + ModelXml("B"));
        LayoutRenameLog log;
        log.Record("Arch", "Arch Left");
        ShowFileMerge::Options o;
        o.modelRenames = &log;
        Plan plan = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", base, mine, theirs), o);
        CHECK(Attr(plan.Result(0), "Arch Left", "Brightness") == "30");
        CHECK_FALSE(HasModel(plan.Result(0), "Arch"));
        // control: without the log it is a delete plus an add
        Plan without = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", base, mine, theirs));
        CHECK(Attr(without.Result(0), "Arch Left", "Brightness") != "30");
    }
    // their rename is reported
    {
        const std::string base = Rgb(ModelXml("Arch", kA));
        const std::string mine = Rgb(ModelXml("Arch", kA, "Brightness=\"30\""));
        const std::string theirs = Rgb(ModelXml("Arch Left", kA));
        Plan plan = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", base, mine, theirs));
        CHECK(Attr(plan.Result(0), "Arch Left", "Brightness") == "30");
        auto report = plan.Report();
        REQUIRE(report.renames.size() == 1);
        CHECK(report.renames[0] == "Model 'Arch' was renamed to 'Arch Left' by someone else.");
        CHECK(report.Text().find("renamed to 'Arch Left'") != std::string::npos);
    }
}

TEST_CASE("Save merge: the controllers merge first and their renames reach the layout", "[showsavemerge]") {
    const std::string nb = Networks(ControllerXml("C1", kA));
    const std::string nm = Networks(ControllerXml("Porch", kA));
    const std::string nt = Networks(ControllerXml("C1", kA) + ControllerXml("C2", kB));
    const std::string rb = Rgb(ModelXml("A", kA, "Controller=\"C1\" StartChannel=\"!C1:1\""));
    const std::string rm = Rgb(ModelXml("A", kA, "Controller=\"Porch\" StartChannel=\"!Porch:1\""));
    const std::string rt = Rgb(ModelXml("A", kA, "Controller=\"C1\" StartChannel=\"!C1:1\"") + ModelXml("B", kB, "Controller=\"C1\" StartChannel=\"!C1:301\""));

    // the layout listed first: the plan still merges the controllers first
    std::vector<FileVersions> files;
    files.push_back(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", rb, rm, rt));
    files.push_back(Changed(ShowFileMerge::FileKind::Networks, "/x/xlights_networks.xml", nb, nm, nt));
    Plan plan = Plan::FromVersions(std::move(files), {});
    REQUIRE_FALSE(plan.HasConflicts());
    CHECK(Attr(plan.Result(0), "B", "Controller") == "Porch");
    CHECK(Attr(plan.Result(0), "B", "StartChannel") == "!Porch:301");

    // a controllers file only consulted (not written) still feeds the layout
    std::vector<FileVersions> ctx;
    ctx.push_back(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", rb, rm, rt));
    ctx.push_back(Changed(ShowFileMerge::FileKind::Networks, "/x/xlights_networks.xml", nb, nm, nt));
    ctx.back().write = false;
    Plan consulted = Plan::FromVersions(std::move(ctx), {});
    CHECK(Attr(consulted.Result(0), "B", "Controller") == "Porch");
    CHECK_FALSE(consulted.Find(ShowFileMerge::FileKind::Networks).has_value());
    CHECK(consulted.Find(ShowFileMerge::FileKind::Networks, false).has_value());

    // control: the layout alone keeps the old controller name
    Plan alone = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", rb, rm, rt));
    CHECK(Attr(alone.Result(0), "B", "Controller") == "C1");
}

TEST_CASE("Save merge: deciding a controller rename conflict re-merges the layout", "[showsavemerge]") {
    const std::string nb = Networks(ControllerXml("C1", kA));
    const std::string nm = Networks(ControllerXml("Porch", kA));
    const std::string nt = Networks(ControllerXml("Garage", kA));
    const std::string rb = Rgb(ModelXml("A", kA, "Controller=\"C1\" StartChannel=\"!C1:1\""));
    const std::string rm = Rgb(ModelXml("A", kA, "Controller=\"Porch\" StartChannel=\"!Porch:1\"") + ModelXml("M", kB, "Controller=\"Porch\" StartChannel=\"!Porch:301\""));
    const std::string rt = Rgb(ModelXml("A", kA, "Controller=\"Garage\" StartChannel=\"!Garage:1\"") + ModelXml("T", kC, "Controller=\"Garage\" StartChannel=\"!Garage:301\""));
    std::vector<FileVersions> files;
    files.push_back(Changed(ShowFileMerge::FileKind::Networks, "/x/xlights_networks.xml", nb, nm, nt));
    files.push_back(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", rb, rm, rt));
    Plan plan = Plan::FromVersions(std::move(files), {});
    auto refs = plan.Conflicts();
    REQUIRE_FALSE(refs.empty());
    // the controllers' conflicts are listed first
    CHECK(plan.Kind(refs[0].file) == ShowFileMerge::FileKind::Networks);
    CHECK(plan.GetConflict(refs[0]).kind == ShowFileMerge::ConflictKind::BothRenamed);

    REQUIRE(plan.Resolve(refs[0], Resolution::Mine));
    plan.ResolveAll(Resolution::Mine);
    REQUIRE(plan.AllResolved());
    const std::string r = plan.Result(1);
    CHECK(Attr(r, "T", "Controller") == "Porch");
    CHECK(Attr(r, "M", "Controller") == "Porch");

    plan.ResolveAll(Resolution::Theirs);
    REQUIRE(plan.AllResolved());
    const std::string t = plan.Result(1);
    CHECK(Attr(t, "T", "Controller") == "Garage");
    CHECK(Attr(t, "M", "Controller") == "Garage");
}

TEST_CASE("Save merge: nothing is written when a file changes again while saving", "[showsavemerge]") {
    SharedFolder f(Rgb(ModelXml("A", kA) + ModelXml("B", kB)));
    f.TheyWrite(Rgb(ModelXml("A", kA, "Brightness=\"20\"") + ModelXml("B", kB)));
    const std::string mine = Rgb(ModelXml("A", kA, "Brightness=\"70\"") + ModelXml("B", kB));
    Plan plan = f.Prepare(mine);
    REQUIRE(plan.Conflicts().size() == 1);
    REQUIRE(plan.Resolve(plan.Conflicts()[0], Resolution::Mine));
    const std::string again = Rgb(ModelXml("A", kA, "Brightness=\"20\"") + ModelXml("B", kB, "Description=\"later\""));
    f.TheyWrite(again);
    CHECK(plan.Write(f.guard) == WriteResult::ChangedAgain);
    CHECK(Read(f.rgb) == again);

    Plan retry = f.Prepare(mine);
    retry.AdoptResolutions(plan);
    REQUIRE(retry.AllResolved());
    REQUIRE(retry.Write(f.guard) == WriteResult::Written);
    const std::string onDisk = Read(f.rgb);
    CHECK(Attr(onDisk, "A", "Brightness") == "70");
    CHECK(Attr(onDisk, "B", "Description") == "later");
}

TEST_CASE("Save merge: only problems the merge brought in are reported", "[showsavemerge]") {
    // mine chains a new model off A; they deleted A. Z's dangling chain was
    // there before and is not news.
    const std::string base = Rgb(ModelXml("A", kA) + ModelXml("Z", kC, "StartChannel=\">Gone:1\""));
    const std::string mine = Rgb(ModelXml("A", kA) + ModelXml("Z", kC, "StartChannel=\">Gone:1\"") + ModelXml("N", kB, "StartChannel=\">A:1\""));
    const std::string theirs = Rgb(ModelXml("Z", kC, "StartChannel=\">Gone:1\""));
    Plan plan = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", base, mine, theirs));
    REQUIRE_FALSE(plan.HasConflicts());
    CHECK_FALSE(HasModel(plan.Result(0), "A"));
    auto report = plan.Report();
    CHECK(report.HasSomethingToSay());
    bool aboutN = false, aboutZ = false;
    for (const auto& i : report.issues) {
        aboutN |= i.record == "N";
        aboutZ |= i.record == "Z";
    }
    CHECK(aboutN);
    CHECK_FALSE(aboutZ);
}

TEST_CASE("Save merge: rename logs are cleared for the files written", "[showsavemerge]") {
    LayoutRenameLog models, objects, controllers;
    auto fill = [&] {
        models.Record("a", "b");
        objects.Record("c", "d");
        controllers.Record("e", "f");
    };
    fill();
    Plan layout = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", Rgb(ModelXml("A")), Rgb(ModelXml("A")), Rgb(ModelXml("A"))));
    ClearRenameLogs(layout, &models, &objects, &controllers);
    CHECK(models.Empty());
    CHECK(objects.Empty());
    CHECK_FALSE(controllers.Empty());

    fill();
    Plan nets = One(Changed(ShowFileMerge::FileKind::Networks, "/x/xlights_networks.xml", Networks(""), Networks(""), Networks("")));
    ClearRenameLogs(nets, &models, &objects, &controllers);
    CHECK(controllers.Empty());
    CHECK_FALSE(models.Empty());
}

TEST_CASE("Save merge: unattended saves merge only when nothing needs deciding", "[showsavemerge]") {
    const std::string base = Rgb(ModelXml("A", kA) + ModelXml("B", kB));
    Plan clean = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", base, Rgb(ModelXml("A", kA, "Brightness=\"1\"") + ModelXml("B", kB)),
                             Rgb(ModelXml("A", kA) + ModelXml("B", kB, "Brightness=\"2\""))));
    CHECK(CanSaveUnattended(clean));
    Plan conflicted = One(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", base, Rgb(ModelXml("A", kA, "Brightness=\"1\"") + ModelXml("B", kB)),
                                  Rgb(ModelXml("A", kA, "Brightness=\"2\"") + ModelXml("B", kB))));
    CHECK_FALSE(CanSaveUnattended(conflicted));
    // once decided it still is not an unattended save's call
    conflicted.ResolveAll(Resolution::Mine);
    CHECK_FALSE(CanSaveUnattended(conflicted));
}

TEST_CASE("Save merge: presets merge per preset", "[showsavemerge]") {
    auto preset = [](const std::string& name, const std::string& settings) {
        return "{\"type\":\"preset\",\"name\":\"" + name + "\",\"settings\":\"" + settings + "\",\"version\":\"1\",\"xLightsVersion\":\"x\"}";
    };
    auto file = [](const std::string& children) { return "{\"version\":\"0007\",\"children\":[" + children + "]}"; };
    const std::string base = file(preset("P1", "a") + "," + preset("P2", "b"));
    const std::string mine = file(preset("P1", "mine") + "," + preset("P2", "b"));
    const std::string theirs = file(preset("P1", "a") + "," + preset("P2", "theirs") + "," + preset("P3", "c"));
    Plan plan = One(Changed(ShowFileMerge::FileKind::Presets, "/x/xlights_effectpresets.json", base, mine, theirs));
    REQUIRE(plan.Status(0) == FileStatus::Merged);
    CHECK_FALSE(plan.HasConflicts());
    const std::string r = plan.Result(0);
    CHECK(r.find("\"mine\"") != std::string::npos);
    CHECK(r.find("\"theirs\"") != std::string::npos);
    CHECK(r.find("\"P3\"") != std::string::npos);
}

TEST_CASE("Save merge: conflict values read as text, not XML", "[showsavemerge]") {
    CHECK(ReadableValue(std::string("12")) == "12");
    CHECK(ReadableValue(std::nullopt) == "(none)");
    const std::string sub = "<subModel name=\"Left\" layout=\"horizontal\" type=\"ranges\" line0=\"1-5\" xlId=\"x\"/>";
    CHECK(ReadableValue(sub) == "subModel 'Left': layout=horizontal, type=ranges, line0=1-5");
    CHECK(RecordKindName("modelGroups") == "Group");
}

// --- autosave -------------------------------------------------------------------------------

TEST_CASE("Save merge: an autosave keeps its base and restoring it merges", "[showsavemerge][autosave]") {
    TempDir dir;
    const std::string loaded = Rgb(ModelXml("A", kA) + ModelXml("B", kB));
    const std::string real = dir.Write("xlights_rgbeffects.xml", loaded);
    const std::string backup = dir.File("xlights_rgbeffects.xbkp");
    const std::string mine = Rgb(ModelXml("A", kA, "Brightness=\"70\"") + ModelXml("B", kB));
    {
        // the session that crashed
        ShowFileGuard guard;
        guard.RecordFromDisk(real);
        dir.Write("xlights_rgbeffects.xbkp", mine);
        REQUIRE(ShowAutosave::WriteSidecarAndBase(backup, real, guard));
        CHECK(ShowAutosave::ReadBaseCopy(backup) == loaded);
    }
    // someone else saves before this install restarts
    dir.Write("xlights_rgbeffects.xml", Rgb(ModelXml("A", kA) + ModelXml("B", kB, "Brightness=\"20\"")));

    auto plan = PrepareAutosaveMerge(ShowFileMerge::FileKind::RgbEffects, real, backup);
    REQUIRE(plan.has_value());
    CHECK(plan->ExternalChanges());
    CHECK_FALSE(plan->HasConflicts());
    ShowFileGuard fresh;
    REQUIRE(plan->Write(fresh) == WriteResult::Written);
    const std::string onDisk = Read(real);
    CHECK(Attr(onDisk, "A", "Brightness") == "70");
    CHECK(Attr(onDisk, "B", "Brightness") == "20");
    CHECK(fresh.RecordedText(real) == onDisk);

    // removing the autosave removes its base copy
    ShowAutosave::Remove(backup);
    CHECK_FALSE(std::filesystem::exists(ShowAutosave::BaseCopyPath(backup)));
}

TEST_CASE("Save merge: a stale or missing autosave base is not used", "[showsavemerge][autosave]") {
    TempDir dir;
    const std::string real = dir.Write("xlights_rgbeffects.xml", Rgb(ModelXml("A")));
    const std::string backup = dir.Write("xlights_rgbeffects.xbkp", Rgb(ModelXml("A", "", "Brightness=\"1\"")));
    ShowFileGuard guard;
    guard.RecordFromDisk(real);
    REQUIRE(ShowAutosave::WriteSidecar(backup, ShowAutosave::BaseFor(real, guard)));
    // control: no base copy at all
    CHECK_FALSE(ShowAutosave::ReadBaseCopy(backup).has_value());
    CHECK_FALSE(PrepareAutosaveMerge(ShowFileMerge::FileKind::RgbEffects, real, backup).has_value());
    // a base copy that is not the version the sidecar names
    dir.Write("xlights_rgbeffects.xbkp.base", Rgb(ModelXml("Other")));
    CHECK_FALSE(ShowAutosave::ReadBaseCopy(backup).has_value());
    // the right one
    REQUIRE(ShowAutosave::WriteBaseCopy(backup, Rgb(ModelXml("A"))));
    CHECK(ShowAutosave::ReadBaseCopy(backup) == Rgb(ModelXml("A")));
}

// --- memory follows the merged file ---------------------------------------------------------

namespace {

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

std::string SeedNetworks() {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks computer=\"one\">"
           "<Controller Type=\"Null\" Id=\"64001\" Name=\"Bulk\"><network NetworkType=\"NULL\" MaxChannels=\"500000\"/></Controller></Networks>";
}

std::string LineModel(const std::string& name, const std::string& start, const std::string& extra = "") {
    return "<model name=\"" + name + "\" DisplayAs=\"Single Line\" StringType=\"RGB Nodes\" NumStrings=\"1\" NodesPerString=\"10\" StartChannel=\"" +
           start + "\" " + extra + "/>";
}

struct Show {
    CurlLogger curl;
    TempDir dir;
    FixFileReset reset;
    HeadlessRenderContext ctx;
    void Load(const std::string& rgb) {
        dir.Write("xlights_networks.xml", SeedNetworks());
        dir.Write("xlights_rgbeffects.xml", rgb);
        REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    }
    Model* M(const std::string& name) { return ctx.AllModels.GetModel(name); }
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
};

pugi::xml_document Parse(const std::string& text) {
    pugi::xml_document d;
    REQUIRE(d.load_string(text.c_str()));
    return d;
}

} // namespace

TEST_CASE("Save merge: memory matches the merged file after the save", "[seq][showsavemerge][apply]") {
    const std::string seed = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models>" + LineModel("A", "1") + LineModel("B", ">A:1") +
                             LineModel("D", ">B:1") + LineModel("E", ">D:1") +
                             "</models><modelGroups><modelGroup name=\"G\" models=\"A,B,E\" layout=\"minimalGrid\" GridSize=\"400\"/></modelGroups>"
                             "<views><view name=\"Front\" models=\"A,E\"/></views></xrgb>";
    Show base;
    base.Load(seed);
    const std::string baseText = base.Rgb();

    // mine: an edit in this process
    Show mine;
    mine.Load(baseText);
    mine.M("A")->SetDescription("mine");
    const std::string mineText = mine.Rgb();

    // theirs: someone else's edits to other records
    Show theirs;
    theirs.Load(baseText);
    theirs.M("B")->SetPixelSize(5);
    REQUIRE(theirs.ctx.AllModels.Delete("D"));
    theirs.ctx.AllModels.Rename("E", "E2");
    theirs.ctx._sequenceViewManager.RenameModel("E", "E2");
    {
        pugi::xml_document d = Parse("<r>" + LineModel("C", ">B:1") + "</r>");
        REQUIRE(theirs.ctx.AllModels.createAndAddModel(d.document_element().first_child(), 100, 100) != nullptr);
    }
    const std::string theirsText = theirs.Rgb();

    Plan plan = One(Changed(ShowFileMerge::FileKind::RgbEffects, mine.dir.File("xlights_rgbeffects.xml"), baseText, mineText, theirsText));
    REQUIRE(plan.Status(0) == FileStatus::Merged);
    REQUIRE_FALSE(plan.HasConflicts());
    const std::string resultText = plan.Result(0);

    Show expected;
    expected.Load(resultText);
    const std::string want = expected.Rgb();
    // control: memory does not match before it is brought up to the result
    CHECK(mine.Rgb() != want);

    pugi::xml_document mineDoc = Parse(mineText);
    pugi::xml_document resultDoc = Parse(resultText);
    REQUIRE(mine.ctx.AbortRender());
    ShowMergeApply::LayoutChanges changes = ShowMergeApply::ApplyLayout(mine.ctx, mineDoc, resultDoc);
    CHECK(mine.Rgb() == want);

    CHECK(mine.M("A")->GetDescription() == "mine");
    REQUIRE(mine.M("B") != nullptr);
    CHECK(mine.M("B")->GetPixelSize() == 5);
    CHECK(mine.M("C") != nullptr);
    CHECK(mine.M("D") == nullptr);
    CHECK(mine.M("E") == nullptr);
    CHECK(mine.M("E2") != nullptr);
    auto* g = dynamic_cast<ModelGroup*>(mine.M("G"));
    REQUIRE(g != nullptr);
    CHECK(g->ModelNames() == std::vector<std::string>{ "A", "B", "E2" });
    CHECK(changes.modelsAdded == std::vector<std::string>{ "C" });
    CHECK(changes.modelsRemoved == std::vector<std::string>{ "D" });
    CHECK(changes.modelsRenamed == std::vector<std::pair<std::string, std::string>>{ { "E", "E2" } });
    CHECK(changes.viewsReloaded);
    // a record that did not change is not rebuilt
    for (const auto& n : changes.modelsChanged) CHECK(n != "A");
    // the layout chains through the merged models
    CHECK(mine.M("C")->GetFirstChannel() == mine.M("B")->GetLastChannel() + 1);

    // applying it again changes nothing
    pugi::xml_document again = Parse(mine.Rgb());
    auto none = ShowMergeApply::ApplyLayout(mine.ctx, again, resultDoc);
    CHECK_FALSE(none.ModelsChanged());
}

TEST_CASE("Save merge: model checks report only what is new", "[seq][showsavemerge][apply]") {
    CHECK(ShowMergeApply::NewMessages({ "a", "b" }, { "b", "c", "c" }) == std::vector<std::string>{ "c" });
    Show s;
    s.Load("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models>" + LineModel("A", "1") + LineModel("B", "1") + "</models></xrgb>");
    auto before = ShowMergeApply::ModelCheckMessages(s.ctx);
    // two models on the same channels: the overlap check speaks
    bool overlap = false;
    for (const auto& m : before) overlap |= m.find("overlap") != std::string::npos || m.find("Overlap") != std::string::npos;
    CHECK(overlap);
}

// --- a merged deletion of something the open sequence has effects on -----------------------

namespace {

SequenceUse FakeUse(std::map<std::string, SequenceUseInfo> uses) {
    return [uses](const std::string& name) {
        auto it = uses.find(name);
        return it == uses.end() ? SequenceUseInfo{} : it->second;
    };
}

Plan WithUse(const std::string& base, const std::string& mine, const std::string& theirs, SequenceUse use) {
    std::vector<FileVersions> files;
    files.push_back(Changed(ShowFileMerge::FileKind::RgbEffects, "/x/xlights_rgbeffects.xml", base, mine, theirs));
    return Plan::FromVersions(std::move(files), {}, std::move(use));
}

std::vector<ConflictRef> SequenceConflicts(const Plan& plan) {
    std::vector<ConflictRef> out;
    for (const auto& r : plan.Conflicts()) {
        if (plan.GetConflict(r).kind == ShowFileMerge::ConflictKind::DeletedWithEffects) out.push_back(r);
    }
    return out;
}

bool HasGroup(const std::string& xml, const std::string& group) {
    pugi::xml_document d;
    REQUIRE(d.load_string(xml.c_str()));
    return (bool)d.document_element().child("modelGroups").find_child_by_attribute("modelGroup", "name", group.c_str());
}

} // namespace

TEST_CASE("Save merge: deleting a model the open sequence has effects on is a conflict", "[showsavemerge][seqdelete]") {
    const std::string base = Rgb(ModelXml("A", kA) + ModelXml("B", kB) + ModelXml("C", kC));
    const std::string theirs = Rgb(ModelXml("A", kA) + ModelXml("C", kC));
    const auto use = FakeUse({ { "B", { true, 5 } }, { "A", { true, 0 } } });

    Plan plan = WithUse(base, base, theirs, use);
    auto refs = SequenceConflicts(plan);
    REQUIRE(refs.size() == 1);
    const auto& c = plan.GetConflict(refs[0]);
    CHECK(c.mineName == "B");
    CHECK(c.section == "models");
    CHECK(c.mapAllowed);
    CHECK_FALSE(plan.AllResolved());
    CHECK_FALSE(CanSaveUnattended(plan));
    ShowFileGuard guard;
    CHECK(plan.Write(guard) == WriteResult::Refused);
    auto t = plan.Describe(refs[0]);
    CHECK(t.what.find("5 effects") != std::string::npos);

    // controls: no effects, a row with no effects, or no sequence -> the deletion just merges
    CHECK(SequenceConflicts(WithUse(base, base, theirs, FakeUse({ { "B", { true, 0 } } }))).empty());
    CHECK(SequenceConflicts(WithUse(base, base, theirs, {})).empty());
    // a rename is not a delete
    CHECK(SequenceConflicts(WithUse(base, base, Rgb(ModelXml("A", kA) + ModelXml("B2", kB) + ModelXml("C", kC)), use)).empty());
}

TEST_CASE("Save merge: deleting a group row with effects is a conflict", "[showsavemerge][seqdelete]") {
    const std::string base = Rgb(ModelXml("A", kA) + ModelXml("B", kB), "<modelGroup name=\"G\" models=\"A,B\" layout=\"grid\"/>");
    const std::string theirs = Rgb(ModelXml("A", kA) + ModelXml("B", kB));
    Plan plan = WithUse(base, base, theirs, FakeUse({ { "G", { true, 2 } } }));
    auto refs = SequenceConflicts(plan);
    REQUIRE(refs.size() == 1);
    CHECK(plan.GetConflict(refs[0]).section == "modelGroups");
    REQUIRE(plan.Resolve(refs[0], Resolution::Mine));
    CHECK(HasGroup(plan.Result(0), "G"));
}

TEST_CASE("Save merge: keep, delete and map decide a deletion with effects", "[showsavemerge][seqdelete]") {
    const std::string base = Rgb(ModelXml("A", kA) + ModelXml("B", kB, "Brightness=\"40\"") + ModelXml("C", kC));
    const std::string theirs = Rgb(ModelXml("A", kA) + ModelXml("C", kC));
    const auto use = FakeUse({ { "B", { true, 3 } }, { "A", { true, 1 } } });

    // Keep: the result has this process's record of B
    {
        Plan plan = WithUse(base, base, theirs, use);
        auto refs = SequenceConflicts(plan);
        REQUIRE(plan.Resolve(refs[0], Resolution::Mine));
        REQUIRE(plan.AllResolved());
        CHECK(Attr(plan.Result(0), "B", "Brightness") == "40");
        CHECK(plan.SequenceChanges().empty());
        CHECK(plan.Report().changes.size() == 1);
    }
    // Delete
    {
        Plan plan = WithUse(base, base, theirs, use);
        auto refs = SequenceConflicts(plan);
        REQUIRE(plan.Resolve(refs[0], Resolution::Theirs));
        REQUIRE(plan.AllResolved());
        CHECK_FALSE(HasModel(plan.Result(0), "B"));
        auto changes = plan.SequenceChanges();
        REQUIRE(changes.size() == 1);
        CHECK(changes[0].model == "B");
        CHECK_FALSE(changes[0].map);
        CHECK(changes[0].effects == 3);
        CHECK(plan.Report().HasSomethingToSay());
    }
    // Map: only to a model the sequence has no row for
    {
        Plan plan = WithUse(base, base, theirs, use);
        auto refs = SequenceConflicts(plan);
        CHECK(plan.MapTargets() == std::vector<std::string>{ "C" });
        CHECK_FALSE(plan.Resolve(refs[0], Resolution::Map, "A"));
        CHECK_FALSE(plan.Resolve(refs[0], Resolution::Map, ""));
        CHECK_FALSE(plan.AllResolved());
        REQUIRE(plan.Resolve(refs[0], Resolution::Map, "C"));
        REQUIRE(plan.AllResolved());
        CHECK_FALSE(HasModel(plan.Result(0), "B"));
        auto changes = plan.SequenceChanges();
        REQUIRE(changes.size() == 1);
        CHECK(changes[0].map);
        CHECK(changes[0].target == "C");
        CHECK(plan.Describe(refs[0]).mine == "Move its effects to 'C'");
    }
    // All Mine / All Theirs decide it too
    {
        Plan plan = WithUse(base, base, theirs, use);
        plan.ResolveAll(Resolution::Theirs);
        CHECK(plan.AllResolved());
        CHECK(plan.SequenceChanges().size() == 1);
        plan.ResolveAll(Resolution::Mine);
        CHECK(plan.SequenceChanges().empty());
        CHECK(HasModel(plan.Result(0), "B"));
    }
}

TEST_CASE("Save merge: keeping theirs on a model I changed asks about its effects next", "[showsavemerge][seqdelete]") {
    const std::string base = Rgb(ModelXml("A", kA) + ModelXml("B", kB));
    const std::string mine = Rgb(ModelXml("A", kA) + ModelXml("B", kB, "Brightness=\"70\""));
    const std::string theirs = Rgb(ModelXml("A", kA));
    Plan plan = WithUse(base, mine, theirs, FakeUse({ { "B", { true, 4 } } }));
    // the merge's own question comes first
    REQUIRE(plan.Conflicts().size() == 1);
    CHECK(plan.GetConflict(plan.Conflicts()[0]).kind == ShowFileMerge::ConflictKind::DeletedByTheirs);
    CHECK(SequenceConflicts(plan).empty());
    // "All Theirs" deletes it, which brings the effects question, undecided
    plan.ResolveAll(Resolution::Theirs);
    auto refs = SequenceConflicts(plan);
    REQUIRE(refs.size() == 1);
    CHECK(plan.GetConflict(refs[0]).resolution == Resolution::Unresolved);
    CHECK_FALSE(plan.AllResolved());
    REQUIRE(plan.Resolve(refs[0], Resolution::Mine));
    REQUIRE(plan.AllResolved());
    CHECK(Attr(plan.Result(0), "B", "Brightness") == "70");

    // a retry after the file moved again keeps the decision
    Plan again = WithUse(base, mine, theirs, FakeUse({ { "B", { true, 4 } } }));
    again.AdoptResolutions(plan);
    CHECK(again.AllResolved());
    CHECK(HasModel(again.Result(0), "B"));
}

namespace {

const char* kSeqShow = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models>"
                       "<model name=\"Grid\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\">"
                       "<subModel name=\"Top\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\"/></model>"
                       "<model name=\"Line\" DisplayAs=\"Single Line\" StringType=\"RGB Nodes\" NumStrings=\"1\" NodesPerString=\"10\" StartChannel=\"100\"/>"
                       "<model name=\"Spare\" DisplayAs=\"Single Line\" StringType=\"RGB Nodes\" NumStrings=\"1\" NodesPerString=\"10\" StartChannel=\"200\"/>"
                       "</models><modelGroups><modelGroup name=\"G\" models=\"Line,Spare\" layout=\"minimalGrid\" GridSize=\"400\"/></modelGroups></xrgb>";

// A show with a sequence: Grid has an effect on its own layer and one on its
// "Top" submodel row; group G has one; Line has a row with none.
struct SeqShow : Show {
    SequenceElements& se;
    SeqShow() : se(ctx.GetSequenceElements()) {
        Load(kSeqShow);
        se.SetViewsManager(&ctx._sequenceViewManager);
        SeqMutationGuard::Scope setup;
        auto add = [&](const std::string& name) {
            auto* m = dynamic_cast<ModelElement*>(se.AddElement(name, "model", true, false, false, false, false));
            REQUIRE(m != nullptr);
            if (Model* model = ctx.AllModels.GetModel(name)) m->Init(*model);
            if (m->GetEffectLayerCount() == 0) m->AddEffectLayer();
            return m;
        };
        ModelElement* grid = add("Grid");
        grid->GetEffectLayer(0)->AddEffect(0, "On", "", "", 0, 500, 0, false);
        SubModelElement* top = grid->GetSubModel("Top", true);
        REQUIRE(top != nullptr);
        if (top->GetEffectLayerCount() == 0) top->AddEffectLayer();
        top->GetEffectLayer(0)->AddEffect(0, "Twinkle", "", "", 0, 500, 0, false);
        add("G")->GetEffectLayer(0)->AddEffect(0, "Bars", "", "", 0, 500, 0, false);
        add("Line");
        se.PopulateRowInformation();
    }
    int Effects(const std::string& name) {
        Element* e = se.GetElement(name);
        return e ? e->GetEffectCount() : -1;
    }
};

// Someone else's save of the same layout (same layout ids) without Grid.
std::string WithoutGrid(const std::string& loaded) {
    Show theirs;
    theirs.Load(loaded);
    REQUIRE(theirs.ctx.AllModels.Delete("Grid"));
    return theirs.Rgb();
}

// Someone else's save of the same layout with Grid renamed (same layout id).
std::string WithGridRenamed(const std::string& loaded, const std::string& newName) {
    Show theirs;
    theirs.Load(loaded);
    theirs.ctx.AllModels.Rename("Grid", newName);
    REQUIRE(theirs.ctx.AllModels.GetModel(newName) != nullptr);
    return theirs.Rgb();
}

} // namespace

TEST_CASE("Save merge: the open sequence's use counts submodel and group rows", "[seq][showsavemerge][seqdelete]") {
    SeqShow s;
    auto use = ShowMergeApply::SequenceUseOf(s.ctx);
    CHECK(use("Grid").present);
    CHECK(use("Grid").effects == 2);
    CHECK(use("G").effects == 1);
    CHECK(use("Line").present);
    CHECK(use("Line").effects == 0);
    CHECK_FALSE(use("Spare").present);
}

TEST_CASE("Save merge: each choice for a deleted model with effects, in memory and undone", "[seq][showsavemerge][seqdelete][apply]") {
    auto run = [&](SeqShow& s, Resolution r, const std::string& target) {
        const std::string mine = s.Rgb();
        const std::string theirs = WithoutGrid(mine);
        Plan plan = WithUse(mine, mine, theirs, ShowMergeApply::SequenceUseOf(s.ctx));
        auto refs = SequenceConflicts(plan);
        REQUIRE(refs.size() == 1);
        REQUIRE(plan.Resolve(refs[0], r, target));
        REQUIRE(plan.AllResolved());
        pugi::xml_document mineDoc = Parse(mine);
        pugi::xml_document resultDoc = Parse(plan.Result(0));
        REQUIRE(s.ctx.AbortRender());
        int dropped = ShowMergeApply::ApplySequenceChanges(s.ctx, plan.SequenceChanges());
        auto changes = ShowMergeApply::ApplyLayout(s.ctx, mineDoc, resultDoc);
        CHECK(changes.removedWithEffects.empty());
        return std::make_pair(plan.Result(0), dropped);
    };

    SECTION("keep") {
        SeqShow s;
        auto [saved, dropped] = run(s, Resolution::Mine, "");
        CHECK(HasModel(saved, "Grid"));
        CHECK(s.M("Grid") != nullptr);
        CHECK(s.Effects("Grid") == 2);
        CHECK(dropped == 0);
    }
    SECTION("delete") {
        SeqShow s;
        auto [saved, dropped] = run(s, Resolution::Theirs, "");
        CHECK_FALSE(HasModel(saved, "Grid"));
        CHECK(s.M("Grid") == nullptr);
        CHECK(s.se.GetElement("Grid") == nullptr);
        // one undo step brings the row and both effects back
        REQUIRE(s.se.get_undo_mgr().CanUndo());
        s.se.get_undo_mgr().UndoLastStep();
        CHECK(s.Effects("Grid") == 2);
        CHECK_FALSE(s.se.get_undo_mgr().CanUndo());
    }
    SECTION("map") {
        SeqShow s;
        auto [saved, dropped] = run(s, Resolution::Map, "Spare");
        CHECK_FALSE(HasModel(saved, "Grid"));
        CHECK(s.M("Grid") == nullptr);
        CHECK(s.se.GetElement("Grid") == nullptr);
        // the row's own effect moved; Spare has no "Top" submodel, so that one was dropped
        CHECK(s.Effects("Spare") == 1);
        CHECK(dropped == 1);
        REQUIRE(s.se.get_undo_mgr().CanUndo());
        s.se.get_undo_mgr().UndoLastStep();
        CHECK(s.Effects("Grid") == 2);
        CHECK(s.se.GetElement("Spare") == nullptr);
        CHECK_FALSE(s.se.get_undo_mgr().CanUndo());
    }
}

TEST_CASE("Save merge: a model they renamed keeps its row and effects in the open sequence", "[seq][showsavemerge][apply]") {
    // Runs under the mutation guard: the row must follow the rename through
    // commands, not by renaming the element directly.
    SeqShow s;
    const std::string mine = s.Rgb();
    const std::string theirs = WithGridRenamed(mine, "Grid Renamed");
    Plan plan = WithUse(mine, mine, theirs, ShowMergeApply::SequenceUseOf(s.ctx));
    REQUIRE(plan.AllResolved());
    CHECK(SequenceConflicts(plan).empty());
    pugi::xml_document mineDoc = Parse(mine);
    pugi::xml_document resultDoc = Parse(plan.Result(0));
    REQUIRE(s.ctx.AbortRender());
    CHECK(ShowMergeApply::ApplySequenceChanges(s.ctx, plan.SequenceChanges()) == 0);
    auto changes = ShowMergeApply::ApplyLayout(s.ctx, mineDoc, resultDoc);
    CHECK(changes.removedWithEffects.empty());
    CHECK(s.M("Grid Renamed") != nullptr);
    CHECK(s.se.GetElement("Grid") == nullptr);
    CHECK(s.Effects("Grid Renamed") == 2);
}
