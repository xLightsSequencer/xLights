#include <catch2/catch_test_macros.hpp>

#include <list>

#include "models/ModelManager.h"
#include "outputs/OutputManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceViewManager.h"
#include "render/TextDrawingContext.h"
#include "render/ValueCurveConsts.h"
#include "XmlSerializer/StringSerializingVisitor.h"

#include <pugixml.hpp>

#include <list>
#include <string>
#include <vector>

namespace {

std::vector<std::string> Models(const SequenceView* v) {
    auto l = v->GetModels();
    return { l.begin(), l.end() };
}

struct VMFixture {
    OutputManager om;
    ModelManager mm{ &om };
    SequenceViewManager vm{ &mm };
};

std::string SaveViews(const SequenceViewManager& vm) {
    StringSerializingVisitor v(false, false);
    vm.Save(v);
    std::string s = v.GetResult();
    auto p = s.find("?>");
    return p == std::string::npos ? s : s.substr(p + 2);
}

struct SeqFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    SequenceViewManager& vm;
    SeqFixture() : se(ctx.GetSequenceElements()), vm(ctx._sequenceViewManager) {
        vm.SetModelManager(&ctx.AllModels);
        se.SetViewsManager(&vm);
    }
    ModelElement* Model(const std::string& name) {
        auto* m = dynamic_cast<ModelElement*>(se.AddElement(name, "model", true, false, true, false, false));
        if (m != nullptr && m->GetEffectLayerCount() == 0) m->AddEffectLayer();
        return m;
    }
    TimingElement* Timing(const std::string& name) {
        auto* t = dynamic_cast<TimingElement*>(se.AddElement(name, "timing", true, false, true, false, false));
        if (t != nullptr && t->GetEffectLayerCount() == 0) t->AddEffectLayer();
        return t;
    }
    // Adds a view to both the view manager and the element view list, as the UI does.
    int View(const std::string& name, const std::string& models) {
        vm.AddView(name)->SetModels(models);
        se.AddView(name);
        int idx = vm.GetViewIndex(name);
        se.PopulateView(models, idx);
        return idx;
    }
    std::vector<std::string> Names(int view) {
        std::vector<std::string> n;
        for (size_t i = 0; i < se.GetElementCount(view); ++i) n.push_back(se.GetElement(i, view)->GetName());
        return n;
    }
};

Effect* Add(EffectLayer* l, const std::string& name, int start, int end, const std::string& settings = "", const std::string& palette = "") {
    return l->AddEffect(0, name, settings, palette, start, end, 0, false);
}

} // namespace

// ---------------------------------------------------------------------------------------------
// SequenceView
// ---------------------------------------------------------------------------------------------

TEST_CASE("SequenceView SetModels trims names", "[seq2][views]") {
    VMFixture f;
    SequenceView v("V", &f.mm);
    v.SetModels(" A , B,C ");
    CHECK(Models(&v) == std::vector<std::string>{ "A", "B", "C" });
    CHECK(v.GetModelsString() == "A,B,C");
    v.SetModels("");
    CHECK(v.GetModels().empty());
    CHECK(v.GetModelsString().empty());
}

TEST_CASE("SequenceView AddModel trims, ignores duplicates and honours position", "[seq2][views]") {
    VMFixture f;
    SequenceView v("V", &f.mm);
    v.AddModel(" A ");
    v.AddModel("C");
    v.AddModel("A");
    CHECK(Models(&v) == std::vector<std::string>{ "A", "C" });
    v.AddModel("B", 1);
    CHECK(Models(&v) == std::vector<std::string>{ "A", "B", "C" });
    v.AddModel("Z", 0);
    CHECK(Models(&v).front() == "Z");
    v.AddModel("End", 99);
    CHECK(Models(&v).back() == "End");
    CHECK(v.ContainsModel("B"));
    CHECK_FALSE(v.ContainsModel(" B"));
}

TEST_CASE("SequenceView insert and move relative to other models", "[seq2][views]") {
    VMFixture f;
    SequenceView v("V", &f.mm);
    v.SetModels("A,B,C,D");

    v.InsertModelAfter("X", "B");
    CHECK(Models(&v) == std::vector<std::string>{ "A", "B", "X", "C", "D" });
    v.InsertModelAfter("X", "");
    CHECK(Models(&v) == std::vector<std::string>{ "X", "A", "B", "C", "D" });
    v.InsertModelAfter("X", "missing");
    CHECK(Models(&v).back() == "X");

    v.InsertModelBefore("X", "B");
    CHECK(Models(&v) == std::vector<std::string>{ "A", "X", "B", "C", "D" });
    v.InsertModelBefore("X", "");
    CHECK(Models(&v).back() == "X");
    v.InsertModelBefore("Y", "missing");
    CHECK(Models(&v).back() == "Y");

    v.SetModels("A,B,C,D");
    v.MoveModelAfter("A", "C");
    CHECK(Models(&v) == std::vector<std::string>{ "B", "C", "A", "D" });
    v.MoveModelBefore("D", "B");
    CHECK(Models(&v) == std::vector<std::string>{ "D", "B", "C", "A" });
}

TEST_CASE("SequenceView rename and remove only touch exact names", "[seq2][views]") {
    VMFixture f;
    SequenceView v("V", &f.mm);
    v.SetModels("Arch,Arch 2,Tree");
    v.RenameModel("Arch", "Bow");
    CHECK(Models(&v) == std::vector<std::string>{ "Bow", "Arch 2", "Tree" });
    v.RenameModel("nothing", "x");
    CHECK(v.GetModelsString() == "Bow,Arch 2,Tree");
    v.RemoveModel("Arch 2");
    CHECK(v.GetModelsString() == "Bow,Tree");
    v.RemoveModel("nothing");
    CHECK(v.GetModelsString() == "Bow,Tree");
}

TEST_CASE("SequenceView loads from an xml node", "[seq2][views]") {
    VMFixture f;
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<view name=\"Front\" models=\"A,B\"/>"));
    SequenceView v(doc.child("view"), &f.mm);
    CHECK(v.GetName() == "Front");
    CHECK(Models(&v) == std::vector<std::string>{ "A", "B" });
    v.SetName("Back");
    CHECK(v.GetName() == "Back");
}

// ---------------------------------------------------------------------------------------------
// SequenceViewManager
// ---------------------------------------------------------------------------------------------

TEST_CASE("SequenceViewManager always has a master view first", "[seq2][views]") {
    VMFixture f;
    CHECK(f.vm.GetViewCount() == 0);
    auto views = f.vm.GetViews();
    REQUIRE(views.size() == 1);
    CHECK(views.front()->GetName() == "Master View");
    CHECK(f.vm.GetViewList().empty());
    CHECK(f.vm.GetViewList(false) == std::vector<std::string>{ "Master View" });
    CHECK(f.vm.GetView(0) == views.front());
    CHECK(f.vm.GetSelectedView() == views.front());
}

TEST_CASE("SequenceViewManager AddView returns the existing view for a duplicate name", "[seq2][views]") {
    VMFixture f;
    auto* a = f.vm.AddView("A");
    auto* b = f.vm.AddView("B");
    CHECK(f.vm.AddView("A") == a);
    CHECK(f.vm.GetViewCount() == 3);
    CHECK(f.vm.GetViewIndex("Master View") == 0);
    CHECK(f.vm.GetViewIndex("A") == 1);
    CHECK(f.vm.GetViewIndex("B") == 2);
    CHECK(f.vm.GetViewIndex("C") == -1);
    CHECK(f.vm.GetView("B") == b);
    CHECK(f.vm.GetView("C") == nullptr);
    CHECK(f.vm.GetView(3) == nullptr);
    CHECK(f.vm.GetView(-1) == nullptr);
    CHECK(f.vm.GetViewList() == std::vector<std::string>{ "A", "B" });
}

TEST_CASE("SequenceViewManager rename and delete views", "[seq2][views]") {
    VMFixture f;
    f.vm.AddView("A");
    f.vm.AddView("B");
    f.vm.RenameView("A", "Alpha");
    f.vm.RenameView("missing", "x");
    CHECK(f.vm.GetViewList() == std::vector<std::string>{ "Alpha", "B" });
    f.vm.DeleteView("Alpha");
    f.vm.DeleteView("missing");
    CHECK(f.vm.GetViewList() == std::vector<std::string>{ "B" });
    f.vm.DeleteView(f.vm.GetView("B"));
    CHECK(f.vm.GetViewList().empty());
}

TEST_CASE("SequenceViewManager model rename and delete apply to every view", "[seq2][views]") {
    VMFixture f;
    f.vm.AddView("A")->SetModels("X,Y");
    f.vm.AddView("B")->SetModels("Y,Z");
    f.vm.RenameModel("Y", "W");
    CHECK(f.vm.GetView("A")->GetModelsString() == "X,W");
    CHECK(f.vm.GetView("B")->GetModelsString() == "W,Z");
    f.vm.DeleteModel("W");
    CHECK(f.vm.GetView("A")->GetModelsString() == "X");
    CHECK(f.vm.GetView("B")->GetModelsString() == "Z");
}

TEST_CASE("SequenceViewManager moves views up and down", "[seq2][views]") {
    VMFixture f;
    f.vm.AddView("A")->SetModels("1");
    f.vm.AddView("B")->SetModels("2");
    f.vm.AddView("C")->SetModels("3");
    f.vm.MoveViewUp(3);
    CHECK(f.vm.GetViewList() == std::vector<std::string>{ "A", "C", "B" });
    CHECK(f.vm.GetView("C")->GetModelsString() == "3");
    f.vm.MoveViewDown(1);
    CHECK(f.vm.GetViewList() == std::vector<std::string>{ "C", "A", "B" });
    // no neighbour, or an invalid index, is a no-op
    f.vm.MoveViewDown(3);
    f.vm.MoveViewUp(99);
    f.vm.MoveViewDown(99);
    CHECK(f.vm.GetViewList() == std::vector<std::string>{ "C", "A", "B" });
}

TEST_CASE("SequenceViewManager Reset drops every view", "[seq2][views]") {
    VMFixture f;
    f.vm.AddView("A");
    f.vm.Reset();
    CHECK(f.vm.GetViewCount() == 0);
    CHECK(f.vm.GetViews().size() == 1);
}

TEST_CASE("SequenceViewManager view name validation", "[seq2][views]") {
    CHECK(SequenceViewManager::IsValidViewName("Front Yard_2-b"));
    CHECK(SequenceViewManager::IsValidViewName(""));
    CHECK_FALSE(SequenceViewManager::IsValidViewName("A,B"));
    CHECK_FALSE(SequenceViewManager::IsValidViewName("A/B"));
    CHECK_FALSE(SequenceViewManager::IsValidViewName("A.B"));
}

TEST_CASE("SequenceViewManager load sanitises comma view names into unique names", "[seq2][views]") {
    VMFixture f;
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<views><view name=\"A B\" models=\"\"/><view name=\"A,B\" models=\"X\"/><view name=\",,\" models=\"\"/><view name=\"A,,B\" models=\"\"/></views>"));
    f.vm.Load(doc.child("views"), 2);
    CHECK(f.vm.GetViewList() == std::vector<std::string>{ "A B", "A B 2", "View", "A B 3" });
    CHECK(f.vm.GetSelectedViewIndex() == 2);
    CHECK(f.vm.GetView("A B 2")->GetModelsString() == "X");
}

TEST_CASE("SequenceViewManager save skips the master view", "[seq2][views]") {
    VMFixture f;
    f.vm.AddView("One")->SetModels("A,B");
    auto s = SaveViews(f.vm);
    CHECK(s.find("Master View") == std::string::npos);
    CHECK(s.find("name=\"One\"") != std::string::npos);
    CHECK(s.find("models=\"A,B\"") != std::string::npos);
}

// ---------------------------------------------------------------------------------------------
// SequenceElements view handling
// ---------------------------------------------------------------------------------------------

TEST_CASE("SequenceElements PopulateView keeps only known elements in order", "[seq2][views]") {
    SeqFixture f;
    f.Model("A");
    f.Model("B");
    f.Model("C");
    int v = f.View("Front", "C,missing,A");
    CHECK(v == 1);
    CHECK(f.se.GetViewCount() == 2);
    CHECK(f.Names(v) == std::vector<std::string>{ "C", "A" });
    CHECK(f.se.GetViewName(v) == "Front");
    CHECK(f.se.GetViewName(0) == "Master View");
    CHECK(f.se.GetViewName(42).empty());
    CHECK(f.se.ElementExists("C", v));
    CHECK_FALSE(f.se.ElementExists("B", v));
    CHECK(f.se.GetElementIndex("A", v) == 1);
    // out of range view is ignored
    f.se.PopulateView("A", 9);
    f.se.PopulateView("A", -1);
    CHECK(f.Names(v) == std::vector<std::string>{ "C", "A" });
}

TEST_CASE("SequenceElements GetViewModels lists master models or the view string", "[seq2][views]") {
    SeqFixture f;
    f.Model("A");
    f.Timing("T");
    f.Model("B");
    CHECK(f.se.GetViewModels("Master View") == "A,B");
    f.View("V", "B,A");
    CHECK(f.se.GetViewModels("V") == "B,A");
}

TEST_CASE("SequenceElements SetCurrentView clamps bad indices to master", "[seq2][views]") {
    SeqFixture f;
    f.Model("A");
    int v = f.View("V", "A");
    f.se.SetCurrentView(v);
    CHECK(f.se.GetCurrentView() == v);
    CHECK(f.vm.GetSelectedViewIndex() == v);
    f.se.SetCurrentView(17);
    CHECK(f.se.GetCurrentView() == 0);
    CHECK(f.vm.GetSelectedViewIndex() == 0);
    f.se.SetCurrentView(-3);
    CHECK(f.se.GetCurrentView() == 0);
}

TEST_CASE("SequenceElements RemoveView drops the element list for that view", "[seq2][views]") {
    SeqFixture f;
    f.Model("A");
    f.View("V1", "A");
    f.View("V2", "A");
    REQUIRE(f.se.GetElementCount(2) == 1);
    f.se.RemoveView(1);
    CHECK(f.se.GetElementCount(1) == 1);
    CHECK(f.se.GetElementCount(2) == 0);
    CHECK(f.se.GetElement((size_t)0, 2) == nullptr);
}

TEST_CASE("SequenceElements timing tracks belong to views by name", "[seq2][views]") {
    SeqFixture f;
    f.Model("A");
    auto* t1 = f.Timing("Beats");
    auto* t2 = f.Timing("Lyrics");
    t1->SetViews("Base");
    t2->SetViews("Base");
    int v1 = f.View("V1", "A");
    int v2 = f.View("V2", "A");

    f.se.AddTimingToView("Beats", "V1");
    f.se.AddTimingToView("Beats", "V1");
    CHECK(t1->GetViews() == "Base,V1");
    CHECK(f.se.TimingIsPartOfView(t1, v1));
    CHECK_FALSE(f.se.TimingIsPartOfView(t1, v2));

    // not a timing element - ignored
    f.se.AddTimingToView("A", "V1");
    f.se.AddTimingToView("missing", "V1");

    f.se.AddViewToTimings({ "Beats", "Lyrics" }, "V2");
    CHECK(t1->GetViews() == "Base,V1,V2");
    CHECK(t2->GetViews() == "Base,V2");

    f.se.SetCurrentView(0);
    f.se.AddTimingToCurrentView("Lyrics");
    CHECK(t2->GetViews() == "Base,V2");
    f.se.SetCurrentView(v1);
    f.se.AddTimingToCurrentView("Lyrics");
    CHECK(t2->GetViews() == "Base,V2,V1");

    f.se.DeleteTimingFromView("Beats", v1);
    CHECK(t1->GetViews() == "Base,V2");
    f.se.DeleteTimingFromView("Beats", v1);
    CHECK(t1->GetViews() == "Base,V2");
    f.se.DeleteTimingFromView("A", v1);

    f.se.AddTimingToAllViews("Beats");
    CHECK(t1->GetViews() == "Base,V2,Master View,V1");
}

TEST_CASE("SequenceElements adding a view to a timing with no views has no empty entry", "[seq2]") {
    SeqFixture f;
    auto* t = f.Timing("Beats");
    f.View("V1", "");
    REQUIRE(t->GetViews().empty());
    f.se.AddTimingToView("Beats", "V1");
    CHECK(t->GetViews() == "V1");
}

TEST_CASE("SequenceElements DeleteTimingsFromView removes the view from every timing in it", "[seq2][views]") {
    SeqFixture f;
    auto* t1 = f.Timing("T1");
    auto* t2 = f.Timing("T2");
    f.Model("A");
    int v = f.View("V", "T1,T2,A");
    t1->SetViews("V,Other");
    t2->SetViews("Other");
    f.se.DeleteTimingsFromView(v);
    CHECK(t1->GetViews() == "Other");
    CHECK(t2->GetViews() == "Other");
}

TEST_CASE("SequenceElements SetTimingVisibility follows view membership", "[seq2][views]") {
    SeqFixture f;
    auto* inView = f.Timing("In");
    auto* outView = f.Timing("Out");
    auto* hidden = f.Timing("Hidden");
    inView->SetViews("V");
    hidden->SetViews("V");
    hidden->SetMasterVisible(false);

    f.se.SetTimingVisibility("V");
    CHECK(inView->GetVisible());
    CHECK_FALSE(outView->GetVisible());
    CHECK_FALSE(hidden->GetVisible());

    f.se.SetTimingVisibility("Master View");
    CHECK(inView->GetVisible());
    CHECK(outView->GetVisible());
    CHECK_FALSE(hidden->GetVisible());
}

TEST_CASE("SequenceElements hidden timing count uses the selected view", "[seq2][views]") {
    SeqFixture f;
    f.Timing("A");
    f.Timing("B");
    CHECK(f.se.GetHiddenTimingCount() == 0);
    f.se.HideAllTimingTracks(true);
    CHECK(f.se.GetHiddenTimingCount() == 2);
    CHECK_FALSE(f.se.GetTimingElement("A")->GetMasterVisible());
    f.se.HideAllTimingTracks(false);
    CHECK(f.se.GetHiddenTimingCount() == 0);
}

TEST_CASE("SequenceElements rows for a view include only its models and its timings", "[seq2][views]") {
    SeqFixture f;
    auto* t1 = f.Timing("T1");
    f.Timing("T2");
    f.Model("A");
    f.Model("B");
    f.Model("C");
    int v = f.View("V", "C,A");
    t1->SetViews("V");
    f.se.SetCurrentView(v);
    f.se.PopulateRowInformation();
    REQUIRE(f.se.GetRowInformationSize() == 3);
    CHECK(f.se.GetRowInformation(0)->element == t1);
    CHECK(f.se.GetRowInformation(1)->element->GetName() == "C");
    CHECK(f.se.GetRowInformation(2)->element->GetName() == "A");
    CHECK(f.se.GetNumberOfTimingRows() == 1);

    f.se.SetCurrentView(0);
    f.se.PopulateRowInformation();
    CHECK(f.se.GetRowInformationSize() == 5);
}

TEST_CASE("SequenceElements view element deletion and moves", "[seq2][views]") {
    SeqFixture f;
    f.Model("A");
    f.Model("B");
    f.Model("C");
    int v = f.View("V", "A,B,C");

    f.se.MoveElementDown("A", v);
    CHECK(f.Names(v) == std::vector<std::string>{ "B", "A", "C" });
    f.se.MoveElementUp("C", v);
    CHECK(f.Names(v) == std::vector<std::string>{ "B", "C", "A" });
    // already at the edge
    f.se.MoveElementUp("B", v);
    f.se.MoveElementDown("A", v);
    CHECK(f.Names(v) == std::vector<std::string>{ "B", "C", "A" });
    // master order is untouched by view moves
    CHECK(f.Names(0) == std::vector<std::string>{ "A", "B", "C" });

    // a destination past the end moves the element to the back
    f.se.MoveSequenceElement(0, 99, v);
    CHECK(f.Names(v) == std::vector<std::string>{ "C", "A", "B" });
    f.se.MoveSequenceElement(2, 0, v);
    CHECK(f.Names(v) == std::vector<std::string>{ "B", "C", "A" });
    f.se.MoveSequenceElement(0, 2, v);
    CHECK(f.Names(v) == std::vector<std::string>{ "C", "B", "A" });

    f.se.DeleteElementFromView("B", v);
    CHECK(f.Names(v) == std::vector<std::string>{ "C", "A" });
    CHECK(f.se.ElementExists("B"));

    f.vm.GetView("V")->SetModels("C,A");
    f.se.DeleteElement("A");
    CHECK(f.Names(v) == std::vector<std::string>{ "C" });
    CHECK(f.vm.GetView("V")->GetModelsString() == "C");
    CHECK_FALSE(f.se.ElementExists("A"));
}

TEST_CASE("SequenceElements master view change count tracks master edits only", "[seq2][views]") {
    SeqFixture f;
    f.Model("A");
    f.Model("B");
    int v = f.View("V", "A,B");
    auto c0 = f.se.GetMasterViewChangeCount();
    f.se.MoveSequenceElement(0, 99, v);
    f.se.DeleteElementFromView("A", v);
    CHECK(f.se.GetMasterViewChangeCount() == c0);
    f.se.MoveSequenceElement(0, 99, 0);
    CHECK(f.se.GetMasterViewChangeCount() > c0);
    auto c1 = f.se.GetMasterViewChangeCount();
    f.se.RenameModelInViews("B", "Bee");
    CHECK(f.se.GetMasterViewChangeCount() > c1);
    CHECK(f.se.GetElement("Bee") != nullptr);
    CHECK(f.Names(v) == std::vector<std::string>{ "Bee" });
}

TEST_CASE("SequenceElements visibility for all models of a view", "[seq2][views]") {
    SeqFixture f;
    auto* a = f.Model("A");
    auto* b = f.Model("B");
    int v = f.View("V", "B");
    f.se.SetVisibilityForAllModels(false, v);
    CHECK(a->GetVisible());
    CHECK_FALSE(b->GetVisible());
    f.se.SetVisibilityForAllModels(false);
    CHECK_FALSE(a->GetVisible());
}

TEST_CASE("SequenceElements model and timing list index mapping", "[seq2][views]") {
    SeqFixture f;
    f.Timing("T0");
    f.Model("M0");
    f.Timing("T1");
    f.Model("M1");
    CHECK(f.se.GetIndexOfModelFromModelIndex(0) == 1);
    CHECK(f.se.GetIndexOfModelFromModelIndex(1) == 3);
    CHECK(f.se.GetIndexOfModelFromModelIndex(2) == -1);
    CHECK(f.se.GetElementIndexOfTimingFromListIndex(0) == 0);
    CHECK(f.se.GetElementIndexOfTimingFromListIndex(1) == 2);
    CHECK(f.se.GetElementIndexOfTimingFromListIndex(2) == -1);
}

TEST_CASE("SequenceElements SortElements and MoveElement on the master view", "[seq2][views]") {
    SeqFixture f;
    f.Model("A");
    f.Model("B");
    f.Model("C");
    f.se.MoveElement(0, 3);
    std::vector<std::string> n = f.Names(0);
    CHECK(n == std::vector<std::string>{ "B", "C", "A" });
    f.se.MoveElement(2, 0);
    CHECK(f.Names(0) == std::vector<std::string>{ "A", "B", "C" });
}

// ---------------------------------------------------------------------------------------------
// SequenceElements effect queries
// ---------------------------------------------------------------------------------------------

TEST_CASE("SequenceElements effect lookups by element name, layer and time", "[seq2][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    a->AddEffectLayer();
    Add(a->GetEffectLayer(0), "On", 0, 1000, "X_Effect_Description=first");
    Add(a->GetEffectLayer(1), "Bars", 500, 1500, "X_Effect_Description=second");
    f.Timing("T");

    CHECK(f.se.GetElementLayerCount("A") == 2);
    std::list<int> layers;
    a->AddEffectLayer();
    CHECK(f.se.GetElementLayerCount("A", &layers) == 3);
    CHECK(layers == std::list<int>{ 0, 1 });
    CHECK(f.se.GetElementLayerCount("missing") == -1);
    CHECK(f.se.GetElementLayerCount("T") == -1);

    CHECK(f.se.GetElementLayerEffects("A", 1).size() == 1);
    CHECK(f.se.GetElementLayerEffects("A", 5).empty());
    CHECK(f.se.GetElementLayerEffects("missing", 0).empty());

    Effect* e = f.se.SelectEffectUsingElementLayerTime("A", 1, 700);
    REQUIRE(e != nullptr);
    CHECK(e->GetEffectName() == "Bars");
    CHECK(e->GetSelected() != 0);
    CHECK(f.se.SelectEffectUsingElementLayerTime("missing", 0, 0) == nullptr);

    Effect* d = f.se.SelectEffectUsingDescription("first");
    REQUIRE(d != nullptr);
    CHECK(d->GetEffectName() == "On");
    CHECK(f.se.SelectEffectUsingDescription("none") == nullptr);

    auto descs = f.se.GetAllEffectDescriptions();
    CHECK(descs == std::list<std::string>{ "first", "second" });
}

TEST_CASE("SequenceElements used effect types include submodels and strands", "[seq2][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    Add(a->GetEffectLayer(0), "On", 0, 100);
    Add(a->GetEffectLayer(0), "On", 100, 200);
    auto* sub = a->GetSubModel("Arm", true);
    REQUIRE(sub != nullptr);
    Add(sub->GetEffectLayer(0), "Twinkle", 0, 100);
    auto* strand = a->GetStrand(1, true);
    REQUIRE(strand != nullptr);
    Add(strand->GetEffectLayer(0), "Bars", 0, 100);
    auto types = f.se.GetAllUsedEffectTypes();
    CHECK(types == std::list<std::string>{ "On", "Twinkle", "Bars" });
}

TEST_CASE("SequenceElements media reference rewriting", "[seq2][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    auto* b = f.Model("B");
    Add(a->GetEffectLayer(0), "Pictures", 100, 400, "E_TEXTCTRL_Pictures_Filename=/abs/pic.png");
    Add(a->GetEffectLayer(0), "Pictures", 1000, 1200, "E_TEXTCTRL_Pictures_Filename=/abs/pic.png");
    Add(b->GetEffectLayer(0), "Pictures", 0, 50, "E_TEXTCTRL_Pictures_Filename=/abs/other.png");
    auto* sub = a->GetSubModel("Sub", true);
    Add(sub->GetEffectLayer(0), "Pictures", 2000, 2500, "E_TEXTCTRL_Pictures_Filename=/abs/pic.png");

    CHECK(f.se.RewriteMediaReferences("same", "same").empty());

    auto values = f.se.GetEffectSettingValues();
    CHECK(values.count("/abs/pic.png") == 1);
    CHECK(values.count("/abs/other.png") == 1);

    auto dirty = f.se.RewriteMediaReferences("/abs/pic.png", "pic.png");
    REQUIRE(dirty.size() == 1);
    REQUIRE(dirty.count("A") == 1);
    CHECK(dirty["A"].first == 100);
    CHECK(dirty["A"].second == 2500);
    CHECK(a->GetEffectLayer(0)->GetEffect(0)->GetSetting("E_TEXTCTRL_Pictures_Filename") == "pic.png");
    CHECK(sub->GetEffectLayer(0)->GetEffect(0)->GetSetting("E_TEXTCTRL_Pictures_Filename") == "pic.png");
    CHECK(b->GetEffectLayer(0)->GetEffect(0)->GetSetting("E_TEXTCTRL_Pictures_Filename") == "/abs/other.png");
}

TEST_CASE("SequenceElements RemoveUnreferencedMedia keeps referenced and metadata media", "[seq2][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    Add(a->GetEffectLayer(0), "Pictures", 0, 100, "E_TEXTCTRL_Pictures_Filename=used.png");
    auto& media = f.se.GetSequenceMedia();
    xlImage img(2, 2);
    media.AddEmbeddedImage("used.png", img);
    media.AddEmbeddedImage("unused.png", img);
    media.AddEmbeddedImage("meta.png", img);
    media.MarkUsedByMetadata("meta.png");
    CHECK(f.se.RemoveUnreferencedMedia() == 1);
    CHECK(media.HasImage("used.png"));
    CHECK(media.HasImage("meta.png"));
    CHECK_FALSE(media.HasImage("unused.png"));
}

TEST_CASE("SequenceElements MakeMediaPathRelative rekeys media inside the show folder", "[seq2][elements]") {
    SeqFixture f;
    f.ctx.showDirectory = "/show/dir";
    CHECK(f.se.MakeMediaPathRelative("").empty());
    CHECK(f.se.MakeMediaPathRelative("/elsewhere/x.png") == "/elsewhere/x.png");

    auto* a = f.Model("A");
    Add(a->GetEffectLayer(0), "Pictures", 0, 100, "E_TEXTCTRL_Pictures_Filename=/show/dir/img/x.png");
    xlImage img(2, 2);
    f.se.GetSequenceMedia().AddEmbeddedImage("/show/dir/img/x.png", img);
    std::string key = f.se.MakeMediaPathRelative("/show/dir/img/x.png");
    CHECK(key == "img/x.png");
    CHECK(f.se.GetSequenceMedia().HasImage("img/x.png"));
    CHECK_FALSE(f.se.GetSequenceMedia().HasImage("/show/dir/img/x.png"));
    CHECK(a->GetEffectLayer(0)->GetEffect(0)->GetSetting("E_TEXTCTRL_Pictures_Filename") == "img/x.png");

    // the key is already taken, so the original path is kept
    f.se.GetSequenceMedia().AddEmbeddedImage("/show/dir/img/y.png", img);
    f.se.GetSequenceMedia().AddEmbeddedImage("img/y.png", img);
    CHECK(f.se.MakeMediaPathRelative("/show/dir/img/y.png") == "/show/dir/img/y.png");
}

TEST_CASE("SequenceElements RenameModelFaceReferences updates Faces effects on the model", "[seq2][elements]") {
    SeqFixture f;
    auto* a = f.Model("A");
    auto* b = f.Model("B");
    Add(a->GetEffectLayer(0), "Faces", 0, 100, "E_CHOICE_Faces_FaceDefinition=Old");
    Add(a->GetEffectLayer(0), "On", 100, 200, "E_CHOICE_Faces_FaceDefinition=Old");
    Add(a->GetEffectLayer(0), "Faces", 200, 300, "E_CHOICE_Faces_FaceDefinition=Other");
    auto* sub = a->GetSubModel("S", true);
    Add(sub->GetEffectLayer(0), "Faces", 0, 100, "E_CHOICE_Faces_FaceDefinition=Old");
    Add(b->GetEffectLayer(0), "Faces", 0, 100, "E_CHOICE_Faces_FaceDefinition=Old");

    CHECK(f.se.RenameModelFaceReferences("A", "Old", "Old") == 0);
    CHECK(f.se.RenameModelFaceReferences("A", "", "New") == 0);
    CHECK(f.se.RenameModelFaceReferences("missing", "Old", "New") == 0);
    auto before = f.se.GetChangeCount();
    CHECK(f.se.RenameModelFaceReferences("A", "Old", "New") == 2);
    CHECK(f.se.GetChangeCount() > before);
    CHECK(a->GetEffectLayer(0)->GetEffect(0)->GetSetting("E_CHOICE_Faces_FaceDefinition") == "New");
    CHECK(a->GetEffectLayer(0)->GetEffect(1)->GetSetting("E_CHOICE_Faces_FaceDefinition") == "Old");
    CHECK(sub->GetEffectLayer(0)->GetEffect(0)->GetSetting("E_CHOICE_Faces_FaceDefinition") == "New");
    CHECK(b->GetEffectLayer(0)->GetEffect(0)->GetSetting("E_CHOICE_Faces_FaceDefinition") == "Old");
}

TEST_CASE("SequenceElements timing changes queue dependent models for render", "[seq2][elements]") {
    SeqFixture f;
    auto* t = f.Timing("Beats");
    f.Model("A");
    f.Model("B");
    std::vector<Element*> out;
    CHECK_FALSE(f.se.GetElementsToRender(out));

    f.se.AddRenderDependency("Beats", "A");
    f.se.AddRenderDependency("Beats", "gone");
    t->GetEffectLayer(0)->AddEffect(0, "", "", "", 100, 200, 0, false);
    f.se.IncrementChangeCount(t);
    REQUIRE(f.se.GetElementsToRender(out));
    REQUIRE(out.size() == 1);
    CHECK(out[0]->GetName() == "A");
    out.clear();
    CHECK_FALSE(f.se.GetElementsToRender(out));

    // a model change never queues anything
    f.se.IncrementChangeCount(f.se.GetElement("B"));
    CHECK_FALSE(f.se.GetElementsToRender(out));
}

// ---------------------------------------------------------------------------------------------
// TextDrawingContext descriptor parsing
// ---------------------------------------------------------------------------------------------

TEST_CASE("Font descriptor parsing reads face, style and size", "[seq2][text]") {
    auto i = TextDrawingContext::ParseFontDescriptor("Arial Bold Italic 14", "Def");
    CHECK(i.faceName == "Arial");
    CHECK(i.bold);
    CHECK(i.italic);
    CHECK_FALSE(i.light);
    CHECK(i.pixelSize == 14);
    CHECK_FALSE(i.antiAliased);

    i = TextDrawingContext::ParseFontDescriptor("Times New Roman underlined strikethrough 10.6", "Def");
    CHECK(i.faceName == "Times New Roman");
    CHECK(i.underlined);
    CHECK(i.strikethrough);
    CHECK(i.pixelSize == 11);

    i = TextDrawingContext::ParseFontDescriptor("Helvetica light 12px", "Def");
    CHECK(i.light);
    CHECK(i.pixelSize == 12);
    i = TextDrawingContext::ParseFontDescriptor("Helvetica thin 8PT", "Def");
    CHECK(i.light);
    CHECK(i.pixelSize == 8);
    i = TextDrawingContext::ParseFontDescriptor("Helvetica oblique heavy 2em", "Def");
    CHECK(i.italic);
    CHECK(i.bold);
    CHECK(i.pixelSize == 2);
}

TEST_CASE("Font descriptor parsing of quoted families and charset markers", "[seq2][text]") {
    auto i = TextDrawingContext::ParseFontDescriptor("'gill sans' 36 macroman", "Def");
    CHECK(i.faceName == "gill sans");
    CHECK(i.pixelSize == 36);
    i = TextDrawingContext::ParseFontDescriptor("bold\"Comic Sans MS\" 9 utf-8", "Def");
    CHECK(i.faceName == "Comic Sans MS");
    CHECK(i.bold);
    CHECK(i.pixelSize == 9);
    i = TextDrawingContext::ParseFontDescriptor("Courier\t12 iso-8859-1 windows-1252 default mac", "Def");
    CHECK(i.faceName == "Courier");
    CHECK(i.pixelSize == 12);
}

TEST_CASE("Font descriptor parsing ignores weight and width words that are not flags", "[seq2][text]") {
    auto i = TextDrawingContext::ParseFontDescriptor("Segoe UI Semi Bold 10", "Def");
    CHECK(i.faceName == "Segoe UI");
    CHECK_FALSE(i.bold);
    CHECK(i.pixelSize == 10);
    i = TextDrawingContext::ParseFontDescriptor("Font semi light condensed regular medium book normal expanded semicondensed 7", "Def");
    CHECK(i.faceName == "Font");
    CHECK_FALSE(i.light);
    CHECK_FALSE(i.bold);
    // a trailing "semi" with nothing after it is part of the face name
    i = TextDrawingContext::ParseFontDescriptor("Font semi", "Def");
    CHECK(i.faceName == "Font semi");
    CHECK(i.pixelSize == 9);
}

TEST_CASE("Font descriptor parsing defaults", "[seq2][text]") {
    auto i = TextDrawingContext::ParseFontDescriptor("", "Def");
    CHECK(i == TextFontInfo{});
    i = TextDrawingContext::ParseFontDescriptor("   ", "Def");
    CHECK(i == TextFontInfo{});
    i = TextDrawingContext::ParseFontDescriptor("bold 20", "Def");
    CHECK(i.faceName == "Def");
    CHECK(i.pixelSize == 20);
    // no size: wx fallback of 9; a number with an unknown unit is part of the name
    i = TextDrawingContext::ParseFontDescriptor("Arial -5 12abc", "Def");
    CHECK(i.pixelSize == 9);
    // "-5" has a digit and a hyphen, so it is dropped as a charset marker
    CHECK(i.faceName == "Arial 12abc");
}

TEST_CASE("Shape font descriptor forces reference size, light and native emoji font", "[seq2][text]") {
    auto i = TextDrawingContext::ParseShapeFontDescriptor("'Segoe UI Emoji' bold 40", "Def");
    CHECK(i.faceName == NATIVE_EMOJI_FONT);
    CHECK(i.pixelSize == 12);
    CHECK(i.light);
    CHECK(i.bold);
    i = TextDrawingContext::ParseShapeFontDescriptor("'Noto Color Emoji' 40", "Def");
    CHECK(i.faceName == NATIVE_EMOJI_FONT);
    i = TextDrawingContext::ParseShapeFontDescriptor("Arial 40", "Def");
    CHECK(i.faceName == "Arial");
    CHECK(i.pixelSize == 12);
}

TEST_CASE("TextFontInfo equality compares every field", "[seq2][text]") {
    TextFontInfo a, b;
    CHECK(a == b);
    b.slant = true;
    CHECK(a != b);
    b = a;
    b.antiAliased = true;
    CHECK(a != b);
}

namespace {
struct NullTextContext : TextDrawingContext {
    static int live;
    NullTextContext() { ++live; }
    ~NullTextContext() override { --live; }
    void ResetSize(int, int) override {}
    size_t GetWidth() const override { return 0; }
    size_t GetHeight() const override { return 0; }
    void Clear() override {}
    const uint8_t* FlushAndGetImage(int* w, int* h) override { *w = *h = 0; return nullptr; }
    void SetFont(const TextFontInfo&, const xlColor&) override {}
    void DrawText(const std::string&, int, int, double) override {}
    void DrawText(const std::string&, int, int) override {}
    void GetTextExtent(const std::string&, double* w, double* h) override { *w = *h = 0; }
    void GetTextExtents(const std::string&, std::vector<double>&) override {}
    void SetOverlayMode(bool) override {}
};
int NullTextContext::live = 0;
}

TEST_CASE("TextDrawingContext pool reuses released contexts and frees them on cleanup", "[seq2][text]") {
    struct Restore {
        ~Restore() {
            TextDrawingContext::CleanUp();
            TextDrawingContext::RegisterFactory(nullptr, nullptr, nullptr);
        }
    } restore;
    TextDrawingContext::CleanUp();
    TextDrawingContext::RegisterFactory(nullptr, nullptr, nullptr);
    // no factory: no pool and the parsers return defaults
    TextDrawingContext::Initialize();
    CHECK(TextDrawingContext::GetContext() == nullptr);
    TextDrawingContext::ReleaseContext(nullptr);
    CHECK(TextDrawingContext::GetTextFont("Arial 9") == TextFontInfo{});
    CHECK(TextDrawingContext::GetShapeFont("Arial 9") == TextFontInfo{});

    TextDrawingContext::RegisterFactory(
        [](int, int, bool) -> TextDrawingContext* { return new NullTextContext(); },
        [](const std::string& s) { return TextDrawingContext::ParseFontDescriptor(s, "T"); },
        [](const std::string& s) { return TextDrawingContext::ParseShapeFontDescriptor(s, "S"); });
    TextDrawingContext::Initialize();
    TextDrawingContext::Initialize(); // second call keeps the existing pool
    CHECK(TextDrawingContext::GetTextFont("bold 5").faceName == "T");
    CHECK(TextDrawingContext::GetShapeFont("bold 5").faceName == "S");

    int base = NullTextContext::live;
    auto* c1 = TextDrawingContext::GetContext();
    auto* c2 = TextDrawingContext::GetContext();
    REQUIRE(c1 != nullptr);
    CHECK(c1 != c2);
    CHECK(NullTextContext::live == base + 2);
    TextDrawingContext::ReleaseContext(c1);
    CHECK(TextDrawingContext::GetContext() == c1);
    TextDrawingContext::ReleaseContext(c1);
    TextDrawingContext::ReleaseContext(c2);
    TextDrawingContext::CleanUp();
    CHECK(NullTextContext::live == base);
    CHECK(TextDrawingContext::GetContext() == nullptr);
}

// ---------------------------------------------------------------------------------------------
// ValueCurveConsts
// ---------------------------------------------------------------------------------------------

TEST_CASE("ValueCurveConsts buffer ranges", "[seq2][valuecurve]") {
    using namespace ValueCurveConsts;
    struct Row { const char* n; double mn, mx; int div; };
    const Row rows[] = {
        { "B_VALUECURVE_Blur", 1, 15, 1 },
        { "B_VALUECURVE_Rotation", 0, 100, 1 },
        { "B_VALUECURVE_Zoom", 0, 30, 10 },
        { "B_VALUECURVE_Rotations", 0, 200, 10 },
        { "B_VALUECURVE_PivotPointX", 0, 100, 1 },
        { "B_VALUECURVE_PivotPointY", 0, 100, 1 },
        { "B_VALUECURVE_XRotation", 0, 360, 1 },
        { "B_VALUECURVE_YRotation", 0, 360, 1 },
        { "B_VALUECURVE_XPivot", 0, 100, 1 },
        { "B_VALUECURVE_YPivot", 0, 100, 1 },
    };
    for (const auto& r : rows) {
        INFO(r.n);
        CHECK(GetBufferSettingVCMin(r.n) == r.mn);
        CHECK(GetBufferSettingVCMax(r.n) == r.mx);
        CHECK(GetBufferSettingVCDivisor(r.n) == r.div);
    }
}

TEST_CASE("ValueCurveConsts color and timing ranges", "[seq2][valuecurve]") {
    using namespace ValueCurveConsts;
    struct Row { const char* n; double mn, mx; };
    const Row colors[] = {
        { "C_VALUECURVE_Brightness", 0, 400 },
        { "C_VALUECURVE_SparkleFrequency", 0, 200 },
        { "C_VALUECURVE_Color_ValueAdjust", -100, 100 },
        { "C_VALUECURVE_Color_HueAdjust", -100, 100 },
        { "C_VALUECURVE_Color_SaturationAdjust", -100, 100 },
    };
    for (const auto& r : colors) {
        INFO(r.n);
        CHECK(GetColorSettingVCMin(r.n) == r.mn);
        CHECK(GetColorSettingVCMax(r.n) == r.mx);
        CHECK(GetColorSettingVCDivisor(r.n) == 1);
    }
    CHECK(GetTimingSettingVCMin("T_VALUECURVE_In_Transition_Adjust") == 0);
    CHECK(GetTimingSettingVCMax("T_VALUECURVE_In_Transition_Adjust") == 100);
    CHECK(GetTimingSettingVCMin("T_VALUECURVE_Out_Transition_Adjust") == 0);
    CHECK(GetTimingSettingVCMax("T_VALUECURVE_Out_Transition_Adjust") == 100);
    CHECK(GetTimingSettingVCDivisor("T_VALUECURVE_In_Transition_Adjust") == 1);
}
