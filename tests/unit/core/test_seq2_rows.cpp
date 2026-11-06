#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <iterator>
#include <list>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"

#include <pugixml.hpp>

#include <string>
#include <vector>

namespace {

// A sequence over a real layout: a 3-strand matrix with a submodel, a line and a
// group holding the line and the matrix submodel.
struct RowFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    ModelElement* mat = nullptr;
    ModelElement* line = nullptr;
    ModelElement* grp = nullptr;
    TimingElement* t1 = nullptr;
    TimingElement* t2 = nullptr;

    RowFixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
        Add("<model name=\"Mat\" DisplayAs=\"Horiz Matrix\" NumStrings=\"1\" NodesPerString=\"12\" StrandsPerString=\"3\" StringType=\"RGB Nodes\" StartChannel=\"1\">"
            "<subModel name=\"Sub\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-4\"/></model>");
        Add("<model name=\"Line\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"5\" StringType=\"RGB Nodes\" StartChannel=\"100\"/>");
        Add("<modelGroup name=\"G\" models=\"Line,Mat/Sub\"/>");
        ctx.AllModels.RecalcStartChannels();
        t1 = dynamic_cast<TimingElement*>(se.AddElement("T1", "timing", true, false, true, false, false));
        t1->AddEffectLayer();
        t2 = dynamic_cast<TimingElement*>(se.AddElement("T2", "timing", true, true, false, false, false));
        t2->AddEffectLayer();
        t2->AddEffectLayer();
        mat = dynamic_cast<ModelElement*>(se.AddElement("Mat", "model", true, false, true, false, false));
        grp = dynamic_cast<ModelElement*>(se.AddElement("G", "model", true, false, true, false, false));
        line = dynamic_cast<ModelElement*>(se.AddElement("Line", "model", true, false, true, false, false));
        for (auto* m : { mat, grp, line }) {
            if (m->GetEffectLayerCount() == 0) m->AddEffectLayer();
        }
    }
    void Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        REQUIRE(ctx.AllModels.createAndAddModel(doc.first_child(), 100, 100) != nullptr);
    }
    std::vector<std::string> Names() {
        std::vector<std::string> n;
        for (int i = 0; i < se.GetRowInformationSize(); ++i) n.push_back(se.GetRowInformation(i)->displayName);
        return n;
    }
};

Effect* Add(EffectLayer* l, const std::string& name, int s, int e, const std::string& settings = "") {
    return l->AddEffect(0, name, settings, "", s, e, 0, false);
}
} // namespace

TEST_CASE("Rows of a model expand into submodels, strands and nodes", "[seq2][rows]") {
    RowFixture f;
    f.se.PopulateRowInformation();
    REQUIRE(f.mat->GetStrandCount() == 3);
    REQUIRE(f.mat->GetSubModelCount() == 1);
    // collapsed timing T2 takes one row
    CHECK(f.se.GetNumberOfTimingRows() == 2);
    CHECK(f.se.HasPapagayoTiming());
    const int baseRows = f.se.GetRowInformationSize();

    f.mat->ShowStrands(true);
    f.mat->ShowSubModels(true);
    f.mat->GetStrand(1)->ShowNodes(true);
    f.se.PopulateRowInformation();
    // + submodel + 3 strands + 4 nodes of strand 1
    CHECK(f.se.GetRowInformationSize() == baseRows + 1 + 3 + 4);

    int subRow = -1, strandRow = -1, nodeRow = -1;
    for (int i = 0; i < f.se.GetRowInformationSize(); ++i) {
        auto* r = f.se.GetRowInformation(i);
        if (r->element->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL && subRow < 0) subRow = i;
        if (r->element == f.mat->GetStrand(1) && r->nodeIndex == -1 && strandRow < 0) strandRow = i;
        if (r->element == f.mat->GetStrand(1) && r->nodeIndex == 2) nodeRow = i;
    }
    REQUIRE(subRow >= 0);
    REQUIRE(strandRow >= 0);
    REQUIRE(nodeRow >= 0);
    CHECK(f.se.GetRowInformation(subRow)->nestDepth == 1);
    CHECK(f.se.GetRowInformation(strandRow)->strandIndex == 1);
    CHECK(f.se.GetRowInformation(nodeRow)->nestDepth == 2);
    CHECK(f.se.GetEffectLayer(subRow) == f.mat->GetSubModel("Sub")->GetEffectLayer(0));
    CHECK(f.se.GetEffectLayer(strandRow) == f.mat->GetStrand(1)->GetEffectLayer(0));
    CHECK(f.se.GetEffectLayer(nodeRow) == f.mat->GetStrand(1)->GetNodeLayer(2));
    CHECK(f.se.GetEffectLayer(-1) == nullptr);
    CHECK(f.se.GetEffectLayer(nullptr) == nullptr);

    // strands only
    f.mat->ShowSubModels(false);
    f.se.PopulateRowInformation();
    CHECK(f.se.GetRowInformationSize() == baseRows + 3 + 4);
    // a collapsed strand shows a single row
    f.mat->GetStrand(1)->ShowNodes(false);
    f.mat->GetStrand(0)->SetCollapsed(true);
    f.mat->GetStrand(0)->AddEffectLayer();
    f.se.PopulateRowInformation();
    CHECK(f.se.GetRowInformationSize() == baseRows + 3);
}

TEST_CASE("Unused submodels can be hidden once the model has effects", "[seq2][rows]") {
    RowFixture f;
    f.mat->ShowSubModels(true);
    f.se.SetHideUnusedSubmodels(true);
    f.se.PopulateRowInformation();
    const int withSub = f.se.GetRowInformationSize();
    Add(f.mat->GetEffectLayer(0), "On", 0, 100);
    f.se.PopulateRowInformation();
    CHECK(f.se.GetRowInformationSize() == withSub - 1);
    Add(f.mat->GetSubModel("Sub")->GetEffectLayer(0), "On", 0, 100);
    f.se.PopulateRowInformation();
    CHECK(f.se.GetRowInformationSize() == withSub);
    CHECK(f.se.GetHideUnusedSubmodels());
}

TEST_CASE("A group shown with strands lists its members nested", "[seq2][rows]") {
    RowFixture f;
    f.grp->ShowStrands(true);
    f.se.PopulateRowInformation();
    auto n = f.Names();
    auto g = std::find(n.begin(), n.end(), "G");
    REQUIRE(g != n.end());
    REQUIRE(std::next(g) != n.end());
    CHECK(*std::next(g) == "Line");
    CHECK(*std::next(g, 2) == "Mat/Sub");
    int gi = (int)std::distance(n.begin(), g);
    CHECK(f.se.GetRowInformation(gi + 1)->nestDepth == 1);
    CHECK(f.se.GetRowInformation(gi + 1)->submodel);
    CHECK(f.se.GetRowInformation(gi + 2)->nestDepth == 1);

    // a collapsed submodel inside the group still shows one row
    f.mat->GetSubModel("Sub")->SetCollapsed(true);
    f.mat->GetSubModel("Sub")->AddEffectLayer();
    f.se.PopulateRowInformation();
    CHECK(f.Names().size() == n.size());
}

TEST_CASE("Visible rows scroll the model rows under the timing rows", "[seq2][rows]") {
    RowFixture f;
    f.mat->ShowStrands(true);
    f.se.PopulateRowInformation();
    const int total = f.se.GetRowInformationSize();
    REQUIRE(total == 2 + 1 + 3 + 1 + 1);
    f.se.SetMaxRowsDisplayed(5);
    f.se.PopulateVisibleRowInformation();
    CHECK(f.se.GetVisibleRowInformationSize() == 5);

    CHECK(f.se.SetFirstVisibleModelRow(2) == 2);
    CHECK(f.se.GetFirstVisibleModelRow() == 2);
    // the last model rows are as far as it scrolls
    f.se.SetFirstVisibleModelRow(100);
    CHECK(f.se.GetFirstVisibleModelRow() == (total - 2) - (5 - 2));
    // timing rows always come first
    CHECK(f.se.GetVisibleRowInformation(0)->element == f.t1);
    auto* r3 = f.se.GetVisibleRowInformationFromRow(3);
    REQUIRE(r3 != nullptr);
    CHECK(r3->RowNumber == 3);
    CHECK(f.se.GetVisibleRowInformationFromRow(99) == nullptr);
    CHECK(f.se.GetRowInformationFromRow(4) == f.se.GetRowInformation(4));
    CHECK(f.se.GetRowInformationFromRow(-5) == nullptr);
    CHECK(f.se.GetVisibleRowInformation(99) == nullptr);

    // everything fits: back to the top
    f.se.SetMaxRowsDisplayed(50);
    f.se.SetFirstVisibleModelRow(3);
    CHECK(f.se.GetFirstVisibleModelRow() == 0);
}

TEST_CASE("Selecting effects by visible rows and timing marks", "[seq2][rows]") {
    RowFixture f;
    Add(f.t1->GetEffectLayer(0), "", 0, 100);
    Add(f.t1->GetEffectLayer(0), "", 100, 300);
    Add(f.t1->GetEffectLayer(0), "", 300, 600);
    Add(f.mat->GetEffectLayer(0), "On", 50, 150);
    Add(f.mat->GetEffectLayer(0), "On", 400, 500);
    Add(f.line->GetEffectLayer(0), "On", 120, 280);
    f.se.PopulateRowInformation();
    f.se.SetMaxRowsDisplayed(20);
    f.se.PopulateVisibleRowInformation();
    REQUIRE(f.se.GetSelectedTimingRow() == 0);
    CHECK(f.se.GetNumberOfActiveTimingEffects() == 3);

    // marks 1..1 cover 100..300 on every model row
    CHECK(f.se.SelectEffectsInRowAndColumnRange(2, -1, 1, 1) == 2);
    f.se.UnSelectAllEffects();
    CHECK(f.se.SelectEffectsInRowAndColumnRange(2, -1, 1, 7) == 0);
    CHECK(f.se.SelectVisibleEffectsInRowAndTimeRange(2, 99, 0, 1000) == 3);
    CHECK(f.se.SelectVisibleEffectsInRowAndTimeRange(99, 100, 0, 1000) == 0);

    f.se.SetSelectedTimingRow(-1);
    CHECK(f.se.GetNumberOfActiveTimingEffects() == 0);
    CHECK(f.se.SelectEffectsInRowAndColumnRange(2, -1, 0, 1) == 0);
}

TEST_CASE("Renaming a timing track updates effects on every layer kind", "[seq2][rows]") {
    RowFixture f;
    const std::string s = "E_CHOICE_Faces_TimingTrack=T1";
    Effect* onModel = Add(f.mat->GetEffectLayer(0), "Faces", 0, 100, s);
    Effect* onSub = Add(f.mat->GetSubModel("Sub")->GetEffectLayer(0), "Faces", 0, 100, s);
    Effect* onStrand = Add(f.mat->GetStrand(0)->GetEffectLayer(0), "Faces", 0, 100, s);
    Effect* onNode = Add(f.mat->GetStrand(2)->GetNodeLayer(1, true), "Faces", 0, 100, s);
    Effect* other = Add(f.line->GetEffectLayer(0), "Faces", 0, 100, "E_CHOICE_Faces_TimingTrack=T2");
    f.se.RenameTimingTrack("T1", "Lyrics");
    for (auto* e : { onModel, onSub, onStrand, onNode }) {
        CHECK(e->GetSetting("E_CHOICE_Faces_TimingTrack") == "Lyrics");
    }
    CHECK(other->GetSetting("E_CHOICE_Faces_TimingTrack") == "T2");
    CHECK(f.se.GetMaxEffectEndTimeMS() == 100);
}

TEST_CASE("AddMissingModelsToSequence adds the parent of a submodel", "[seq2][rows]") {
    HeadlessRenderContext ctx;
    auto& se = ctx.GetSequenceElements();
    se.SetViewsManager(&ctx._sequenceViewManager);
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<model name=\"Mat\" DisplayAs=\"Horiz Matrix\" NumStrings=\"1\" NodesPerString=\"8\" StrandsPerString=\"2\" StringType=\"RGB Nodes\" StartChannel=\"1\">"
                            "<subModel name=\"Sub\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-4\"/></model>"));
    REQUIRE(ctx.AllModels.createAndAddModel(doc.first_child(), 100, 100) != nullptr);
    se.AddMissingModelsToSequence("Mat/Sub, missing", false);
    REQUIRE(se.GetElementCount() == 1);
    CHECK(se.GetElement((size_t)0)->GetName() == "Mat");
    CHECK_FALSE(se.GetElement((size_t)0)->GetVisible());
    se.AddMissingModelsToSequence("Mat");
    CHECK(se.GetElementCount() == 1);
    se.AddMissingModelsToSequence("");
    CHECK(se.GetElementCount() == 1);
}
