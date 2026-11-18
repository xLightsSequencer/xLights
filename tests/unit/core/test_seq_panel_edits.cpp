#include <catch2/catch_test_macros.hpp>

#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectEditCoalescer.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceMutationGuard.h"
#include "render/UndoManager.h"

#include <string>
#include <utility>
#include <vector>

// Settings-panel edits (EffectEditCoalescer), live previews of drags and
// nudges, and command batches.

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
    Effect* Add(const std::string& name, int start, int end, const std::string& settings = "", const std::string& palette = "") {
        return Exec().AddEffect(layer, name, settings, palette, start, end, 0, false, false);
    }
};

struct Recorder : ICommandObserver {
    CommandExecutor& exec;
    std::vector<CommandType> types;
    std::vector<std::string> labels;
    std::vector<size_t> batchSizes;
    explicit Recorder(CommandExecutor& e) : exec(e) { exec.AddObserver(this); }
    ~Recorder() override { exec.RemoveObserver(this); }
    void OnCommandApplied(const SequenceCommand& applied, const SequenceCommand&) override {
        types.push_back(applied.Type());
        if (applied.Type() == CommandType::Batch) {
            const auto& b = static_cast<const BatchCommand&>(applied);
            labels.push_back(b.GetLabel());
            batchSizes.push_back(b.GetCommands().size());
        }
    }
};

SettingsChange Keys(KeyChanges k) {
    return SettingsChange{ false, std::move(k) };
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

// ---------------------------------------------------------------------------
// UndoManager history serial
// ---------------------------------------------------------------------------

TEST_CASE("Undo history serial moves on every history change", "[seq][undo]") {
    SeqFixture f;
    Effect* e = f.Add("On", 0, 100);
    uint64_t s = f.Undo().GetHistorySerial();
    f.Undo().CreateUndoStep();
    CHECK(f.Undo().GetHistorySerial() != s);
    s = f.Undo().GetHistorySerial();
    f.Exec().SetEffectSetting(e, "E_SLIDER_A", std::string("1"));
    CHECK(f.Undo().GetHistorySerial() != s);
    s = f.Undo().GetHistorySerial();
    f.Exec().SetEffectSetting(e, "E_SLIDER_A", std::string("2"), false);
    CHECK(f.Undo().GetHistorySerial() == s); // nothing recorded
    f.Undo().UndoLastStep();
    CHECK(f.Undo().GetHistorySerial() != s);
    s = f.Undo().GetHistorySerial();
    f.Undo().RedoLastStep();
    CHECK(f.Undo().GetHistorySerial() != s);
    s = f.Undo().GetHistorySerial();
    f.Undo().Clear();
    CHECK(f.Undo().GetHistorySerial() != s);
}

// ---------------------------------------------------------------------------
// EffectEditCoalescer
// ---------------------------------------------------------------------------

TEST_CASE("A burst of panel edits is one undo step that restores the whole effect", "[seq][panel][undo]") {
    SeqFixture f;
    Effect* e = f.Add("On", 0, 1000, "E_TEXTCTRL_Eff_On_Start=100,E_SLIDER_Gone=5,X_Effect_Locked=False", "C_BUTTON_Palette1=#FF0000");
    REQUIRE(e != nullptr);
    const std::string settings0 = e->GetSettingsAsString();
    const std::string palette0 = e->GetPaletteAsString();
    Recorder rec(f.Exec());
    EffectEditCoalescer co;

    // The panel's full state each time, as the desktop timer sends it: a key
    // added, a key dropped, a palette change, then the effect type.
    CHECK(co.Apply(f.se, e, std::nullopt, EffectEditCoalescer::ReplaceSettingsKeepingX(*e, "E_TEXTCTRL_Eff_On_Start=50,E_SLIDER_New=1"),
                   SettingsChange::ReplaceWith("C_BUTTON_Palette1=#00FF00"), 1000));
    CHECK(co.Apply(f.se, e, std::nullopt, EffectEditCoalescer::ReplaceSettingsKeepingX(*e, "E_TEXTCTRL_Eff_On_Start=60,E_SLIDER_New=2"),
                   SettingsChange::ReplaceWith("C_BUTTON_Palette1=#0000FF,C_BUTTON_Palette2=#FFFFFF"), 1400));
    CHECK(co.Apply(f.se, e, std::string("Off"), EffectEditCoalescer::ReplaceSettingsKeepingX(*e, "E_SLIDER_New=3"), SettingsChange{}, 1800));
    // No change: no step, burst state untouched.
    CHECK_FALSE(co.Apply(f.se, e, std::string("Off"), SettingsChange{}, SettingsChange{}, 1900));

    CHECK(e->GetEffectName() == "Off");
    CHECK(e->GetSetting("E_SLIDER_New") == "3");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_Gone"));
    CHECK(e->GetSetting("X_Effect_Locked") == "False"); // X_ keys survive panel edits
    CHECK(rec.types == std::vector<CommandType>(3, CommandType::ModifyEffect));
    const std::string settings1 = e->GetSettingsAsString();
    const std::string palette1 = e->GetPaletteAsString();

    f.Undo().UndoLastStep();
    CHECK(e->GetEffectName() == "On");
    CHECK(e->GetSettingsAsString() == settings0);
    CHECK(e->GetPaletteAsString() == palette0);
    CHECK_FALSE(f.Undo().CanUndo()); // the whole burst was one step

    f.Undo().RedoLastStep();
    CHECK(e->GetEffectName() == "Off");
    CHECK(e->GetSettingsAsString() == settings1);
    CHECK(e->GetPaletteAsString() == palette1);
}

TEST_CASE("Panel edit bursts end on idle, other undo activity, another effect or EndBurst", "[seq][panel][undo]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 1000, "E_SLIDER_A=0");
    Effect* b = f.Add("On", 1000, 2000, "E_SLIDER_A=0");
    EffectEditCoalescer co(1000);
    auto set = [&](Effect* e, const std::string& v, int64_t t) {
        return co.Apply(f.se, e, std::nullopt, Keys({ { "E_SLIDER_A", v } }), SettingsChange{}, t);
    };
    auto undoAll = [&]() {
        int steps = 0;
        while (f.Undo().CanUndo()) {
            f.Undo().UndoLastStep();
            ++steps;
        }
        return steps;
    };

    SECTION("idle gap") {
        set(a, "1", 0);
        set(a, "2", 1000); // exactly at the window: same burst
        set(a, "3", 2001); // past it: new burst
        f.Undo().UndoLastStep();
        CHECK(a->GetSetting("E_SLIDER_A") == "2");
        CHECK(undoAll() == 1);
        CHECK(a->GetSetting("E_SLIDER_A") == "0");
    }
    SECTION("another step recorded in between") {
        set(a, "1", 0);
        f.Undo().CreateUndoStep();
        f.Exec().MoveEffect(b, 1000, 1500);
        set(a, "2", 10);
        f.Undo().UndoLastStep();
        CHECK(a->GetSetting("E_SLIDER_A") == "1");
        CHECK(b->GetEndTimeMS() == 1500);
        CHECK(undoAll() == 2);
        CHECK(a->GetSetting("E_SLIDER_A") == "0");
        CHECK(b->GetEndTimeMS() == 2000);
    }
    SECTION("undo in between") {
        set(a, "1", 0);
        f.Undo().UndoLastStep();
        set(a, "2", 10);
        CHECK(f.Undo().CanUndo());
        f.Undo().UndoLastStep();
        CHECK(a->GetSetting("E_SLIDER_A") == "0");
    }
    SECTION("another effect") {
        set(a, "1", 0);
        set(b, "1", 10);
        f.Undo().UndoLastStep();
        CHECK(a->GetSetting("E_SLIDER_A") == "1");
        CHECK(b->GetSetting("E_SLIDER_A") == "0");
        CHECK(undoAll() == 1);
    }
    SECTION("EndBurst") {
        set(a, "1", 0);
        co.EndBurst();
        set(a, "2", 10);
        f.Undo().UndoLastStep();
        CHECK(a->GetSetting("E_SLIDER_A") == "1");
        CHECK(undoAll() == 1);
    }
    SECTION("unrecorded edits don't end a burst") {
        set(a, "1", 0);
        f.Exec().SetEffectSetting(b, "E_SLIDER_A", std::string("9"), false);
        set(a, "2", 10);
        CHECK(undoAll() == 1);
        CHECK(a->GetSetting("E_SLIDER_A") == "0");
    }
}

TEST_CASE("Panel edits change the document only through commands", "[seq][panel][guard]") {
    SeqFixture f;
    Effect* e = f.Add("On", 0, 1000, "E_SLIDER_A=0");
#ifndef NDEBUG
    GuardCount guard;
#endif
    EffectEditCoalescer co;
    co.Apply(f.se, e, std::string("Off"), EffectEditCoalescer::ReplaceSettingsKeepingX(*e, "E_SLIDER_B=1"), SettingsChange::ReplaceWith("C_BUTTON_Palette1=#123456"), 0);
    f.Undo().UndoLastStep();
    f.Undo().RedoLastStep();
#ifndef NDEBUG
    CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    CHECK(e->GetEffectName() == "Off");
}

// ---------------------------------------------------------------------------
// Live preview
// ---------------------------------------------------------------------------

TEST_CASE("A live preview records its net change as one step on commit", "[seq][preview][undo]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100);
    Effect* b = f.Add("On", 200, 300);
    Recorder rec(f.Exec());
    f.Undo().CreateUndoStep(); // the grid's mouse-down step

    f.Exec().BeginPreview();
    CHECK(f.Exec().InPreview());
    for (int i = 1; i <= 5; ++i) {
        f.Exec().MoveEffect(a, 10 * i, 100 + 10 * i);
        f.Exec().MoveEffect(b, 200, 300 + 10 * i, i == 1); // recordUndo is moot in a preview
    }
    CHECK(rec.types.empty());
    CHECK_FALSE(f.Undo().ChangeCaptured());
    CHECK(f.Exec().CommitPreview(""));
    CHECK_FALSE(f.Exec().InPreview());

    CHECK(rec.types == std::vector<CommandType>{ CommandType::Batch });
    CHECK(rec.labels == std::vector<std::string>{ "Effect(s) Moved" });
    CHECK(rec.batchSizes == std::vector<size_t>{ 2 });
    CHECK(f.Undo().GetUndoString() == "Undo: Effect(s) Moved");

    f.Undo().UndoLastStep();
    CHECK(a->GetStartTimeMS() == 0);
    CHECK(a->GetEndTimeMS() == 100);
    CHECK(b->GetEndTimeMS() == 300);
    CHECK_FALSE(f.Undo().CanUndo());
    f.Undo().RedoLastStep();
    CHECK(a->GetStartTimeMS() == 50);
    CHECK(b->GetEndTimeMS() == 350);
}

TEST_CASE("A preview that changes settings is labelled a modification", "[seq][preview][undo]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 1000, "T_TEXTCTRL_Fadein=0.00");
    Recorder rec(f.Exec());
    f.Exec().BeginPreview();
    f.Exec().SetEffectSetting(a, "T_TEXTCTRL_Fadein", std::string("0.50"));
    f.Exec().SetEffectSetting(a, "T_TEXTCTRL_Fadein", std::string("0.75"));
    f.Exec().EditEffect(a, [](Effect& e) { e.SetSetting("E_SLIDER_New", "1"); });
    CHECK(f.Exec().CommitPreview(""));
    CHECK(rec.labels == std::vector<std::string>{ "Effect(s) Modified" });
    CHECK(rec.batchSizes == std::vector<size_t>{ 1 });
    f.Undo().UndoLastStep();
    CHECK(a->GetSetting("T_TEXTCTRL_Fadein") == "0.00");
    CHECK_FALSE(a->GetSettings().Contains("E_SLIDER_New"));
}

TEST_CASE("A preview that ends where it began records nothing", "[seq][preview][undo]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100);
    Recorder rec(f.Exec());
    f.Exec().BeginPreview();
    f.Exec().MoveEffect(a, 10, 110);
    f.Exec().MoveEffect(a, 0, 100);
    CHECK_FALSE(f.Exec().CommitPreview(""));
    CHECK(rec.types.empty());
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("Cancelling a preview puts everything back silently", "[seq][preview]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100, "E_SLIDER_A=1");
    Recorder rec(f.Exec());
    const std::string before = DescribeSequence(f.se);
    f.Exec().BeginPreview();
    f.Exec().MoveEffect(a, 50, 150);
    f.Exec().SetEffectSetting(a, "E_SLIDER_A", std::nullopt);
    f.Exec().AddEffect(f.layer, "Off", "", "", 500, 600, 0, false);
    f.Exec().CancelPreview();
    CHECK(DescribeSequence(f.se) == before);
    CHECK(rec.types.empty());
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("Adds during a preview go first in the committed batch", "[seq][preview][undo]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100);
    const std::string before = DescribeSequence(f.se);
    Recorder rec(f.Exec());
    f.Exec().BeginPreview();
    f.Exec().MoveEffect(a, 0, 150);
    Effect* added = f.Exec().AddEffect(f.layer, "Off", "", "", 500, 600, 0, false);
    REQUIRE(added != nullptr);
    f.Exec().MoveEffect(added, 500, 700);
    CHECK(f.Exec().CommitPreview("Drop"));
    CHECK(rec.labels == std::vector<std::string>{ "Drop" });
    // AddEffect, then the net moves of both effects.
    CHECK(rec.batchSizes == std::vector<size_t>{ 3 });
    const std::string after = DescribeSequence(f.se);

    f.Undo().UndoLastStep();
    CHECK(DescribeSequence(f.se) == before);
    f.Undo().RedoLastStep();
    CHECK(DescribeSequence(f.se) == after);
}

TEST_CASE("Previews nest and join an open batch", "[seq][preview]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100);
    Effect* b = f.Add("On", 200, 300);
    Recorder rec(f.Exec());

    f.Exec().BeginPreview();
    f.Exec().BeginPreview();
    f.Exec().MoveEffect(a, 10, 110);
    CHECK_FALSE(f.Exec().CommitPreview(""));
    CHECK(f.Exec().InPreview());
    CHECK(rec.types.empty());
    CHECK(f.Exec().CommitPreview(""));
    CHECK(rec.types == std::vector<CommandType>{ CommandType::Batch });

    rec.types.clear();
    rec.batchSizes.clear();
    {
        ScopedCommandBatch batch(f.Exec(), "Outer");
        f.Exec().BeginPreview();
        f.Exec().MoveEffect(b, 210, 310);
        f.Exec().CommitPreview("");
        f.Exec().SetEffectSetting(b, "E_SLIDER_A", std::string("1"));
        CHECK(rec.types.empty());
    }
    CHECK(rec.types == std::vector<CommandType>{ CommandType::Batch });
    CHECK(rec.labels.back() == "Outer");
    CHECK(rec.batchSizes == std::vector<size_t>{ 2 });
    f.Undo().UndoLastStep();
    CHECK(b->GetStartTimeMS() == 200);
    CHECK_FALSE(b->GetSettings().Contains("E_SLIDER_A"));
}

TEST_CASE("Layer move helpers inside a preview record one step", "[seq][preview][undo]") {
    SeqFixture f;
    Effect* a = f.Add("On", 100, 200);
    Effect* b = f.Add("On", 300, 400);
    Recorder rec(f.Exec());
    f.Undo().SetCaptureUndo(true);
    f.Undo().CreateUndoStep();
#ifndef NDEBUG
    GuardCount guard;
#endif
    f.Exec().BeginPreview();
    for (int i = 0; i < 4; ++i) {
        a->SetSelected(EFFECT_SELECTED);
        a->SetTagged(true);
        b->SetSelected(EFFECT_SELECTED);
        b->SetTagged(true);
        f.layer->MoveAllSelectedEffects(10, f.Undo());
    }
    f.Exec().CommitPreview("");
#ifndef NDEBUG
    CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    CHECK(rec.types == std::vector<CommandType>{ CommandType::Batch });
    CHECK(rec.batchSizes == std::vector<size_t>{ 2 });
    f.Undo().UndoLastStep();
    CHECK(a->GetStartTimeMS() == 100);
    CHECK(b->GetStartTimeMS() == 300);
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("A scoped command batch is one step and one notification", "[seq][editing][undo]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100);
    Recorder rec(f.Exec());
    f.Undo().CreateUndoStep();
    {
        ScopedCommandBatch batch(f.Exec(), "Effect(s) Added");
        f.Exec().AddEffect(f.layer, "Off", "", "", 200, 300, 0, false);
        f.Exec().SetEffectSetting(a, "E_SLIDER_A", std::string("1"));
    }
    CHECK(rec.types == std::vector<CommandType>{ CommandType::Batch });
    CHECK(f.Undo().GetUndoString() == "Undo: Effect(s) Added");
    f.Undo().UndoLastStep();
    CHECK(f.layer->GetEffectCount() == 1);
    CHECK_FALSE(a->GetSettings().Contains("E_SLIDER_A"));
}
