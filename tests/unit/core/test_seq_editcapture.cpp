#include <catch2/catch_test_macros.hpp>

#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceEditCapture.h"
#include "render/SequenceElements.h"
#include "render/SequenceMutationGuard.h"
#include "render/UndoManager.h"

#include <memory>
#include <string>
#include <vector>

// SequenceEditCapture: direct edits (imports and other bulk tools) recorded
// as one batch of ordinary commands.

namespace {
struct SeqFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    ModelElement* model = nullptr;
    EffectLayer* layer = nullptr;
    SeqFixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
        model = dynamic_cast<ModelElement*>(se.AddElement("Tree", "model", true, false, true, false, false));
        layer = model->GetEffectLayerCount() > 0 ? model->GetEffectLayer(0) : model->AddEffectLayer();
    }
    UndoManager& Undo() { return se.get_undo_mgr(); }
    CommandExecutor& Exec() { return se.GetCommandExecutor(); }
    Effect* Add(EffectLayer* l, const std::string& name, int start, int end, const std::string& settings = "", const std::string& palette = "") {
        return Exec().AddEffect(l, name, settings, palette, start, end, 0, false, false);
    }
    std::string Describe() const { return DescribeSequence(se); }
};

// Keeps every batch the executor reports, as JSON.
struct Recorder : ICommandObserver {
    CommandExecutor& exec;
    std::vector<nlohmann::json> applied;
    std::vector<std::string> labels;
    explicit Recorder(CommandExecutor& e) : exec(e) { exec.AddObserver(this); }
    ~Recorder() override { exec.RemoveObserver(this); }
    void OnCommandApplied(const SequenceCommand& cmd, const SequenceCommand&) override {
        nlohmann::json j;
        cmd.Serialize(j);
        applied.push_back(j);
        const BatchCommand* b = dynamic_cast<const BatchCommand*>(&cmd);
        labels.push_back(b != nullptr ? b->GetLabel() : "");
    }
    std::vector<std::string> MemberTypes(size_t i) const {
        std::vector<std::string> out;
        for (const auto& m : applied[i]["commands"]) {
            out.push_back(m["type"].get<std::string>());
        }
        return out;
    }
};

#ifndef NDEBUG
struct GuardCount {
    SeqMutationGuard::Mode prev;
    GuardCount() : prev(SeqMutationGuard::GetMode()) {
        SeqMutationGuard::SetMode(SeqMutationGuard::Mode::Count);
        SeqMutationGuard::ResetViolationCount();
    }
    ~GuardCount() { SeqMutationGuard::SetMode(prev); }
};
#endif

// The recorded batch, replayed strictly onto the state before the edit,
// must give the state after it (ids included).
void CheckReplay(SeqFixture& f, const nlohmann::json& batch, const std::string& before, const std::string& after) {
    f.Undo().UndoLastStep();
    REQUIRE(f.Describe() == before);
    auto cmd = DeserializeCommand(batch);
    REQUIRE(cmd != nullptr);
    CommandResult r;
    auto inverse = cmd->Apply(f.se, ApplyMode::Strict, &r);
    INFO(r.message);
    REQUIRE(inverse != nullptr);
    CHECK(f.Describe() == after);
    // and the batch's own inverse takes it back
    REQUIRE(inverse->Apply(f.se, ApplyMode::Strict) != nullptr);
    CHECK(f.Describe() == before);
}
} // namespace

TEST_CASE("Captured effect edits become one labelled undo step", "[seq][capture][undo]") {
    SeqFixture f;
    Effect* a = f.Add(f.layer, "On", 0, 100, "E_TEXTCTRL_Eff_On_Start=10", "C_BUTTON_Palette1=#FF0000");
    Effect* b = f.Add(f.layer, "On", 200, 300);
    f.Add(f.layer, "On", 400, 500);
    f.Undo().Clear();
    std::string before = f.Describe();
    Recorder rec(f.Exec());
    {
#ifndef NDEBUG
        GuardCount guard;
#endif
        SequenceEditCapture capture(f.se, "Import Effects");
        f.layer->DeleteEffectByUid(b->GetUid());
        a->SetSetting("E_TEXTCTRL_Eff_On_Start", "50");
        a->SetSetting("E_TEXTCTRL_Eff_On_End", "75");
        a->SetEndTimeMS(150);
        f.layer->AddEffect(0, "Color Wash", "", "C_BUTTON_Palette1=#00FF00", 600, 700, 0, false);
#ifndef NDEBUG
        CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
        capture.Commit();
        CHECK(capture.MadeUndoStep());
    }
    std::string after = f.Describe();
    REQUIRE(after != before);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.labels[0] == "Import Effects");
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "DeleteEffect", "MoveEffect", "ModifyEffect", "AddEffect" });
    CHECK(f.Undo().GetUndoString() == "Undo: Import Effects");

    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    CHECK_FALSE(f.Undo().CanUndo());
    f.Undo().RedoLastStep();
    CHECK(f.Describe() == after);

    CheckReplay(f, rec.applied[0], before, after);
}

TEST_CASE("Captured layer adds, removals, renames and reorders undo exactly", "[seq][capture][undo]") {
    SeqFixture f;
    EffectLayer* a = f.layer;
    EffectLayer* b = f.Exec().InsertLayer(f.model, 1, false);
    EffectLayer* c = f.Exec().InsertLayer(f.model, 2, false);
    EffectLayer* d = f.Exec().InsertLayer(f.model, 3, false);
    f.Add(a, "On", 0, 100);
    f.Add(b, "On", 0, 100);
    f.Add(c, "On", 0, 100);
    f.Add(d, "On", 0, 100);
    f.Exec().SetLayerName(b, "bee", false);
    f.Undo().Clear();
    std::string before = f.Describe();
    Recorder rec(f.Exec());
    {
        SequenceEditCapture capture(f.se, "Rearrange");
        // A B C D  ->  N A C' B   (D gone, B moved after C, N new, C renamed)
        SeqObjectId bUid = b->GetUid();
        f.model->RemoveEffectLayer(3); // A B C
        f.model->RemoveEffectLayer(1); // A C
        EffectLayer* back = f.model->AddEffectLayer();
        back->SetUid(bUid);
        back->SetLayerName("bee");
        back->AddEffect(0, "On", "", "", 0, 100, 0, false); // A C B
        EffectLayer* fresh = f.model->InsertEffectLayer(0); // N A C B
        fresh->AddEffect(0, "On", "", "", 500, 600, 0, false);
        fresh->AddEffect(0, "On", "", "", 600, 700, 0, false);
        c->SetLayerName("sea");
    }
    std::string after = f.Describe();
    REQUIRE(after != before);
    REQUIRE(rec.applied.size() == 1);

    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    f.Undo().RedoLastStep();
    CHECK(f.Describe() == after);

    CheckReplay(f, rec.applied[0], before, after);
}

TEST_CASE("Captured edits on submodel, strand and node layers are recorded", "[seq][capture][undo]") {
    SeqFixture f;
    StrandElement* strand = f.model->GetStrand(0, true);
    NodeLayer* node = strand->GetNodeLayer(2, true);
    f.Add(node, "On", 0, 100);
    f.Undo().Clear();
    std::string before = f.Describe();
    Recorder rec(f.Exec());
    {
        SequenceEditCapture capture(f.se, "Convert");
        node->DeleteAllEffects();
        node->AddEffect(0, "Color Wash", "", "", 0, 50, 0, false);
        strand->GetEffectLayer(0)->AddEffect(0, "On", "", "", 100, 200, 0, false);
    }
    std::string after = f.Describe();
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "DeleteEffect", "AddEffect", "AddEffect" });
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    f.Undo().RedoLastStep();
    CHECK(f.Describe() == after);
}

TEST_CASE("A capture replacing most of a big layer records ReplaceLayerContents", "[seq][capture]") {
    SeqFixture f;
    for (int i = 0; i < 40; ++i) {
        f.Add(f.layer, "On", i * 100, i * 100 + 50);
    }
    f.Undo().Clear();
    std::string before = f.Describe();
    Recorder rec(f.Exec());
    {
        SequenceEditCapture capture(f.se, "Import Effects");
        f.layer->DeleteAllEffects();
        for (int i = 0; i < 40; ++i) {
            f.layer->AddEffect(0, "Color Wash", "", "", i * 100, i * 100 + 80, 0, false);
        }
    }
    std::string after = f.Describe();
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "ReplaceLayerContents" });
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    f.Undo().RedoLastStep();
    CHECK(f.Describe() == after);
    CheckReplay(f, rec.applied[0], before, after);
}

TEST_CASE("Executor edits inside a capture are folded into its batch", "[seq][capture]") {
    SeqFixture f;
    Effect* a = f.Add(f.layer, "On", 0, 100);
    f.Undo().Clear();
    std::string before = f.Describe();
    Recorder rec(f.Exec());
    {
        SequenceEditCapture capture(f.se, "Import Effects");
        CHECK(f.Exec().IsCapturing());
        f.Exec().DeleteEffect(a);
        EffectLayer* l = f.Exec().InsertLayer(f.model, 1);
        f.Exec().AddEffect(l, "On", "", "", 0, 100, 0, false);
        {
            // nested captures and batches add nothing of their own
            SequenceEditCapture inner(f.se, "Inner");
            ScopedCommandBatch batch(f.Exec(), "Batch");
            f.layer->AddEffect(0, "On", "", "", 200, 300, 0, false);
        }
        CHECK(rec.applied.empty());
    }
    CHECK_FALSE(f.Exec().IsCapturing());
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.labels[0] == "Import Effects");
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "DeleteEffect", "AddEffect", "AddLayer" });
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("A capture that changes nothing, or is discarded, records nothing", "[seq][capture]") {
    SeqFixture f;
    f.Add(f.layer, "On", 0, 100);
    f.Undo().Clear();
    Recorder rec(f.Exec());
    {
        SequenceEditCapture capture(f.se, "Nothing");
        capture.Commit();
        CHECK_FALSE(capture.MadeUndoStep());
    }
    {
        SequenceEditCapture capture(f.se, "Discarded");
        f.layer->AddEffect(0, "On", "", "", 200, 300, 0, false);
        capture.Discard();
    }
    CHECK(rec.applied.empty());
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("A capture joins an open batch", "[seq][capture]") {
    SeqFixture f;
    f.Undo().Clear();
    std::string before = f.Describe();
    Recorder rec(f.Exec());
    {
        ScopedCommandBatch batch(f.Exec(), "Outer");
        f.Exec().AddEffect(f.layer, "On", "", "", 0, 100, 0, false);
        SequenceEditCapture capture(f.se, "Inner");
        f.layer->AddEffect(0, "On", "", "", 200, 300, 0, false);
    }
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.labels[0] == "Outer");
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "AddEffect", "AddEffect" });
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
}

namespace {
// Stands in for a command a capture doesn't record itself (an element or
// media command): adds a timing track, or removes it again.
class TrackCommand : public SequenceCommand {
public:
    TrackCommand(std::string name, bool add) : _name(std::move(name)), _add(add) {}
    CommandType Type() const override { return static_cast<CommandType>(1000); }
    CommandResult Validate(const SequenceElements&, ApplyMode) const override { return CommandResult::Success(); }
    void Serialize(nlohmann::json& out) const override { out = nlohmann::json{ { "type", "Track" }, { "name", _name }, { "add", _add } }; }
    std::unique_ptr<SequenceCommand> Clone() const override { return std::make_unique<TrackCommand>(*this); }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode, CommandResult*) override {
        if (_add) {
            Element* e = seq.AddElement(_name, "timing", true, false, true, false, false);
            if (e->GetEffectLayerCount() == 0) {
                e->AddEffectLayer();
            }
        } else {
            seq.DeleteElement(_name);
        }
        return std::make_unique<TrackCommand>(_name, !_add);
    }

private:
    std::string _name;
    bool _add;
};
} // namespace

TEST_CASE("Other commands inside a capture join its batch, and new elements are compared", "[seq][capture][undo]") {
    SeqFixture f;
    f.Undo().Clear();
    std::string before = f.Describe();
    Recorder rec(f.Exec());
    {
#ifndef NDEBUG
        GuardCount guard;
#endif
        SequenceEditCapture capture(f.se, "Import Timing");
        f.layer->AddEffect(0, "On", "", "", 0, 100, 0, false);
        REQUIRE(f.Exec().Execute(std::make_unique<TrackCommand>("Beats", true)).Ok());
        Element* track = f.se.GetElement("Beats");
        REQUIRE(track != nullptr);
        track->GetEffectLayer(0)->AddEffect(0, "", "", "", 0, 500, 0, false);
        track->GetEffectLayer(0)->AddEffect(0, "", "", "", 500, 1000, 0, false);
#ifndef NDEBUG
        CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    }
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.labels[0] == "Import Timing");
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "Track", "AddEffect", "AddEffect", "AddEffect" });

    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    CHECK(f.se.GetElement("Beats") == nullptr);
    CHECK_FALSE(f.Undo().CanUndo());
}

#ifndef NDEBUG
TEST_CASE("A capture still reports non-effect mutations made outside commands", "[seq][capture][guard]") {
    SeqFixture f;
    GuardCount guard;
    SequenceEditCapture capture(f.se, "Import");
    f.layer->AddEffect(0, "On", "", "", 0, 100, 0, false);
    CHECK(SeqMutationGuard::ViolationCount() == 0);
    SeqMutationGuard::Check("SequenceMedia::AddEmbeddedImage", nullptr);
    CHECK(SeqMutationGuard::ViolationCount() == 1);
}
#endif

#include "test_impexp2_super_common.h"

TEST_CASE("A real importer run inside a capture is one undo step", "[seq][capture][undo]") {
    using namespace ss2;
    Fixture f;
    f.Seq().get_undo_mgr().Clear();
    // the import keeps what is on the model and adds two layers
    f.Seq().GetCommandExecutor().InsertLayer(f.el, 0, false);
    REQUIRE(f.Seq().GetCommandExecutor().AddEffect(f.el->GetEffectLayer(0), "On", "", "", 5000, 6000, 0, false, false) != nullptr);
    std::string before = DescribeSequence(f.Seq());
    {
#ifndef NDEBUG
        GuardCount guard;
#endif
        SequenceEditCapture capture(f.Seq(), "Import Effects");
        REQUIRE(f.RunRaw(Doc(Layout(), "<scenes>" + Scene(1, 1, 0, 100, RED, Grid(3, 4)) + Scene(2, 2, 50, 150, RED_TO_BLUE, Elem(0, 0) + Elem(1, 0)) +
                                            "</scenes>")));
#ifndef NDEBUG
        CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    }
    std::string after = DescribeSequence(f.Seq());
    REQUIRE(f.Count() == 3);
    REQUIRE(f.el->GetEffectLayerCount() == 3);
    REQUIRE(after != before);
    CHECK(f.Seq().get_undo_mgr().GetUndoString() == "Undo: Import Effects");
    f.Seq().get_undo_mgr().UndoLastStep();
    CHECK(DescribeSequence(f.Seq()) == before);
    CHECK_FALSE(f.Seq().get_undo_mgr().CanUndo());
    f.Seq().get_undo_mgr().RedoLastStep();
    CHECK(DescribeSequence(f.Seq()) == after);
}
