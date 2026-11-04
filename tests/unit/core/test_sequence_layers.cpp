#include <catch2/catch_test_macros.hpp>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"

#include <algorithm>
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
};

Effect* Add(EffectLayer* l, const std::string& name, int start, int end) {
    return l->AddEffect(0, name, "", "", start, end, 0, false);
}
}

TEST_CASE("EffectLayer starts empty", "[sequence][layer]") {
    SeqFixture f;
    auto* m = f.Model();
    REQUIRE(m != nullptr);
    auto* l = m->AddEffectLayer();
    CHECK(l->GetEffectCount() == 0);
    CHECK_FALSE(l->HasEffects());
    CHECK(l->GetEffect(0) == nullptr);
    CHECK(l->GetEffect(-1) == nullptr);
    CHECK(l->GetParentElement() == m);
}

TEST_CASE("EffectLayer AddEffect stores times and name", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 100, 500);
    REQUIRE(e != nullptr);
    CHECK(e->GetStartTimeMS() == 100);
    CHECK(e->GetEndTimeMS() == 500);
    CHECK(e->GetEffectName() == "On");
    CHECK(e->GetEffectIndex() >= 0);
    CHECK(l->GetEffectCount() == 1);
    CHECK(l->HasEffects());
    CHECK(l->IsEffectValid(e));
}

TEST_CASE("EffectLayer AddEffect sorts by start time and renumbers ids", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 2000, 3000);
    Add(l, "Off", 0, 1000);
    Add(l, "Bars", 1000, 2000);
    REQUIRE(l->GetEffectCount() == 3);
    CHECK(l->GetEffect(0)->GetStartTimeMS() == 0);
    CHECK(l->GetEffect(1)->GetStartTimeMS() == 1000);
    CHECK(l->GetEffect(2)->GetStartTimeMS() == 2000);
    for (int i = 0; i < 3; ++i) {
        CHECK(l->GetEffect(i)->GetID() == i);
    }
}

TEST_CASE("EffectLayer AddEffect with suppress_sort keeps insertion order", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    l->AddEffect(0, "On", "", "", 2000, 3000, 0, false, true);
    l->AddEffect(0, "On", "", "", 0, 1000, 0, false, true);
    CHECK(l->GetEffect(0)->GetStartTimeMS() == 2000);
}

TEST_CASE("EffectLayer rejects effects with end before start", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    CHECK(Add(l, "On", 500, 100) == nullptr);
    CHECK(l->GetEffectCount() == 0);
}

TEST_CASE("EffectLayer rejects effects that lie entirely before zero", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    CHECK(Add(l, "On", -500, -100) == nullptr);
    CHECK(Add(l, "On", -500, 0) == nullptr);
    CHECK(l->GetEffectCount() == 0);
}

TEST_CASE("EffectLayer clips a negative start time to zero", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", -500, 400);
    REQUIRE(e != nullptr);
    CHECK(e->GetStartTimeMS() == 0);
    CHECK(e->GetEndTimeMS() == 400);
}

TEST_CASE("EffectLayer rejects unknown effect names on model rows only", "[sequence][layer]") {
    SeqFixture f;
    auto* ml = f.Model()->AddEffectLayer();
    CHECK(Add(ml, "NoSuchEffect", 0, 100) == nullptr);
    CHECK(ml->GetEffectCount() == 0);

    auto* tl = f.Timing()->AddEffectLayer();
    Effect* label = Add(tl, "verse", 0, 100);
    REQUIRE(label != nullptr);
    CHECK(label->GetEffectName() == "verse");
    CHECK(label->GetEffectIndex() == -1);
}

TEST_CASE("EffectLayer blank effect name on a model becomes Off", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "", 0, 100);
    REQUIRE(e != nullptr);
    CHECK(e->GetEffectName() == "Off");
}

TEST_CASE("EffectLayer zero length effects are accepted", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    CHECK(Add(l, "On", 100, 100) != nullptr);
}

TEST_CASE("EffectLayer settings string is parsed into settings", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = l->AddEffect(0, "On", "E_TEXTCTRL_Eff_On_Start=80,X_Custom=hello", "", 0, 100, 0, false);
    REQUIRE(e != nullptr);
    CHECK(e->GetSetting("X_Custom") == "hello");
    CHECK(e->GetSetting("E_TEXTCTRL_Eff_On_Start") == "80");
    CHECK(e->GetSetting("missing") == "");
}

TEST_CASE("Effect SetSetting reports whether anything changed", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100);
    REQUIRE(e != nullptr);
    CHECK(e->SetSetting("X_Foo", "1"));
    CHECK_FALSE(e->SetSetting("X_Foo", "1"));
    CHECK(e->SetSetting("X_Foo", "2"));
    CHECK(e->GetSetting("X_Foo") == "2");
}

TEST_CASE("Effect locked and render disabled flags are stored as settings", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100);
    REQUIRE(e != nullptr);
    CHECK_FALSE(e->IsLocked());
    e->SetLocked(true);
    CHECK(e->IsLocked());
    CHECK(e->GetSettingsAsString().find("X_Effect_Locked=True") != std::string::npos);
    e->SetLocked(false);
    CHECK_FALSE(e->IsLocked());

    CHECK_FALSE(e->IsEffectRenderDisabled());
    e->SetEffectRenderDisabled(true);
    CHECK(e->IsEffectRenderDisabled());
    CHECK(e->IsRenderDisabled());
    e->SetEffectRenderDisabled(false);
    CHECK_FALSE(e->IsRenderDisabled());
}

TEST_CASE("Effect render is disabled when its element is disabled", "[sequence][effect]") {
    SeqFixture f;
    auto* m = f.Model();
    auto* l = m->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100);
    REQUIRE(e != nullptr);
    CHECK_FALSE(e->IsRenderDisabled());
    m->SetRenderDisabled(true);
    CHECK(e->IsModelRenderDisabled());
    CHECK(e->IsRenderDisabled());
}

TEST_CASE("Effect OverlapsWith uses half open semantics", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 100, 200);
    REQUIRE(e != nullptr);
    CHECK(e->OverlapsWith(150, 160));
    CHECK(e->OverlapsWith(0, 101));
    CHECK(e->OverlapsWith(199, 300));
    CHECK_FALSE(e->OverlapsWith(0, 100));
    CHECK_FALSE(e->OverlapsWith(200, 300));
}

TEST_CASE("Effect description comes from X_Effect_Description", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = l->AddEffect(0, "On", "X_Effect_Description=my effect", "", 0, 100, 0, false);
    REQUIRE(e != nullptr);
    CHECK(e->GetDescription() == "my effect");
    CHECK(l->SelectEffectUsingDescription("my effect") == e);
    CHECK(e->GetSelected() == EFFECT_SELECTED);
    CHECK(l->SelectEffectUsingDescription("nope") == nullptr);
}

TEST_CASE("Effect SetEffectName changes the effect type", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100);
    REQUIRE(e != nullptr);
    int on = e->GetEffectIndex();
    e->SetEffectName("Bars");
    CHECK(e->GetEffectName() == "Bars");
    CHECK(e->GetEffectIndex() != on);
    CHECK(e->GetEffectIndex() >= 0);
}

TEST_CASE("Effect palette is parsed to colours", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = l->AddEffect(0, "On", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=#00FF00,C_CHECKBOX_Palette2=1", 0, 100, 0, false);
    REQUIRE(e != nullptr);
    REQUIRE(e->GetPaletteSize() == 2);
    CHECK(e->GetPalette()[0].Red() == 255);
    CHECK(e->GetPalette()[0].Green() == 0);
    CHECK(e->GetPalette()[1].Green() == 255);
    CHECK(e->GetPaletteAsString().find("C_BUTTON_Palette1=#FF0000") != std::string::npos);
    e->ErasePalette();
    CHECK(e->GetPaletteMap().empty());
    e->PaletteMapUpdated();
    CHECK(e->GetPaletteSize() == 0);
}

TEST_CASE("Effect UsesColour finds palette colours", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = l->AddEffect(0, "On", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
    REQUIRE(e != nullptr);
    CHECK(e->UsesColour("#FF0000"));
    CHECK_FALSE(e->UsesColour("#0000FF"));
}

TEST_CASE("Effect SettingsChanged ignores setting order", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = l->AddEffect(0, "On", "X_A=1,X_B=2", "", 0, 100, 0, false);
    REQUIRE(e != nullptr);
    CHECK_FALSE(e->SettingsChanged("X_B=2,X_A=1"));
    CHECK(e->SettingsChanged("X_A=1,X_B=3"));
    CHECK(e->SettingsChanged("X_A=1"));
}

TEST_CASE("Effect SetSettings can keep X_ settings", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = l->AddEffect(0, "On", "X_Keep=1,E_TEXTCTRL_Eff_On_Start=10", "", 0, 100, 0, false);
    REQUIRE(e != nullptr);
    e->SetSettings("E_TEXTCTRL_Eff_On_Start=50", true);
    CHECK(e->GetSetting("X_Keep") == "1");
    CHECK(e->GetSetting("E_TEXTCTRL_Eff_On_Start") == "50");
    e->SetSettings("E_TEXTCTRL_Eff_On_Start=60", false);
    CHECK(e->GetSetting("X_Keep") == "");
}

TEST_CASE("Effect FilteredIn matches whole tokens of the label", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Timing()->AddEffectLayer();
    Effect* e = Add(l, "verse one", 0, 100);
    REQUIRE(e != nullptr);
    CHECK(e->FilteredIn("", false));
    CHECK(e->FilteredIn("verse", false));
    CHECK(e->FilteredIn("one", false));
    CHECK_FALSE(e->FilteredIn("ver", false));
    CHECK(e->FilteredIn("^verse", true));
    CHECK_FALSE(e->FilteredIn("^one", true));
}

TEST_CASE("EffectLayer RemoveEffect removes and renumbers", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Add(l, "Off", 100, 200);
    Add(l, "Bars", 200, 300);
    l->RemoveEffect(1);
    REQUIRE(l->GetEffectCount() == 2);
    CHECK(l->GetEffect(0)->GetStartTimeMS() == 0);
    CHECK(l->GetEffect(1)->GetStartTimeMS() == 200);
    CHECK(l->GetEffect(1)->GetID() == 1);
}

TEST_CASE("EffectLayer RemoveEffect leaves locked effects", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 0, 100);
    e->SetLocked(true);
    l->RemoveEffect(0);
    CHECK(l->GetEffectCount() == 1);
}

TEST_CASE("EffectLayer RemoveEffect out of range is harmless for negative id handling", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    l->RemoveEffect(5);
    CHECK(l->GetEffectCount() == 1);
}

TEST_CASE("EffectLayer DeleteEffect by id and by index", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Add(l, "Off", 100, 200);
    Add(l, "Bars", 200, 300);
    l->DeleteEffect(1);
    REQUIRE(l->GetEffectCount() == 2);
    CHECK(l->GetEffect(1)->GetStartTimeMS() == 200);
    l->DeleteEffectByIndex(0);
    REQUIRE(l->GetEffectCount() == 1);
    CHECK(l->GetEffect(0)->GetStartTimeMS() == 200);
}

TEST_CASE("EffectLayer RemoveAllEffects and DeleteAllEffects keep locked ones", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Add(l, "Off", 100, 200)->SetLocked(true);
    Add(l, "Bars", 200, 300);
    l->RemoveAllEffects(nullptr);
    REQUIRE(l->GetEffectCount() == 1);
    CHECK(l->GetEffect(0)->IsLocked());
    l->DeleteAllEffects();
    CHECK(l->GetEffectCount() == 1);
}

TEST_CASE("EffectLayer RemoveAllEffects captures undo", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Add(l, "Off", 100, 200);
    UndoManager& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    l->RemoveAllEffects(&um);
    CHECK(l->GetEffectCount() == 0);
    CHECK(um.CanUndo());
}

TEST_CASE("EffectLayer GetEffectAtTime includes both endpoints", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 0, 100);
    Effect* b = Add(l, "Off", 100, 200);
    CHECK(l->GetEffectAtTime(0) == a);
    CHECK(l->GetEffectAtTime(50) == a);
    CHECK(l->GetEffectAtTime(100) == a);
    CHECK(l->GetEffectAtTime(101) == b);
    CHECK(l->GetEffectAtTime(201) == nullptr);
    CHECK(l->GetEffectByTime(150) == b);
    CHECK(l->GetEffectByTime(500) == nullptr);
}

TEST_CASE("EffectLayer GetEffectStartingAtTime requires exact start", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Effect* b = Add(l, "Off", 100, 200);
    CHECK(l->GetEffectStartingAtTime(100) == b);
    CHECK(l->GetEffectStartingAtTime(101) == nullptr);
}

TEST_CASE("EffectLayer GetEffectBeforeTime and AfterTime", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 0, 100);
    Effect* b = Add(l, "Off", 200, 300);
    Effect* c = Add(l, "Bars", 400, 500);
    CHECK(l->GetEffectBeforeTime(250) == b);
    CHECK(l->GetEffectBeforeTime(200) == a);
    CHECK(l->GetEffectBeforeTime(0) == nullptr);
    CHECK(l->GetEffectBeforeTime(1000) == c);
    CHECK(l->GetEffectAfterTime(0) == b);
    CHECK(l->GetEffectAfterTime(200) == c);
    CHECK(l->GetEffectAfterTime(400) == nullptr);
}

TEST_CASE("EffectLayer GetEffectBeforeEmptyTime and AfterEmptyTime", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 0, 100);
    Effect* b = Add(l, "Off", 200, 300);
    CHECK(l->GetEffectBeforeEmptyTime(150) == a);
    CHECK(l->GetEffectBeforeEmptyTime(50) == nullptr);
    CHECK(l->GetEffectAfterEmptyTime(150) == b);
    CHECK(l->GetEffectAfterEmptyTime(250) == nullptr);
}

TEST_CASE("EffectLayer before and after honour label filters", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Timing()->AddEffectLayer();
    Effect* a = Add(l, "a", 0, 100);
    Add(l, "b", 100, 200);
    Effect* c = Add(l, "a", 200, 300);
    CHECK(l->GetEffectAfterTime(0, "a", false) == c);
    CHECK(l->GetEffectBeforeTime(250, "a", false) == c);
    CHECK(l->GetEffectBeforeTime(200, "a", false) == a);
    CHECK(l->GetEffectAtTime(150, "a", false) == nullptr);
    CHECK(l->GetEffectAtTime(150, "b", false) != nullptr);
}

TEST_CASE("EffectLayer HitTestEffectByTime reports index", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Add(l, "Off", 200, 300);
    int idx = -1;
    CHECK(l->HitTestEffectByTime(250, idx));
    CHECK(idx == 1);
    CHECK_FALSE(l->HitTestEffectByTime(150, idx));
}

TEST_CASE("EffectLayer HitTestEffectBetweenTime detects interior boundaries", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Timing()->AddEffectLayer();
    Add(l, "a", 100, 200);
    CHECK(l->HitTestEffectBetweenTime(50, 150));
    CHECK(l->HitTestEffectBetweenTime(150, 250));
    CHECK(l->HitTestEffectBetweenTime(100, 200));
    CHECK_FALSE(l->HitTestEffectBetweenTime(0, 100));
    CHECK_FALSE(l->HitTestEffectBetweenTime(200, 300));
}

TEST_CASE("EffectLayer GetRangeIsClearMS rules for adjacent and overlapping ranges", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 100, 200);
    CHECK(l->GetRangeIsClearMS(0, 100));
    CHECK(l->GetRangeIsClearMS(200, 300));
    CHECK_FALSE(l->GetRangeIsClearMS(50, 150));
    CHECK_FALSE(l->GetRangeIsClearMS(150, 250));
    CHECK_FALSE(l->GetRangeIsClearMS(120, 180));
    CHECK_FALSE(l->GetRangeIsClearMS(100, 200));
    CHECK_FALSE(l->GetRangeIsClearMS(0, 300));
}

TEST_CASE("EffectLayer GetRangeIsClearMS can ignore selected effects", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = Add(l, "On", 100, 200);
    e->SetSelected(EFFECT_SELECTED);
    CHECK_FALSE(l->GetRangeIsClearMS(100, 200, false));
    CHECK(l->GetRangeIsClearMS(100, 200, true));
}

TEST_CASE("EffectLayer HasEffectsInTimeRange and HasEffectsByType", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 100, 200);
    CHECK(l->HasEffectsInTimeRange(150, 160));
    CHECK_FALSE(l->HasEffectsInTimeRange(200, 300));
    CHECK(l->HasEffectsByType("On"));
    CHECK_FALSE(l->HasEffectsByType("Bars"));
}

TEST_CASE("Element FindOpenLayer reuses a free layer or creates a new one", "[sequence][layer]") {
    SeqFixture f;
    auto* m = f.Model();
    auto* l0 = m->AddEffectLayer();
    Add(l0, "On", 0, 1000);
    CHECK(m->FindOpenLayer(1000, 2000) == l0);
    EffectLayer* l1 = m->FindOpenLayer(500, 1500);
    CHECK(l1 != l0);
    CHECK(m->GetEffectLayerCount() == 2);
}

TEST_CASE("EffectLayer neighbours are found chronologically", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 0, 100);
    Effect* b = Add(l, "Off", 100, 200);
    Effect* c = Add(l, "Bars", 300, 400);
    CHECK(l->GetPriorEffect(1) == a);
    CHECK(l->GetNextEffect(1) == c);
    CHECK(l->GetPriorEffect(0) == nullptr);
    CHECK(l->GetNextEffect(2) == nullptr);
    CHECK(l->GetPriorEffect(99) == nullptr);
    CHECK(l->IsStartTimeLinked(1));
    CHECK(l->IsEndTimeLinked(0));
    CHECK_FALSE(l->IsEndTimeLinked(1));
    CHECK_FALSE(l->IsStartTimeLinked(2));
    (void)b;
}

TEST_CASE("EffectLayer limits for resizing against neighbours", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Add(l, "Off", 100, 200);
    Add(l, "Bars", 300, 400);
    CHECK(l->GetMaximumEndTimeMS(1, false, 50) == 300);
    CHECK(l->GetMaximumEndTimeMS(0, false, 50) == 100);
    CHECK(l->GetMaximumEndTimeMS(0, true, 50) == 150);
    CHECK(l->GetMaximumEndTimeMS(2, false, 50) == NO_MIN_MAX_TIME);
    CHECK(l->GetMinimumStartTimeMS(1, false, 50) == 100);
    CHECK(l->GetMinimumStartTimeMS(1, true, 50) == 50);
    CHECK(l->GetMinimumStartTimeMS(0, false, 50) == NO_MIN_MAX_TIME);
    CHECK(l->GetMinimumStartTimeMS(2, false, 50) == 200);
}

TEST_CASE("EffectLayer selection by time range", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Add(l, "Off", 200, 300);
    Add(l, "Bars", 400, 500);
    CHECK(l->SelectEffectsInTimeRange(150, 350) == 1);
    CHECK(l->GetSelectedEffectCount() == 1);
    CHECK(l->GetEffect(1)->GetSelected() == EFFECT_SELECTED);
    l->UnSelectAllEffects();
    CHECK(l->GetSelectedEffectCount() == 0);
    l->SelectAllEffects();
    CHECK(l->GetSelectedEffectCount() == 3);
    CHECK(l->GetFirstSelectedEffectStartMS() == 0);
    CHECK(l->GetLastSelectedEffectEndMS() == 500);
    l->UnSelectAllEffects();
    CHECK(l->GetFirstSelectedEffectStartMS() == -1);
    CHECK(l->GetLastSelectedEffectEndMS() == -1);
}

TEST_CASE("EffectLayer selecting part of an effect marks the touched side", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 1000);
    CHECK(l->SelectEffectsInTimeRange(0, 200) == 1);
    CHECK(l->GetEffect(0)->GetSelected() == EFFECT_LT_SELECTED);
    l->UnSelectAllEffects();
    CHECK(l->SelectEffectsInTimeRange(800, 1000) == 1);
    CHECK(l->GetEffect(0)->GetSelected() == EFFECT_RT_SELECTED);
}

TEST_CASE("EffectLayer GetAllEffectsByTime and by type", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Add(l, "Off", 100, 200);
    Add(l, "On", 200, 300);
    CHECK(l->GetAllEffectsByTime(50, 250).size() == 3);
    CHECK(l->GetAllEffectsByTime(100, 200).size() == 1);
    CHECK(l->GetEffectsByTypeAndTime("On", 0, 1000).size() == 2);
    CHECK(l->GetEffectsByTypeAndTime("Bars", 0, 1000).empty());
    CHECK(l->GetAllEffects().size() == 3);
}

TEST_CASE("EffectLayer MoveAllSelectedEffects moves only tagged selected effects", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 100, 200);
    Effect* b = Add(l, "Off", 300, 400);
    a->SetSelected(EFFECT_SELECTED);
    l->TagAllSelectedEffects();
    CHECK(l->GetTaggedEffectCount() == 1);
    UndoManager& um = f.se.get_undo_mgr();
    l->MoveAllSelectedEffects(50, um);
    CHECK(a->GetStartTimeMS() == 150);
    CHECK(a->GetEndTimeMS() == 250);
    CHECK(b->GetStartTimeMS() == 300);
    CHECK(l->GetTaggedEffectCount() == 0);
}

TEST_CASE("EffectLayer MoveAllSelectedEffects honours left and right selections", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 100, 200);
    Effect* b = Add(l, "Off", 300, 400);
    a->SetSelected(EFFECT_LT_SELECTED);
    b->SetSelected(EFFECT_RT_SELECTED);
    l->TagAllSelectedEffects();
    l->MoveAllSelectedEffects(20, f.se.get_undo_mgr());
    CHECK(a->GetStartTimeMS() == 120);
    CHECK(a->GetEndTimeMS() == 200);
    CHECK(b->GetStartTimeMS() == 300);
    CHECK(b->GetEndTimeMS() == 420);
}

TEST_CASE("EffectLayer MoveAllSelectedEffects skips locked effects", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 100, 200);
    a->SetSelected(EFFECT_SELECTED);
    l->TagAllSelectedEffects();
    a->SetLocked(true);
    l->MoveAllSelectedEffects(50, f.se.get_undo_mgr());
    CHECK(a->GetStartTimeMS() == 100);
}

TEST_CASE("EffectLayer StretchAllSelectedEffects extends in the drag direction", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 100, 200);
    a->SetSelected(EFFECT_SELECTED);
    l->TagAllSelectedEffects();
    l->StretchAllSelectedEffects(50, f.se.get_undo_mgr());
    CHECK(a->GetStartTimeMS() == 100);
    CHECK(a->GetEndTimeMS() == 250);
    a->SetTagged(true);
    l->StretchAllSelectedEffects(-30, f.se.get_undo_mgr());
    CHECK(a->GetStartTimeMS() == 70);
    CHECK(a->GetEndTimeMS() == 250);
}

TEST_CASE("EffectLayer ButtUpStretch stretches to neighbours or sequence length", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 100, 200);
    Effect* b = Add(l, "Off", 500, 600);
    a->SetSelected(EFFECT_SELECTED);
    b->SetSelected(EFFECT_SELECTED);
    l->TagAllSelectedEffects();
    l->ButtUpStretchAllSelectedEffects(true, 1000, f.se.get_undo_mgr());
    CHECK(a->GetEndTimeMS() == 500);
    CHECK(b->GetEndTimeMS() == 1000);
}

TEST_CASE("EffectLayer maximum movement range is bounded by unselected neighbours", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Add(l, "On", 0, 100);
    Effect* b = Add(l, "Off", 200, 300);
    Add(l, "Bars", 500, 600);
    b->SetSelected(EFFECT_SELECTED);
    int toLeft = 0, toRight = 0;
    l->GetMaximumRangeOfMovementForSelectedEffects(toLeft, toRight);
    CHECK(toLeft == 100);
    CHECK(toRight == 200);
}

TEST_CASE("EffectLayer DeleteSelectedEffects removes selected unlocked effects with undo", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 0, 100);
    Effect* b = Add(l, "Off", 100, 200);
    Effect* c = Add(l, "Bars", 200, 300);
    a->SetSelected(EFFECT_SELECTED);
    b->SetSelected(EFFECT_SELECTED);
    b->SetLocked(true);
    c->SetSelected(EFFECT_NOT_SELECTED);
    UndoManager& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    l->DeleteSelectedEffects(um);
    REQUIRE(l->GetEffectCount() == 2);
    CHECK(l->GetEffect(0)->GetEffectName() == "Off");
    CHECK(l->GetEffect(1)->GetEffectName() == "Bars");
}

TEST_CASE("EffectLayer UpdateAllSelectedEffects applies palette only to selected", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* a = Add(l, "On", 0, 100);
    Effect* b = Add(l, "Off", 100, 200);
    a->SetSelected(EFFECT_SELECTED);
    l->UpdateAllSelectedEffects("C_BUTTON_Palette1=#123456,C_CHECKBOX_Palette1=1");
    CHECK(a->GetPaletteSize() == 1);
    CHECK(b->GetPaletteSize() == 0);
}

TEST_CASE("EffectLayer used colours and replacement", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    l->AddEffect(0, "On", "", "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
    l->AddEffect(0, "Off", "", "C_BUTTON_Palette1=#00FF00,C_CHECKBOX_Palette1=1", 100, 200, 0, false);
    auto used = l->GetUsedColours(false);
    CHECK(std::find(used.begin(), used.end(), "#ff0000") != used.end());
    CHECK(std::find(used.begin(), used.end(), "#00ff00") != used.end());
    CHECK(l->GetUsedColours(true).empty());
}

TEST_CASE("EffectLayer layer name is optional", "[sequence][layer]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    CHECK(l->GetLayerName().empty());
    l->SetLayerName("Background");
    CHECK(l->GetLayerName() == "Background");
    l->SetLayerName("");
    CHECK(l->GetLayerName().empty());
}

TEST_CASE("EffectLayer timing layer flags", "[sequence][layer]") {
    SeqFixture f;
    auto* ml = f.Model()->AddEffectLayer();
    auto* te = f.Timing();
    auto* tl = te->AddEffectLayer();
    CHECK_FALSE(ml->IsTimingLayer());
    CHECK(tl->IsTimingLayer());
    CHECK_FALSE(tl->IsFixedTimingLayer());
    te->SetFixedTiming(50);
    CHECK(tl->IsFixedTimingLayer());
    te->Unfix();
    CHECK_FALSE(tl->IsFixedTimingLayer());
}

TEST_CASE("EffectLayer layer numbers are one based per element", "[sequence][layer]") {
    SeqFixture f;
    auto* m = f.Model();
    auto* l0 = m->AddEffectLayer();
    auto* l1 = m->AddEffectLayer();
    CHECK(l0->GetLayerNumber() == 1);
    CHECK(l1->GetLayerNumber() == 2);
    CHECK(m->GetEffectLayerFromExclusiveIndex(l1->GetIndex()) == l1);
    CHECK(m->GetEffectLayerFromExclusiveIndex(-12345) == nullptr);
}

TEST_CASE("Effect copy constructor copies timing and settings but not symbol link", "[sequence][effect]") {
    SeqFixture f;
    auto* l = f.Model()->AddEffectLayer();
    Effect* e = l->AddEffect(0, "On", "X_A=1", "", 10, 20, 0, false);
    REQUIRE(e != nullptr);
    Effect copy(*e);
    CHECK(copy.GetStartTimeMS() == 10);
    CHECK(copy.GetEndTimeMS() == 20);
    CHECK(copy.GetEffectIndex() == e->GetEffectIndex());
    CHECK(copy.GetSetting("X_A") == "1");
    CHECK_FALSE(copy.IsLinkedToSymbol());
}
