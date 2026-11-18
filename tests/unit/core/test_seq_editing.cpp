#include <catch2/catch_test_macros.hpp>

#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceMutationGuard.h"
#include "render/UndoManager.h"

#include <string>
#include <vector>

// The CommandExecutor edit-site API (edits made from existing UI code) and
// the debug check that edits go through commands.

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
};

struct Recorder : ICommandObserver {
    std::vector<CommandType> types;
    void OnCommandApplied(const SequenceCommand& applied, const SequenceCommand&) override { types.push_back(applied.Type()); }
};

// Restores the guard mode however the test exits.
struct GuardMode {
    SeqMutationGuard::Mode prev;
    explicit GuardMode(SeqMutationGuard::Mode m) : prev(SeqMutationGuard::GetMode()) {
        SeqMutationGuard::SetMode(m);
        SeqMutationGuard::ResetViolationCount();
    }
    ~GuardMode() { SeqMutationGuard::SetMode(prev); }
};
} // namespace

TEST_CASE("Edit-site calls join the caller's current undo step", "[seq][editing][undo]") {
    SeqFixture f;
    f.Undo().CreateUndoStep();
    Effect* a = f.Exec().AddEffect(f.layer, "On", "E_SLIDER_X=1", "", 0, 100, 0, false);
    Effect* b = f.Exec().AddEffect(f.layer, "Bars", "", "", 200, 300, 0, false);
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    CHECK(a->GetSetting("E_SLIDER_X") == "1");
    CHECK(f.layer->GetEffectCount() == 2);
    CHECK(f.Undo().GetUndoString() == "Undo: Effect(s) Added");

    f.Undo().UndoLastStep(); // one step holds both adds
    CHECK(f.layer->GetEffectCount() == 0);
    CHECK_FALSE(f.Undo().CanUndo());
    f.Undo().RedoLastStep();
    CHECK(f.layer->GetEffectCount() == 2);
}

TEST_CASE("Edit-site calls mix with legacy captures in one step", "[seq][editing][undo]") {
    SeqFixture f;
    Effect* e = f.Exec().AddEffect(f.layer, "On", "", "", 0, 100, 0, false, false);
    f.Undo().CreateUndoStep();
    f.Undo().CaptureEffectToBeMoved("Tree", f.layer->GetIndex(), e->GetID(), 0, 100);
    e->SetStartTimeMS(50);
    f.Exec().SetEffectSetting(e, "E_SLIDER_X", std::string("7"));
    f.Undo().UndoLastStep();
    CHECK(e->GetStartTimeMS() == 0);
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_X"));
}

TEST_CASE("Edit-site changes apply leniently", "[seq][editing]") {
    SeqFixture f;
    Effect* a = f.Exec().AddEffect(f.layer, "On", "", "", 0, 100, 0, false);
    // Overlapping and locked edits are what the old direct code allowed.
    Effect* b = f.Exec().AddEffect(f.layer, "Off", "", "", 50, 150, 0, false);
    CHECK(b != nullptr);
    f.Exec().SetEffectLocked(a, true);
    CHECK(a->IsLocked());
    CHECK(f.Exec().MoveEffect(a, 10, 20));
    CHECK(a->GetStartTimeMS() == 10);
    CHECK(a->IsLocked());
    CHECK(f.Exec().DeleteEffect(a));
    CHECK(f.layer->GetEffectCount() == 1);
}

TEST_CASE("Edit-site settings helpers", "[seq][editing]") {
    SeqFixture f;
    Effect* e = f.Exec().AddEffect(f.layer, "On", "E_SLIDER_A=1,X_Effect_Description=keep", "C_BUTTON_Palette1=#FF0000", 0, 100, 0, false);
    f.Undo().CreateUndoStep();

    f.Exec().SetEffectSettings(e, "E_SLIDER_B=2", true);
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_A"));
    CHECK(e->GetSetting("E_SLIDER_B") == "2");
    CHECK(e->GetSetting("X_Effect_Description") == "keep");

    f.Exec().SetEffectSettings(e, "E_SLIDER_C=3", false);
    CHECK_FALSE(e->GetSettings().Contains("X_Effect_Description"));

    f.Exec().SetEffectPalette(e, "C_BUTTON_Palette1=#00FF00,C_CHECKBOX_Palette1=1");
    CHECK(e->GetPalette().size() == 1);
    CHECK(e->GetPalette()[0].Green() == 255);

    f.Exec().SetEffectSetting(e, "E_SLIDER_C", std::nullopt);
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_C"));

    f.Exec().SetEffectRenderDisabled(e, true);
    CHECK(e->IsEffectRenderDisabled());

    f.Undo().UndoLastStep();
    CHECK(e->GetSetting("E_SLIDER_A") == "1");
    CHECK(e->GetSetting("X_Effect_Description") == "keep");
    CHECK(e->GetPaletteMap().Get(std::string("C_BUTTON_Palette1"), std::string()) == "#FF0000");
    CHECK_FALSE(e->IsEffectRenderDisabled());
}

TEST_CASE("EditEffect records what a direct edit changed", "[seq][editing][undo]") {
    SeqFixture f;
    Effect* e = f.Exec().AddEffect(f.layer, "On", "E_SLIDER_A=1", "C_BUTTON_Palette1=#FF0000", 0, 100, 0, false, false);
    Recorder rec;
    f.Exec().AddObserver(&rec);
    f.Undo().CreateUndoStep();

    bool changed = f.Exec().EditEffect(e, [](Effect& ef) {
        ef.GetSettings()["E_SLIDER_A"] = "5";
        ef.GetSettings()["E_SLIDER_NEW"] = "x";
        ef.GetPaletteMap().erase("C_BUTTON_Palette1");
        ef.PaletteMapUpdated();
        ef.SetStartTimeMS(10);
    });
    CHECK(changed);
    CHECK(rec.types == std::vector<CommandType>{ CommandType::MoveEffect, CommandType::ModifyEffect });
    CHECK_FALSE(f.Exec().EditEffect(e, [](Effect&) {}));

    f.Undo().UndoLastStep();
    CHECK(e->GetSetting("E_SLIDER_A") == "1");
    CHECK_FALSE(e->GetSettings().Contains("E_SLIDER_NEW"));
    CHECK(e->GetPaletteMap().Contains("C_BUTTON_Palette1"));
    CHECK(e->GetStartTimeMS() == 0);
    f.Exec().RemoveObserver(&rec);
}

TEST_CASE("Edit-site layer helpers", "[seq][editing][undo]") {
    SeqFixture f;
    EffectLayer* first = f.layer;
    f.Undo().CreateUndoStep();
    EffectLayer* last = f.Exec().InsertLayer(f.model, 99);
    REQUIRE(last != nullptr);
    CHECK(f.model->GetEffectLayer(1) == last);
    EffectLayer* front = f.Exec().InsertLayer(f.model, 0);
    REQUIRE(front != nullptr);
    CHECK(f.model->GetEffectLayer(0) == front);
    CHECK(f.model->GetEffectLayer(1) == first);
    CHECK(f.Exec().SetLayerName(last, "Top"));
    CHECK(last->GetLayerName() == "Top");
    CHECK(f.Exec().RemoveLayer(f.model, 0));
    CHECK(f.model->GetEffectLayerCount() == 2);
    CHECK_FALSE(f.Exec().RemoveLayer(f.model, 5));

    f.Undo().UndoLastStep();
    CHECK(f.model->GetEffectLayerCount() == 1);
    CHECK(f.model->GetEffectLayer(0) == first);
}

TEST_CASE("Lock, description and render-disable edits don't mark the effect dirty", "[seq][editing]") {
    SeqFixture f;
    Effect* e = f.Exec().AddEffect(f.layer, "On", "", "", 0, 100, 0, false, false);
    REQUIRE(e != nullptr);
    int before = f.model->getChangeCount();
    f.Exec().SetEffectLocked(e, true, false);
    f.Exec().SetEffectRenderDisabled(e, true, false);
    f.Exec().SetEffectSetting(e, "X_Effect_Description", std::string("note"), false);
    CHECK(e->IsLocked());
    CHECK(e->IsEffectRenderDisabled());
    CHECK(f.model->getChangeCount() == before);

    f.Exec().SetEffectSetting(e, "E_SLIDER_X", std::string("1"), false);
    CHECK(f.model->getChangeCount() > before);
}

TEST_CASE("FindOpenLayer's new layer is part of the undo step", "[seq][editing][undo]") {
    SeqFixture f;
    f.Exec().AddEffect(f.layer, "On", "", "", 0, 100, 0, false, false);
    f.Undo().CreateUndoStep();
    EffectLayer* open = f.model->FindOpenLayer(50, 150);
    REQUIRE(open != nullptr);
    CHECK(open != f.layer);
    CHECK(f.model->GetEffectLayerCount() == 2);
    CHECK(f.model->FindOpenLayer(200, 300) == f.layer);
    f.Undo().UndoLastStep();
    CHECK(f.model->GetEffectLayerCount() == 1);
}

TEST_CASE("Applying a palette to selected effects goes through commands", "[seq][editing]") {
    SeqFixture f;
    Effect* a = f.Exec().AddEffect(f.layer, "On", "", "C_BUTTON_Palette1=#FF0000", 0, 100, 0, false, false);
    Effect* b = f.Exec().AddEffect(f.layer, "On", "", "C_BUTTON_Palette1=#FF0000", 200, 300, 0, false, false);
    a->SetSelected(EFFECT_SELECTED);
    Recorder rec;
    f.Exec().AddObserver(&rec);
    f.layer->UpdateAllSelectedEffects("C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=1");
    f.Exec().RemoveObserver(&rec);
    CHECK(rec.types == std::vector<CommandType>{ CommandType::ModifyEffect });
    CHECK(a->GetPaletteMap().Get(std::string("C_BUTTON_Palette1"), std::string()) == "#0000FF");
    CHECK(b->GetPaletteMap().Get(std::string("C_BUTTON_Palette1"), std::string()) == "#FF0000");
}

TEST_CASE("Edit-site calls without undo still notify observers", "[seq][editing]") {
    SeqFixture f;
    Recorder rec;
    f.Exec().AddObserver(&rec);
    Effect* e = f.Exec().AddEffect(f.layer, "On", "", "", 0, 100, 0, false, false);
    REQUIRE(e != nullptr);
    CHECK_FALSE(f.Undo().CanUndo());
    CHECK(rec.types.size() == 1);
    f.Exec().RemoveObserver(&rec);
}

TEST_CASE("Inside a batch, unrecorded edits reach observers in order with the rest", "[seq][editing]") {
    SeqFixture f;
    struct BatchRecorder : ICommandObserver {
        std::vector<std::vector<CommandType>> batches;
        void OnCommandApplied(const SequenceCommand& applied, const SequenceCommand&) override {
            std::vector<CommandType> members;
            if (applied.Type() == CommandType::Batch) {
                for (const auto& c : static_cast<const BatchCommand&>(applied).GetCommands()) {
                    members.push_back(c->Type());
                }
            } else {
                members.push_back(applied.Type());
            }
            batches.push_back(members);
        }
    } rec;
    f.Exec().AddObserver(&rec);
    f.Exec().BeginBatch("Paste");
    Effect* e = f.Exec().AddEffect(f.layer, "On", "", "", 0, 100, 0, false);
    f.Exec().SetEffectLocked(e, false, false);
    f.Exec().SetEffectSetting(e, "E_SLIDER_X", std::string("1"), false);
    f.Exec().EndBatch();
    REQUIRE(rec.batches.size() == 1);
    // A no-op change (unlocking an unlocked effect) still applies and is reported.
    CHECK(rec.batches[0] == std::vector<CommandType>{ CommandType::AddEffect, CommandType::ModifyEffect, CommandType::ModifyEffect });

    // Only the add was recorded for undo.
    CHECK(f.Undo().GetUndoString() == "Undo: Paste");
    f.Undo().UndoLastStep();
    CHECK(f.layer->GetEffectCount() == 0);
    f.Exec().RemoveObserver(&rec);
}

#ifndef NDEBUG
TEST_CASE("Mutation guard counts edits made outside commands", "[seq][editing][guard]") {
    SeqFixture f;
    GuardMode mode(SeqMutationGuard::Mode::Count);

    Effect* e = f.Exec().AddEffect(f.layer, "On", "", "", 0, 100, 0, false);
    f.Exec().SetEffectSetting(e, "E_SLIDER_X", std::string("1"));
    f.Exec().EditEffect(e, [](Effect& ef) { ef.SetSetting("E_SLIDER_Y", "2"); });
    f.Undo().UndoLastStep();
    f.Undo().RedoLastStep();
    CHECK(SeqMutationGuard::ViolationCount() == 0);

    e = f.layer->GetEffect(0);
    REQUIRE(e != nullptr);
    e->SetSetting("E_SLIDER_Z", "3");
    f.layer->AddEffect(0, "Off", "", "", 200, 300, 0, false);
    f.model->AddEffectLayer();
    CHECK(SeqMutationGuard::ViolationCount() == 3);

    {
        SeqMutationGuard::Scope exempt;
        e->SetSetting("E_SLIDER_Z", "4");
    }
    CHECK(SeqMutationGuard::ViolationCount() == 3);
}

TEST_CASE("Mutation guard counts sequence media edits made outside commands", "[seq][editing][guard]") {
    SeqFixture f;
    GuardMode mode(SeqMutationGuard::Mode::Count);
    auto& media = f.se.GetSequenceMedia();

    xlImage img(2, 2);
    media.AddEmbeddedImage("images/a.png", img);
    CHECK(SeqMutationGuard::ViolationCount() == 1);
    media.RenameMedia("images/a.png", "images/b.png");
    media.RemoveMedia("images/b.png");
    CHECK(SeqMutationGuard::ViolationCount() == 3);

    // Clearing the document (close / reload) is not an edit.
    media.Clear();
    CHECK(SeqMutationGuard::ViolationCount() == 3);
}
#endif
