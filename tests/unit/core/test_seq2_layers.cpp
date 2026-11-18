#include <catch2/catch_test_macros.hpp>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/UndoManager.h"
#include "render/ValueCurve.h"
#include "effects/EffectManager.h"
#include "support/TempDir.h"
#include "utils/UtilClasses.h"

#include <filesystem>
#include <list>
#include <string>
#include <vector>

namespace {
struct SeqFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    SeqFixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
    }
    ModelElement* Model(const std::string& name = "Tree") {
        return dynamic_cast<ModelElement*>(se.AddElement(name, "model", true, false, true, false, false));
    }
    TimingElement* Timing(const std::string& name = "Beats") {
        return dynamic_cast<TimingElement*>(se.AddElement(name, "timing", true, false, true, false, false));
    }
    UndoManager& Undo() { return se.get_undo_mgr(); }
};

Effect* Add(EffectLayer* l, const std::string& name, int start, int end, const std::string& settings = "", const std::string& palette = "") {
    return l->AddEffect(0, name, settings, palette, start, end, 0, false);
}

std::vector<std::pair<int, int>> Ranges(RangeAccumulator& r) {
    return { r.begin(), r.end() };
}
} // namespace

// ---------------------------------------------------------------------------------------------
// EffectLayer
// ---------------------------------------------------------------------------------------------

TEST_CASE("EffectLayer ConvertEffectsToPerModel rewrites whole-group buffer styles", "[seq2][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100, "B_CHOICE_BufferStyle=Default");
    Add(l, "On", 100, 200, "B_CHOICE_BufferStyle=Per Preview");
    Add(l, "On", 200, 300, "B_CHOICE_BufferStyle=Single Line");
    Add(l, "On", 300, 400);
    Add(l, "On", 400, 500, "B_CHOICE_BufferStyle=Horizontal Stack");
    f.Undo().CreateUndoStep();
    l->ConvertEffectsToPerModel(f.Undo());
    CHECK(l->GetEffect(0)->GetSetting("B_CHOICE_BufferStyle") == "Per Model Default");
    CHECK(l->GetEffect(1)->GetSetting("B_CHOICE_BufferStyle") == "Per Model Per Preview");
    CHECK(l->GetEffect(2)->GetSetting("B_CHOICE_BufferStyle") == "Per Model Single Line");
    CHECK(l->GetEffect(3)->GetSetting("B_CHOICE_BufferStyle") == "Per Model Default");
    CHECK(l->GetEffect(4)->GetSetting("B_CHOICE_BufferStyle") == "Horizontal Stack");
    REQUIRE(f.Undo().CanUndo());
    f.Undo().UndoLastStep();
    CHECK(l->GetEffect(0)->GetSetting("B_CHOICE_BufferStyle") == "Default");
}

TEST_CASE("EffectLayer movement range with edge selections", "[seq2][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 100, 200);
    Effect* b = Add(l, "Off", 300, 500);
    Effect* c = Add(l, "Bars", 700, 800);
    int toLeft = 0, toRight = 0;

    // nothing selected: unbounded
    l->GetMaximumRangeOfMovementForSelectedEffects(toLeft, toRight);
    CHECK(toLeft == NO_MAX);
    CHECK(toRight == NO_MAX);

    // dragging the left edge of b: may move back to a's end, forward until min period
    b->SetSelected(EFFECT_LT_SELECTED);
    l->GetMaximumRangeOfMovementForSelectedEffects(toLeft, toRight);
    CHECK(toLeft == 100);
    CHECK(toRight == 200 - f.se.GetMinPeriod());

    // dragging the right edge of b
    b->SetSelected(EFFECT_RT_SELECTED);
    l->GetMaximumRangeOfMovementForSelectedEffects(toLeft, toRight);
    CHECK(toLeft == 200 - f.se.GetMinPeriod());
    CHECK(toRight == 200);

    // a's right edge and b's left edge dragged together
    a->SetSelected(EFFECT_RT_SELECTED);
    b->SetSelected(EFFECT_LT_SELECTED);
    l->GetMaximumRangeOfMovementForSelectedEffects(toLeft, toRight);
    CHECK(toLeft == 100 - f.se.GetMinPeriod());
    CHECK(toRight == 200 - f.se.GetMinPeriod());

    // b's right edge and c's left edge dragged together
    a->SetSelected(EFFECT_NOT_SELECTED);
    b->SetSelected(EFFECT_RT_SELECTED);
    c->SetSelected(EFFECT_LT_SELECTED);
    l->GetMaximumRangeOfMovementForSelectedEffects(toLeft, toRight);
    CHECK(toLeft == 200 - f.se.GetMinPeriod());
    CHECK(toRight == 100 - f.se.GetMinPeriod());

    // everything selected: only the start of the first effect bounds the move
    a->SetSelected(EFFECT_SELECTED);
    b->SetSelected(EFFECT_SELECTED);
    c->SetSelected(EFFECT_SELECTED);
    l->GetMaximumRangeOfMovementForSelectedEffects(toLeft, toRight);
    CHECK(toLeft == 100);
    CHECK(toRight == NO_MAX);
}

TEST_CASE("EffectLayer edge moves and stretches capture undo", "[seq2][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 100, 200);
    Effect* b = Add(l, "Off", 300, 400);
    f.Undo().CreateUndoStep();
    f.Undo().SetCaptureUndo(true);

    a->SetSelected(EFFECT_LT_SELECTED);
    b->SetSelected(EFFECT_RT_SELECTED);
    l->TagAllSelectedEffects();
    CHECK(l->GetTaggedEffectCount() == 2);
    l->MoveAllSelectedEffects(-20, f.Undo());
    CHECK(a->GetStartTimeMS() == 80);
    CHECK(a->GetEndTimeMS() == 200);
    CHECK(b->GetEndTimeMS() == 380);
    CHECK(l->GetTaggedEffectCount() == 0);

    // stretch: left edge only moves left, right edge only moves right
    l->TagAllSelectedEffects();
    l->StretchAllSelectedEffects(30, f.Undo());
    CHECK(a->GetStartTimeMS() == 80);
    CHECK(b->GetEndTimeMS() == 410);
    l->TagAllSelectedEffects();
    l->StretchAllSelectedEffects(-30, f.Undo());
    CHECK(a->GetStartTimeMS() == 50);
    CHECK(b->GetEndTimeMS() == 410);

    l->TagAllSelectedEffects();
    l->UnTagAllEffects();
    CHECK(l->GetTaggedEffectCount() == 0);

    REQUIRE(f.Undo().CanUndo());
    f.Undo().UndoLastStep();
    CHECK(a->GetStartTimeMS() == 100);
    CHECK(b->GetEndTimeMS() == 400);
}

TEST_CASE("EffectLayer butt up stretch of edge selections", "[seq2][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 100, 200);
    Effect* b = Add(l, "Off", 300, 400);
    Effect* c = Add(l, "Bars", 600, 700);
    f.Undo().CreateUndoStep();
    f.Undo().SetCaptureUndo(true);

    b->SetSelected(EFFECT_LT_SELECTED);
    l->TagAllSelectedEffects();
    l->ButtUpStretchAllSelectedEffects(false, 5000, f.Undo());
    CHECK(b->GetStartTimeMS() == 200);
    // a right drag of a left selection does nothing
    l->TagAllSelectedEffects();
    l->ButtUpStretchAllSelectedEffects(true, 5000, f.Undo());
    CHECK(b->GetEndTimeMS() == 400);

    b->SetSelected(EFFECT_RT_SELECTED);
    l->TagAllSelectedEffects();
    l->ButtUpStretchAllSelectedEffects(true, 5000, f.Undo());
    CHECK(b->GetEndTimeMS() == 600);

    // first effect stretched left goes to zero, last stretched right to the sequence end
    a->SetSelected(EFFECT_LT_SELECTED);
    c->SetSelected(EFFECT_RT_SELECTED);
    b->SetSelected(EFFECT_NOT_SELECTED);
    l->TagAllSelectedEffects();
    l->ButtUpStretchAllSelectedEffects(false, 5000, f.Undo());
    CHECK(a->GetStartTimeMS() == 0);
    l->TagAllSelectedEffects();
    l->ButtUpStretchAllSelectedEffects(true, 5000, f.Undo());
    CHECK(c->GetEndTimeMS() == 5000);

    // whole selection to the left
    c->SetSelected(EFFECT_SELECTED);
    l->TagAllSelectedEffects();
    l->ButtUpStretchAllSelectedEffects(false, 5000, f.Undo());
    CHECK(c->GetStartTimeMS() == 600);
}

TEST_CASE("EffectLayer selected counts by effect type", "[seq2][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100)->SetSelected(EFFECT_SELECTED);
    Add(l, "On", 100, 200)->SetSelected(EFFECT_LT_SELECTED);
    Add(l, "Bars", 200, 300)->SetSelected(EFFECT_RT_SELECTED);
    Add(l, "On", 300, 400);
    CHECK(l->GetSelectedEffectCount("") == 3);
    CHECK(l->GetSelectedEffectCount("On") == 2);
    CHECK(l->GetSelectedEffectCount("Bars") == 1);
    CHECK(l->GetSelectedEffectCount("Twinkle") == 0);
}

TEST_CASE("EffectLayer apply setting to selected effects of a type", "[seq2][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 0, 100);
    Effect* b = Add(l, "Bars", 100, 200);
    Effect* c = Add(l, "On", 300, 400);
    a->SetSelected(EFFECT_SELECTED);
    b->SetSelected(EFFECT_SELECTED);
    f.Undo().CreateUndoStep();
    RangeAccumulator r;
    auto& em = f.se.GetEffectManager();
    l->ApplyEffectSettingToSelected(nullptr, f.Undo(), "On", "E_TEXTCTRL_Eff_On_Start", "42", nullptr, "E_VALUECURVE_Eff_On_Start", em, r);
    CHECK(a->GetSetting("E_TEXTCTRL_Eff_On_Start") == "42");
    CHECK(b->GetSetting("E_TEXTCTRL_Eff_On_Start").empty());
    CHECK(c->GetSetting("E_TEXTCTRL_Eff_On_Start").empty());
    CHECK(Ranges(r) == std::vector<std::pair<int, int>>{ { 0, 100 } });

    r.clear();
    l->ApplyEffectSettingToSelected(nullptr, f.Undo(), "", "C_SLIDER_Brightness", "150", nullptr, "C_VALUECURVE_Brightness", em, r);
    CHECK(a->GetPaletteMap().Get("C_SLIDER_Brightness", "") == "150");
    CHECK(b->GetPaletteMap().Get("C_SLIDER_Brightness", "") == "150");
    CHECK(r.size() == 2);

    f.Undo().UndoLastStep();
    CHECK(a->GetSetting("E_TEXTCTRL_Eff_On_Start").empty());
    CHECK(b->GetPaletteMap().Get("C_SLIDER_Brightness", "none") == "none");
}

TEST_CASE("EffectLayer callback to selected effects records only changed ones", "[seq2][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 0, 100);
    Effect* b = Add(l, "On", 200, 300);
    Effect* c = Add(l, "On", 400, 500);
    a->SetSelected(EFFECT_SELECTED);
    b->SetSelected(EFFECT_SELECTED);
    f.Undo().CreateUndoStep();
    RangeAccumulator r;
    int calls = 0;
    l->ApplyCallbackToSelected(nullptr, f.Undo(), "On", [&](Effect* e) {
        ++calls;
        if (e == a) {
            e->SetSetting("E_X", "1");
            return true;
        }
        return false;
    }, f.se.GetEffectManager(), r);
    CHECK(calls == 2);
    CHECK(a->GetSetting("E_X") == "1");
    CHECK(c->GetSetting("E_X").empty());
    CHECK(Ranges(r) == std::vector<std::pair<int, int>>{ { 0, 100 } });
}

TEST_CASE("EffectLayer button press on selected effects", "[seq2][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 0, 100, "E_TEXTCTRL_Eff_On_Start=10");
    a->SetSelected(EFFECT_SELECTED);
    f.Undo().CreateUndoStep();
    RangeAccumulator r;
    l->ApplyButtonPressToSelected(nullptr, f.Undo(), "", "E_BUTTON_Unknown", f.se.GetEffectManager(), r);
    l->ApplyButtonPressToSelected(nullptr, f.Undo(), "On", "C_BUTTON_Palette1", f.se.GetEffectManager(), r);
    CHECK(r.size() == 2);
    CHECK(a->GetSetting("E_TEXTCTRL_Eff_On_Start") == "10");
}

TEST_CASE("RangeAccumulator consolidates overlapping and near ranges", "[seq2][layer]") {
    RangeAccumulator r;
    r.Add(500, 600);
    r.Add(0, 100);
    r.Add(90, 200);
    r.Add(210, 300);
    r.Add(50, 60);
    r.Consolidate();
    CHECK(Ranges(r) == std::vector<std::pair<int, int>>{ { 0, 200 }, { 210, 300 }, { 500, 600 } });
    r.Consolidate(10);
    CHECK(Ranges(r) == std::vector<std::pair<int, int>>{ { 0, 300 }, { 500, 600 } });
    CHECK(r.front() == std::pair<int, int>{ 0, 300 });
    CHECK(r.back() == std::pair<int, int>{ 500, 600 });
}

TEST_CASE("Element file references, faces and cleanup walk every layer", "[seq2][layer]") {
    SeqFixture f;
    TempDir d;
    auto pic = d.Write("p.png", "x");
    auto* m = f.Model();
    auto* l0 = m->AddEffectLayer();
    auto* l1 = m->AddEffectLayer();
    Add(l0, "Pictures", 0, 100, "E_TEXTCTRL_Pictures_Filename=" + pic);
    Add(l1, "Faces", 0, 100, "E_CHOICE_Faces_FaceDefinition=Happy");
    Add(l1, "On", 100, 200);
    auto refs = m->GetFileReferences(&f.ctx, nullptr, f.se.GetEffectManager());
    CHECK(std::find(refs.begin(), refs.end(), pic) != refs.end());
    auto faces = m->GetFacesUsed(f.se.GetEffectManager());
    CHECK(std::find(faces.begin(), faces.end(), "Happy") != faces.end());
    m->CleanupFileLocations(&f.ctx, f.se.GetEffectManager());
    auto all = f.se.GetAllReferencedFiles();
    CHECK(std::find(all.begin(), all.end(), pic) != all.end());

    auto* t = f.Timing();
    CHECK(t->GetFileReferences(&f.ctx, nullptr, f.se.GetEffectManager()).empty());
    CHECK(t->GetFacesUsed(f.se.GetEffectManager()).empty());
    CHECK_FALSE(t->CleanupFileLocations(&f.ctx, f.se.GetEffectManager()));
}

// ---------------------------------------------------------------------------------------------
// Element
// ---------------------------------------------------------------------------------------------

TEST_CASE("Element type descriptions for sub elements", "[seq2][elements]") {
    SeqFixture f;
    auto* m = f.Model();
    CHECK(m->GetSubModel("Arm", true)->GetTypeDescription() == "Submodel");
    CHECK(m->GetStrand(0, true)->GetTypeDescription() == "Strand");
    CHECK(f.Timing()->GetTypeDescription() == "Timing");
    CHECK(m->GetTypeDescription() == "Model/Model Group");
}

TEST_CASE("Element HasEffects looks at every layer", "[seq2][elements]") {
    SeqFixture f;
    auto* t = f.Timing();
    t->AddEffectLayer();
    t->AddEffectLayer();
    CHECK_FALSE(t->HasEffects());
    Add(t->GetEffectLayer(1), "", 0, 100);
    CHECK(t->HasEffects());
}

TEST_CASE("ModelElement render ownership parks and hands over jobs", "[seq2][elements]") {
    SeqFixture f;
    auto* m = f.Model();
    int j1 = 0, j2 = 0, j3 = 0;
    CHECK(m->TryTakeRenderOwnership(&j1));
    CHECK(m->TryTakeRenderOwnership(&j1));
    CHECK_FALSE(m->TryTakeRenderOwnership(&j2));
    CHECK_FALSE(m->TryTakeRenderOwnership(&j3));
    CHECK(m->HasParkedRenderJobs());
    CHECK(m->ReleaseRenderOwnership(&j2) == nullptr); // not the owner
    CHECK(m->CancelParkedRenderJob(&j3));
    CHECK_FALSE(m->CancelParkedRenderJob(&j3));
    CHECK(m->ReleaseRenderOwnership(&j1) == &j2);
    CHECK_FALSE(m->HasParkedRenderJobs());
    CHECK_FALSE(m->TryTakeRenderOwnership(&j1));
    m->AbandonRenderOwnership(&j1);
    m->AbandonRenderOwnership(&j2);
    CHECK(m->TryTakeRenderOwnership(&j3));
    CHECK(m->ReleaseRenderOwnership(&j3) == nullptr);
}

TEST_CASE("ModelElement node layers are found across strands", "[seq2][elements]") {
    SeqFixture f;
    auto* m = f.Model();
    auto* s0 = m->GetStrand(0, true);
    auto* s1 = m->GetStrand(1, true);
    s0->GetNodeLayer(1, true);
    s1->GetNodeLayer(2, true);
    CHECK(m->GetNodeEffectLayer(0) == s0->GetNodeLayer(0));
    CHECK(m->GetNodeEffectLayer(1) == s0->GetNodeLayer(1));
    CHECK(m->GetNodeEffectLayer(2) == s1->GetNodeLayer(0));
    CHECK(m->GetNodeEffectLayer(4) == s1->GetNodeLayer(2));
    CHECK(m->GetNodeEffectLayer(5) == nullptr);
    CHECK(s1->GetNodeEffectLayer(9) == nullptr);

    // exclusive layer indices reach sub model, strand and node layers
    auto* sub = m->GetSubModel("Sub", true);
    CHECK(m->GetEffectLayerFromExclusiveIndex(sub->GetEffectLayer(0)->GetIndex()) == sub->GetEffectLayer(0));
    CHECK(m->GetEffectLayerFromExclusiveIndex(s1->GetEffectLayer(0)->GetIndex()) == s1->GetEffectLayer(0));
    CHECK(m->GetEffectLayerFromExclusiveIndex(s1->GetNodeLayer(2)->GetIndex()) == s1->GetNodeLayer(2));
    CHECK(m->GetEffectLayerFromExclusiveIndex(-12345) == nullptr);

    // a const lookup by index runs past sub models into strands
    const ModelElement* cm = m;
    CHECK(cm->GetSubModel(0) == sub);
    CHECK(cm->GetSubModel(1) == s0);
    CHECK(cm->GetSubModel(2) == s1);
    CHECK(cm->GetSubModel(3) == nullptr);
}

TEST_CASE("StrandElement effect validity includes node layers", "[seq2][elements]") {
    SeqFixture f;
    auto* m = f.Model();
    auto* s = m->GetStrand(0, true);
    Effect* onStrand = Add(s->GetEffectLayer(0), "On", 0, 100);
    Effect* onNode = Add(s->GetNodeLayer(3, true), "On", 0, 100);
    Effect* elsewhere = Add(m->AddEffectLayer(), "On", 0, 100);
    CHECK(s->IsEffectValid(onStrand));
    CHECK(s->IsEffectValid(onNode));
    CHECK_FALSE(s->IsEffectValid(elsewhere));
    CHECK(s->GetEffectCount() == 2);
    CHECK(m->GetEffectCount() >= 3);
}

TEST_CASE("ModelElement AddSubModel and RemoveAllSubModels", "[seq2][elements]") {
    SeqFixture f;
    auto* m = f.Model();
    auto* sub = m->GetSubModel("A", true);
    m->RemoveAllSubModels();
    CHECK(m->GetSubModelCount() == 0);
    m->AddSubModel(sub);
    CHECK(m->GetSubModelCount() == 1);
    CHECK(m->GetSubModel("A") == sub);
}

TEST_CASE("TimingElement Papagayo export of a three layer lyric track", "[seq2][elements]") {
    SeqFixture f;
    auto* t = f.Timing("Lyrics");
    CHECK(t->GetPapagayoExport(20).empty());
    auto* phrases = t->AddEffectLayer();
    auto* words = t->AddEffectLayer();
    auto* phonemes = t->AddEffectLayer();
    Add(phrases, "hello world", 0, 1000);
    Add(words, "hello", 0, 500);
    Add(words, "world", 500, 1000);
    Add(phonemes, "E", 0, 250);
    Add(phonemes, "L", 250, 500);
    Add(phonemes, "O", 500, 1000);
    auto out = t->GetPapagayoExport(20);
    CHECK(out.rfind("lipsync version 1\nxlights.wav\n20\n20\n1\n", 0) == 0);
    CHECK(out.find("\t\thello world\n\t\t0\n\t\t20\n\t\t2\n") != std::string::npos);
    CHECK(out.find("\t\t\thello 0 10 2\n\t\t\t\t0 E\n\t\t\t\t5 L\n") != std::string::npos);
    CHECK(out.find("\t\t\tworld 10 20 1\n\t\t\t\t10 O\n") != std::string::npos);
}

// ---------------------------------------------------------------------------------------------
// Effect
// ---------------------------------------------------------------------------------------------

TEST_CASE("Effect ApplySetting with value curves and palette keys", "[seq2][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100, "E_VALUECURVE_X=old");
    e->ApplySetting("E_TEXTCTRL_X", "5", nullptr, "E_VALUECURVE_X");
    CHECK(e->GetSetting("E_TEXTCTRL_X") == "5");
    CHECK_FALSE(e->GetSettings().Contains("E_VALUECURVE_X"));

    ValueCurve vc("Active=TRUE|Id=ID_VALUECURVE_X|Type=Ramp|Min=0.00|Max=100.00|P1=0.00|P2=100.00|");
    REQUIRE(vc.IsActive());
    e->ApplySetting("E_TEXTCTRL_X", "7", &vc, "E_VALUECURVE_X");
    CHECK(e->GetSetting("E_VALUECURVE_X").find("Ramp") != std::string::npos);

    e->ApplySetting("C_SLIDER_Brightness", "80", nullptr, "C_VALUECURVE_Brightness");
    CHECK(e->GetPaletteMap().Get("C_SLIDER_Brightness", "") == "80");
    e->ApplySetting("C_SLIDER_Brightness", "80", &vc, "C_VALUECURVE_Brightness");
    CHECK(e->GetPaletteMap().Get("C_VALUECURVE_Brightness", "").find("Ramp") != std::string::npos);
}

TEST_CASE("Effect ApplySetting file picker pseudo ids", "[seq2][effect]") {
    SeqFixture f;
    TempDir d;
    std::filesystem::create_directories(d.path / "show" / "media" / "pics");
    auto found = d.Write("show/media/pics/a.png", "x");
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "Pictures", 0, 100);

    e->ApplySetting("E_FILEPICKERCTRL_Pic_FN", "/old/place/a.png", nullptr, "");
    CHECK(e->GetSetting("E_FILEPICKERCTRL_Pic") == "/old/place/a.png");

    // _PN keeps the file name and swaps the folder
    e->ApplySetting("E_FILEPICKERCTRL_Pic_PN", "/new/folder", nullptr, "");
    CHECK(e->GetSetting("E_FILEPICKERCTRL_Pic") == (std::filesystem::path("/new/folder") / "a.png").string());

    // a Windows path keeps its file name too
    e->ApplySetting("E_FILEPICKERCTRL_Pic_FN", "C:\\Users\\x\\a.png", nullptr, "");
    e->ApplySetting("E_FILEPICKERCTRL_Pic_PN", "/mac", nullptr, "");
    CHECK(e->GetSetting("E_FILEPICKERCTRL_Pic") == (std::filesystem::path("/mac") / "a.png").string());

    // _SF looks for the deepest existing match under the new show folder
    e->ApplySetting("E_FILEPICKERCTRL_Pic_FN", "/elsewhere/media/pics/a.png", nullptr, "");
    e->ApplySetting("E_FILEPICKERCTRL_Pic_SF", (d.path / "show").string(), nullptr, "");
    CHECK(std::filesystem::equivalent(e->GetSetting("E_FILEPICKERCTRL_Pic"), found));

    // not found leaves the setting alone
    e->ApplySetting("E_FILEPICKERCTRL_Pic_FN", "/elsewhere/none.png", nullptr, "");
    e->ApplySetting("E_FILEPICKERCTRL_Pic_SF", (d.path / "show").string(), nullptr, "");
    CHECK(e->GetSetting("E_FILEPICKERCTRL_Pic") == "/elsewhere/none.png");

    // _PN and _SF do nothing when there is no current value
    Effect* g = Add(l, "Pictures", 200, 300);
    g->ApplySetting("E_FILEPICKERCTRL_Pic_PN", "/x", nullptr, "");
    g->ApplySetting("E_FILEPICKERCTRL_Pic_SF", "/x", nullptr, "");
    CHECK(g->GetSetting("E_FILEPICKERCTRL_Pic").empty());
}

TEST_CASE("Effect palette editing helpers", "[seq2][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100, "E_A=1,E_B=2,T_C=3,B_CHECKBOX_OverlayBkg=1",
                    "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=0,C_SLIDER_Brightness=90");
    CHECK(e->IsPersistent());
    xlColorVector cols;
    xlColorCurveVector cc;
    e->CopyPalette(cols, cc);
    REQUIRE(cols.size() == 1);
    CHECK(cols[0] == xlColor(255, 0, 0));

    // replace just the colours, keeping checkboxes and other colour settings
    e->SetColourOnlyPalette("C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=0,C_BUTTON_Palette2=#FFFFFF", false, true);
    CHECK(e->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#0000FF");
    CHECK(e->GetPaletteMap().Get("C_CHECKBOX_Palette1", "") == "1");
    CHECK(e->GetPaletteMap().Get("C_SLIDER_Brightness", "") == "90");
    e->CopyPalette(cols, cc);
    REQUIRE(cols.size() == 1);
    CHECK(cols[0] == xlColor(0, 0, 255));

    // without keepCheckboxes the new checkboxes win
    e->SetColourOnlyPalette("C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=0,C_BUTTON_Palette2=#FFFFFF,C_CHECKBOX_Palette2=1", false, false);
    e->CopyPalette(cols, cc);
    REQUIRE(cols.size() == 1);
    CHECK(cols[0] == xlColor(255, 255, 255));

    e->SetColourOnlyPalette("{\"C_BUTTON_Palette1\":\"#00FF00\",\"C_CHECKBOX_Palette1\":\"1\"}", true, false);
    CHECK(e->GetPaletteMap().Get("C_BUTTON_Palette1", "") == "#00FF00");

    SettingsMap stripped;
    e->CopySettingsMap(stripped, true);
    CHECK(stripped.Get("A", "") == "1");
    CHECK(stripped.Get("SLIDER_Brightness", "") == "90");
    CHECK_FALSE(stripped.Contains("BUTTON_Palette1"));
    SettingsMap full;
    e->CopySettingsMap(full, false);
    CHECK(full.Get("E_A", "") == "1");

    e->EraseSettingsStartingWith("E_");
    CHECK_FALSE(e->GetSettings().Contains("E_A"));
    CHECK(e->GetSettings().Contains("T_C"));

    e->EraseColourSettings();
    CHECK_FALSE(e->GetPaletteMap().Contains("C_SLIDER_Brightness"));
    CHECK(e->GetPaletteMap().Contains("C_BUTTON_Palette1"));
    e->ErasePalette();
    CHECK(e->GetPaletteMap().empty());
    e->PaletteMapUpdated();
    e->CopyPalette(cols, cc);
    CHECK(cols.empty());

    e->GetPaletteMap()["C_BUTTON_Palette3"] = "#123456";
    e->GetPaletteMap()["C_CHECKBOX_Palette3"] = "1";
    e->PaletteMapUpdated();
    e->CopyPalette(cols, cc);
    REQUIRE(cols.size() == 1);
    CHECK(cols[0] == xlColor(0x12, 0x34, 0x56));
    CHECK(e->ReplaceColours(nullptr, "#123456", "#654321") == 1);
    CHECK(e->ReplaceColours(nullptr, "#123456", "#654321") == 0);
    e->FixBuffer(nullptr);
}

TEST_CASE("Effect settings as JSON round trip", "[seq2][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100, "E_A=1,E_B=x&comma;y", "C_BUTTON_Palette1=#FF0000");
    auto json = e->GetSettingsAsJSON();
    CHECK(json.find("\"E_A\"") != std::string::npos);
    Effect* g = Add(l, "On", 200, 300);
    g->SetSettings(json, false, true);
    CHECK(g->GetSetting("E_A") == "1");
    CHECK(g->GetSetting("E_B") == "x,y");
    CHECK(e->GetPaletteAsJSON().find("#FF0000") != std::string::npos);
}

TEST_CASE("Effect constructed with reversed times swaps them", "[seq2][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect e(&f.se.GetEffectManager(), l, 1, "On", "", "", 500, 200, 0, false, false);
    CHECK(e.GetStartTimeMS() == 200);
    CHECK(e.GetEndTimeMS() == 500);
}

TEST_CASE("Effect names resolve through the effect manager", "[seq2][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100);
    int barsIdx = f.se.GetEffectManager().GetEffectIndex("Bars");
    REQUIRE(barsIdx >= 0);
    CHECK(e->GetEffectName(barsIdx) == "Bars");
    CHECK(e->GetEffectName(-1) == "On");
    e->SetEffectIndex(barsIdx);
    CHECK(e->GetEffectName() == "Bars");
    // an unknown name is kept verbatim
    e->SetEffectName("NotAnEffect");
    CHECK(e->GetEffectIndex() == -1);
    CHECK(e->GetEffectName() == "NotAnEffect");
    e->SetEffectName("On");
    CHECK(e->GetEffectName() == "On");
}

TEST_CASE("Effect deletion time is a minute in the future", "[seq2][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100);
    e->SetTimeToDelete();
    CHECK_FALSE(e->IsTimeToDelete());
}
