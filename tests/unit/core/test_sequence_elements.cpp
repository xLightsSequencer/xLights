#include <catch2/catch_test_macros.hpp>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/UndoManager.h"

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
    ModelElement* Model(const std::string& name) {
        return dynamic_cast<ModelElement*>(se.AddElement(name, "model", true, false, true, false, false));
    }
    TimingElement* Timing(const std::string& name) {
        return dynamic_cast<TimingElement*>(se.AddElement(name, "timing", true, false, true, false, false));
    }
    std::vector<std::string> Names() {
        std::vector<std::string> n;
        for (size_t i = 0; i < se.GetElementCount(); ++i) {
            n.push_back(se.GetElement(i)->GetName());
        }
        return n;
    }
};

Effect* Add(EffectLayer* l, const std::string& name, int start, int end) {
    return l->AddEffect(0, name, "", "", start, end, 0, false);
}
}

TEST_CASE("SequenceElements starts empty with defaults", "[sequence][elements]") {
    SeqFixture f;
    CHECK(f.se.GetElementCount() == 0);
    CHECK(f.se.GetFrequency() == 20.0);
    CHECK(f.se.GetFrameMS() == 50);
    CHECK(f.se.GetMinPeriod() == 50);
    CHECK(f.se.GetSequenceEnd() == 0);
    CHECK(f.se.GetCurrentView() == 0);
    CHECK(f.se.GetRowInformationSize() == 0);
    CHECK(f.se.GetElement("nope") == nullptr);
    CHECK(f.se.GetElement((size_t)0) == nullptr);
}

TEST_CASE("SequenceElements frequency drives frame time", "[sequence][elements]") {
    SeqFixture f;
    f.se.SetFrequency(40);
    CHECK(f.se.GetFrameMS() == 25);
    f.se.SetFrequency(30);
    CHECK(f.se.GetMinPeriod() == 33);
    f.se.SetFrequency(100);
    CHECK(f.se.GetFrameMS() == 10);
    f.se.SetFrequency(1);
    CHECK(f.se.GetFrameMS() == 1000);
}

TEST_CASE("SequenceElements sequence end is stored", "[sequence][elements]") {
    SeqFixture f;
    f.se.SetSequenceEnd(123456);
    CHECK(f.se.GetSequenceEnd() == 123456);
}

TEST_CASE("SequenceElements AddElement creates typed elements", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    auto* t = f.Timing("Beats");
    REQUIRE(m != nullptr);
    REQUIRE(t != nullptr);
    CHECK(m->GetType() == ElementType::ELEMENT_TYPE_MODEL);
    CHECK(t->GetType() == ElementType::ELEMENT_TYPE_TIMING);
    CHECK(m->GetName() == "Tree");
    CHECK(f.se.GetElementCount() == 2);
    CHECK(f.se.GetNumberOfTimingElements() == 1);
    CHECK(f.se.ElementExists("Tree"));
    CHECK_FALSE(f.se.ElementExists("Other"));
    CHECK(f.se.IsValidElement(m));
}

TEST_CASE("SequenceElements AddElement rejects duplicate names", "[sequence][elements]") {
    SeqFixture f;
    REQUIRE(f.Model("Tree") != nullptr);
    CHECK(f.se.AddElement("Tree", "model", true, false, true, false, false) == nullptr);
    CHECK(f.se.AddElement("Tree", "timing", true, false, true, false, false) == nullptr);
    CHECK(f.se.GetElementCount() == 1);
}

TEST_CASE("SequenceElements AddElement carries flags", "[sequence][elements]") {
    SeqFixture f;
    auto* m = dynamic_cast<ModelElement*>(f.se.AddElement("A", "model", false, true, true, true, true));
    REQUIRE(m != nullptr);
    CHECK_FALSE(m->GetVisible());
    CHECK(m->GetCollapsed());
    CHECK(m->IsRenderDisabled());
    CHECK(m->GetSelected());
    auto* t = dynamic_cast<TimingElement*>(f.se.AddElement("T", "timing", true, false, false, false, false));
    REQUIRE(t != nullptr);
    CHECK_FALSE(t->GetActive());
    CHECK(t->GetMasterVisible());
}

TEST_CASE("SequenceElements AddElement at an index inserts in place", "[sequence][elements]") {
    SeqFixture f;
    f.Model("A");
    f.Model("C");
    REQUIRE(f.se.AddElement(1, "B", "model", true, false, true, false, false) != nullptr);
    CHECK(f.Names() == std::vector<std::string>{ "A", "B", "C" });
    CHECK(f.se.AddElement(0, "Z", "model", true, false, true, false, false) != nullptr);
    CHECK(f.Names().front() == "Z");
    CHECK(f.se.AddElement(f.se.GetElementCount(), "End", "model", true, false, true, false, false) != nullptr);
    CHECK(f.Names().back() == "End");
}

TEST_CASE("SequenceElements AddElement at a bad index or duplicate fails", "[sequence][elements]") {
    SeqFixture f;
    f.Model("A");
    CHECK(f.se.AddElement(-1, "B", "model", true, false, true, false, false) == nullptr);
    CHECK(f.se.AddElement(5, "B", "model", true, false, true, false, false) == nullptr);
    CHECK(f.se.AddElement(0, "A", "model", true, false, true, false, false) == nullptr);
    CHECK(f.se.GetElementCount() == 1);
}

TEST_CASE("SequenceElements element lookup by name index and full name", "[sequence][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    auto* b = f.Model("B");
    CHECK(f.se.GetElement("A") == a);
    CHECK(f.se.GetElement((size_t)1) == b);
    CHECK(f.se.GetElement((size_t)2) == nullptr);
    CHECK(f.se.GetElement((size_t)0, 7) == nullptr);
    CHECK(f.se.GetElementIndex("B") == 1);
    CHECK(f.se.GetElementIndex("none") == -1);
    CHECK(f.se.GetElementCount(5) == 0);
    CHECK_FALSE(f.se.ElementExists("A", 5));
}

TEST_CASE("SequenceElements timing element lookup", "[sequence][elements]") {
    SeqFixture f;
    f.Model("Tree");
    auto* t1 = f.Timing("T1");
    auto* t2 = f.Timing("T2");
    CHECK(f.se.GetTimingElement(0) == t1);
    CHECK(f.se.GetTimingElement(1) == t2);
    CHECK(f.se.GetTimingElement(2) == nullptr);
    CHECK(f.se.GetTimingElement("T2") == t2);
    CHECK(f.se.GetTimingElement("Tree") == nullptr);
    CHECK(f.se.GetNumberOfTimingElements() == 2);
}

TEST_CASE("SequenceElements UniqueElementName appends a counter", "[sequence][elements]") {
    SeqFixture f;
    CHECK(f.se.UniqueElementName("Beats") == "Beats");
    f.Timing("Beats");
    CHECK(f.se.UniqueElementName("Beats") == "Beats-1");
    f.Timing("Beats-1");
    CHECK(f.se.UniqueElementName("Beats") == "Beats-2");
}

TEST_CASE("SequenceElements DeleteElement removes the element", "[sequence][elements]") {
    SeqFixture f;
    f.Model("A");
    f.Model("B");
    f.Model("C");
    f.se.DeleteElement("B");
    CHECK(f.Names() == std::vector<std::string>{ "A", "C" });
    CHECK(f.se.GetElement("B") == nullptr);
    f.se.DeleteElement("missing");
    CHECK(f.se.GetElementCount() == 2);
    f.se.DeleteElement("A");
    f.se.DeleteElement("C");
    CHECK(f.se.GetElementCount() == 0);
}

TEST_CASE("SequenceElements Clear removes everything", "[sequence][elements]") {
    SeqFixture f;
    f.Model("A")->AddEffectLayer();
    f.Timing("T");
    f.se.SetTagPosition(2, 500);
    f.se.Clear();
    CHECK(f.se.GetElementCount() == 0);
    CHECK(f.se.GetTagPosition(2) == -1);
    CHECK(f.se.GetRowInformationSize() == 0);
    REQUIRE(f.Model("A") != nullptr);
}

TEST_CASE("SequenceElements MoveElementUp and Down swap neighbours", "[sequence][elements]") {
    SeqFixture f;
    f.Model("A");
    f.Model("B");
    f.Model("C");
    f.se.MoveElementUp("C", MASTER_VIEW);
    CHECK(f.Names() == std::vector<std::string>{ "A", "C", "B" });
    f.se.MoveElementUp("A", MASTER_VIEW);
    CHECK(f.Names() == std::vector<std::string>{ "A", "C", "B" });
    f.se.MoveElementDown("A", MASTER_VIEW);
    CHECK(f.Names() == std::vector<std::string>{ "C", "A", "B" });
    f.se.MoveElementDown("B", MASTER_VIEW);
    CHECK(f.Names() == std::vector<std::string>{ "C", "A", "B" });
}

TEST_CASE("SequenceElements MoveSequenceElement moves to a destination", "[sequence][elements]") {
    SeqFixture f;
    for (auto n : { "A", "B", "C", "D" }) {
        f.Model(n);
    }
    f.se.MoveSequenceElement(3, 0, MASTER_VIEW);
    CHECK(f.Names() == std::vector<std::string>{ "D", "A", "B", "C" });
    f.se.MoveSequenceElement(0, 3, MASTER_VIEW);
    CHECK(f.Names() == std::vector<std::string>{ "A", "B", "D", "C" });
    f.se.MoveSequenceElement(0, 99, MASTER_VIEW);
    CHECK(f.Names() == std::vector<std::string>{ "B", "D", "C", "A" });
    f.se.MoveSequenceElement(99, 0, MASTER_VIEW);
    CHECK(f.Names() == std::vector<std::string>{ "B", "D", "C", "A" });
}

TEST_CASE("SequenceElements renaming a timing track renames the element", "[sequence][elements]") {
    SeqFixture f;
    auto* t = f.Timing("Old");
    t->SetName("New");
    CHECK(f.se.GetTimingElement("New") == t);
    CHECK(f.se.GetTimingElement("Old") == nullptr);
    f.se.RenameTimingTrack("New", "Newer");
    SUCCEED();
}

TEST_CASE("SequenceElements change count increases on edits", "[sequence][elements]") {
    SeqFixture f;
    unsigned before = f.se.GetChangeCount();
    auto* m = f.Model("A");
    CHECK(f.se.GetChangeCount() > before);
    unsigned mid = f.se.GetChangeCount();
    Add(m->AddEffectLayer(), "On", 0, 100);
    CHECK(f.se.GetChangeCount() > mid);
    CHECK(m->getChangeCount() > 0);
}

TEST_CASE("Element tracks dirty range of edits", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("A");
    auto* l = m->AddEffectLayer();
    int changes = 0, s = 0, e = 0;
    m->GetAndResetDirtyRange(changes, s, e);
    Add(l, "On", 100, 200);
    m->GetDirtyRange(s, e);
    CHECK(s == 100);
    CHECK(e == 200);
    Add(l, "Off", 500, 600);
    m->GetDirtyRange(s, e);
    CHECK(s == 100);
    CHECK(e == 600);
    m->GetAndResetDirtyRange(changes, s, e);
    CHECK(changes > 0);
    m->GetDirtyRange(s, e);
    CHECK(s == -1);
    CHECK(e == -1);
}

TEST_CASE("Element layer management", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("A");
    CHECK(m->GetEffectLayerCount() == 0);
    auto* l0 = m->AddEffectLayer();
    auto* l1 = m->AddEffectLayer();
    auto* l2 = m->InsertEffectLayer(1);
    REQUIRE(m->GetEffectLayerCount() == 3);
    CHECK(m->GetEffectLayer(0) == l0);
    CHECK(m->GetEffectLayer(1) == l2);
    CHECK(m->GetEffectLayer(2) == l1);
    CHECK(m->GetEffectLayer(3) == nullptr);
    CHECK(m->GetEffectLayer(-1) == nullptr);
    m->RemoveEffectLayer(1);
    CHECK(m->GetEffectLayerCount() == 2);
    CHECK(m->GetEffectLayer(1) == l1);
    CHECK(m->GetLayerNumberFromIndex(l1->GetIndex()) == 2);
    CHECK(m->GetLayerNumberFromIndex(-99) == -1);
}

TEST_CASE("Element aggregates effects over all layers", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("A");
    CHECK_FALSE(m->HasEffects());
    auto* l0 = m->AddEffectLayer();
    auto* l1 = m->AddEffectLayer();
    Add(l0, "On", 0, 100);
    Add(l0, "Off", 100, 200);
    Add(l1, "Bars", 0, 200)->SetSelected(EFFECT_SELECTED);
    CHECK(m->HasEffects());
    CHECK(m->GetEffectCount() == 3);
    CHECK(m->GetSelectedEffectCount() == 1);
    CHECK(m->GetFirstSelectedEffectStartMS() == 0);
    CHECK(m->GetLastSelectedEffectEndMS() == 200);
    auto layers = m->GetLayersWithEffectsByTime(150, 160);
    CHECK(layers == std::vector<int>{ 0, 1 });
    CHECK(m->GetLayersWithEffectsByTime(300, 400).empty());
}

TEST_CASE("Element type descriptions", "[sequence][elements]") {
    SeqFixture f;
    CHECK(f.Timing("T")->GetTypeDescription() == "Timing");
    CHECK(f.Model("M")->GetTypeDescription() == "Model/Model Group");
}

TEST_CASE("Element equality and ordering rules", "[sequence][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    auto* b = f.Model("B");
    auto* t = f.Timing("T");
    CHECK(*a == *a);
    CHECK_FALSE(*a == *b);
    CHECK(*a < *b);
    CHECK_FALSE(*b < *a);
    CHECK(*t < *a);
    CHECK_FALSE(*a < *t);
}

TEST_CASE("Element ordering is number aware", "[sequence][elements]") {
    SeqFixture f;
    auto* t2 = f.Model("Tree 2");
    auto* t10 = f.Model("Tree 10");
    CHECK(*t2 < *t10);
}

TEST_CASE("TimingElement properties", "[sequence][elements]") {
    SeqFixture f;
    auto* t = f.Timing("T");
    t->SetViews("Master,Other");
    t->SetSubType("Beats");
    CHECK(t->GetViews() == "Master,Other");
    CHECK(t->GetSubType() == "Beats");
    CHECK_FALSE(t->IsFixedTiming());
    t->SetFixedTiming(100);
    CHECK(t->IsFixedTiming());
    CHECK(t->GetFixedTiming() == 100);
    t->Unfix();
    CHECK_FALSE(t->IsFixedTiming());
    CHECK(t->GetNodeEffectLayer(0) == nullptr);
}

TEST_CASE("TimingElement HasLyrics looks at non blank labels", "[sequence][elements]") {
    SeqFixture f;
    auto* t = f.Timing("T");
    auto* l = t->AddEffectLayer();
    CHECK_FALSE(t->HasLyrics(0));
    Add(l, "", 0, 100);
    CHECK_FALSE(t->HasLyrics(0));
    Add(l, "hello", 100, 200);
    CHECK(t->HasLyrics(0));
    CHECK_FALSE(t->HasLyrics(5));
}

TEST_CASE("TimingElement export lists labelled marks", "[sequence][elements]") {
    SeqFixture f;
    auto* t = f.Timing("T");
    auto* l = t->AddEffectLayer();
    Add(l, "one", 0, 500);
    Add(l, "two", 500, 1000);
    auto ex = t->GetExport();
    CHECK(ex.find("one") != std::string::npos);
    CHECK(ex.find("two") != std::string::npos);
}

TEST_CASE("TimingElement master visibility is independent of visible", "[sequence][elements]") {
    SeqFixture f;
    auto* t = f.Timing("T");
    t->SetVisible(false);
    CHECK(t->GetMasterVisible());
    t->SetMasterVisible(false);
    CHECK_FALSE(t->GetMasterVisible());
    f.se.HideAllTimingTracks(false);
    CHECK(t->GetVisible());
    CHECK(t->GetMasterVisible());
    f.se.HideAllTimingTracks(true);
    CHECK_FALSE(t->GetVisible());
}

TEST_CASE("SequenceElements DeactivateAllTimingElements", "[sequence][elements]") {
    SeqFixture f;
    auto* a = f.Timing("A");
    auto* b = f.Timing("B");
    REQUIRE(a->GetActive());
    f.se.DeactivateAllTimingElements(true);
    CHECK_FALSE(a->GetActive());
    CHECK_FALSE(b->GetActive());
}

TEST_CASE("ModelElement creates submodels on demand", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    CHECK(m->GetSubModel("Left", false) == nullptr);
    auto* sm = m->GetSubModel("Left", true);
    REQUIRE(sm != nullptr);
    CHECK(sm->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL);
    CHECK(sm->GetName() == "Left");
    CHECK(sm->GetFullName() == "Tree/Left");
    CHECK(sm->GetModelName() == "Tree");
    CHECK(sm->GetModelElement() == m);
    CHECK(m->GetSubModel("Left", false) == sm);
    CHECK(m->GetSubModelCount() == 1);
    CHECK(m->GetSubModelAndStrandCount() == 1);
    CHECK(m->GetSubModel(0) == sm);
    CHECK(m->GetSubModel(5) == nullptr);
    CHECK(sm->GetEffectLayerCount() == 1);
}

TEST_CASE("SequenceElements finds submodel elements by full name", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    auto* sm = m->GetSubModel("Left", true);
    CHECK(f.se.GetElement("Tree/Left") == sm);
    CHECK(f.se.GetElement("Tree/Right") == nullptr);
}

TEST_CASE("ModelElement removes submodels", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    m->GetSubModel("A", true);
    m->GetSubModel("B", true);
    m->RemoveSubModel("A");
    CHECK(m->GetSubModelCount() == 1);
    CHECK(m->GetSubModel("A", false) == nullptr);
    m->RemoveSubModel("zzz");
    m->RemoveAllSubModels();
    CHECK(m->GetSubModelCount() == 0);
}

TEST_CASE("ModelElement strands and node layers", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    CHECK(m->GetStrand(0) == nullptr);
    CHECK(m->GetStrand(-1, true) == nullptr);
    auto* s2 = m->GetStrand(2, true);
    REQUIRE(s2 != nullptr);
    CHECK(m->GetStrandCount() == 3);
    CHECK(s2->GetStrand() == 2);
    CHECK(s2->GetType() == ElementType::ELEMENT_TYPE_STRAND);
    CHECK(s2->GetNodeLayerCount() == 0);
    CHECK(s2->GetNodeLayer(3) == nullptr);
    auto* nl = s2->GetNodeLayer(3, true);
    REQUIRE(nl != nullptr);
    CHECK(s2->GetNodeLayerCount() == 4);
    CHECK(s2->GetNodeNumber(nl) == 3);
    CHECK(m->GetSubModelAndStrandCount() == 3);
    CHECK(m->GetSubModel(1) == m->GetStrand(1));
}

TEST_CASE("ModelElement effect counts include only its own layers for GetModelEffectCount", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    Add(m->AddEffectLayer(), "On", 0, 100);
    auto* sm = m->GetSubModel("Left", true);
    Add(sm->GetEffectLayer(0), "Off", 0, 100);
    CHECK(m->GetModelEffectCount() == 1);
    CHECK(m->GetEffectCount() == 2);
    CHECK(m->HasEffects());
    CHECK(sm->HasEffects());
}

TEST_CASE("Strand node layers hold effects and count towards the strand", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    auto* st = m->GetStrand(0, true);
    auto* nl = st->GetNodeLayer(1, true);
    CHECK_FALSE(st->HasEffects());
    Add(nl, "On", 0, 100);
    CHECK(st->HasEffects());
    CHECK(st->GetEffectCount() == 1);
    CHECK(m->HasEffects());
}

TEST_CASE("Strand element names and render disabled inheritance", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    auto* st = m->GetStrand(1, true);
    CHECK(st->GetStrandName() == "Strand 2");
    CHECK(st->GetFullName().find("Tree") == 0);
    CHECK_FALSE(st->IsRenderDisabled());
    m->SetRenderDisabled(true);
    CHECK(st->IsRenderDisabled());
    auto* sm = m->GetSubModel("S", true);
    CHECK(sm->IsRenderDisabled());
}

TEST_CASE("SequenceElements effect validity is checked across all elements", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    auto* sm = m->GetSubModel("S", true);
    Effect* a = Add(m->AddEffectLayer(), "On", 0, 100);
    Effect* b = Add(sm->GetEffectLayer(0), "Off", 0, 100);
    Effect* c = Add(f.Timing("T")->AddEffectLayer(), "x", 0, 100);
    CHECK(f.se.IsValidEffect(a));
    CHECK(f.se.IsValidEffect(b));
    CHECK(f.se.IsValidEffect(c));
    CHECK_FALSE(f.se.IsValidEffect(nullptr));
    m->GetEffectLayer(0)->RemoveEffect(0);
    CHECK_FALSE(f.se.IsValidEffect(a));
}

TEST_CASE("SequenceElements row information lists timing rows first", "[sequence][elements]") {
    SeqFixture f;
    f.Model("A")->AddEffectLayer();
    auto* b = f.Model("B");
    b->AddEffectLayer();
    b->AddEffectLayer();
    f.Timing("T1")->AddEffectLayer();
    f.se.PopulateRowInformation();
    CHECK(f.se.GetNumberOfTimingRows() == 1);
    REQUIRE(f.se.GetRowInformationSize() == 4);
    CHECK(f.se.GetRowInformation(0)->element->GetName() == "T1");
    CHECK(f.se.GetRowInformation(1)->element->GetName() == "A");
    CHECK(f.se.GetRowInformation(2)->element->GetName() == "B");
    CHECK(f.se.GetRowInformation(2)->layerIndex == 0);
    CHECK(f.se.GetRowInformation(3)->layerIndex == 1);
    CHECK(f.se.GetTotalNumberOfModelRows() == 3);
    CHECK(f.se.GetEffectLayer(1) == f.se.GetElement("A")->GetEffectLayer(0));
    CHECK(f.se.GetEffectLayer(-1) == nullptr);
}

TEST_CASE("SequenceElements collapsed models show one row", "[sequence][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    a->AddEffectLayer();
    a->AddEffectLayer();
    a->AddEffectLayer();
    a->SetCollapsed(true);
    f.se.PopulateRowInformation();
    CHECK(f.se.GetRowInformationSize() == 1);
    a->SetCollapsed(false);
    f.se.PopulateRowInformation();
    CHECK(f.se.GetRowInformationSize() == 3);
}

TEST_CASE("SequenceElements hidden models have no rows", "[sequence][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    a->AddEffectLayer();
    f.Model("B")->AddEffectLayer();
    a->SetVisible(false);
    f.se.PopulateRowInformation();
    REQUIRE(f.se.GetRowInformationSize() == 1);
    CHECK(f.se.GetRowInformation(0)->element->GetName() == "B");
    f.se.SetVisibilityForAllModels(true);
    f.se.PopulateRowInformation();
    CHECK(f.se.GetRowInformationSize() == 2);
}

TEST_CASE("SequenceElements model row filter limits rows", "[sequence][elements]") {
    SeqFixture f;
    f.Model("Alpha")->AddEffectLayer();
    f.Model("Beta")->AddEffectLayer();
    f.se.SetModelRowFilter([](const std::string& n) { return n == "Beta"; });
    CHECK(f.se.HasModelRowFilter());
    f.se.PopulateRowInformation();
    REQUIRE(f.se.GetRowInformationSize() == 1);
    CHECK(f.se.GetRowInformation(0)->element->GetName() == "Beta");
}

TEST_CASE("SequenceElements visible rows are bounded by max rows displayed", "[sequence][elements]") {
    SeqFixture f;
    for (auto n : { "A", "B", "C", "D", "E" }) {
        f.Model(n)->AddEffectLayer();
    }
    f.se.SetMaxRowsDisplayed(3);
    f.se.PopulateRowInformation();
    CHECK(f.se.GetVisibleRowInformationSize() == 3);
    CHECK(f.se.GetMaxModelsDisplayed() == 3);
    CHECK(f.se.GetVisibleRowInformation(0)->element->GetName() == "A");
    CHECK(f.se.GetVisibleEffectLayer(1) == f.se.GetElement("B")->GetEffectLayer(0));
    f.se.SetFirstVisibleModelRow(2);
    f.se.PopulateVisibleRowInformation();
    CHECK(f.se.GetVisibleRowInformation(0)->element->GetName() == "C");
    CHECK(f.se.GetFirstVisibleModelRow() == 2);
}

TEST_CASE("SequenceElements selects effects across rows and time", "[sequence][elements]") {
    SeqFixture f;
    auto* a = f.Model("A")->AddEffectLayer();
    auto* b = f.Model("B")->AddEffectLayer();
    Add(a, "On", 0, 100);
    Add(a, "Off", 200, 300);
    Add(b, "On", 0, 100);
    f.se.PopulateRowInformation();
    CHECK(f.se.SelectEffectsInRowAndTimeRange(0, 1, 0, 150) == 2);
    CHECK(f.se.SelectEffectsInRowAndTimeRange(0, -1, 0, 1000) == 3);
    CHECK(f.se.SelectEffectsInRowAndTimeRange(-1, 1, 0, 1000) == 0);
    CHECK(f.se.SelectEffectsInRowAndTimeRange(5, 6, 0, 1000) == 0);
    f.se.UnSelectAllEffects();
    CHECK(a->GetSelectedEffectCount() == 0);
    f.se.SelectAllEffects();
    CHECK(a->GetSelectedEffectCount() == 2);
    CHECK(b->GetSelectedEffectCount() == 1);
    f.se.UnSelectAllEffects();
    f.se.SelectAllEffectsInRow(0);
    CHECK(a->GetSelectedEffectCount() == 2);
    CHECK(b->GetSelectedEffectCount() == 0);
}

TEST_CASE("SequenceElements SelectAllEffectsNoTiming skips timing rows", "[sequence][elements]") {
    SeqFixture f;
    auto* a = f.Model("A")->AddEffectLayer();
    auto* t = f.Timing("T")->AddEffectLayer();
    Add(a, "On", 0, 100);
    Add(t, "x", 0, 100);
    f.se.PopulateRowInformation();
    f.se.SelectAllEffectsNoTiming();
    CHECK(a->GetSelectedEffectCount() == 1);
    CHECK(t->GetSelectedEffectCount() == 0);
}

TEST_CASE("SequenceElements element selection flags", "[sequence][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    auto* b = f.Model("B");
    f.Timing("T");
    f.se.SelectAllElements();
    CHECK(a->GetSelected());
    CHECK(b->GetSelected());
    f.se.UnSelectAllElements();
    CHECK_FALSE(a->GetSelected());
}

TEST_CASE("SequenceElements selected ranges", "[sequence][elements]") {
    SeqFixture f;
    EffectRange r1{ 0.0, 1.0, nullptr, 1 };
    EffectRange r2{ 2.0, 3.0, nullptr, 2 };
    f.se.AddSelectedRange(&r1);
    f.se.AddSelectedRange(&r2);
    CHECK(f.se.GetSelectedRangeCount() == 2);
    CHECK(f.se.GetSelectedRange(1)->Row == 2);
    f.se.DeleteSelectedRange(5);
    f.se.DeleteSelectedRange(-1);
    CHECK(f.se.GetSelectedRangeCount() == 2);
    f.se.DeleteSelectedRange(0);
    CHECK(f.se.GetSelectedRangeCount() == 1);
    f.se.ClearSelectedRanges();
    CHECK(f.se.GetSelectedRangeCount() == 0);
}

TEST_CASE("SequenceElements tags are range checked", "[sequence][elements]") {
    SeqFixture f;
    f.se.SetTagPosition(3, 999);
    CHECK(f.se.GetTagPosition(3) == 999);
    f.se.SetTagPosition(-1, 1);
    f.se.SetTagPosition(SequenceElements::TagCount, 1);
    CHECK(f.se.GetTagPosition(-1) == -1);
    CHECK(f.se.GetTagPosition(SequenceElements::TagCount) == -1);
    f.se.ClearTags();
    CHECK(f.se.GetTagPosition(3) == -1);
}

TEST_CASE("SequenceElements GetMaxEffectEndTimeMS covers submodels and strand nodes", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    CHECK(f.se.GetMaxEffectEndTimeMS() == 0);
    Add(m->AddEffectLayer(), "On", 0, 1000);
    CHECK(f.se.GetMaxEffectEndTimeMS() == 1000);
    Add(m->GetSubModel("S", true)->GetEffectLayer(0), "On", 0, 2500);
    CHECK(f.se.GetMaxEffectEndTimeMS() == 2500);
    Add(m->GetStrand(0, true)->GetNodeLayer(0, true), "On", 0, 4000);
    CHECK(f.se.GetMaxEffectEndTimeMS() == 4000);
    Add(f.Timing("T")->AddEffectLayer(), "x", 0, 99999);
    CHECK(f.se.GetMaxEffectEndTimeMS() == 4000);
}

TEST_CASE("SequenceElements element summaries", "[sequence][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    auto* b = f.Model("B");
    f.Model("Empty")->AddEffectLayer();
    auto* la = a->AddEffectLayer();
    Add(la, "On", 0, 100)->SetSetting("X_Effect_Description", "desc1");
    Add(la, "Off", 100, 200);
    a->AddEffectLayer();
    Add(b->AddEffectLayer(), "On", 0, 50);

    auto types = f.se.GetAllUsedEffectTypes();
    CHECK(std::find(types.begin(), types.end(), "On") != types.end());
    CHECK(std::find(types.begin(), types.end(), "Off") != types.end());
    CHECK(types.size() == 2);

    auto names = f.se.GetAllElementNamesWithEffects();
    CHECK(names == std::list<std::string>{ "A", "B" });

    std::list<int> layers;
    CHECK(f.se.GetElementLayerCount("A", &layers) == 2);
    CHECK(layers == std::list<int>{ 0 });
    CHECK(f.se.GetElementLayerCount("missing") == -1);
    CHECK(f.se.GetElementLayerEffects("A", 0).size() == 2);
    CHECK(f.se.GetElementLayerEffects("A", 5).empty());

    auto descs = f.se.GetAllEffectDescriptions();
    CHECK(descs == std::list<std::string>{ "desc1" });
    Effect* sel = f.se.SelectEffectUsingDescription("desc1");
    REQUIRE(sel != nullptr);
    CHECK(sel->GetSelected() == EFFECT_SELECTED);
    CHECK(f.se.SelectEffectUsingDescription("none") == nullptr);
    CHECK(f.se.SelectEffectUsingElementLayerTime("A", 0, 150) == la->GetEffect(1));
    CHECK(f.se.SelectEffectUsingElementLayerTime("zzz", 0, 150) == nullptr);
}

TEST_CASE("SequenceElements GetAllElementNamesWithEffectsExtended includes submodels", "[sequence][elements]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    Add(m->AddEffectLayer(), "On", 0, 100);
    Add(m->GetSubModel("S", true)->GetEffectLayer(0), "On", 0, 100);
    m->GetSubModel("Idle", true);
    auto ext = f.se.GetAllElementNamesWithEffectsExtended();
    CHECK(ext == std::list<std::string>{ "Tree", "Tree/S" });
}

TEST_CASE("SequenceElements unique property values", "[sequence][elements]") {
    SeqFixture f;
    auto* l = f.Model("A")->AddEffectLayer();
    l->AddEffect(0, "On", "X_Prop=a", "", 0, 100, 0, false);
    l->AddEffect(0, "On", "X_Prop=b", "", 100, 200, 0, false);
    l->AddEffect(0, "On", "X_Prop=a", "", 200, 300, 0, false);
    l->AddEffect(0, "On", "", "", 300, 400, 0, false);
    auto vals = f.se.GetUniqueEffectPropertyValues("X_Prop");
    CHECK(vals == std::list<std::string>{ "a", "b" });
}

TEST_CASE("SequenceElements used colours across rows", "[sequence][elements]") {
    SeqFixture f;
    auto* l = f.Model("A")->AddEffectLayer();
    l->AddEffect(0, "On", "", "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
    l->AddEffect(0, "On", "", "C_BUTTON_Palette1=#00ff00,C_CHECKBOX_Palette1=1", 100, 200, 0, false)->SetSelected(EFFECT_SELECTED);
    f.se.PopulateRowInformation();
    CHECK(f.se.GetUsedColours(false).size() == 2);
    auto sel = f.se.GetUsedColours(true);
    REQUIRE(sel.size() == 1);
    CHECK(sel[0] == "#00ff00");
}

TEST_CASE("SequenceElements ReplaceColours replaces matching palette entries", "[sequence][elements]") {
    SeqFixture f;
    auto* l = f.Model("A")->AddEffectLayer();
    Effect* e = l->AddEffect(0, "On", "", "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
    f.se.PopulateRowInformation();
    CHECK(f.se.ReplaceColours(&f.ctx, "#ff0000", "#0000ff", false) == 1);
    CHECK(e->GetPaletteAsString().find("#0000ff") != std::string::npos);
    CHECK(f.se.ReplaceColours(&f.ctx, "#123456", "#0000ff", false) == 0);
}

TEST_CASE("SequenceElements AddMissingModelsToSequence only adds models known to the layout", "[sequence][elements]") {
    SeqFixture f;
    f.Model("Existing");
    f.se.AddMissingModelsToSequence("A, B ,Existing");
    CHECK(f.se.GetElementCount() == 1);
    f.se.AddMissingModelsToSequence("");
    CHECK(f.se.GetElementCount() == 1);
}

TEST_CASE("SequenceElements RenderDependency tracking", "[sequence][elements]") {
    SeqFixture f;
    f.Model("Group");
    f.Model("Member");
    f.se.AddRenderDependency("Group", "Member");
    SUCCEED();
}

TEST_CASE("UndoManager starts with nothing to undo or redo", "[sequence][undo]") {
    SeqFixture f;
    auto& um = f.se.get_undo_mgr();
    CHECK_FALSE(um.CanUndo());
    CHECK_FALSE(um.CanRedo());
    CHECK_FALSE(um.ChangeCaptured());
    CHECK(um.GetUndoString() == "Undo");
    CHECK(um.GetRedoString() == "Redo");
    CHECK_FALSE(um.GetCaptureUndo());
    um.SetCaptureUndo(true);
    CHECK(um.GetCaptureUndo());
}

TEST_CASE("UndoManager marker alone is not a change", "[sequence][undo]") {
    SeqFixture f;
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    CHECK_FALSE(um.ChangeCaptured());
    CHECK_FALSE(um.CanUndo());
    um.CreateUndoStep();
    um.CreateUndoStep();
    CHECK_FALSE(um.CanUndo());
}

TEST_CASE("UndoManager undo and redo of an added effect", "[sequence][undo]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    auto* l = m->AddEffectLayer();
    Add(l, "On", 0, 100);
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    Effect* e = l->AddEffect(0, "Off", "X_A=1", "", 200, 300, 0, false);
    um.CaptureAddedEffect("Tree", l->GetIndex(), e->GetID());
    CHECK(um.ChangeCaptured());
    CHECK(um.CanUndo());
    CHECK(um.GetUndoString() == "Undo: Effect(s) Added");

    um.UndoLastStep();
    CHECK(l->GetEffectCount() == 1);
    CHECK(um.CanRedo());
    CHECK(um.GetRedoString() == "Redo: Effect(s) Deleted");

    um.RedoLastStep();
    REQUIRE(l->GetEffectCount() == 2);
    CHECK(l->GetEffect(1)->GetEffectName() == "Off");
    CHECK(l->GetEffect(1)->GetStartTimeMS() == 200);
    CHECK(l->GetEffect(1)->GetEndTimeMS() == 300);
    CHECK(l->GetEffect(1)->GetSetting("X_A") == "1");
}

TEST_CASE("UndoManager undo of a deletion restores the effect", "[sequence][undo]") {
    SeqFixture f;
    auto* l = f.Model("Tree")->AddEffectLayer();
    Effect* a = l->AddEffect(0, "On", "X_A=1", "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
    Add(l, "Off", 100, 200);
    a->SetSelected(EFFECT_SELECTED);
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    l->DeleteSelectedEffects(um);
    REQUIRE(l->GetEffectCount() == 1);
    CHECK(um.GetUndoString() == "Undo: Effect(s) Deleted");
    um.UndoLastStep();
    REQUIRE(l->GetEffectCount() == 2);
    CHECK(l->GetEffect(0)->GetEffectName() == "On");
    CHECK(l->GetEffect(0)->GetSetting("X_A") == "1");
    CHECK(l->GetEffect(0)->GetPaletteSize() == 1);
    um.RedoLastStep();
    CHECK(l->GetEffectCount() == 1);
}

TEST_CASE("UndoManager undo of RemoveAllEffects restores every effect", "[sequence][undo]") {
    SeqFixture f;
    auto* l = f.Model("Tree")->AddEffectLayer();
    Add(l, "On", 0, 100);
    Add(l, "Off", 100, 200);
    Add(l, "Bars", 200, 300);
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    l->RemoveAllEffects(&um);
    CHECK(l->GetEffectCount() == 0);
    um.UndoLastStep();
    CHECK(l->GetEffectCount() == 3);
}

TEST_CASE("UndoManager undo of a move restores the times", "[sequence][undo]") {
    SeqFixture f;
    auto* l = f.Model("Tree")->AddEffectLayer();
    Effect* e = Add(l, "On", 100, 200);
    e->SetSelected(EFFECT_SELECTED);
    l->TagAllSelectedEffects();
    auto& um = f.se.get_undo_mgr();
    um.SetCaptureUndo(true);
    um.CreateUndoStep();
    l->MoveAllSelectedEffects(500, um);
    CHECK(l->GetEffect(0)->GetStartTimeMS() == 600);
    CHECK(um.GetUndoString() == "Undo: Effect(s) Moved");
    um.UndoLastStep();
    CHECK(l->GetEffect(0)->GetStartTimeMS() == 100);
    CHECK(l->GetEffect(0)->GetEndTimeMS() == 200);
    um.RedoLastStep();
    CHECK(l->GetEffect(0)->GetStartTimeMS() == 600);
}

TEST_CASE("UndoManager undo of a modification restores settings and palette", "[sequence][undo]") {
    SeqFixture f;
    auto* l = f.Model("Tree")->AddEffectLayer();
    Effect* e = l->AddEffect(0, "On", "X_A=1", "C_BUTTON_Palette1=#ff0000,C_CHECKBOX_Palette1=1", 0, 100, 0, false);
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    um.CaptureModifiedEffect("Tree", l->GetIndex(), e);
    e->SetSettings("X_A=2", false);
    e->SetPalette("C_BUTTON_Palette1=#00ff00,C_CHECKBOX_Palette1=1");
    CHECK(um.GetUndoString() == "Undo: Effect(s) Modified");
    um.UndoLastStep();
    CHECK(l->GetEffect(0)->GetSetting("X_A") == "1");
    CHECK(l->GetEffect(0)->GetPalette()[0].Red() == 255);
    um.RedoLastStep();
    CHECK(l->GetEffect(0)->GetSetting("X_A") == "2");
}

TEST_CASE("UndoManager undo of an added layer removes it", "[sequence][undo]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    m->AddEffectLayer();
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    auto* l2 = m->AddEffectLayer();
    um.CaptureAddedLayer("Tree", l2->GetIndex(), 1);
    CHECK(um.GetUndoString() == "Undo: Layer(s) Added");
    um.UndoLastStep();
    CHECK(m->GetEffectLayerCount() == 1);
    CHECK(um.GetRedoString() == "Redo: Layer(s) Removed");
    um.RedoLastStep();
    CHECK(m->GetEffectLayerCount() == 2);
}

TEST_CASE("UndoManager undo never removes the last layer", "[sequence][undo]") {
    SeqFixture f;
    auto* m = f.Model("Tree");
    auto* l = m->AddEffectLayer();
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    um.CaptureAddedLayer("Tree", l->GetIndex(), 0);
    um.UndoLastStep();
    CHECK(m->GetEffectLayerCount() == 1);
}

TEST_CASE("UndoManager new work after an undo clears the redo list", "[sequence][undo]") {
    SeqFixture f;
    auto* l = f.Model("Tree")->AddEffectLayer();
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    Effect* e = Add(l, "On", 0, 100);
    um.CaptureAddedEffect("Tree", l->GetIndex(), e->GetID());
    um.UndoLastStep();
    REQUIRE(um.CanRedo());
    um.CreateUndoStep();
    CHECK_FALSE(um.CanRedo());
}

TEST_CASE("UndoManager undoes whole groups between markers", "[sequence][undo]") {
    SeqFixture f;
    auto* l = f.Model("Tree")->AddEffectLayer();
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    Effect* a = Add(l, "On", 0, 100);
    um.CaptureAddedEffect("Tree", l->GetIndex(), a->GetID());
    um.CreateUndoStep();
    Effect* b = Add(l, "Off", 100, 200);
    um.CaptureAddedEffect("Tree", l->GetIndex(), b->GetID());
    Effect* c = Add(l, "Bars", 200, 300);
    um.CaptureAddedEffect("Tree", l->GetIndex(), c->GetID());
    REQUIRE(l->GetEffectCount() == 3);
    um.UndoLastStep();
    CHECK(l->GetEffectCount() == 1);
    um.UndoLastStep();
    CHECK(l->GetEffectCount() == 0);
    CHECK_FALSE(um.CanUndo());
}

TEST_CASE("UndoManager Clear and CancelLastStep", "[sequence][undo]") {
    SeqFixture f;
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    um.CaptureAddedEffect("x", 0, 0);
    CHECK(um.CanUndo());
    um.CancelLastStep();
    um.RemoveUnusedMarkers();
    CHECK_FALSE(um.CanUndo());
    um.CaptureAddedEffect("x", 0, 0);
    CHECK(um.CanUndo());
    um.Clear();
    CHECK_FALSE(um.CanUndo());
    CHECK_FALSE(um.CanRedo());
    um.CancelLastStep();
}

TEST_CASE("UndoManager step cap drops the oldest steps", "[sequence][undo]") {
    SeqFixture f;
    auto& um = f.se.get_undo_mgr();
    CHECK(um.GetMaxSteps() == 0);
    for (int i = 0; i < 20; ++i) {
        um.CaptureEffectToBeMoved("x", 0, i, 0, 1);
    }
    um.SetMaxSteps(5);
    CHECK(um.GetMaxSteps() == 5);
    for (int i = 0; i < 20; ++i) {
        um.CaptureEffectToBeMoved("x", 0, i, 0, 1);
    }
    int undone = 0;
    while (um.CanUndo() && undone < 100) {
        um.UndoLastStep();
        ++undone;
    }
    CHECK(undone <= 1);
}

TEST_CASE("UndoManager undo of steps for a vanished element is harmless", "[sequence][undo]") {
    SeqFixture f;
    auto& um = f.se.get_undo_mgr();
    um.CreateUndoStep();
    um.CaptureAddedEffect("Gone", 0, 0);
    um.CaptureEffectToBeMoved("Gone", 0, 0, 0, 10);
    um.CaptureModifiedEffect("Gone", 0, 0, "", "");
    um.CaptureEffectToBeDeleted("Gone", 0, "On", "", "", 0, 10, 0, false);
    um.CaptureAddedLayer("Gone", 0, 0);
    um.CaptureRemovedLayer("Gone", 0);
    um.UndoLastStep();
    um.RedoLastStep();
    SUCCEED();
}

TEST_CASE("UndoManager info classes keep their fields", "[sequence][undo]") {
    DeletedEffectInfo d("E", 3, "On", "s", "p", 1, 2, 3, true);
    CHECK(d.element_name == "E");
    CHECK(d.layer_index == 3);
    CHECK(d.name == "On");
    CHECK(d.startTimeMS == 1);
    CHECK(d.endTimeMS == 2);
    CHECK(d.Selected == 3);
    CHECK(d.Protected);
    AddedEffectInfo a("E", 1, 9);
    CHECK(a.id == 9);
    MovedEffectInfo m("E", 1, 9, 5, 6);
    CHECK(m.startTimeMS == 5);
    CHECK(m.endTimeMS == 6);
    LayerInfo l("E", 4, 2);
    CHECK(l.exclusive_layer_index == 4);
    CHECK(l.layer_number == 2);
}
