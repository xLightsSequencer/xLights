#include <catch2/catch_test_macros.hpp>

#include "effects/EffectManager.h"
#include "lyrics/LyricBreakdown.h"
#include "lyrics/PhonemeDictionary.h"
#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceMutationGuard.h"
#include "render/UndoManager.h"
#include "support/TempDir.h"
#include "utils/UtilClasses.h"

#include <string>
#include <utility>
#include <vector>

// The bulk EffectLayer edits and the lyric breakdowns go through the
// CommandExecutor edit-site API: observers see every change and undo still
// restores it.

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
        // The move helpers only record undo while capture is on, as in the app.
        se.get_undo_mgr().SetCaptureUndo(true);
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
    explicit Recorder(CommandExecutor& e) : exec(e) { exec.AddObserver(this); }
    ~Recorder() override { exec.RemoveObserver(this); }
    void OnCommandApplied(const SequenceCommand& applied, const SequenceCommand&) override { types.push_back(applied.Type()); }
    size_t Count(CommandType t) const {
        size_t n = 0;
        for (auto x : types) {
            n += x == t ? 1 : 0;
        }
        return n;
    }
};

std::vector<std::pair<int, int>> Times(EffectLayer* l) {
    std::vector<std::pair<int, int>> out;
    for (int i = 0; i < l->GetEffectCount(); ++i) {
        out.emplace_back(l->GetEffect(i)->GetStartTimeMS(), l->GetEffect(i)->GetEndTimeMS());
    }
    return out;
}

void SelectTagged(Effect* e, int sel) {
    e->SetSelected(sel);
    e->SetTagged(true);
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

TEST_CASE("MoveAllSelectedEffects moves through commands", "[seq][editsites]") {
    SeqFixture f;
    Effect* a = f.Add("On", 100, 200);
    Effect* b = f.Add("On", 300, 400);
    Effect* c = f.Add("On", 500, 600);
    Effect* d = f.Add("On", 700, 800);
    SelectTagged(a, EFFECT_LT_SELECTED);
    SelectTagged(b, EFFECT_RT_SELECTED);
    SelectTagged(c, EFFECT_SELECTED);
    Recorder rec(f.Exec());
    f.Undo().CreateUndoStep();
#ifndef NDEBUG
    GuardCount guard;
#endif
    f.layer->MoveAllSelectedEffects(10, f.Undo());
#ifndef NDEBUG
    CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 110, 200 }, { 300, 410 }, { 510, 610 }, { 700, 800 } });
    CHECK(rec.types == std::vector<CommandType>(3, CommandType::MoveEffect));
    CHECK_FALSE(a->GetTagged());
    CHECK_FALSE(d->GetTagged());

    f.Undo().UndoLastStep();
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 100, 200 }, { 300, 400 }, { 500, 600 }, { 700, 800 } });
}

TEST_CASE("MoveAllSelectedEffects without undo capture still notifies", "[seq][editsites]") {
    SeqFixture f;
    Effect* a = f.Add("On", 100, 200);
    SelectTagged(a, EFFECT_SELECTED);
    Recorder rec(f.Exec());
    f.Undo().SetCaptureUndo(false);
    f.layer->MoveAllSelectedEffects(10, f.Undo());
    f.Undo().SetCaptureUndo(true);
    CHECK(a->GetStartTimeMS() == 110);
    CHECK(rec.types.size() == 1);
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("StretchAllSelectedEffects moves through commands", "[seq][editsites]") {
    SeqFixture f;
    Effect* a = f.Add("On", 100, 200);
    Effect* b = f.Add("On", 300, 400);
    Effect* c = f.Add("On", 500, 600);
    SelectTagged(a, EFFECT_LT_SELECTED); // stretch right doesn't move a left edge
    SelectTagged(b, EFFECT_RT_SELECTED);
    SelectTagged(c, EFFECT_SELECTED);
    Recorder rec(f.Exec());
    f.Undo().CreateUndoStep();
    f.layer->StretchAllSelectedEffects(20, f.Undo());
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 100, 200 }, { 300, 420 }, { 500, 620 } });
    CHECK(rec.types == std::vector<CommandType>(2, CommandType::MoveEffect));

    f.Undo().UndoLastStep();
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 100, 200 }, { 300, 400 }, { 500, 600 } });
}

TEST_CASE("ButtUpStretchAllSelectedEffects moves through commands", "[seq][editsites]") {
    SeqFixture f;
    f.Add("On", 0, 100);
    Effect* b = f.Add("On", 300, 400);
    f.Add("On", 700, 800);
    SelectTagged(b, EFFECT_SELECTED);
    Recorder rec(f.Exec());
    f.Undo().CreateUndoStep();
    f.layer->ButtUpStretchAllSelectedEffects(false, 5000, f.Undo());
    CHECK(b->GetStartTimeMS() == 100);
    SelectTagged(b, EFFECT_SELECTED);
    f.layer->ButtUpStretchAllSelectedEffects(true, 5000, f.Undo());
    CHECK(b->GetEndTimeMS() == 700);
    CHECK(rec.types == std::vector<CommandType>(2, CommandType::MoveEffect));

    f.Undo().UndoLastStep();
    CHECK(b->GetStartTimeMS() == 300);
    CHECK(b->GetEndTimeMS() == 400);
}

TEST_CASE("DeleteSelectedEffects deletes through commands", "[seq][editsites]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100);
    Effect* b = f.Add("Bars", 200, 300);
    Effect* c = f.Add("On", 400, 500);
    Effect* d = f.Add("On", 600, 700);
    a->SetSelected(EFFECT_SELECTED);
    c->SetSelected(EFFECT_SELECTED);
    d->SetSelected(EFFECT_SELECTED);
    f.Exec().SetEffectLocked(d, true, false);
    Recorder rec(f.Exec());
    f.Undo().CreateUndoStep();
#ifndef NDEBUG
    GuardCount guard;
#endif
    f.layer->DeleteSelectedEffects(f.Undo());
#ifndef NDEBUG
    CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    REQUIRE(f.layer->GetEffectCount() == 2);
    CHECK(f.layer->GetEffect(0) == b);
    CHECK(f.layer->GetEffect(1) == d);
    CHECK(b->GetID() == 0);
    CHECK(d->GetID() == 1);
    CHECK(rec.types == std::vector<CommandType>(2, CommandType::DeleteEffect));

    f.Undo().UndoLastStep();
    CHECK(Times(f.layer) == std::vector<std::pair<int, int>>{ { 0, 100 }, { 200, 300 }, { 400, 500 }, { 600, 700 } });
}

TEST_CASE("RemoveAllEffects deletes through commands", "[seq][editsites]") {
    SeqFixture f;
    f.Add("On", 0, 100);
    Effect* b = f.Add("On", 200, 300);
    f.Add("On", 400, 500);
    f.Exec().SetEffectLocked(b, true, false);

    SECTION("with undo") {
        Recorder rec(f.Exec());
        f.Undo().CreateUndoStep();
        f.layer->RemoveAllEffects(&f.Undo());
        REQUIRE(f.layer->GetEffectCount() == 1);
        CHECK(f.layer->GetEffect(0) == b);
        CHECK(b->GetID() == 0);
        CHECK(rec.Count(CommandType::DeleteEffect) == 2);
        f.Undo().UndoLastStep();
        CHECK(f.layer->GetEffectCount() == 3);
    }
    SECTION("without undo") {
        Recorder rec(f.Exec());
        f.layer->RemoveAllEffects(nullptr);
        CHECK(f.layer->GetEffectCount() == 1);
        CHECK(rec.Count(CommandType::DeleteEffect) == 2);
        CHECK_FALSE(f.Undo().CanUndo());
    }
}

TEST_CASE("ConvertEffectsToPerModel modifies through commands", "[seq][editsites]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100, "B_CHOICE_BufferStyle=Default");
    Effect* b = f.Add("On", 100, 200);
    Effect* c = f.Add("On", 200, 300, "B_CHOICE_BufferStyle=Horizontal Stack");
    Recorder rec(f.Exec());
    f.Undo().CreateUndoStep();
#ifndef NDEBUG
    GuardCount guard;
#endif
    f.layer->ConvertEffectsToPerModel(f.Undo());
#ifndef NDEBUG
    CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    CHECK(a->GetSetting("B_CHOICE_BufferStyle") == "Per Model Default");
    CHECK(b->GetSetting("B_CHOICE_BufferStyle") == "Per Model Default");
    CHECK(c->GetSetting("B_CHOICE_BufferStyle") == "Horizontal Stack");
    CHECK(rec.types == std::vector<CommandType>(2, CommandType::ModifyEffect));

    f.Undo().UndoLastStep();
    CHECK(a->GetSetting("B_CHOICE_BufferStyle") == "Default");
    CHECK_FALSE(b->GetSettings().Contains("B_CHOICE_BufferStyle"));
}

TEST_CASE("Layer ReplaceColours modifies through commands", "[seq][editsites]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100, "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1");
    Effect* b = f.Add("On", 200, 300, "", "C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=1");
    Recorder rec(f.Exec());
    f.Undo().CreateUndoStep();
    CHECK(f.layer->ReplaceColours(nullptr, "#FF0000", "#00FF00", false, f.Undo()) == 1);
    CHECK(a->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#00FF00");
    CHECK(b->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#0000FF");
    CHECK(rec.types == std::vector<CommandType>{ CommandType::ModifyEffect });

    f.Undo().UndoLastStep();
    CHECK(a->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#FF0000");
    CHECK(a->GetPalette().size() == 1);
}

TEST_CASE("Apply-to-selected helpers modify through commands", "[seq][editsites]") {
    SeqFixture f;
    Effect* a = f.Add("On", 0, 100);
    Effect* b = f.Add("On", 200, 300);
    a->SetSelected(EFFECT_SELECTED);
    b->SetSelected(EFFECT_SELECTED);
    auto& em = f.se.GetEffectManager();
    Recorder rec(f.Exec());
    RangeAccumulator r;
    f.Undo().CreateUndoStep();
#ifndef NDEBUG
    GuardCount guard;
#endif
    f.layer->ApplyEffectSettingToSelected(nullptr, f.Undo(), "On", "E_TEXTCTRL_Eff_On_Start", "42", nullptr, "E_VALUECURVE_Eff_On_Start", em, r);
    CHECK(rec.Count(CommandType::ModifyEffect) == 2);
    f.layer->ApplyCallbackToSelected(nullptr, f.Undo(), "On", [&](Effect* e) {
        if (e == a) {
            e->SetSetting("E_X", "1");
            return true;
        }
        return false;
    }, em, r);
    CHECK(rec.Count(CommandType::ModifyEffect) == 3);
#ifndef NDEBUG
    CHECK(SeqMutationGuard::ViolationCount() == 0);
#endif
    CHECK(a->GetSetting("E_TEXTCTRL_Eff_On_Start") == "42");
    CHECK(a->GetSetting("E_X") == "1");

    f.Undo().UndoLastStep();
    CHECK(a->GetSetting("E_TEXTCTRL_Eff_On_Start").empty());
    CHECK(b->GetSetting("E_TEXTCTRL_Eff_On_Start").empty());
    CHECK(a->GetSetting("E_X").empty());
}

TEST_CASE("BreakdownPhrase and BreakdownWord add through commands", "[seq][editsites]") {
    HeadlessRenderContext ctx;
    auto& se = ctx.GetSequenceElements();
    se.SetViewsManager(&ctx._sequenceViewManager);
    Element* t = nullptr;
    EffectLayer* words = nullptr;
    EffectLayer* phonemes = nullptr;
    {
        SeqMutationGuard::Scope setup;
        t = se.AddElement("Lyrics", "timing", true, false, true, false, false);
        words = t->AddEffectLayer();
        phonemes = t->AddEffectLayer();
    }

    TempDir dir;
    dir.Write("standard_dictionary", "HELLO HH AH0 L OW1\nWORLD W ER1 L D\n");
    dir.Write("extended_dictionary", "");
    dir.Write("user_dictionary", "");
    dir.Write("phoneme_mapping", ". AI\n. E\n. O\n. etc\n. MBP\nHH etc\nAH0 AI\nL L\nOW1 O\nW O\nER1 E\nD etc\n");
    PhonemeDictionary dict;
    dict.LoadDictionaries({ dir.path.string() });

    Recorder rec(se.GetCommandExecutor());
    se.get_undo_mgr().CreateUndoStep();
    BreakdownPhrase(words, 0, 2000, "hello world", 20, se.get_undo_mgr());
    CHECK(words->GetEffectCount() == 2);
    CHECK(rec.Count(CommandType::AddEffect) == 2);
    BreakdownWord(phonemes, 0, 1000, "hello", 20, dict, se.get_undo_mgr());
    CHECK(phonemes->GetEffectCount() > 0);
    CHECK(rec.Count(CommandType::AddEffect) == 2 + (size_t)phonemes->GetEffectCount());

    se.get_undo_mgr().UndoLastStep();
    CHECK(words->GetEffectCount() == 0);
    CHECK(phonemes->GetEffectCount() == 0);
}
