#include <catch2/catch_test_macros.hpp>

#include "import_export/ImportDataRow.h"
#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceBulkEdits.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceMutationGuard.h"
#include "render/UndoManager.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

// Whole-sequence tools (Shift Effects, Promote Node Effects, Convert Data To
// Effects) change the sequence through commands, as one undo step.

namespace {
struct SeqFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    ModelElement* model = nullptr;
    EffectLayer* layer = nullptr;
    SeqFixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
        SeqMutationGuard::Scope setup;
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

struct Recorder : ICommandObserver {
    CommandExecutor& exec;
    std::vector<std::string> labels;
    explicit Recorder(CommandExecutor& e) : exec(e) { exec.AddObserver(this); }
    ~Recorder() override { exec.RemoveObserver(this); }
    void OnCommandApplied(const SequenceCommand& cmd, const SequenceCommand&) override {
        const BatchCommand* b = dynamic_cast<const BatchCommand*>(&cmd);
        labels.push_back(b != nullptr ? b->GetLabel() : CommandTypeName(cmd.Type()));
    }
};

std::vector<std::pair<int, int>> Times(EffectLayer* l) {
    std::vector<std::pair<int, int>> out;
    for (int i = 0; i < l->GetEffectCount(); ++i) {
        out.emplace_back(l->GetEffect(i)->GetStartTimeMS(), l->GetEffect(i)->GetEndTimeMS());
    }
    std::sort(out.begin(), out.end());
    return out;
}

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
} // namespace

TEST_CASE("Shift Effects moves, truncates and deletes in one undo step", "[seq][bulk][undo]") {
    SeqFixture f;
    f.Add(f.layer, "On", 0, 100);
    f.Add(f.layer, "On", 200, 300);
    f.Add(f.layer, "On", 400, 500);
    Effect* locked = f.Add(f.layer, "On", 600, 700);
    f.Exec().SetEffectLocked(locked, true, false);
    SeqMutationGuard::Scope setup;
    TimingElement* timing = dynamic_cast<TimingElement*>(f.se.AddElement("Beats", "timing", true, false, true, false, false));
    EffectLayer* marks = timing->GetEffectLayerCount() > 0 ? timing->GetEffectLayer(0) : timing->AddEffectLayer();
    f.Add(marks, "", 1000, 1100);
    f.Undo().Clear();
    std::string before = f.Describe();
    Recorder rec(f.Exec());
    {
#ifndef NDEBUG
        GuardCount guard;
#endif
        ShiftEffects(f.se, -250, false, false, "Shift Effects");
#ifndef NDEBUG
        CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    }
    // the locked effect stays where it is
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 0, 50 }, { 150, 250 }, { 600, 700 } });
    CHECK(Times(marks) == std::vector<std::pair<int, int>>{ { 1000, 1100 } });
    CHECK(rec.labels == std::vector<std::string>{ "Shift Effects" });
    CHECK(f.Undo().GetUndoString() == "Undo: Shift Effects");
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    CHECK_FALSE(f.Undo().CanUndo());

    ShiftEffects(f.se, 100, true, false, "Shift Effects And Timing");
    CHECK(Times(marks) == std::vector<std::pair<int, int>>{ { 1100, 1200 } });
}

TEST_CASE("Shift Effects never moves or deletes a locked effect", "[seq][bulk]") {
    SeqFixture f;
    Effect* gone = f.Add(f.layer, "On", 0, 100);
    Effect* moved = f.Add(f.layer, "On", 1000, 1100);
    f.Exec().SetEffectLocked(gone, true, false);
    f.Exec().SetEffectLocked(moved, true, false);
    ShiftEffects(f.se, -500, false, false, "Shift Effects");
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 0, 100 }, { 1000, 1100 } });
    gone->SetSelected(EFFECT_SELECTED);
    moved->SetSelected(EFFECT_SELECTED);
    ShiftEffects(f.se, -500, false, true, "Shift Selected Effects");
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 0, 100 }, { 1000, 1100 } });
    ShiftEffects(f.se, 500, false, true, "Shift Selected Effects");
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 0, 100 }, { 1000, 1100 } });
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("Shift Selected Effects skips clashes and only moves the selection", "[seq][bulk][undo]") {
    SeqFixture f;
    Effect* a = f.Add(f.layer, "On", 0, 100);
    Effect* b = f.Add(f.layer, "On", 200, 300);
    f.Add(f.layer, "On", 400, 500);
    Effect* c = f.Add(f.layer, "On", 700, 800);
    a->SetSelected(EFFECT_SELECTED);
    b->SetSelected(EFFECT_SELECTED);
    c->SetSelected(EFFECT_SELECTED);
    f.Undo().Clear();
    std::string before = f.Describe();
    ShiftEffects(f.se, 150, false, true, "Shift Selected Effects");
    // b would land on the unselected 400-500 effect and a on b, so both stay
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 0, 100 }, { 200, 300 }, { 400, 500 }, { 850, 950 } });
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);

    ShiftEffects(f.se, -150, false, true, "Shift Selected Effects");
    // a is deleted (moved entirely before 0), b moves, c moves
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 50, 150 }, { 400, 500 }, { 550, 650 } });
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
}

namespace {
// A single-strand model with three node layers carrying `settings`.
StrandElement* NodesWith(SeqFixture& f, const std::vector<std::string>& settings) {
    StrandElement* strand = f.model->GetStrand(0, true);
    for (size_t n = 0; n < settings.size(); ++n) {
        NodeLayer* node = strand->GetNodeLayer((int)n, true);
        f.Add(node, "On", 100, 200, settings[n], "C_BUTTON_Palette1=#FF0000");
    }
    return strand;
}
} // namespace

TEST_CASE("Promote Node Effects collapses identical node effects in one undo step", "[seq][bulk][undo]") {
    SeqFixture f;
    f.Add(f.layer, "On", 0, 50); // the top layer is in use, so a new one is inserted
    StrandElement* strand = NodesWith(f, { "E_TEXTCTRL_Eff_On_Start=50", "E_TEXTCTRL_Eff_On_Start=50", "E_TEXTCTRL_Eff_On_Start=50" });
    f.Undo().Clear();
    std::string before = f.Describe();
    Recorder rec(f.Exec());
    int promoted = 0;
    {
#ifndef NDEBUG
        GuardCount guard;
#endif
        promoted = PromoteNodeEffects(f.se, f.model);
#ifndef NDEBUG
        CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    }
    CHECK(promoted == 1);
    REQUIRE(f.model->GetEffectLayerCount() == 2);
    CHECK(Times(f.model->GetEffectLayer(0)) == std::vector<std::pair<int, int>>{ { 100, 200 } });
    CHECK(f.model->GetEffectLayer(0)->GetEffect(0)->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "50");
    for (int n = 0; n < strand->GetNodeLayerCount(); ++n) {
        CHECK(strand->GetNodeLayer(n)->GetEffectCount() == 0);
    }
    CHECK(rec.labels == std::vector<std::string>{ "Promote Node Effects" });
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("Promote Node Effects with nothing to promote changes nothing", "[seq][bulk]") {
    SeqFixture f;
    f.Add(f.layer, "On", 0, 50);
    NodesWith(f, { "E_TEXTCTRL_Eff_On_Start=50", "E_TEXTCTRL_Eff_On_Start=60", "E_TEXTCTRL_Eff_On_Start=50" });
    f.Undo().Clear();
    std::string before = f.Describe();
    CHECK(PromoteNodeEffects(f.se, f.model) == 0);
    // no empty layer is left behind and no undo step is recorded
    CHECK(f.Describe() == before);
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("Convert Data To Effects adds through the executor", "[seq][bulk][undo]") {
    SeqFixture f;
    f.Add(f.layer, "On", 0, 50);
    f.Undo().Clear();
    std::string before = f.Describe();
    xlColorVector colors;
    for (int i = 0; i < 10; ++i) {
        colors.push_back(i < 4 ? xlRED : xlBLACK);
    }
    Recorder rec(f.Exec());
    int added = 0;
    {
#ifndef NDEBUG
        GuardCount guard;
#endif
        ScopedCommandBatch batch(f.Exec(), "Convert Data To Effects");
        added = ConvertDataRowToEffects(f.layer, colors, 50, true);
#ifndef NDEBUG
        CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    }
    CHECK(added == 1);
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 0, 200 } });
    CHECK(rec.labels == std::vector<std::string>{ "Convert Data To Effects" });
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
}
