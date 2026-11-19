#include <catch2/catch_test_macros.hpp>

#include "support/TempDir.h"
#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceFaces.h"
#include "render/SequenceIds.h"
#include "render/SequenceMutationGuard.h"
#include "render/SequenceViewManager.h"
#include "render/SongStructureManager.h"
#include "render/UndoManager.h"
#include "utils/FileUtils.h"

#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <chrono>
#include <thread>
#include <future>
#include <string>
#include <vector>

// Commands for elements (models and timing tracks in a sequence), timing
// track properties, tags, sequence faces and song structure
// (SequenceElementCommands.h), and the edit-site helpers that use them.

using nlohmann::json;

namespace {

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

// A layout with a 3-strand x 4-node matrix that has one submodel, so model
// elements get submodel, strand and node rows.
struct Show {
    TempDir dir;
    FixFileReset reset;
    Show() {
        dir.Write("xlights_networks.xml", "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb version=\"2\"/>\n");
        dir.Write("xlights_rgbeffects.xml",
                  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n"
                  "<model name=\"Grid\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\">"
                  "<subModel name=\"Top\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\"/></model>\n"
                  "<model name=\"Line\" DisplayAs=\"Single Line\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"4\"/>\n"
                  "</models>\n<modelGroups/>\n</xrgb>\n");
    }
};

struct SeqFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    explicit SeqFixture(Show* show = nullptr) : se(ctx.GetSequenceElements()) {
        if (show != nullptr) {
            REQUIRE(ctx.LoadShowFolder(show->dir.path.string()));
        }
        se.SetViewsManager(&ctx._sequenceViewManager);
    }
    ModelElement* Model(const std::string& name = "Tree") {
        SeqMutationGuard::Scope setup;
        auto* m = dynamic_cast<ModelElement*>(se.AddElement(name, "model", true, false, false, false, false));
        m->AddEffectLayer();
        return m;
    }
    TimingElement* Timing(const std::string& name = "Beats") {
        SeqMutationGuard::Scope setup;
        auto* t = dynamic_cast<TimingElement*>(se.AddElement(se.GetNumberOfTimingElements(), name, "timing", true, false, false, false, false));
        t->AddEffectLayer();
        return t;
    }
    Effect* Add(EffectLayer* l, const std::string& name, int start, int end, const std::string& settings = "") {
        SeqMutationGuard::Scope setup;
        return l->AddEffect(0, name, settings, "", start, end, 0, false);
    }
    UndoManager& Undo() { return se.get_undo_mgr(); }
    CommandExecutor& Exec() { return se.GetCommandExecutor(); }
    std::string Describe(bool ids = true) const { return DescribeSequence(se, ids); }
    std::vector<std::string> Order() const {
        std::vector<std::string> out;
        for (size_t i = 0; i < se.GetElementCount(); ++i) {
            out.push_back(se.GetElement(i)->GetName());
        }
        return out;
    }
    void RequireValid() {
        std::string problem;
        bool ok = se.ValidateUids(&problem);
        INFO(problem);
        REQUIRE(ok);
    }
};

json Ser(const SequenceCommand& c) {
    json j;
    c.Serialize(j);
    return j;
}

void CheckJsonRoundTrip(const SequenceCommand& c) {
    json j = Ser(c);
    INFO(j.dump());
    auto d = DeserializeCommand(j);
    REQUIRE(d != nullptr);
    CHECK(d->Type() == c.Type());
    CHECK(Ser(*d) == j);
    auto t = CommandFromJson(c.ToJson());
    REQUIRE(t != nullptr);
    CHECK(Ser(*t) == j);
    auto cl = c.Clone();
    CHECK(Ser(*cl) == j);
}

struct Applied {
    std::string before;
    std::string after;
    std::unique_ptr<SequenceCommand> inverse;
};

Applied ApplyOk(SeqFixture& f, SequenceCommand& cmd, ApplyMode mode = ApplyMode::Strict) {
    Applied a;
    a.before = f.Describe();
    CommandResult r;
    a.inverse = cmd.Apply(f.se, mode, &r);
    INFO("apply: " << r.message);
    REQUIRE(r.Ok());
    REQUIRE(a.inverse != nullptr);
    a.after = f.Describe();
    return a;
}

// inverse -> state before (ids included); inverse of the inverse -> after.
void CheckInverses(SeqFixture& f, Applied& a, ApplyMode mode = ApplyMode::Strict) {
    CommandResult r;
    auto inv2 = a.inverse->Apply(f.se, mode, &r);
    {
        INFO("inverse: " << r.message);
        REQUIRE(inv2 != nullptr);
    }
    CHECK(f.Describe() == a.before);
    f.RequireValid();
    auto inv3 = inv2->Apply(f.se, mode, &r);
    {
        INFO("inverse of inverse: " << r.message);
        REQUIRE(inv3 != nullptr);
    }
    CHECK(f.Describe() == a.after);
    f.RequireValid();
}

void ExpectRejected(SeqFixture& f, SequenceCommand& cmd, ApplyMode mode, CommandStatus status) {
    std::string before = f.Describe();
    CHECK(cmd.Validate(f.se, mode).status == status);
    CommandResult r;
    CHECK(cmd.Apply(f.se, mode, &r) == nullptr);
    CHECK(r.status == status);
    CHECK(f.Describe() == before);
}

struct Recorder : ICommandObserver {
    std::vector<json> applied;
    void OnCommandApplied(const SequenceCommand& a, const SequenceCommand&) override { applied.push_back(Ser(a)); }
};

struct GuardMode {
    SeqMutationGuard::Mode prev;
    explicit GuardMode(SeqMutationGuard::Mode m) : prev(SeqMutationGuard::GetMode()) {
        SeqMutationGuard::SetMode(m);
        SeqMutationGuard::ResetViolationCount();
    }
    ~GuardMode() { SeqMutationGuard::SetMode(prev); }
};

// Puts effects on every kind of row of the Grid model element.
void Populate(SeqFixture& f, ModelElement* grid) {
    REQUIRE(grid->GetSubModelCount() == 1);
    REQUIRE(grid->GetStrandCount() == 3);
    f.Add(grid->GetEffectLayer(0), "On", 0, 100);
    {
        SeqMutationGuard::Scope setup;
        grid->AddEffectLayer()->SetLayerName("second");
    }
    f.Add(grid->GetEffectLayer(1), "Bars", 50, 150, "E_SLIDER_Bars_BarCount=3");
    f.Add(grid->GetSubModel(0)->GetEffectLayer(0), "On", 0, 50);
    StrandElement* st = grid->GetStrand(1);
    REQUIRE(st->GetNodeLayerCount() == 4);
    f.Add(st->GetEffectLayer(0), "On", 100, 200);
    f.Add(st->GetNodeLayer(2), "On", 300, 400);
}

} // namespace

// ---------------------------------------------------------------------------
// AddElement / DeleteElement
// ---------------------------------------------------------------------------

TEST_CASE("AddElement adds a timing track; its inverse removes it", "[seq][elements]") {
    SeqFixture f;
    TimingElement* beats = f.Timing("Beats");
    f.Model("Tree");

    ElementSnapshot s;
    s.timing = true;
    s.name = "Lyrics";
    s.subType = "Lyrics";
    s.fixedTimingMS = 0;
    LayerSnapshot l;
    l.name = "Phrases";
    EffectState mark;
    mark.name = "hello";
    mark.startTimeMS = 0;
    mark.endTimeMS = 500;
    l.effects.push_back(mark);
    s.layers.push_back(l);
    AddElementCommand add(s, beats->GetUid());
    CHECK(add.GetUid() != kNoSeqObjectId);
    CHECK(add.GetState().layers[0].uid != kNoSeqObjectId);
    CHECK(add.GetState().layers[0].effects[0].uid != kNoSeqObjectId);
    CheckJsonRoundTrip(add);

    Applied a = ApplyOk(f, add);
    CHECK(f.Order() == std::vector<std::string>{ "Beats", "Lyrics", "Tree" });
    auto* te = dynamic_cast<TimingElement*>(f.se.GetElementFromUid(add.GetUid()));
    REQUIRE(te != nullptr);
    CHECK(te->GetSubType() == "Lyrics");
    CHECK(te->GetEffectLayer(0)->GetLayerName() == "Phrases");
    CHECK(te->GetEffectLayer(0)->GetUid() == add.GetState().layers[0].uid);
    REQUIRE(te->GetEffectLayer(0)->GetEffectCount() == 1);
    CHECK(te->GetEffectLayer(0)->GetEffect(0)->GetUid() == add.GetState().layers[0].effects[0].uid);
    f.RequireValid();
    CHECK(a.inverse->Type() == CommandType::DeleteElement);
    CheckInverses(f, a);
}

TEST_CASE("AddElement with no layers gets one", "[seq][elements]") {
    SeqFixture f;
    ElementSnapshot s;
    s.name = "Tree";
    AddElementCommand add(s, kNoSeqObjectId);
    REQUIRE(add.GetState().layers.size() == 1);
    ApplyOk(f, add);
    Element* e = f.se.GetElementFromUid(add.GetUid());
    REQUIRE(e != nullptr);
    REQUIRE(e->GetEffectLayerCount() == 1);
    CHECK(e->GetEffectLayer(0)->GetUid() == add.GetState().layers[0].uid);
}

TEST_CASE("AddElement validation: strict and lenient", "[seq][elements]") {
    SeqFixture f;
    f.Timing("Beats");
    f.Model("Tree");

    ElementSnapshot dup;
    dup.name = "Tree";
    AddElementCommand sameName(dup, kNoSeqObjectId);
    ExpectRejected(f, sameName, ApplyMode::Strict, CommandStatus::Invalid);
    ExpectRejected(f, sameName, ApplyMode::Lenient, CommandStatus::Invalid);

    ElementSnapshot noName;
    AddElementCommand empty(noName, kNoSeqObjectId);
    ExpectRejected(f, empty, ApplyMode::Lenient, CommandStatus::Invalid);

    ElementSnapshot clash;
    clash.name = "Other";
    clash.uid = f.se.GetElement(1)->GetUid();
    AddElementCommand idInUse(clash, kNoSeqObjectId);
    ExpectRejected(f, idInUse, ApplyMode::Lenient, CommandStatus::Invalid);

    // Missing sibling: strict refuses, lenient falls back to the index hint.
    ElementSnapshot s;
    s.name = "House";
    AddElementCommand orphan(s, SeqIds::Next(), 1);
    ExpectRejected(f, orphan, ApplyMode::Strict, CommandStatus::TargetMissing);
    Applied a = ApplyOk(f, orphan, ApplyMode::Lenient);
    CHECK(f.Order() == std::vector<std::string>{ "Beats", "House", "Tree" });
    CheckInverses(f, a, ApplyMode::Lenient);
}

TEST_CASE("DeleteElement removes a model with every row; the inverse restores ids and contents", "[seq][elements]") {
    Show show;
    SeqFixture f(&show);
    f.Timing("Beats");
    ModelElement* grid = f.Model("Grid");
    f.Model("Line");
    Populate(f, grid);
    f.RequireValid();
    std::string before = f.Describe();
    SeqObjectId gridUid = grid->GetUid();
    SeqObjectId nodeLayerUid = grid->GetStrand(2)->GetNodeLayer(3)->GetUid();
    SeqObjectId subUid = grid->GetSubModel(0)->GetUid();

    DeleteElementCommand del(gridUid);
    CheckJsonRoundTrip(del);
    Applied a = ApplyOk(f, del);
    CHECK(f.Order() == std::vector<std::string>{ "Beats", "Line" });
    CHECK(f.se.GetElementFromUid(gridUid) == nullptr);
    REQUIRE(a.inverse->Type() == CommandType::AddElement);
    CheckJsonRoundTrip(*a.inverse);

    CheckInverses(f, a);
    CHECK(f.Describe() == a.after);
    // Back to the original, ids and all - including empty node layers,
    // which DescribeSequence lists with their ids.
    auto again = a.inverse->Apply(f.se, ApplyMode::Strict);
    REQUIRE(again != nullptr);
    CHECK(f.Describe() == before);
    auto* restored = dynamic_cast<ModelElement*>(f.se.GetElementFromUid(gridUid));
    REQUIRE(restored != nullptr);
    CHECK(restored->GetSubModel(0)->GetUid() == subUid);
    CHECK(restored->GetStrand(2)->GetNodeLayer(3)->GetUid() == nodeLayerUid);
    CHECK(restored->GetEffectLayer(1)->GetLayerName() == "second");
}

TEST_CASE("A deleted element's inverse, serialized, rebuilds it in another sequence", "[seq][elements]") {
    Show show;
    SeqFixture a(&show);
    SeqFixture b(&show);
    ModelElement* grid = a.Model("Grid");
    Populate(a, grid);
    std::string described = a.Describe();
    auto inverse = DeleteElementCommand(grid->GetUid()).Apply(a.se, ApplyMode::Strict);
    REQUIRE(inverse != nullptr);
    auto copy = CommandFromJson(inverse->ToJson());
    REQUIRE(copy != nullptr);
    REQUIRE(copy->Apply(b.se, ApplyMode::Strict) != nullptr);
    CHECK(b.Describe() == described);
    b.RequireValid();
}

TEST_CASE("Adding a model records the ids of the rows the layout gives it", "[seq][elements]") {
    // Rows the model in the layout adds (submodels, strands, nodes) must
    // come out with the same ids wherever the command is applied, or later
    // edits on those rows can't find them.
    Show show;
    SeqFixture a(&show);
    SeqFixture b(&show);
    ElementSnapshot s;
    s.name = "Grid";
    AddElementCommand add(s, kNoSeqObjectId);
    CHECK(add.GetState().subElements.empty());
    ApplyOk(a, add);
    CHECK(add.GetState().subElements.size() == 4); // 1 submodel + 3 strands
    auto copy = CommandFromJson(add.ToJson());
    REQUIRE(copy != nullptr);
    REQUIRE(copy->Apply(b.se, ApplyMode::Strict) != nullptr);
    CHECK(b.Describe() == a.Describe());

    // ... so an edit on a strand's node layer replays.
    auto* grid = dynamic_cast<ModelElement*>(a.se.GetElementFromUid(add.GetUid()));
    REQUIRE(grid != nullptr);
    EffectState on;
    on.name = "On";
    on.startTimeMS = 0;
    on.endTimeMS = 100;
    AddEffectCommand addEffect(LocatorOf(grid->GetStrand(0)->GetNodeLayer(1)), on);
    REQUIRE(addEffect.Apply(a.se, ApplyMode::Strict) != nullptr);
    auto remote = CommandFromJson(addEffect.ToJson());
    REQUIRE(remote->Apply(b.se, ApplyMode::Strict) != nullptr);
    CHECK(b.Describe() == a.Describe());
}

TEST_CASE("DeleteElement: locked effects, missing targets, submodel rows", "[seq][elements]") {
    Show show;
    SeqFixture f(&show);
    ModelElement* grid = f.Model("Grid");
    Populate(f, grid);
    Effect* nodeEffect = grid->GetStrand(1)->GetNodeLayer(2)->GetEffect(0);
    {
        SeqMutationGuard::Scope setup;
        nodeEffect->SetLocked(true);
    }
    SeqObjectId gridUid = grid->GetUid();
    DeleteElementCommand del(gridUid);
    ExpectRejected(f, del, ApplyMode::Strict, CommandStatus::Locked);
    Applied a = ApplyOk(f, del, ApplyMode::Lenient);
    CHECK(f.se.GetElementCount() == 0);
    REQUIRE(a.inverse->Apply(f.se, ApplyMode::Lenient) != nullptr);
    CHECK(f.Describe() == a.before);
    auto* restored = dynamic_cast<ModelElement*>(f.se.GetElementFromUid(gridUid));
    REQUIRE(restored != nullptr);
    CHECK(restored->GetStrand(1)->GetNodeLayer(2)->GetEffect(0)->IsLocked());

    DeleteElementCommand gone(SeqIds::Next());
    ExpectRejected(f, gone, ApplyMode::Lenient, CommandStatus::TargetMissing);
    // Only top-level elements can be deleted.
    DeleteElementCommand sub(restored->GetSubModel(0)->GetUid());
    ExpectRejected(f, sub, ApplyMode::Lenient, CommandStatus::TargetMissing);
}

TEST_CASE("Deleting a timing track restores its properties and, locally, its views", "[seq][elements]") {
    SeqFixture f;
    TimingElement* t = f.Timing("Lyrics");
    f.Model("Tree");
    {
        SeqMutationGuard::Scope setup;
        t->SetSubType("Lyrics");
        t->SetFixedTiming(0);
    }
    t->SetViews("Front,Back");
    f.Add(t->GetEffectLayer(0), "hello world", 0, 1000);
    SeqObjectId uid = t->GetUid();

    auto inverse = DeleteElementCommand(uid).Apply(f.se, ApplyMode::Strict);
    REQUIRE(inverse != nullptr);
    // Views are per-user: kept on the local inverse, never serialized.
    CHECK(Ser(*inverse).dump().find("Front") == std::string::npos);
    REQUIRE(inverse->Apply(f.se, ApplyMode::Strict) != nullptr);
    auto* back = dynamic_cast<TimingElement*>(f.se.GetElementFromUid(uid));
    REQUIRE(back != nullptr);
    CHECK(back->GetSubType() == "Lyrics");
    CHECK(back->GetViews() == "Front,Back");
    CHECK(back->GetEffectLayer(0)->GetEffect(0)->GetEffectName() == "hello world");
    CHECK(f.Order() == std::vector<std::string>{ "Lyrics", "Tree" });
}

TEST_CASE("DeleteElement drops the model from view definitions unless asked to keep it", "[seq][elements]") {
    SeqFixture f;
    f.Model("A");
    f.Model("B");
    f.ctx._sequenceViewManager.SetModelManager(&f.ctx.AllModels);
    SequenceView* view = f.ctx._sequenceViewManager.AddView("Mine");
    REQUIRE(view != nullptr);
    view->SetModels("A,B");

    DeleteElementCommand keep(f.se.GetElement(0)->GetUid(), true);
    CheckJsonRoundTrip(keep);
    auto inverse = keep.Apply(f.se, ApplyMode::Strict);
    REQUIRE(inverse != nullptr);
    CHECK(view->ContainsModel("A"));
    REQUIRE(inverse->Apply(f.se, ApplyMode::Strict) != nullptr);

    DeleteElementCommand drop(f.se.GetElement(0)->GetUid());
    REQUIRE(drop.Apply(f.se, ApplyMode::Strict) != nullptr);
    CHECK_FALSE(view->ContainsModel("A"));
    CHECK(view->ContainsModel("B"));
}

TEST_CASE("Replacing a timing track's layers as one recorded step undoes completely", "[seq][elements][undo]") {
    // The lyric import / breakdown pattern: add the new layer first (an
    // element's last layer can't go), remove the old ones, fill the new
    // one. Recording only the marks (the layer changes with
    // recordUndo=false, as those edit sites used to) left an empty layer
    // and lost the old ones on undo.
    for (bool recordLayers : { true, false }) {
        SeqFixture f;
        TimingElement* t = f.Timing("Lyrics");
        f.Add(t->GetEffectLayer(0), "old phrase", 0, 1000);
        {
            SeqMutationGuard::Scope setup;
            t->AddEffectLayer();
            t->SetFixedTiming(50);
        }
        f.Add(t->GetEffectLayer(1), "old", 0, 500);
        std::string before = f.Describe();

        f.Undo().CreateUndoStep();
        ElementEdits::SetTimingFixed(f.se, t, 0, recordLayers);
        CommandExecutor& exec = f.Exec();
        int n = (int)t->GetEffectLayerCount();
        EffectLayer* phrases = exec.InsertLayer(t, n, recordLayers);
        for (int k = n - 1; k >= 0; --k) {
            exec.RemoveLayer(t, k, recordLayers);
        }
        exec.AddEffect(phrases, "new phrase", "", "", 0, 2000, 0, false);
        REQUIRE(t->GetEffectLayerCount() == 1);

        f.Undo().UndoLastStep();
        INFO("layer changes recorded: " << recordLayers);
        if (recordLayers) {
            CHECK(f.Describe() == before);
            CHECK(t->GetFixedTiming() == 50);
        } else {
            CHECK(f.Describe() != before); // the control: what the old code did
        }
    }
}

// ---------------------------------------------------------------------------
// MoveElement
// ---------------------------------------------------------------------------

TEST_CASE("MoveElementCommand::FromIndices matches MoveSequenceElement for every index pair", "[seq][elements]") {
    const std::vector<std::string> names{ "A", "B", "C", "D", "E" };
    for (int index = 0; index < 5; ++index) {
        for (int dest = -1; dest <= 6; ++dest) {
            SeqFixture direct;
            SeqFixture viaCommand;
            for (const auto& n : names) {
                direct.Model(n);
                viaCommand.Model(n);
            }
            {
                SeqMutationGuard::Scope reference;
                direct.se.MoveSequenceElement(index, dest, MASTER_VIEW);
            }
            auto cmd = MoveElementCommand::FromIndices(viaCommand.se, index, dest);
            INFO("index " << index << " dest " << dest);
            if (direct.Order() == viaCommand.Order()) {
                CHECK(cmd == nullptr);
                continue;
            }
            REQUIRE(cmd != nullptr);
            CheckJsonRoundTrip(*cmd);
            Applied a = ApplyOk(viaCommand, *cmd);
            CHECK(viaCommand.Order() == direct.Order());
            CheckInverses(viaCommand, a);
        }
    }
}

TEST_CASE("MoveElement: missing sibling, missing element", "[seq][elements]") {
    SeqFixture f;
    f.Model("A");
    ModelElement* b = f.Model("B");
    f.Model("C");
    MoveElementCommand orphan(b->GetUid(), SeqIds::Next(), 2);
    ExpectRejected(f, orphan, ApplyMode::Strict, CommandStatus::TargetMissing);
    Applied a = ApplyOk(f, orphan, ApplyMode::Lenient);
    CHECK(f.Order() == std::vector<std::string>{ "A", "C", "B" });
    CheckInverses(f, a, ApplyMode::Lenient);

    MoveElementCommand gone(SeqIds::Next(), kNoSeqObjectId);
    ExpectRejected(f, gone, ApplyMode::Lenient, CommandStatus::TargetMissing);
    MoveElementCommand self(b->GetUid(), b->GetUid());
    ExpectRejected(f, self, ApplyMode::Lenient, CommandStatus::Invalid);

    MoveElementCommand first(b->GetUid(), kNoSeqObjectId);
    ApplyOk(f, first);
    CHECK(f.Order() == std::vector<std::string>{ "B", "A", "C" });
}

// ---------------------------------------------------------------------------
// Timing track rename / fixed
// ---------------------------------------------------------------------------

TEST_CASE("RenameTimingTrack follows references and its inverse restores exactly what it changed", "[seq][elements]") {
    SeqFixture f;
    TimingElement* t = f.Timing("Words");
    ModelElement* m = f.Model("Tree");
    Effect* follows = f.Add(m->GetEffectLayer(0), "Faces", 0, 100, "E_CHOICE_Faces_TimingTrack=Words");
    // Already names the new name (a track that no longer exists): a rename
    // must not "restore" it to the old name on undo.
    Effect* dangling = f.Add(m->GetEffectLayer(0), "Faces", 200, 300, "E_CHOICE_Faces_TimingTrack=Lyrics");
    REQUIRE(follows->GetEffectIndex() >= 0);

    RenameTimingTrackCommand rename(t->GetUid(), "Lyrics");
    CheckJsonRoundTrip(rename);
    Applied a = ApplyOk(f, rename);
    CHECK(t->GetName() == "Lyrics");
    CHECK(follows->GetSetting("E_CHOICE_Faces_TimingTrack") == "Lyrics");
    CHECK(dangling->GetSetting("E_CHOICE_Faces_TimingTrack") == "Lyrics");

    CheckJsonRoundTrip(*a.inverse);
    auto redo = a.inverse->Apply(f.se, ApplyMode::Strict);
    REQUIRE(redo != nullptr);
    CHECK(t->GetName() == "Words");
    CHECK(follows->GetSetting("E_CHOICE_Faces_TimingTrack") == "Words");
    CHECK(dangling->GetSetting("E_CHOICE_Faces_TimingTrack") == "Lyrics");
    REQUIRE(redo->Apply(f.se, ApplyMode::Strict) != nullptr);
    CHECK(follows->GetSetting("E_CHOICE_Faces_TimingTrack") == "Lyrics");

    RenameTimingTrackCommand noRefs(t->GetUid(), "Other", false);
    ApplyOk(f, noRefs);
    CHECK(follows->GetSetting("E_CHOICE_Faces_TimingTrack") == "Lyrics");
}

TEST_CASE("RenameTimingTrack validation", "[seq][elements]") {
    SeqFixture f;
    TimingElement* t = f.Timing("Words");
    ModelElement* m = f.Model("Tree");
    RenameTimingTrackCommand taken(t->GetUid(), "Tree");
    ExpectRejected(f, taken, ApplyMode::Strict, CommandStatus::Invalid);
    RenameTimingTrackCommand empty(t->GetUid(), "");
    ExpectRejected(f, empty, ApplyMode::Lenient, CommandStatus::Invalid);
    RenameTimingTrackCommand model(m->GetUid(), "X");
    ExpectRejected(f, model, ApplyMode::Lenient, CommandStatus::Invalid);
    RenameTimingTrackCommand gone(SeqIds::Next(), "X");
    ExpectRejected(f, gone, ApplyMode::Lenient, CommandStatus::TargetMissing);
}

TEST_CASE("SetTimingFixed sets and restores the interval", "[seq][elements]") {
    SeqFixture f;
    TimingElement* t = f.Timing("50ms");
    {
        SeqMutationGuard::Scope setup;
        t->SetFixedTiming(50);
    }
    SetTimingFixedCommand cmd(t->GetUid(), 0);
    CheckJsonRoundTrip(cmd);
    auto inv = cmd.Apply(f.se, ApplyMode::Strict);
    REQUIRE(inv != nullptr);
    CHECK(t->GetFixedTiming() == 0);
    REQUIRE(inv->Apply(f.se, ApplyMode::Strict) != nullptr);
    CHECK(t->GetFixedTiming() == 50);
    SetTimingFixedCommand bad(t->GetUid(), -5);
    ExpectRejected(f, bad, ApplyMode::Lenient, CommandStatus::Invalid);
    ModelElement* m = f.Model("Tree");
    SetTimingFixedCommand notTiming(m->GetUid(), 0);
    ExpectRejected(f, notTiming, ApplyMode::Lenient, CommandStatus::Invalid);
}

// ---------------------------------------------------------------------------
// Tags, faces, song structure
// ---------------------------------------------------------------------------

TEST_CASE("SetTag sets and restores a tag", "[seq][elements]") {
    SeqFixture f;
    SetTagCommand set(3, 1500);
    CheckJsonRoundTrip(set);
    auto inv = set.Apply(f.se, ApplyMode::Strict);
    REQUIRE(inv != nullptr);
    CHECK(f.se.GetTagPosition(3) == 1500);
    REQUIRE(inv->Apply(f.se, ApplyMode::Strict) != nullptr);
    CHECK(f.se.GetTagPosition(3) == -1);
    SetTagCommand tooHigh(SequenceElements::TagCount, 0);
    ExpectRejected(f, tooHigh, ApplyMode::Lenient, CommandStatus::Invalid);
    SetTagCommand negative(-1, 0);
    ExpectRejected(f, negative, ApplyMode::Lenient, CommandStatus::Invalid);
    SetTagCommand badPos(0, -7);
    ExpectRejected(f, badPos, ApplyMode::Lenient, CommandStatus::Invalid);
}

TEST_CASE("SetSequenceFace sets, replaces and removes a face", "[seq][elements]") {
    SeqFixture f;
    SetSequenceFaceCommand::FaceDefinition def{ { "Mouth-AI-EyesOpen", "a.png" } };
    SetSequenceFaceCommand set("Singer", def);
    CheckJsonRoundTrip(set);
    auto inv = set.Apply(f.se, ApplyMode::Strict);
    REQUIRE(inv != nullptr);
    REQUIRE(f.se.GetSequenceFaces().GetFace("Singer") != nullptr);
    CHECK(f.se.GetSequenceFaces().GetFace("Singer")->at("Mouth-AI-EyesOpen") == "a.png");
    CheckJsonRoundTrip(*inv);
    REQUIRE(inv->Apply(f.se, ApplyMode::Strict) != nullptr);
    CHECK(f.se.GetSequenceFaces().GetFace("Singer") == nullptr);

    SetSequenceFaceCommand removeMissing("Nobody", std::nullopt);
    CheckJsonRoundTrip(removeMissing);
    ExpectRejected(f, removeMissing, ApplyMode::Strict, CommandStatus::TargetMissing);
    CHECK(removeMissing.Apply(f.se, ApplyMode::Lenient) != nullptr);
    SetSequenceFaceCommand unnamed("", def);
    ExpectRejected(f, unnamed, ApplyMode::Lenient, CommandStatus::Invalid);
}

TEST_CASE("EditSequenceFaces records a rename as one undo step", "[seq][elements][undo]") {
    SeqFixture f;
    f.Exec().ExecuteInStep(std::make_unique<SetSequenceFaceCommand>("Singer", SetSequenceFaceCommand::FaceDefinition{ { "Mouth-AI-EyesOpen", "a.png" } }), false);
    Recorder rec;
    f.Exec().AddObserver(&rec);
    f.Undo().CreateUndoStep();
    CHECK(ElementEdits::EditSequenceFaces(f.se, [](SequenceFaces& faces) { faces.RenameFace("Singer", "Lead"); }));
    CHECK(f.se.GetSequenceFaces().GetFace("Lead") != nullptr);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.applied[0]["type"] == "Batch");
    CHECK(rec.applied[0]["commands"].size() == 2);
    CHECK_FALSE(ElementEdits::EditSequenceFaces(f.se, [](SequenceFaces&) {}));
    f.Undo().UndoLastStep();
    CHECK(f.se.GetSequenceFaces().GetFace("Lead") == nullptr);
    REQUIRE(f.se.GetSequenceFaces().GetFace("Singer") != nullptr);
    CHECK(f.se.GetSequenceFaces().GetFace("Singer")->at("Mouth-AI-EyesOpen") == "a.png");
    f.Exec().RemoveObserver(&rec);
}

TEST_CASE("EditSongStructure records the change; undo keeps the active view", "[seq][elements][undo]") {
    SeqFixture f;
    SongStructureManager& ssm = f.se.GetSongStructureManager();
    f.Undo().CreateUndoStep();
    CHECK(ElementEdits::EditSongStructure(f.se, [](SongStructureManager& m) {
        m.AddBoundary(1000, 4000);
        m.AddView("Alt");
    }));
    REQUIRE(ssm.GetViewCount() == 2);
    REQUIRE(ssm.GetRegionCount() == 2);
    CHECK(f.Undo().GetUndoString() == "Undo: Song Structure Changed");

    ssm.SetActiveView(1);
    f.Undo().CreateUndoStep();
    CHECK(ElementEdits::EditSongStructure(f.se, [](SongStructureManager& m) { m.RenameView(1, "Chorus"); }));
    CHECK(ssm.GetViewName(1) == "Chorus");
    f.Undo().UndoLastStep();
    CHECK(ssm.GetViewName(1) == "Alt");
    CHECK(ssm.GetActiveViewIndex() == 1);
    f.Undo().UndoLastStep();
    CHECK(ssm.GetViewCount() == 0);
    CHECK(ssm.GetActiveViewIndex() == -1);
    f.Undo().RedoLastStep();
    CHECK(ssm.GetViewCount() == 2);
    CHECK(ssm.GetActiveViewIndex() == 0);
    CHECK_FALSE(ElementEdits::EditSongStructure(f.se, [](SongStructureManager& m) { m.SetActiveView(1); }));

    SetSongStructureCommand cmd(ssm.GetViews());
    CheckJsonRoundTrip(cmd);
    std::vector<SongStructureView> bad(1);
    bad[0].regions.push_back(SongStructureRegion(1, 500, 100, "x", 0));
    SetSongStructureCommand backwards(bad);
    ExpectRejected(f, backwards, ApplyMode::Lenient, CommandStatus::Invalid);
}

// ---------------------------------------------------------------------------
// Deserialization
// ---------------------------------------------------------------------------

TEST_CASE("Element commands reject malformed JSON", "[seq][elements]") {
    for (const char* text : {
             R"({"type":"AddElement"})",
             R"({"type":"AddElement","element":{"uid":0,"timing":true,"name":"x","renderDisabled":false,"subType":"","fixed":0,"layers":[],"subElements":[]},"after":0,"indexHint":-1})",
             R"({"type":"AddElement","element":{"uid":5,"timing":"yes","name":"x","renderDisabled":false,"subType":"","fixed":0,"layers":[],"subElements":[]},"after":0,"indexHint":-1})",
             R"({"type":"AddElement","element":{"uid":5,"timing":false,"name":"x","renderDisabled":false,"subType":"","fixed":0,"layers":[],"subElements":[{"strand":true,"name":"","strandIndex":-1,"uid":6,"layers":[],"nodeLayers":[],"nodeEffects":[]}]},"after":0,"indexHint":-1})",
             R"({"type":"AddElement","element":{"uid":5,"timing":false,"name":"x","renderDisabled":false,"subType":"","fixed":0,"layers":[{"uid":7,"name":"","effects":[{"uid":0}]}],"subElements":[]},"after":0,"indexHint":-1})",
             R"({"type":"DeleteElement","element":"7"})",
             R"({"type":"MoveElement","element":7,"after":-1,"indexHint":0})",
             R"({"type":"RenameTimingTrack","element":7,"name":3,"updateReferences":true})",
             R"({"type":"SetTimingFixed","element":7})",
             R"({"type":"SetTag","tag":1.5,"position":0})",
             R"({"type":"SetSequenceFace","name":"x"})",
             R"({"type":"SetSequenceFace","name":"x","definition":{"k":1}})",
             R"({"type":"SetSongStructure","views":[{"name":"a","nextId":1,"regions":[{"id":1}]}]})",
             R"({"type":"SetSongStructure","views":[{"name":"a","nextId":1,"regions":[{"id":1,"start":0,"end":5,"name":"","color":99999999999}]}]})",
             R"({"type":"NoSuchElementCommand"})" }) {
        INFO(text);
        std::unique_ptr<SequenceCommand> c;
        CHECK_NOTHROW(c = CommandFromJson(text));
        CHECK(c == nullptr);
    }
}

TEST_CASE("Element commands nest inside a serialized batch", "[seq][elements]") {
    SeqFixture f;
    f.Model("A");
    ModelElement* b = f.Model("B");
    std::vector<std::unique_ptr<SequenceCommand>> cmds;
    cmds.push_back(std::make_unique<SetTagCommand>(0, 100));
    cmds.push_back(std::make_unique<MoveElementCommand>(b->GetUid(), kNoSeqObjectId));
    BatchCommand batch("mixed", std::move(cmds));
    auto copy = CommandFromJson(batch.ToJson());
    REQUIRE(copy != nullptr);
    REQUIRE(copy->Apply(f.se, ApplyMode::Strict) != nullptr);
    CHECK(f.Order() == std::vector<std::string>{ "B", "A" });
    CHECK(f.se.GetTagPosition(0) == 100);
}

// ---------------------------------------------------------------------------
// Edit-site helpers, recorder, undo
// ---------------------------------------------------------------------------

TEST_CASE("ElementEdits: one undo step per action with readable labels", "[seq][elements][undo]") {
    SeqFixture f;
    f.Timing("Beats");
    f.Model("Tree");
    f.Model("House");

    f.Undo().CreateUndoStep();
    TimingElement* t = ElementEdits::AddTimingTrack(f.se, -1, "Lyrics", "Lyrics");
    REQUIRE(t != nullptr);
    CHECK(f.Order() == std::vector<std::string>{ "Beats", "Lyrics", "Tree", "House" });
    CHECK(t->GetSubType() == "Lyrics");
    CHECK(t->GetEffectLayerCount() == 1);
    CHECK(f.Undo().GetUndoString() == "Undo: Row(s) Added");

    f.Undo().CreateUndoStep();
    REQUIRE(ElementEdits::MoveElement(f.se, 3, 2, MASTER_VIEW));
    CHECK(f.Order() == std::vector<std::string>{ "Beats", "Lyrics", "House", "Tree" });
    CHECK(f.Undo().GetUndoString() == "Undo: Row(s) Moved");

    f.Undo().CreateUndoStep();
    REQUIRE(ElementEdits::RenameTimingTrack(f.se, "Lyrics", "Words"));
    CHECK(f.Undo().GetUndoString() == "Undo: Timing Track Renamed");

    f.Undo().CreateUndoStep();
    REQUIRE(ElementEdits::SetTimingFixed(f.se, t, 25));
    f.Undo().CreateUndoStep();
    REQUIRE(ElementEdits::SetTagPosition(f.se, 2, 750));
    CHECK(f.Undo().GetUndoString() == "Undo: Tag Changed");

    f.Undo().CreateUndoStep();
    REQUIRE(ElementEdits::DeleteElement(f.se, "Tree"));
    CHECK(f.Undo().GetUndoString() == "Undo: Row(s) Deleted");
    CHECK_FALSE(ElementEdits::DeleteElement(f.se, "Tree"));

    f.Undo().UndoLastStep();
    CHECK(f.Order() == std::vector<std::string>{ "Beats", "Words", "House", "Tree" });
    CHECK(f.Undo().GetRedoString() == "Redo: Row(s) Deleted");
    f.Undo().UndoLastStep();
    CHECK(f.se.GetTagPosition(2) == -1);
    f.Undo().UndoLastStep();
    CHECK(t->GetFixedTiming() == 0);
    f.Undo().UndoLastStep();
    CHECK(t->GetName() == "Lyrics");
    f.Undo().UndoLastStep();
    CHECK(f.Order() == std::vector<std::string>{ "Beats", "Lyrics", "Tree", "House" });
    f.Undo().UndoLastStep();
    CHECK(f.Order() == std::vector<std::string>{ "Beats", "Tree", "House" });
    CHECK_FALSE(f.Undo().CanUndo());

    f.Undo().RedoLastStep();
    f.Undo().RedoLastStep();
    CHECK(f.Order() == std::vector<std::string>{ "Beats", "Lyrics", "House", "Tree" });
    f.RequireValid();
}

TEST_CASE("ElementEdits::MoveElement in a per-user view is not a command", "[seq][elements]") {
    SeqFixture f;
    f.Model("A");
    f.Model("B");
    f.se.AddView("Mine");
    f.se.PopulateView("A,B", 1);
    Recorder rec;
    f.Exec().AddObserver(&rec);
    GuardMode mode(SeqMutationGuard::Mode::Count);
    REQUIRE(ElementEdits::MoveElement(f.se, 1, 0, 1));
    CHECK(f.se.GetElement(0, 1)->GetName() == "B");
    CHECK(f.Order() == std::vector<std::string>{ "A", "B" });
    CHECK(rec.applied.empty());
    CHECK(SeqMutationGuard::ViolationCount() == 0);
    f.Exec().RemoveObserver(&rec);
}

TEST_CASE("AddMissingModelsToSequence adds models as commands without an undo step", "[seq][elements]") {
    Show show;
    SeqFixture f(&show);
    Recorder rec;
    f.Exec().AddObserver(&rec);
    GuardMode mode(SeqMutationGuard::Mode::Count);
    f.se.AddMissingModelsToSequence("Grid,Nope,Line", false);
    CHECK(f.Order() == std::vector<std::string>{ "Grid", "Line" });
    CHECK_FALSE(f.se.GetElement(0)->GetVisible());
    CHECK(rec.applied.size() == 2);
    CHECK(SeqMutationGuard::ViolationCount() == 0);
    CHECK_FALSE(f.Undo().CanUndo());
    f.Exec().RemoveObserver(&rec);
}

TEST_CASE("NewElementsRecorder records a populated new track as one step", "[seq][elements][undo]") {
    SeqFixture f;
    f.Timing("Beats");
    ModelElement* tree = f.Model("Tree");
    Recorder rec;
    f.Exec().AddObserver(&rec);
    GuardMode mode(SeqMutationGuard::Mode::Count);
    std::string before = f.Describe();
    SeqObjectId uid;
    {
        NewElementsRecorder recorder(f.se, "Import Timing");
        CHECK(NewElementsRecorder::Active(f.se) == &recorder);
        TimingElement* t = ElementEdits::AddTimingTrack(f.se, -1, "Imported", "");
        REQUIRE(t != nullptr);
        uid = t->GetUid();
        t->GetEffectLayer(0)->AddEffect(0, "a", "", "", 0, 100, 0, false);
        t->AddEffectLayer()->AddEffect(0, "b", "", "", 0, 50, 0, false);
    }
    CHECK(NewElementsRecorder::Active(f.se) == nullptr);
    CHECK(SeqMutationGuard::ViolationCount() == 0);
    std::string after = f.Describe();
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.applied[0]["type"] == "Batch");
    REQUIRE(rec.applied[0]["commands"].size() == 1);
    CHECK(rec.applied[0]["commands"][0]["type"] == "AddElement");
    CHECK(rec.applied[0]["commands"][0]["element"]["layers"].size() == 2);
    CHECK(f.Undo().GetUndoString() == "Undo: Import Timing");

    // The recorded command alone rebuilds the track, ids and all.
    SeqFixture other;
    other.Timing("Beats");
    other.Model("Tree");
    auto replay = DeserializeCommand(rec.applied[0]);
    REQUIRE(replay != nullptr);
    REQUIRE(replay->Apply(other.se, ApplyMode::Lenient) != nullptr);
    CHECK(other.Describe(false) == f.Describe(false));
    CHECK(other.se.GetElementFromUid(uid) != nullptr);

    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    f.Undo().RedoLastStep();
    CHECK(f.Describe() == after);
    CHECK(tree == f.se.GetElement(2));

    // Nothing new: no step.
    {
        NewElementsRecorder recorder(f.se, "Nothing");
    }
    CHECK(f.Undo().GetUndoString() == "Undo: Import Timing");
    f.Exec().RemoveObserver(&rec);
}

TEST_CASE("NewElementsRecorder reports a direct change to an existing row", "[seq][elements][undo]") {
    SeqFixture f;
    TimingElement* beats = f.Timing("Beats");
    ModelElement* tree = f.Model("Tree");
    GuardMode mode(SeqMutationGuard::Mode::Count);
    // Changing existing rows through commands inside the recorder is fine:
    // they join its step.
    SeqMutationGuard::ResetViolationCount();
    {
        NewElementsRecorder recorder(f.se, "Import Timing");
        TimingElement* t = ElementEdits::AddTimingTrack(f.se, -1, "Imported", "");
        t->GetEffectLayer(0)->AddEffect(0, "a", "", "", 0, 100, 0, false);
        REQUIRE(f.Exec().AddEffect(tree->GetEffectLayer(0), "On", "", "", 0, 100, 0, false) != nullptr);
    }
    CHECK(SeqMutationGuard::ViolationCount() == 0);
    // A direct change to a row that existed before goes unrecorded (and so
    // never reaches undo or a live session): the guard reports it.
    {
        NewElementsRecorder recorder(f.se, "Import Timing");
        ElementEdits::AddTimingTrack(f.se, -1, "Second", "");
        beats->GetEffectLayer(0)->AddEffect(0, "x", "", "", 0, 100, 0, false);
    }
    CHECK(SeqMutationGuard::ViolationCount() == 1);
}

TEST_CASE("NewElementsRecorder inside an open batch or preview takes command edits for recorded ones", "[seq][elements][undo]") {
    // The outer batch or preview holds back the notification, so the
    // recorder must learn what its commands touched as they are recorded.
    SeqFixture f;
    f.Timing("Beats");
    ModelElement* tree = f.Model("Tree");
    GuardMode mode(SeqMutationGuard::Mode::Count);
    Recorder rec;
    f.Exec().AddObserver(&rec);
    {
        ScopedCommandBatch batch(f.Exec(), "Caller");
        NewElementsRecorder recorder(f.se, "Add Timing Track");
        ElementEdits::AddTimingTrack(f.se, -1, "Imported", "");
        REQUIRE(f.Exec().AddEffect(tree->GetEffectLayer(0), "On", "", "", 0, 100, 0, false) != nullptr);
    }
    CHECK(SeqMutationGuard::ViolationCount() == 0);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.applied[0]["commands"].size() == 2);
    SeqMutationGuard::ResetViolationCount();
    f.Exec().BeginPreview();
    {
        NewElementsRecorder recorder(f.se, "Add Timing Track");
        ElementEdits::AddTimingTrack(f.se, -1, "Second", "");
        REQUIRE(f.Exec().AddEffect(tree->GetEffectLayer(0), "On", "", "", 200, 300, 0, false) != nullptr);
    }
    f.Exec().CommitPreview("");
    CHECK(SeqMutationGuard::ViolationCount() == 0);
    // A direct change inside the same nesting is still reported.
    SeqMutationGuard::ResetViolationCount();
    {
        ScopedCommandBatch batch(f.Exec(), "Caller");
        NewElementsRecorder recorder(f.se, "Add Timing Track");
        ElementEdits::AddTimingTrack(f.se, -1, "Third", "");
        REQUIRE(f.Exec().AddEffect(tree->GetEffectLayer(0), "On", "", "", 400, 500, 0, false) != nullptr);
        f.se.GetTimingElement("Beats")->GetEffectLayer(0)->AddEffect(0, "x", "", "", 0, 100, 0, false);
    }
    CHECK(SeqMutationGuard::ViolationCount() == 1);
    f.Exec().RemoveObserver(&rec);
}

TEST_CASE("NewElementsRecorder reports a direct change to a submodel or strand row", "[seq][elements][undo]") {
    Show show;
    SeqFixture f(&show);
    ModelElement* grid = f.Model("Grid");
    ModelElement* line = f.Model("Line");
    Populate(f, grid);
    GuardMode mode(SeqMutationGuard::Mode::Count);
    // A change only the submodel row's own count shows.
    {
        NewElementsRecorder recorder(f.se, "Import Timing");
        ElementEdits::AddTimingTrack(f.se, -1, "One", "");
        grid->GetSubModel(0)->SetName("Renamed");
    }
    CHECK(SeqMutationGuard::ViolationCount() == 1);
    // A strand edit, while a command edits another model.
    SeqMutationGuard::ResetViolationCount();
    {
        NewElementsRecorder recorder(f.se, "Import Timing");
        ElementEdits::AddTimingTrack(f.se, -1, "Two", "");
        REQUIRE(f.Exec().AddEffect(line->GetEffectLayer(0), "On", "", "", 0, 100, 0, false) != nullptr);
        grid->GetStrand(1)->GetEffectLayer(0)->AddEffect(0, "On", "", "", 500, 600, 0, false);
    }
    CHECK(SeqMutationGuard::ViolationCount() == 1);
    // The same strand edit made as a command is fine.
    SeqMutationGuard::ResetViolationCount();
    {
        NewElementsRecorder recorder(f.se, "Import Timing");
        ElementEdits::AddTimingTrack(f.se, -1, "Three", "");
        REQUIRE(f.Exec().AddEffect(grid->GetStrand(1)->GetEffectLayer(0), "On", "", "", 700, 800, 0, false) != nullptr);
    }
    CHECK(SeqMutationGuard::ViolationCount() == 0);
}

TEST_CASE("Nested NewElementsRecorders record once, in the outer step", "[seq][elements][undo]") {
    SeqFixture f;
    Recorder rec;
    f.Exec().AddObserver(&rec);
    {
        NewElementsRecorder outer(f.se, "Outer");
        ElementEdits::AddTimingTrack(f.se, -1, "One", "");
        {
            NewElementsRecorder inner(f.se, "Inner");
            CHECK(NewElementsRecorder::Active(f.se) == &outer);
            ElementEdits::AddTimingTrack(f.se, -1, "Two", "");
        }
        CHECK(rec.applied.empty());
    }
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.applied[0]["label"] == "Outer");
    CHECK(rec.applied[0]["commands"].size() == 2);
    CHECK(f.Undo().GetUndoString() == "Undo: Outer");
    f.Undo().UndoLastStep();
    CHECK(f.se.GetElementCount() == 0);
    CHECK_FALSE(f.Undo().CanUndo());
    f.Exec().RemoveObserver(&rec);
}

TEST_CASE("NewElementsRecorder can skip undo", "[seq][elements][undo]") {
    SeqFixture f;
    Recorder rec;
    f.Exec().AddObserver(&rec);
    {
        NewElementsRecorder recorder(f.se, "New Sequence", false);
        ElementEdits::AddTimingTrack(f.se, -1, "Beats", "");
    }
    CHECK(rec.applied.size() == 1);
    CHECK_FALSE(f.Undo().CanUndo());
    f.Exec().RemoveObserver(&rec);
}

#ifndef NDEBUG
TEST_CASE("Mutation guard covers element, tag, face and song structure edits", "[seq][elements][guard]") {
    SeqFixture f;
    TimingElement* t = f.Timing("Beats");
    f.Model("A");
    f.Model("B");
    GuardMode mode(SeqMutationGuard::Mode::Count);

    f.se.AddElement("C", "model", true, false, false, false, false);
    CHECK(SeqMutationGuard::ViolationCount() == 1);
    f.se.MoveSequenceElement(1, 0, MASTER_VIEW);
    CHECK(SeqMutationGuard::ViolationCount() == 2);
    f.se.DeleteElement("C");
    CHECK(SeqMutationGuard::ViolationCount() == 3);
    f.se.SetTagPosition(1, 10);
    f.se.ClearTags();
    CHECK(SeqMutationGuard::ViolationCount() == 5);
    t->SetName("Beat");
    t->SetFixedTiming(10);
    t->SetSubType("x");
    t->Unfix();
    CHECK(SeqMutationGuard::ViolationCount() == 9);
    f.se.GetSequenceFaces().SetFace("F", {});
    f.se.GetSequenceFaces().RenameFace("F", "G");
    f.se.GetSequenceFaces().RemoveFace("G");
    CHECK(SeqMutationGuard::ViolationCount() == 12);
    f.se.GetSongStructureManager().AddBoundary(100, 1000);
    f.se.GetSongStructureManager().AddView("V");
    CHECK(SeqMutationGuard::ViolationCount() == 14);

    // Per-user state and closing the sequence are not edits.
    SeqMutationGuard::ResetViolationCount();
    f.se.GetSongStructureManager().SetActiveView(1);
    t->SetViews("X");
    t->SetActive(true);
    t->SetVisible(false);
    f.se.AddView("Mine");
    f.se.PopulateView("A", 1);
    f.se.MoveSequenceElement(0, 0, 1);
    f.se.Clear();
    CHECK(SeqMutationGuard::ViolationCount() == 0);
}

TEST_CASE("Element edits through ElementEdits trip nothing", "[seq][elements][guard]") {
    SeqFixture f;
    f.Model("A");
    GuardMode mode(SeqMutationGuard::Mode::Count);
    f.Undo().CreateUndoStep();
    TimingElement* t = ElementEdits::AddTimingTrack(f.se, -1, "T", "");
    ElementEdits::AddModel(f.se, -1, "B");
    ElementEdits::MoveElement(f.se, 2, 0, MASTER_VIEW);
    ElementEdits::RenameTimingTrack(f.se, "T", "U");
    ElementEdits::SetTimingFixed(f.se, t, 0);
    ElementEdits::SetTagPosition(f.se, 0, 5);
    ElementEdits::EditSequenceFaces(f.se, [](SequenceFaces& fc) { fc.SetFace("F", {}); });
    ElementEdits::EditSongStructure(f.se, [](SongStructureManager& m) { m.AddBoundary(10, 100); });
    ElementEdits::DeleteElement(f.se, "A");
    f.Undo().UndoLastStep();
    f.Undo().RedoLastStep();
    CHECK(SeqMutationGuard::ViolationCount() == 0);
}
#endif

namespace {
// Counts render aborts and can pretend one timed out.
struct AbortWatchContext : HeadlessRenderContext {
    int aborts = 0;
    bool abortSucceeds = true;
    bool sequenceOpen = false;
    // Runs inside the abort, like events the desktop handles while it yields.
    std::function<void()> whileAborting;
    bool IsSequenceLoaded() const override { return sequenceOpen; }
    bool AbortRender(int maxTimeMs = 60000) override {
        (void)maxTimeMs;
        ++aborts;
        if (whileAborting) {
            auto run = std::move(whileAborting);
            run();
        }
        return abortSucceeds;
    }
};
} // namespace

TEST_CASE("Undoing or redoing a row add or delete stops the render and keeps the row for a while", "[seq][elements][undo]") {
    AbortWatchContext ctx;
    SequenceElements& se = ctx.GetSequenceElements();
    UndoManager& undo = se.get_undo_mgr();
    se.SetDeletedRowGraceMs(0);

    undo.CreateUndoStep();
    REQUIRE(ElementEdits::AddTimingTrack(se, -1, "Lyrics", "Lyrics") != nullptr);
    undo.CreateUndoStep();
    REQUIRE(ElementEdits::AddTimingTrack(se, -1, "Beats", "") != nullptr);

    ctx.aborts = 0;
    undo.UndoLastStep();
    CHECK(se.GetElement("Beats") == nullptr);
    CHECK(ctx.aborts >= 1);
    CHECK(se.GetElementsAwaitingDelete() == 0); // no grace period: freed once the render stopped

    // A render that won't stop: undo of a row step isn't replayed at all...
    ctx.abortSucceeds = false;
    ctx.aborts = 0;
    CHECK_FALSE(undo.UndoLastStep());
    CHECK(se.GetElement("Lyrics") != nullptr);
    CHECK(ctx.aborts >= 1);
    CHECK(undo.CanUndo());
    // ...and a direct delete keeps the row alive instead of freeing it under the render.
    undo.CreateUndoStep();
    REQUIRE(ElementEdits::DeleteElement(se, "Lyrics"));
    CHECK(se.GetElement("Lyrics") == nullptr);
    CHECK(se.GetElementsAwaitingDelete() == 1);
    // Adding a row is refused outright while the render runs in an open sequence.
    ctx.sequenceOpen = true;
    {
        SeqMutationGuard::Scope direct;
        CHECK(se.AddElement("Late", "timing", true, false, false, false, false) == nullptr);
    }
    CHECK(se.GetElement("Late") == nullptr);
    ctx.sequenceOpen = false;

    // Undo brings it back; deleting a row once the render stops frees the waiting one too.
    ctx.abortSucceeds = true;
    REQUIRE(undo.UndoLastStep());
    REQUIRE(se.GetElement("Lyrics") != nullptr);
    undo.CreateUndoStep();
    REQUIRE(ElementEdits::AddTimingTrack(se, -1, "Beats", "") != nullptr);
    undo.CreateUndoStep();
    REQUIRE(ElementEdits::DeleteElement(se, "Beats"));
    CHECK(se.GetElementsAwaitingDelete() == 0);

    // Undoing a timing track rename stops the render before effects are rewritten.
    undo.CreateUndoStep();
    REQUIRE(ElementEdits::RenameTimingTrack(se, "Lyrics", "Words"));
    ctx.aborts = 0;
    undo.UndoLastStep();
    CHECK(se.GetElement("Lyrics") != nullptr);
    CHECK(ctx.aborts >= 1);
}

TEST_CASE("A deleted row and its effects stay valid for the grace period, like deleted effects", "[seq][elements][undo]") {
    AbortWatchContext ctx;
    SequenceElements& se = ctx.GetSequenceElements();
    UndoManager& undo = se.get_undo_mgr();
    undo.CreateUndoStep();
    TimingElement* t = ElementEdits::AddTimingTrack(se, -1, "Lyrics", "Lyrics");
    REQUIRE(t != nullptr);
    Effect* selected = nullptr;
    {
        SeqMutationGuard::Scope setup;
        selected = t->GetEffectLayer(0)->AddEffect(0, "hello", "", "", 0, 500, EFFECT_SELECTED, false);
    }
    REQUIRE(selected != nullptr);

    // The UI still holds `selected` when undo removes its row.
    undo.UndoLastStep();
    REQUIRE(se.GetElement("Lyrics") == nullptr);
    CHECK(se.GetElementsAwaitingDelete() == 1);
    CHECK(selected->GetEffectName() == "hello");
    CHECK(selected->GetParentEffectLayer()->GetParentElement() == t);

    // Later deletes (render stopped) don't free it before its time.
    undo.CreateUndoStep();
    REQUIRE(ElementEdits::AddTimingTrack(se, -1, "Beats", "") != nullptr);
    undo.UndoLastStep();
    CHECK(se.GetElementsAwaitingDelete() == 2);

    se.Clear(); // closing the sequence frees them
    CHECK(se.GetElementsAwaitingDelete() == 0);
}

TEST_CASE("Edits render threads read by name or reference stop the render first", "[seq][elements]") {
    AbortWatchContext ctx;
    SequenceElements& se = ctx.GetSequenceElements();
    Element* tree = nullptr;
    {
        SeqMutationGuard::Scope setup;
        tree = se.AddElement("Tree", "model", true, false, false, false, false);
    }
    REQUIRE(tree != nullptr);
    CHECK(ctx.aborts == 0); // loading rows into a sequence that isn't open yet doesn't

    ctx.sequenceOpen = true;
    ctx.aborts = 0;
    {
        SeqMutationGuard::Scope setup;
        REQUIRE(se.AddElement(0, "Beats", "timing", true, false, false, false, false) != nullptr);
    }
    CHECK(ctx.aborts == 1);

    ctx.aborts = 0;
    RenameModelElementCommand rename(tree->GetUid(), "Big Tree");
    REQUIRE(rename.Apply(se, ApplyMode::Strict) != nullptr);
    CHECK(ctx.aborts == 1);

    ctx.aborts = 0;
    SetSequenceFaceCommand face("Singer", SetSequenceFaceCommand::FaceDefinition{ { "Mouth-AI", "a.png" } });
    REQUIRE(face.Apply(se, ApplyMode::Strict) != nullptr);
    CHECK(ctx.aborts == 1);
}

TEST_CASE("Layer edits on a submodel row wait for the lock its model's render holds", "[seq][elements]") {
    Show show;
    AbortWatchContext ctx;
    REQUIRE(ctx.LoadShowFolder(show.dir.path.string()));
    SequenceElements& se = ctx.GetSequenceElements();
    ModelElement* grid = nullptr;
    {
        SeqMutationGuard::Scope setup;
        grid = dynamic_cast<ModelElement*>(se.AddElement("Grid", "model", true, false, false, false, false));
    }
    REQUIRE(grid != nullptr);
    REQUIRE(grid->GetSubModelCount() > 0);
    SubModelElement* top = grid->GetSubModel(0);
    REQUIRE(top != nullptr);
    {
        SeqMutationGuard::Scope setup;
        if (top->GetEffectLayerCount() == 0) {
            top->AddEffectLayer();
        }
    }
    const size_t layers = top->GetEffectLayerCount();

    // A render job for Grid holds Grid's lock for a whole slice.
    std::promise<void> held;
    std::promise<void> release;
    std::thread render([&]() {
        std::unique_lock<std::recursive_timed_mutex> lock(grid->GetRenderLock());
        held.set_value();
        release.get_future().wait();
    });
    held.get_future().wait();
    ctx.aborts = 0;
    std::thread releaser([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        release.set_value();
    });
    {
        SeqMutationGuard::Scope direct;
        REQUIRE(top->InsertEffectLayer(0) != nullptr);
    }
    // Contended for 500ms, so it asked the render to stop and then waited for the slice to end.
    CHECK(ctx.aborts == 1);
    CHECK(top->GetEffectLayerCount() == layers + 1);
    releaser.join();
    render.join();
}

TEST_CASE("Undo stops the render once before replaying a row step and refuses a nested undo", "[seq][elements][undo]") {
    AbortWatchContext ctx;
    SequenceElements& se = ctx.GetSequenceElements();
    UndoManager& undo = se.get_undo_mgr();
    undo.CreateUndoStep();
    REQUIRE(ElementEdits::AddTimingTrack(se, -1, "Lyrics", "Lyrics") != nullptr);
    // One step: a new track, then an effect on the first one. Undo replays
    // the effect removal first, then the row delete.
    undo.CreateUndoStep();
    REQUIRE(ElementEdits::AddTimingTrack(se, -1, "Beats", "") != nullptr);
    REQUIRE(se.GetCommandExecutor().AddEffect(se.GetElement("Lyrics")->GetEffectLayer(0), "word", "", "", 0, 100, 0, false) != nullptr);

    // A Cmd-Z handled while the replay waits for the render to stop.
    bool nestedRan = false;
    bool nestedReplayed = true;
    bool nothingReplayedYet = false;
    ctx.whileAborting = [&]() {
        nestedRan = true;
        // The stop comes before any of the step is replayed.
        nothingReplayedYet = se.GetElement("Beats") != nullptr && se.GetElement("Lyrics")->GetEffectLayer(0)->GetEffectCount() == 1;
        nestedReplayed = undo.UndoLastStep();
    };
    REQUIRE(undo.UndoLastStep());
    CHECK(nestedRan);
    CHECK(nothingReplayedYet);
    CHECK_FALSE(nestedReplayed);
    CHECK(se.GetElement("Lyrics")->GetEffectLayer(0)->GetEffectCount() == 0);
    // Exactly one step undone; the earlier one is intact and redo has one step.
    CHECK(se.GetElement("Beats") == nullptr);
    CHECK(se.GetElement("Lyrics") != nullptr);
    REQUIRE(undo.RedoLastStep());
    CHECK(se.GetElement("Beats") != nullptr);
    CHECK_FALSE(undo.CanRedo());

    // A step with only effect edits doesn't stop a render that is running.
    Element* lyrics = se.GetElement("Lyrics");
    undo.CreateUndoStep();
    REQUIRE(se.GetCommandExecutor().AddEffect(lyrics->GetEffectLayer(0), "hello", "", "", 0, 100, 0, false) != nullptr);
    ctx.aborts = 0;
    REQUIRE(undo.UndoLastStep());
    CHECK(ctx.aborts == 0);
}

TEST_CASE("Edits in progress are visible to hosts, and an undo can't start a new step mid-replay", "[seq][elements][undo]") {
    AbortWatchContext ctx;
    SequenceElements& se = ctx.GetSequenceElements();
    UndoManager& undo = se.get_undo_mgr();
    CHECK_FALSE(se.IsEditInProgress());
    {
        ScopedCommandBatch batch(se, "Batch");
        CHECK(se.IsEditInProgress());
    }
    CHECK_FALSE(se.IsEditInProgress());

    undo.CreateUndoStep();
    REQUIRE(ElementEdits::AddTimingTrack(se, -1, "Lyrics", "Lyrics") != nullptr);
    undo.CreateUndoStep();
    REQUIRE(se.GetCommandExecutor().AddEffect(se.GetElement("Lyrics")->GetEffectLayer(0), "word", "", "", 0, 100, 0, false) != nullptr);
    CHECK_FALSE(undo.NextStepNeedsRenderStopped(false)); // only an effect
    undo.CreateUndoStep();
    REQUIRE(ElementEdits::AddTimingTrack(se, -1, "Beats", "") != nullptr);
    CHECK(undo.NextStepNeedsRenderStopped(false)); // a row

    // Something that runs while the replay waits tries to start its own step.
    bool sawEdit = false;
    ctx.whileAborting = [&]() {
        sawEdit = se.IsEditInProgress();
        undo.CreateUndoStep();
        undo.CreateUndoStep();
    };
    REQUIRE(undo.UndoLastStep());
    CHECK(sawEdit);
    CHECK(se.GetElement("Beats") == nullptr);
    // The effect step is still next, whole, and redo still has the row step.
    CHECK(undo.CanRedo());
    REQUIRE(undo.UndoLastStep());
    CHECK(se.GetElement("Lyrics")->GetEffectLayer(0)->GetEffectCount() == 0);
    REQUIRE(undo.UndoLastStep());
    CHECK(se.GetElement("Lyrics") == nullptr);
    CHECK_FALSE(undo.CanUndo());
}
