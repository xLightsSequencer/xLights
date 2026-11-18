#include <catch2/catch_test_macros.hpp>

#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceIds.h"
#include "render/SequenceMutationGuard.h"
#include "render/UndoManager.h"

#include <memory>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

// Stable identity for sequence objects (SequenceIds.h) and undo built on it.

namespace {
struct SeqFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    SeqFixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
    }
    ModelElement* Model(const std::string& name = "Tree") {
        SeqMutationGuard::Scope setup;
        return dynamic_cast<ModelElement*>(se.AddElement(name, "model", true, false, true, false, false));
    }
    TimingElement* Timing(const std::string& name = "Beats") {
        SeqMutationGuard::Scope setup;
        return dynamic_cast<TimingElement*>(se.AddElement(name, "timing", true, false, true, false, false));
    }
    UndoManager& Undo() { return se.get_undo_mgr(); }
    void RequireValid() {
        std::string problem;
        bool ok = se.ValidateUids(&problem);
        INFO(problem);
        REQUIRE(ok);
    }
};

// Direct (unrecorded) setup edits, as an import or a script makes them.
Effect* Add(EffectLayer* l, const std::string& name, int start, int end) {
    SeqMutationGuard::Scope setup;
    return l->AddEffect(0, name, "", "", start, end, 0, false);
}

EffectLayer* Layer0(Element* e) {
    SeqMutationGuard::Scope setup;
    return e->GetEffectLayerCount() > 0 ? e->GetEffectLayer(0) : e->AddEffectLayer();
}

EffectLayer* AddLayer(Element* e) {
    SeqMutationGuard::Scope setup;
    return e->AddEffectLayer();
}
} // namespace

TEST_CASE("SeqIds are unique, non-zero and carry the peer prefix", "[seq][identity]") {
    uint16_t oldPeer = SeqIds::GetPeer();
    std::unordered_set<SeqObjectId> ids;
    for (int i = 0; i < 10000; ++i) {
        SeqObjectId id = SeqIds::Next();
        CHECK(id != kNoSeqObjectId);
        CHECK(ids.insert(id).second);
    }
    SeqIds::SetPeer(7);
    SeqObjectId p = SeqIds::Next();
    CHECK(SeqIds::PeerOf(p) == 7);
    CHECK(ids.count(p) == 0);
    SeqIds::SetPeer(oldPeer);
}

TEST_CASE("Effect ids survive sorting while positions renumber", "[seq][identity]") {
    SeqFixture f;
    EffectLayer* l = Layer0(f.Model());
    Effect* late = Add(l, "On", 500, 600);
    SeqObjectId lateUid = late->GetUid();
    CHECK(late->GetID() == 0);
    Effect* early = Add(l, "Off", 0, 100); // sorts in front of `late`
    CHECK(late->GetID() == 1);             // position changed...
    CHECK(late->GetUid() == lateUid);      // ...identity did not
    CHECK(early->GetUid() != lateUid);
    CHECK(l->GetEffectFromUid(lateUid) == late);
    CHECK(l->GetEffectFromUid(early->GetUid()) == early);
    CHECK(l->GetEffectFromUid(kNoSeqObjectId) == nullptr);

    {
        SeqMutationGuard::Scope direct;
        REQUIRE(l->DeleteEffectByUid(early->GetUid()));
        CHECK_FALSE(l->DeleteEffectByUid(12345678));
    }
    CHECK(late->GetID() == 0);
    CHECK(l->GetEffectFromUid(lateUid) == late);
    f.RequireValid();
}

TEST_CASE("A render copy of an effect keeps its identity", "[seq][identity]") {
    SeqFixture f;
    Effect* e = Add(Layer0(f.Model()), "On", 0, 100);
    std::unique_ptr<Effect> copy(new Effect(*e));
    CHECK(copy->GetUid() == e->GetUid());
}

TEST_CASE("Uid lookups reach submodel, strand and node layers", "[seq][identity]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    SubModelElement* sub = m->GetSubModel("Sub", true);
    StrandElement* strand = m->GetStrand(0, true);
    NodeLayer* node = strand->GetNodeLayer(2, true);
    TimingElement* t = f.Timing();

    CHECK(f.se.GetElementFromUid(m->GetUid()) == m);
    CHECK(f.se.GetElementFromUid(sub->GetUid()) == sub);
    CHECK(f.se.GetElementFromUid(strand->GetUid()) == strand);
    CHECK(f.se.GetElementFromUid(t->GetUid()) == t);
    CHECK(f.se.GetElementFromUid(kNoSeqObjectId) == nullptr);

    CHECK(m->GetEffectLayerFromUid(Layer0(m)->GetUid()) == Layer0(m));
    CHECK(m->GetEffectLayerFromUid(Layer0(sub)->GetUid()) == Layer0(sub));
    CHECK(m->GetEffectLayerFromUid(Layer0(strand)->GetUid()) == Layer0(strand));
    CHECK(m->GetEffectLayerFromUid(node->GetUid()) == node);
    CHECK(strand->GetEffectLayerFromUid(node->GetUid()) == node);
    CHECK(sub->GetEffectLayerFromUid(node->GetUid()) == nullptr);
    CHECK(t->GetEffectLayerFromUid(node->GetUid()) == nullptr);
    f.RequireValid();
}

TEST_CASE("Undo of a modify finds its effect after an uncaptured insert shifts positions", "[seq][identity][undo]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    EffectLayer* l = Layer0(m);
    Effect* a = Add(l, "On", 0, 100);
    Effect* b = Add(l, "On", 200, 300);
    {
        SeqMutationGuard::Scope setup;
        a->SetSetting("E_SLIDER_X", "1");
        b->SetSetting("E_SLIDER_X", "2");
    }

    f.Undo().CreateUndoStep();
    f.se.GetCommandExecutor().SetEffectSetting(b, "E_SLIDER_X", std::string("99"));

    // Something that records no undo inserts an effect ahead of b, so b's
    // old position now names a.
    Add(l, "Off", 120, 150);
    REQUIRE(l->GetEffect(1) != b);

    f.Undo().UndoLastStep();
    CHECK(b->GetSetting("E_SLIDER_X") == "2");
    CHECK(a->GetSetting("E_SLIDER_X") == "1");
}

TEST_CASE("Undo of a delete restores the effect's identity; redo deletes it again", "[seq][identity][undo]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    EffectLayer* l = Layer0(m);
    Add(l, "On", 0, 100);
    Effect* victim = Add(l, "Bars", 200, 300);
    SeqObjectId uid = victim->GetUid();

    victim->SetSelected(EFFECT_SELECTED);
    f.Undo().CreateUndoStep();
    l->DeleteSelectedEffects(f.Undo());
    REQUIRE(l->GetEffectFromUid(uid) == nullptr);

    f.Undo().UndoLastStep();
    Effect* back = l->GetEffectFromUid(uid);
    REQUIRE(back != nullptr);
    CHECK(back->GetEffectName() == "Bars");
    CHECK(back->GetStartTimeMS() == 200);
    f.RequireValid();

    REQUIRE(f.Undo().CanRedo());
    f.Undo().RedoLastStep();
    CHECK(l->GetEffectFromUid(uid) == nullptr);
    CHECK(l->GetEffectCount() == 1);

    f.Undo().UndoLastStep();
    CHECK(l->GetEffectFromUid(uid) != nullptr);
    f.RequireValid();
}

TEST_CASE("Undo across a removed layer restores the layer and the effects deleted from it", "[seq][identity][undo]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    Layer0(m);
    EffectLayer* second = AddLayer(m);
    SeqObjectId layerUid = second->GetUid();
    Effect* e = Add(second, "Bars", 0, 100);
    e->SetSelected(EFFECT_SELECTED);

    // Step 1: delete the only effect on the second layer.
    f.Undo().CreateUndoStep();
    second->DeleteSelectedEffects(f.Undo());
    // Step 2: remove the now-empty layer.
    f.Undo().CreateUndoStep();
    REQUIRE(f.se.GetCommandExecutor().RemoveLayer(m, 1));
    REQUIRE(m->GetEffectLayerCount() == 1);

    f.Undo().UndoLastStep(); // layer comes back under the same id
    REQUIRE(m->GetEffectLayerCount() == 2);
    CHECK(m->GetEffectLayer(1)->GetUid() == layerUid);

    f.Undo().UndoLastStep(); // so the effect deleted from it can be restored
    CHECK(m->GetEffectLayer(1)->GetEffectCount() == 1);
    f.RequireValid();
}

TEST_CASE("Undo still finds a timing track after it is renamed", "[seq][identity][undo]") {
    SeqFixture f;
    TimingElement* t = f.Timing("Beats");
    EffectLayer* l = Layer0(t);
    f.Undo().CreateUndoStep();
    REQUIRE(f.se.GetCommandExecutor().AddEffect(l, "", "", "", 0, 100, 0, false) != nullptr);

    REQUIRE(ElementEdits::RenameTimingTrack(f.se, "Beats", "Downbeats", false));

    f.Undo().UndoLastStep();
    CHECK(l->GetEffectCount() == 0);
}

TEST_CASE("Undo of a submodel layer add removes it from the submodel", "[seq][identity][undo]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    Layer0(m);
    AddLayer(m);
    SubModelElement* sub = m->GetSubModel("Sub", true);
    Layer0(sub);
    REQUIRE(m->GetEffectLayerCount() == 2);
    size_t subLayers = sub->GetEffectLayerCount();

    f.Undo().CreateUndoStep();
    REQUIRE(f.se.GetCommandExecutor().InsertLayer(sub, (int)sub->GetEffectLayerCount()) != nullptr);

    f.Undo().UndoLastStep();
    CHECK(sub->GetEffectLayerCount() == subLayers);
    CHECK(m->GetEffectLayerCount() == 2);
}

TEST_CASE("Ids stay unique through random edits with undo and redo", "[seq][identity][undo]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    std::vector<EffectLayer*> layers{ Layer0(m), AddLayer(m), Layer0(m->GetSubModel("Sub", true)) };
    std::mt19937 rng(1234);
    auto pick = [&](int n) { return (int)(rng() % (unsigned)n); };

    for (int i = 0; i < 400; ++i) {
        EffectLayer* l = layers[pick((int)layers.size())];
        f.Undo().CreateUndoStep();
        switch (pick(5)) {
        case 0:
        case 1: {
            int start = pick(100) * 50;
            if (l->GetRangeIsClearMS(start, start + 40)) {
                f.se.GetCommandExecutor().AddEffect(l, "On", "", "", start, start + 40, 0, false);
            }
            break;
        }
        case 2:
            if (l->GetEffectCount() > 0) {
                l->GetEffect(pick(l->GetEffectCount()))->SetSelected(EFFECT_SELECTED);
                l->DeleteSelectedEffects(f.Undo());
            }
            break;
        case 3:
            if (f.Undo().CanUndo()) {
                f.Undo().UndoLastStep();
            }
            break;
        case 4:
            if (f.Undo().CanRedo()) {
                f.Undo().RedoLastStep();
            }
            break;
        }
        std::string problem;
        bool ok = f.se.ValidateUids(&problem);
        INFO("step " << i << ": " << problem);
        REQUIRE(ok);
    }
}
