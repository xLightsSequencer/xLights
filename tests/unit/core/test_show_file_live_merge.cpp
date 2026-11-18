#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "effects/EffectPresetManager.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/ViewObjectManager.h"
#include "outputs/Controller.h"
#include "outputs/OutputManager.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceMutationGuard.h"
#include "render/ShowFileLiveMerge.h"
#include "render/ShowMergeApply.h"
#include "render/UndoManager.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/ShowFileGuard.h"
#include "utils/ShowFileWatcher.h"
#include "utils/ShowPresence.h"
#include "utils/ShowSaveMerge.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializingVisitor.h"

// Watching the shared show files while a show is open (utils/ShowFileWatcher.h)
// and bringing someone else's changes into memory right away
// (render/ShowFileLiveMerge.h).
using ShowFileMerge::FileKind;
using ShowFileMerge::Resolution;
using ShowFileLiveMerge::Incoming;
using ShowFileLiveMerge::LocalFile;

namespace {

std::string Read(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// A watcher over one folder whose files this process "loaded" into `guard`.
struct Watched {
    TempDir dir;
    ShowFileGuard guard;
    ShowFileWatcher watcher{ guard };
    std::string rgb;
    std::string nets;
    Watched() {
        rgb = dir.Write("xlights_rgbeffects.xml", "<xrgb><models/></xrgb>");
        nets = dir.Write("xlights_networks.xml", "<Networks/>");
        guard.RecordFromDisk(rgb);
        guard.RecordFromDisk(nets);
        watcher.SetPlatformNotifications(false);
        watcher.SetDebounce(2000);
        watcher.Watch({ rgb, nets });
    }
};

} // namespace

TEST_CASE("Show file watch: nothing is reported while the files are as loaded", "[showfilewatch]") {
    Watched w;
    CHECK(w.watcher.Poll(0).empty());
    CHECK(w.watcher.Poll(5000).empty());
    CHECK(w.watcher.Poll(60000).empty());
}

TEST_CASE("Show file watch: a change is reported once it has settled, once", "[showfilewatch]") {
    Watched w;
    CHECK(w.watcher.Poll(0).empty());
    w.dir.Write("xlights_rgbeffects.xml", "<xrgb><models><model name=\"A\"/></models></xrgb>");
    // seen, but a sync client may still be writing
    CHECK(w.watcher.Poll(1000).empty());
    CHECK(w.watcher.Poll(2500).empty());
    CHECK(w.watcher.Poll(3000) == std::vector<std::string>{ w.rgb });
    // the same contents are not reported again
    CHECK(w.watcher.Poll(4000).empty());
    CHECK(w.watcher.Poll(40000).empty());
    // until the host asks for it again
    w.watcher.Rearm(w.rgb);
    CHECK(w.watcher.Poll(41000).empty());
    CHECK(w.watcher.Poll(43000) == std::vector<std::string>{ w.rgb });
}

TEST_CASE("Show file watch: a burst of writes restarts the wait and is reported together", "[showfilewatch]") {
    Watched w;
    CHECK(w.watcher.Poll(0).empty());
    w.dir.Write("xlights_rgbeffects.xml", "<xrgb><models><model name=\"A\"/></models></xrgb>");
    CHECK(w.watcher.Poll(1000).empty());
    w.dir.Write("xlights_rgbeffects.xml", "<xrgb><models><model name=\"AB\"/></models></xrgb>");
    CHECK(w.watcher.Poll(2500).empty());
    w.dir.Write("xlights_networks.xml", "<Networks><Controller Name=\"C\"/></Networks>");
    CHECK(w.watcher.Poll(4000).empty());
    auto both = w.watcher.Poll(6000);
    CHECK(both == std::vector<std::string>{ w.rgb, w.nets });
}

TEST_CASE("Show file watch: this process's own saves are not reported", "[showfilewatch]") {
    Watched w;
    CHECK(w.watcher.Poll(0).empty());
    SECTION("saved through the guard") {
        REQUIRE(w.guard.SaveFile(w.rgb, std::string("<xrgb><models><model name=\"Mine\"/></models></xrgb>")));
        CHECK(w.watcher.Poll(1000).empty());
        CHECK(w.watcher.Poll(5000).empty());
        CHECK(w.watcher.Poll(60000).empty());
    }
    SECTION("control: the same bytes written by someone else are reported") {
        w.dir.Write("xlights_rgbeffects.xml", "<xrgb><models><model name=\"Mine\"/></models></xrgb>");
        CHECK(w.watcher.Poll(1000).empty());
        CHECK(w.watcher.Poll(5000) == std::vector<std::string>{ w.rgb });
    }
    SECTION("a save that lands while a change is settling cancels it") {
        w.dir.Write("xlights_rgbeffects.xml", "<xrgb><models><model name=\"Theirs\"/></models></xrgb>");
        CHECK(w.watcher.Poll(1000).empty());
        // the host merged and saved: the guard now holds what is on disk
        REQUIRE(w.guard.SaveFile(w.rgb, std::string("<xrgb><models><model name=\"Both\"/></models></xrgb>")));
        CHECK(w.watcher.Poll(5000).empty());
    }
}

TEST_CASE("Show file watch: a change is confirmed by content, not by time stamp", "[showfilewatch]") {
    Watched w;
    int64_t mtime = 1;
    w.watcher.SetStampFunction([&](const std::string& p) {
        auto s = ShowFileWatcher::StampOf(p);
        s.mtime = mtime;
        return s;
    });
    CHECK(w.watcher.Poll(0).empty());
    // rewritten with the same bytes (a sync client touching it)
    mtime = 2;
    w.dir.Write("xlights_rgbeffects.xml", Read(w.rgb));
    CHECK(w.watcher.Poll(1000).empty());
    CHECK(w.watcher.Poll(5000).empty());
    // a change that keeps size and time stamp is found by the periodic read
    const std::string same = "<xrgb><models/></xrgy>";
    REQUIRE(same.size() == Read(w.rgb).size());
    w.dir.Write("xlights_rgbeffects.xml", same);
    CHECK(w.watcher.Poll(6000).empty());
    CHECK(w.watcher.Poll(6000 + ShowFileWatcher::kFullCheckMs).empty());
    CHECK(w.watcher.Poll(9000 + ShowFileWatcher::kFullCheckMs) == std::vector<std::string>{ w.rgb });
}

TEST_CASE("Show file watch: missing, unreadable and untracked files are not reported", "[showfilewatch]") {
    Watched w;
    bool readable = true;
    w.watcher.SetFingerprintFunction([&](const std::string& p) -> std::optional<ShowFileFingerprint> {
        if (!readable) return std::nullopt;
        return ShowFileGuard::FingerprintFile(p);
    });
    CHECK(w.watcher.Poll(0).empty());
    // being replaced by a sync client
    std::filesystem::remove(w.rgb);
    CHECK(w.watcher.Poll(1000).empty());
    CHECK(w.watcher.Poll(10000).empty());
    // still downloading
    readable = false;
    w.dir.Write("xlights_rgbeffects.xml", "<xrgb><models><model name=\"A\"/></models></xrgb>");
    CHECK(w.watcher.Poll(11000).empty());
    CHECK(w.watcher.Poll(20000).empty());
    readable = true;
    CHECK(w.watcher.Poll(21000).empty());
    CHECK(w.watcher.Poll(24000) == std::vector<std::string>{ w.rgb });
    // a file the guard never recorded (not loaded) is not this process's business
    w.guard.Forget(w.nets);
    w.dir.Write("xlights_networks.xml", "<Networks><Controller Name=\"X\"/></Networks>");
    CHECK(w.watcher.Poll(25000).empty());
    CHECK(w.watcher.Poll(30000).empty());
}

TEST_CASE("Show file watch: a poke reads the contents without waiting for the time stamp", "[showfilewatch]") {
    Watched w;
    w.watcher.SetStampFunction([](const std::string&) { ShowFileWatcher::Stamp s; s.exists = true; return s; });
    CHECK(w.watcher.Poll(0).empty());
    w.dir.Write("xlights_rgbeffects.xml", "<xrgb><models><model name=\"A\"/></models></xrgb>");
    CHECK(w.watcher.Poll(1000).empty());
    CHECK(w.watcher.Poll(4000).empty());
    w.watcher.Poke();
    CHECK(w.watcher.Poll(5000).empty());
    CHECK(w.watcher.Poll(7000) == std::vector<std::string>{ w.rgb });
}

// --- the texts ------------------------------------------------------------------------------

TEST_CASE("Live merge: changed items, who and the banner", "[showlivemerge]") {
    const std::string a = "<xrgb><models><model name=\"A\" x=\"1\"/><model name=\"B\"/></models><settings><s value=\"1\"/></settings></xrgb>";
    const std::string b = "<xrgb><models><model name=\"A\" x=\"2\"/><model name=\"C\"/></models><settings><s value=\"2\"/></settings></xrgb>";
    // A changed, B removed, C added, the settings changed
    CHECK(ShowFileLiveMerge::CountChangedItems(FileKind::RgbEffects, a, b) == 4);
    CHECK(ShowFileLiveMerge::CountChangedItems(FileKind::RgbEffects, a, a) == 0);
    CHECK(ShowFileLiveMerge::CountChangedItems(FileKind::Networks, "<Networks><Controller Name=\"X\" IP=\"1\"/></Networks>",
                                               "<Networks><Controller Name=\"X\" IP=\"2\"/><Controller Name=\"Y\"/></Networks>") == 2);

    ShowPresenceEntry sam;
    sam.installId = "1";
    sam.userName = "Sam";
    sam.deviceName = "PC";
    ShowPresenceEntry ipad;
    ipad.installId = "2";
    ipad.deviceName = "iPad";
    CHECK(ShowFileLiveMerge::WhoChanged({ sam }, { FileKind::RgbEffects }) == "Sam on PC");
    CHECK(ShowFileLiveMerge::WhoChanged({ sam, ipad }, { FileKind::RgbEffects }).empty());
    ipad.editingLayout = true;
    CHECK(ShowFileLiveMerge::WhoChanged({ sam, ipad }, { FileKind::RgbEffects }) == "iPad");
    CHECK(ShowFileLiveMerge::WhoChanged({ sam, ipad }, { FileKind::Networks }).empty());
    CHECK(ShowFileLiveMerge::WhoChanged({}, { FileKind::RgbEffects }).empty());

    CHECK(ShowFileLiveMerge::BannerText("Sam on PC", { FileKind::RgbEffects }, 3, 0) == "Layout updated by Sam on PC — 3 items changed");
    CHECK(ShowFileLiveMerge::BannerText("", { FileKind::Networks, FileKind::RgbEffects }, 1, 2) ==
          "Controllers and layout updated by someone else — 1 item changed, 2 need your decision");
    CHECK(ShowFileLiveMerge::BannerText("", { FileKind::Presets }, 0, 1) == "Effect presets updated by someone else — 1 needs your decision");
}

// --- bringing the changes into memory --------------------------------------------------------

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

std::string NetworksWith(const std::string& more = "") {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks computer=\"one\">"
           "<Controller Type=\"Null\" Id=\"64001\" Name=\"Bulk\"><network NetworkType=\"NULL\" MaxChannels=\"500000\"/></Controller>" +
           more + "</Networks>";
}

std::string NullController(const std::string& name, int id) {
    return "<Controller Type=\"Null\" Id=\"" + std::to_string(id) + "\" Name=\"" + name + "\"><network NetworkType=\"NULL\" MaxChannels=\"512\"/></Controller>";
}

const char* kShow = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models>"
                    "<model name=\"Grid\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\">"
                    "<subModel name=\"Top\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\"/></model>"
                    "<model name=\"Line\" DisplayAs=\"Single Line\" StringType=\"RGB Nodes\" NumStrings=\"1\" NodesPerString=\"10\" StartChannel=\"100\"/>"
                    "<model name=\"Spare\" DisplayAs=\"Single Line\" StringType=\"RGB Nodes\" NumStrings=\"1\" NodesPerString=\"10\" StartChannel=\"200\"/>"
                    "</models><modelGroups><modelGroup name=\"G\" models=\"Line,Spare\" layout=\"minimalGrid\" GridSize=\"400\"/></modelGroups></xrgb>";

// A show this process has open, with a sequence: Grid has an effect on its
// own layer and one on its "Top" submodel row (two in all); Line has one. The guard holds
// the files as loaded.
struct OpenShow {
    CurlLogger curl;
    TempDir dir;
    FixFileReset reset;
    HeadlessRenderContext ctx;
    ShowFileGuard guard;
    std::string rgb;
    std::string nets;
    std::string presets;
    SequenceElements* se = nullptr;

    explicit OpenShow(const std::string& layout = kShow, bool sequence = true) {
        nets = dir.Write("xlights_networks.xml", NetworksWith());
        dir.Write("xlights_rgbeffects.xml", layout);
        REQUIRE(ctx.LoadShowFolder(dir.path.string()));
        // What the desktop's first save writes, so the file and memory agree.
        rgb = dir.Write("xlights_rgbeffects.xml", Rgb());
        presets = dir.Write("xlights_effectpresets.json", ctx._effectPresetManager.SaveToText());
        for (const auto& p : { rgb, nets, presets }) guard.RecordFromDisk(p);
        if (sequence) {
            se = &ctx.GetSequenceElements();
            se->SetViewsManager(&ctx._sequenceViewManager);
            SeqMutationGuard::Scope setup;
            auto add = [&](const std::string& name) {
                auto* m = dynamic_cast<ModelElement*>(se->AddElement(name, "model", true, false, false, false, false));
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
            add("Line")->GetEffectLayer(0)->AddEffect(0, "Bars", "", "", 0, 500, 0, false);
            se->PopulateRowInformation();
        }
    }
    Model* M(const std::string& name) { return ctx.AllModels.GetModel(name); }
    int Effects(const std::string& name) {
        Element* e = se->GetElement(name);
        return e ? e->GetEffectCount() : -1;
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
    ShowFileMerge::Options Options() {
        ShowFileMerge::Options o;
        o.modelRenames = &ctx.AllModels.GetRenameLog();
        o.viewObjectRenames = &ctx.AllObjects.GetRenameLog();
        o.controllerRenames = &ctx._outputManager.GetRenameLog();
        return o;
    }
    // What the host passes: every shared file, `edited` with its serialization.
    std::vector<LocalFile> Local(bool layoutEdited, bool controllersEdited = false, bool presetsEdited = false) {
        return { { FileKind::RgbEffects, rgb, layoutEdited, layoutEdited ? Rgb() : std::string() },
                 { FileKind::Networks, nets, controllersEdited, controllersEdited ? ctx._outputManager.SaveToString() : std::string() },
                 { FileKind::Presets, presets, presetsEdited, presetsEdited ? ctx._effectPresetManager.SaveToText() : std::string() } };
    }
    Incoming Prepare(bool layoutEdited, bool controllersEdited = false, bool presetsEdited = false) {
        return Incoming::Prepare(Local(layoutEdited, controllersEdited, presetsEdited), Options(), guard,
                                 ShowMergeApply::SequenceUseOf(ctx));
    }
    ShowFileLiveMerge::Applied Apply(const Incoming& in) {
        REQUIRE(ctx.AbortRender());
        return ShowFileLiveMerge::Apply(ctx, in, guard);
    }
    // The save the host would run next.
    ShowSaveMerge::Plan SavePlan() {
        return ShowSaveMerge::Plan::Prepare({ { FileKind::RgbEffects, rgb, Rgb(), true } }, Options(), guard, ShowMergeApply::SequenceUseOf(ctx));
    }
};

// Someone else's save of a show: load it, change it, serialize it.
template <typename F>
std::string TheirEdit(const std::string& loaded, F&& edit) {
    CurlLogger curl;
    TempDir dir;
    FixFileReset reset;
    HeadlessRenderContext ctx;
    dir.Write("xlights_networks.xml", NetworksWith());
    dir.Write("xlights_rgbeffects.xml", loaded);
    REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    edit(ctx);
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

// What memory would be after a fresh load of `layout`.
std::string FreshLoad(const std::string& layout) {
    OpenShow fresh(layout, false);
    return fresh.Rgb();
}

} // namespace

TEST_CASE("Live merge: with no edits here, their layout is reloaded in place", "[seq][showlivemerge]") {
    OpenShow s;
    const std::string theirs = TheirEdit(Read(s.rgb), [](HeadlessRenderContext& c) {
        c.AllModels.GetModel("Line")->SetDescription("theirs");
        REQUIRE(c.AllModels.Delete("Spare"));
    });
    s.dir.Write("xlights_rgbeffects.xml", theirs);
    const size_t undoBefore = s.se->get_undo_mgr().CanUndo() ? 1 : 0;

    Incoming in = s.Prepare(false);
    REQUIRE(in.HasChanges());
    CHECK(in.ChangedPaths() == std::vector<std::string>{ s.rgb });
    REQUIRE(in.CanApply());
    CHECK(in.Decisions() == 0);
    CHECK(in.ChangedItems() == 3); // Line changed, Spare removed, G lost a member
    // control: memory is not their layout yet
    CHECK(s.Rgb() != FreshLoad(theirs));

    auto applied = s.Apply(in);
    CHECK(applied.layoutApplied);
    CHECK(s.Rgb() == FreshLoad(theirs));
    CHECK(s.M("Line")->GetDescription() == "theirs");
    CHECK(s.M("Spare") == nullptr);
    // the open sequence was not touched
    CHECK(s.Effects("Grid") == 2);
    CHECK(dynamic_cast<ModelElement*>(s.se->GetElement("Grid"))->GetSubModel("Top", false)->GetEffectCount() == 1);
    CHECK(s.Effects("Line") == 1);
    CHECK((s.se->get_undo_mgr().CanUndo() ? 1 : 0) == undoBefore);
    // their file is the new base: nothing to merge at the next save
    CHECK_FALSE(s.guard.HasChangedOnDisk(s.rgb));
    CHECK(s.guard.RecordedText(s.rgb) == theirs);
}

TEST_CASE("Live merge: with edits here, both changes are in memory and the next save writes them without asking", "[seq][showlivemerge]") {
    OpenShow s;
    const std::string theirs = TheirEdit(Read(s.rgb), [](HeadlessRenderContext& c) { c.AllModels.GetModel("Line")->SetDescription("theirs"); });
    s.M("Grid")->SetDescription("mine");
    s.dir.Write("xlights_rgbeffects.xml", theirs);

    SECTION("before it is applied, a save still has to merge") {
        auto plan = s.SavePlan();
        CHECK(plan.ExternalChanges());
    }
    SECTION("applied") {
        Incoming in = s.Prepare(true);
        REQUIRE(in.CanApply());
        s.Apply(in);
        CHECK(s.M("Grid")->GetDescription() == "mine");
        CHECK(s.M("Line")->GetDescription() == "theirs");
        CHECK(s.Effects("Grid") == 2);
        // base bookkeeping: their file is the base, my edit sits on top of it
        CHECK(s.guard.RecordedText(s.rgb) == theirs);
        auto plan = s.SavePlan();
        CHECK_FALSE(plan.ExternalChanges());
        CHECK_FALSE(plan.HasConflicts());
        REQUIRE(plan.Write(s.guard) == ShowSaveMerge::WriteResult::Written);
        const std::string saved = Read(s.rgb);
        CHECK(saved.find("mine") != std::string::npos);
        CHECK(saved.find("theirs") != std::string::npos);
        // and the save is not mistaken for someone else's
        CHECK_FALSE(s.guard.HasChangedOnDisk(s.rgb));
    }
}

TEST_CASE("Live merge: a conflict changes nothing until it is decided", "[seq][showlivemerge]") {
    OpenShow s;
    const std::string loaded = Read(s.rgb);
    const std::string theirs = TheirEdit(loaded, [](HeadlessRenderContext& c) {
        c.AllModels.GetModel("Grid")->SetDescription("theirs");
        c.AllModels.GetModel("Line")->SetDescription("theirs too");
    });
    s.M("Grid")->SetDescription("mine");
    s.dir.Write("xlights_rgbeffects.xml", theirs);

    Incoming in = s.Prepare(true);
    REQUIRE(in.CanMerge());
    CHECK(in.NeedsDecisions());
    CHECK_FALSE(in.CanApply());
    CHECK(in.Decisions() == 1);
    // refused while undecided: memory and the base stay as they were
    auto none = ShowFileLiveMerge::Apply(s.ctx, in, s.guard);
    CHECK_FALSE(none.layoutApplied);
    CHECK(s.M("Line")->GetDescription().empty());
    CHECK(s.guard.RecordedText(s.rgb) == loaded);
    CHECK(s.SavePlan().ExternalChanges());

    SECTION("decided mine") {
        REQUIRE(in.Plan().Resolve(in.Plan().Conflicts()[0], Resolution::Mine));
        REQUIRE(in.StillOnDisk());
        s.Apply(in);
        CHECK(s.M("Grid")->GetDescription() == "mine");
        CHECK(s.M("Line")->GetDescription() == "theirs too");
        auto plan = s.SavePlan();
        CHECK_FALSE(plan.ExternalChanges());
        REQUIRE(plan.Write(s.guard) == ShowSaveMerge::WriteResult::Written);
        CHECK(Read(s.rgb).find("mine") != std::string::npos);
    }
    SECTION("decided theirs") {
        REQUIRE(in.Plan().Resolve(in.Plan().Conflicts()[0], Resolution::Theirs));
        s.Apply(in);
        CHECK(s.M("Grid")->GetDescription() == "theirs");
        CHECK_FALSE(s.SavePlan().ExternalChanges());
    }
    SECTION("their file moved again while deciding: prepared again, the decision carries over") {
        REQUIRE(in.Plan().Resolve(in.Plan().Conflicts()[0], Resolution::Mine));
        const std::string again = TheirEdit(theirs, [](HeadlessRenderContext& c) { c.AllModels.GetModel("Spare")->SetDescription("later"); });
        s.dir.Write("xlights_rgbeffects.xml", again);
        CHECK_FALSE(in.StillOnDisk());
        Incoming next = s.Prepare(true);
        next.AdoptResolutions(in);
        REQUIRE(next.CanApply());
        s.Apply(next);
        CHECK(s.M("Grid")->GetDescription() == "mine");
        CHECK(s.M("Spare")->GetDescription() == "later");
        CHECK(s.guard.RecordedText(s.rgb) == again);
    }
}

TEST_CASE("Live merge: deleting a model the open sequence has effects on waits for a decision", "[seq][showlivemerge]") {
    OpenShow s;
    const std::string theirs = TheirEdit(Read(s.rgb), [](HeadlessRenderContext& c) { REQUIRE(c.AllModels.Delete("Grid")); });
    s.dir.Write("xlights_rgbeffects.xml", theirs);

    // even with no edits here: the effects are never dropped silently
    Incoming in = s.Prepare(false);
    REQUIRE(in.NeedsDecisions());
    auto refs = in.Plan().Conflicts();
    REQUIRE(refs.size() == 1);
    CHECK(in.Plan().GetConflict(refs[0]).kind == ShowFileMerge::ConflictKind::DeletedWithEffects);
    CHECK(s.M("Grid") != nullptr);

    SECTION("keep") {
        REQUIRE(in.Plan().Resolve(refs[0], Resolution::Mine));
        s.Apply(in);
        REQUIRE(s.M("Grid") != nullptr);
        CHECK(s.Effects("Grid") == 2);
        // kept here: the next save puts it back in their file, without asking
        auto plan = s.SavePlan();
        CHECK_FALSE(plan.ExternalChanges());
        REQUIRE(plan.Write(s.guard) == ShowSaveMerge::WriteResult::Written);
        CHECK(Read(s.rgb).find("\"Grid\"") != std::string::npos);
    }
    SECTION("delete, as one undo step") {
        REQUIRE(in.Plan().Resolve(refs[0], Resolution::Theirs));
        s.Apply(in);
        CHECK(s.M("Grid") == nullptr);
        CHECK(s.se->GetElement("Grid") == nullptr);
        REQUIRE(s.se->get_undo_mgr().CanUndo());
        s.se->get_undo_mgr().UndoLastStep();
        CHECK(s.Effects("Grid") == 2);
    }
    SECTION("map") {
        REQUIRE(in.Plan().Resolve(refs[0], Resolution::Map, "Spare"));
        auto applied = s.Apply(in);
        CHECK(s.M("Grid") == nullptr);
        CHECK(s.Effects("Spare") == 1);
        CHECK(applied.droppedEffects == 1);
    }
}

TEST_CASE("Live merge: their controllers are reloaded, keeping controller edits made here", "[showlivemerge]") {
    OpenShow s("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models/></xrgb>", false);
    s.dir.Write("xlights_networks.xml", NetworksWith(NullController("Theirs", 64002)));

    SECTION("no edits here") {
        Incoming in = s.Prepare(false);
        CHECK(in.ChangedPaths() == std::vector<std::string>{ s.nets });
        CHECK(in.ChangedItems() == 1);
        auto applied = s.Apply(in);
        CHECK(applied.controllersReloaded);
        CHECK(s.ctx._outputManager.GetController("Theirs") != nullptr);
        CHECK(s.ctx._outputManager.GetController("Bulk") != nullptr);
        CHECK_FALSE(s.guard.HasChangedOnDisk(s.nets));
    }
    SECTION("a controller added here") {
        const std::string mine = NetworksWith(NullController("Mine", 64003));
        std::vector<LocalFile> local = { { FileKind::Networks, s.nets, true, mine } };
        Incoming in = Incoming::Prepare(local, s.Options(), s.guard);
        REQUIRE(in.CanApply());
        REQUIRE(s.ctx.AbortRender());
        ShowFileLiveMerge::Apply(s.ctx, in, s.guard);
        CHECK(s.ctx._outputManager.GetController("Theirs") != nullptr);
        CHECK(s.ctx._outputManager.GetController("Mine") != nullptr);
        // the file still holds only theirs: the edit is still unsaved here
        CHECK(Read(s.nets).find("Mine") == std::string::npos);
        CHECK(s.guard.RecordedText(s.nets) == Read(s.nets));
    }
}

TEST_CASE("Live merge: their presets are reloaded", "[showlivemerge]") {
    OpenShow s("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models/></xrgb>", false);
    EffectPresetManager theirs;
    theirs.LoadFromJson(nlohmann::json::parse(Read(s.presets)));
    theirs.AddPreset(&theirs.GetRoot(), "Theirs", "E_X=1\tE_Y=2", "1", "x");
    s.dir.Write("xlights_effectpresets.json", theirs.SaveToText());
    CHECK(s.ctx._effectPresetManager.FindPresetByPath("Theirs") == nullptr);
    Incoming in = s.Prepare(false);
    REQUIRE(in.CanApply());
    auto applied = s.Apply(in);
    CHECK(applied.presetsReloaded);
    CHECK(s.ctx._effectPresetManager.FindPresetByPath("Theirs") != nullptr);
    CHECK_FALSE(s.guard.HasChangedOnDisk(s.presets));
}

TEST_CASE("Live merge: an unknown base is not merged", "[showlivemerge]") {
    OpenShow s("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models/></xrgb>", false);
    s.guard.Forget(s.rgb);
    s.dir.Write("xlights_rgbeffects.xml", "<?xml version=\"1.0\"?>\n<xrgb><models/><settings/></xrgb>");
    Incoming in = s.Prepare(false);
    CHECK_FALSE(in.CanMerge());
    CHECK_FALSE(in.CanApply());
}
