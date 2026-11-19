#include <catch2/catch_test_macros.hpp>

#include "render/SequenceMutationGuard.h"
#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceIds.h"
#include "render/UndoManager.h"

#include <nlohmann/json.hpp>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Serializable, invertible sequence-edit commands (SequenceCommands.h) and
// the executor that records them for undo (CommandExecutor.h).

using nlohmann::json;

namespace {

struct SeqFixture {
    // Command unit tests set up and inspect state directly throughout; the
    // commands under test hold their own scope anyway.
    SeqMutationGuard::Scope direct;
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
    CommandExecutor& Exec() { return se.GetCommandExecutor(); }
    std::string Describe(bool ids = true) const { return DescribeSequence(se, ids); }
    void RequireValid() {
        std::string problem;
        bool ok = se.ValidateUids(&problem);
        INFO(problem);
        REQUIRE(ok);
    }
};

EffectLayer* Layer0(Element* e) {
    return e->GetEffectLayerCount() > 0 ? e->GetEffectLayer(0) : e->AddEffectLayer();
}

Effect* Add(EffectLayer* l, const std::string& name, int start, int end,
            const std::string& settings = "", const std::string& palette = "") {
    return l->AddEffect(0, name, settings, palette, start, end, 0, false);
}

// Built by hand so the tests don't depend on LocatorOf (which is tested separately).
LayerLocator Loc(const EffectLayer* l) {
    return { l->GetParentElement()->GetUid(), l->GetUid() };
}
EffectLocator Loc(const Effect* e) {
    const EffectLayer* l = e->GetParentEffectLayer();
    return { l->GetParentElement()->GetUid(), l->GetUid(), e->GetUid() };
}

EffectState State(const std::string& name, int start, int end, const std::string& settings = "",
                  const std::string& palette = "", SeqObjectId uid = SeqIds::Next()) {
    EffectState s;
    s.uid = uid;
    s.name = name;
    s.settings = settings;
    s.palette = palette;
    s.startTimeMS = start;
    s.endTimeMS = end;
    return s;
}

EffectState NoSel(EffectState s) {
    s.selected = 0;
    return s;
}

SeqObjectId Bogus() { return SeqIds::Next(); } // allocated, never given to an object

const std::string kSettings = "E_SLIDER_A=1,E_SLIDER_B=2";
const std::string kPalette = "C_BUTTON_Palette1=#FF0000,C_CHECKBOX_Palette1=1";

struct Applied {
    std::string before;
    std::string after;
    std::unique_ptr<SequenceCommand> inverse;
};

Applied ApplyOk(SeqFixture& f, SequenceCommand& cmd, ApplyMode mode = ApplyMode::Strict) {
    Applied a;
    a.before = f.Describe();
    CommandResult r;
    a.inverse = cmd.Apply(f.se, mode, &r);
    INFO("apply: " << r.message);
    REQUIRE(r.Ok());
    REQUIRE(a.inverse != nullptr);
    a.after = f.Describe();
    REQUIRE(a.after != a.before);
    return a;
}

// inverse -> state before (ids included); inverse of the inverse -> state after.
void CheckInverses(SeqFixture& f, Applied& a, ApplyMode inverseMode = ApplyMode::Strict) {
    CommandResult r;
    auto inv2 = a.inverse->Apply(f.se, inverseMode, &r);
    {
        INFO("inverse: " << r.message);
        REQUIRE(inv2 != nullptr);
    }
    CHECK(f.Describe() == a.before);
    f.RequireValid();
    auto inv3 = inv2->Apply(f.se, inverseMode, &r);
    {
        INFO("inverse of inverse: " << r.message);
        REQUIRE(inv3 != nullptr);
    }
    CHECK(f.Describe() == a.after);
    f.RequireValid();
}

void ExpectRejected(SeqFixture& f, SequenceCommand& cmd, ApplyMode mode, CommandStatus status) {
    std::string before = f.Describe();
    CHECK(cmd.Validate(f.se, mode).status == status);
    CommandResult r;
    auto inv = cmd.Apply(f.se, mode, &r);
    CHECK(inv == nullptr);
    CHECK(r.status == status);
    CHECK_FALSE(r.Ok());
    CHECK(f.Describe() == before);
}

std::unique_ptr<SequenceCommand> SafeDeserialize(const json& j) {
    try {
        return DeserializeCommand(j);
    } catch (...) {
        FAIL("DeserializeCommand threw for " << j.dump());
    }
    return nullptr;
}

std::unique_ptr<SequenceCommand> SafeFromJson(const std::string& text) {
    try {
        return CommandFromJson(text);
    } catch (...) {
        FAIL("CommandFromJson threw for " << text);
    }
    return nullptr;
}

json Ser(const SequenceCommand& c) {
    json j;
    c.Serialize(j);
    return j;
}

void CheckJsonRoundTrip(const SequenceCommand& c) {
    json j = Ser(c);
    INFO(j.dump());
    auto d = SafeDeserialize(j);
    REQUIRE(d != nullptr);
    CHECK(d->Type() == c.Type());
    CHECK(Ser(*d) == j);

    auto t = SafeFromJson(c.ToJson());
    REQUIRE(t != nullptr);
    CHECK(Ser(*t) == j);

    auto cl = c.Clone();
    REQUIRE(cl != nullptr);
    CHECK(cl->Type() == c.Type());
    CHECK(Ser(*cl) == j);
}

std::vector<std::unique_ptr<SequenceCommand>> Cmds() { return {}; }
template <typename... Rest>
std::vector<std::unique_ptr<SequenceCommand>> Cmds(std::unique_ptr<SequenceCommand> first, Rest... rest) {
    auto v = Cmds(std::move(rest)...);
    v.insert(v.begin(), std::move(first));
    return v;
}

struct CountingObserver : ICommandObserver {
    int calls = 0;
    std::vector<CommandType> appliedTypes;
    std::vector<json> applied;
    void OnCommandApplied(const SequenceCommand& a, const SequenceCommand& inverse) override {
        ++calls;
        appliedTypes.push_back(a.Type());
        applied.push_back(Ser(a));
        (void)inverse;
    }
};

// A model with one populated layer.
struct ModelSetup {
    ModelElement* m;
    EffectLayer* l;
    Effect* a; // On 0-100
    Effect* b; // Bars 200-300 with settings/palette
    explicit ModelSetup(SeqFixture& f) {
        m = f.Model();
        l = Layer0(m);
        a = Add(l, "On", 0, 100);
        b = Add(l, "Bars", 200, 300, kSettings, kPalette);
    }
};

} // namespace

// ---------------------------------------------------------------------------
// Description and lookup helpers
// ---------------------------------------------------------------------------

TEST_CASE("DescribeSequence reflects ids only when asked, and ignores selection", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    std::string withIds = f.Describe(true);
    std::string noIds = f.Describe(false);
    CHECK(withIds != noIds);

    s.b->SetSelected(EFFECT_SELECTED);
    CHECK(f.Describe(true) == withIds);

    // Same document, different effect identity.
    EffectState st = EffectState::Of(*s.b);
    REQUIRE(s.l->DeleteEffectByUid(st.uid));
    Effect* again = s.l->AddEffect(0, st.name, st.settings, st.palette, st.startTimeMS, st.endTimeMS, 0, st.isProtected);
    REQUIRE(again != nullptr);
    REQUIRE(again->GetUid() != st.uid);
    CHECK(f.Describe(false) == noIds);
    CHECK(f.Describe(true) != withIds);

    again->SetSetting("E_SLIDER_A", "7");
    CHECK(f.Describe(false) != noIds);
}

TEST_CASE("FindLayer / FindEffect / LocatorOf agree, including via an ancestor element", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    SubModelElement* sub = s.m->GetSubModel("Sub", true);
    EffectLayer* subLayer = Layer0(sub);
    Effect* se = Add(subLayer, "On", 0, 50);

    CHECK(LocatorOf(s.l) == Loc(s.l));
    CHECK(LocatorOf(s.b) == Loc(s.b));
    CHECK(FindLayer(f.se, LocatorOf(s.l)) == s.l);
    CHECK(FindEffect(f.se, LocatorOf(s.b)) == s.b);
    CHECK(FindEffect(f.se, LocatorOf(se)) == se);

    // A submodel layer can be named through its parent model.
    CHECK(FindLayer(f.se, { s.m->GetUid(), subLayer->GetUid() }) == subLayer);
    CHECK(FindEffect(f.se, { s.m->GetUid(), subLayer->GetUid(), se->GetUid() }) == se);

    CHECK(FindLayer(f.se, { s.m->GetUid(), Bogus() }) == nullptr);
    CHECK(FindLayer(f.se, { Bogus(), s.l->GetUid() }) == nullptr);
    CHECK(FindEffect(f.se, { s.m->GetUid(), s.l->GetUid(), Bogus() }) == nullptr);
    // An effect on a different layer than the one named is not found.
    CHECK(FindEffect(f.se, { s.m->GetUid(), s.l->GetUid(), se->GetUid() }) == nullptr);
}

TEST_CASE("EffectState::Of captures an effect", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    s.b->SetProtected(true);
    EffectState st = EffectState::Of(*s.b);
    CHECK(st.uid == s.b->GetUid());
    CHECK(st.name == "Bars");
    CHECK(st.settings == s.b->GetSettingsAsString());
    CHECK(st.palette == s.b->GetPaletteAsString());
    CHECK(st.startTimeMS == 200);
    CHECK(st.endTimeMS == 300);
    CHECK(st.isProtected);
}

// ---------------------------------------------------------------------------
// Each command: apply, inverse, inverse of inverse
// ---------------------------------------------------------------------------

TEST_CASE("AddEffect adds the effect and its inverse removes it", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    SeqObjectId uid = SeqIds::Next();
    AddEffectCommand cmd(Loc(s.l), State("Color Wash", 400, 500, kSettings, kPalette, uid));
    CHECK(cmd.Target() == EffectLocator{ s.m->GetUid(), s.l->GetUid(), uid });

    Applied a = ApplyOk(f, cmd);
    Effect* e = s.l->GetEffectFromUid(uid);
    REQUIRE(e != nullptr);
    CHECK(e->GetEffectName() == "Color Wash");
    CHECK(e->GetStartTimeMS() == 400);
    CHECK(e->GetEndTimeMS() == 500);
    CHECK(e->GetSetting("E_SLIDER_A") == "1");
    CHECK(e->GetPaletteMap().Contains("C_BUTTON_Palette1"));
    CHECK(s.l->GetEffectCount() == 3);

    CheckInverses(f, a);
    CHECK(s.l->GetEffectFromUid(uid) != nullptr);
}

TEST_CASE("AddEffect without an id allocates a fresh one", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    AddEffectCommand cmd(Loc(s.l), State("On", 400, 500, "", "", kNoSeqObjectId));
    Applied a = ApplyOk(f, cmd);
    Effect* e = s.l->GetEffectStartingAtTime(400);
    REQUIRE(e != nullptr);
    CHECK(e->GetUid() != kNoSeqObjectId);
    CHECK(e->GetUid() != s.a->GetUid());
    CHECK(e->GetUid() != s.b->GetUid());
    f.RequireValid();
    CheckInverses(f, a);
}

TEST_CASE("AddEffect carries the local selection hint", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    EffectState st = State("On", 400, 500);
    st.selected = EFFECT_SELECTED;
    AddEffectCommand cmd(Loc(s.l), st);
    ApplyOk(f, cmd);
    Effect* e = s.l->GetEffectFromUid(st.uid);
    REQUIRE(e != nullptr);
    CHECK(e->GetSelected() == EFFECT_SELECTED);
}

TEST_CASE("AddEffect works on a timing track", "[seq][commands]") {
    SeqFixture f;
    TimingElement* t = f.Timing();
    EffectLayer* l = Layer0(t);
    AddEffectCommand cmd(Loc(l), State("Chorus", 1000, 2000));
    Applied a = ApplyOk(f, cmd);
    REQUIRE(l->GetEffectCount() == 1);
    CHECK(l->GetEffect(0)->GetEffectName() == "Chorus");
    CheckInverses(f, a);
}

TEST_CASE("DeleteEffect removes the effect; the inverse restores it under its original uid", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    s.b->SetProtected(true);
    EffectState orig = NoSel(EffectState::Of(*s.b));
    DeleteEffectCommand cmd(Loc(s.b));

    Applied a = ApplyOk(f, cmd);
    CHECK(s.l->GetEffectFromUid(orig.uid) == nullptr);
    CHECK(s.l->GetEffectCount() == 1);

    CommandResult r;
    auto inv2 = a.inverse->Apply(f.se, ApplyMode::Strict, &r);
    INFO(r.message);
    REQUIRE(inv2 != nullptr);
    CHECK(f.Describe() == a.before);
    Effect* back = s.l->GetEffectFromUid(orig.uid);
    REQUIRE(back != nullptr);
    CHECK(NoSel(EffectState::Of(*back)) == orig);
    f.RequireValid();

    REQUIRE(inv2->Apply(f.se, ApplyMode::Strict, &r) != nullptr);
    CHECK(f.Describe() == a.after);
    CHECK(s.l->GetEffectFromUid(orig.uid) == nullptr);
}

TEST_CASE("MoveEffect changes times and its inverse moves it back", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    SeqObjectId uid = s.b->GetUid();
    MoveEffectCommand cmd(Loc(s.b), 350, 480);
    CHECK(cmd.GetStartTimeMS() == 350);
    CHECK(cmd.GetEndTimeMS() == 480);

    Applied a = ApplyOk(f, cmd);
    Effect* e = s.l->GetEffectFromUid(uid);
    REQUIRE(e != nullptr);
    CHECK(e->GetStartTimeMS() == 350);
    CHECK(e->GetEndTimeMS() == 480);
    CheckInverses(f, a);
    CHECK(s.l->GetEffectFromUid(uid)->GetStartTimeMS() == 350);
}

TEST_CASE("ModifyEffect key-level changes touch only the listed keys", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    s.b->SetSetting("E_SLIDER_KEEP", "k");
    std::string paletteBefore = s.b->GetPaletteAsString();

    SettingsChange sc;
    sc.keys["E_SLIDER_A"] = "5";
    sc.keys["E_SLIDER_B"] = std::nullopt;
    sc.keys["E_SLIDER_C"] = "3";
    ModifyEffectCommand cmd(Loc(s.b), std::nullopt, sc, SettingsChange{});

    Applied a = ApplyOk(f, cmd);
    CHECK(s.b->GetSetting("E_SLIDER_A") == "5");
    CHECK_FALSE(s.b->GetSettings().Contains("E_SLIDER_B"));
    CHECK(s.b->GetSetting("E_SLIDER_C") == "3");
    CHECK(s.b->GetSetting("E_SLIDER_KEEP") == "k");
    CHECK(s.b->GetPaletteAsString() == paletteBefore);
    CHECK(s.b->GetEffectName() == "Bars");

    // "The inverse always lists keys individually (never replaceAll)".
    auto* inv = dynamic_cast<ModifyEffectCommand*>(a.inverse.get());
    REQUIRE(inv != nullptr);
    CHECK_FALSE(inv->GetSettings().replaceAll);
    CHECK_FALSE(inv->GetPalette().replaceAll);

    CheckInverses(f, a);
}

TEST_CASE("ModifyEffect palette key changes and removal", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    std::string settingsBefore = s.b->GetSettingsAsString();
    SettingsChange pc;
    pc.keys["C_BUTTON_Palette2"] = "#00FF00";
    pc.keys["C_CHECKBOX_Palette1"] = std::nullopt;
    ModifyEffectCommand cmd(Loc(s.b), std::nullopt, SettingsChange{}, pc);

    Applied a = ApplyOk(f, cmd);
    CHECK(s.b->GetPaletteMap().Get(std::string("C_BUTTON_Palette2"), std::string()) == "#00FF00");
    CHECK_FALSE(s.b->GetPaletteMap().Contains("C_CHECKBOX_Palette1"));
    CHECK(s.b->GetPaletteMap().Get(std::string("C_BUTTON_Palette1"), std::string()) == "#FF0000");
    CHECK(s.b->GetSettingsAsString() == settingsBefore);
    CheckInverses(f, a);
}

TEST_CASE("ModifyEffect replaceAll removes unlisted keys and the inverse restores them", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    SettingsChange sc = SettingsChange::ReplaceWith("E_SLIDER_Z=9");
    CHECK(sc.replaceAll);
    CHECK_FALSE(sc.IsEmpty());
    REQUIRE(sc.keys.count("E_SLIDER_Z") == 1);
    CHECK(sc.keys.at("E_SLIDER_Z") == std::optional<std::string>("9"));

    ModifyEffectCommand cmd(Loc(s.b), std::nullopt, sc, SettingsChange{});
    Applied a = ApplyOk(f, cmd);
    CHECK(s.b->GetSetting("E_SLIDER_Z") == "9");
    CHECK_FALSE(s.b->GetSettings().Contains("E_SLIDER_A"));
    CHECK_FALSE(s.b->GetSettings().Contains("E_SLIDER_B"));

    auto* inv = dynamic_cast<ModifyEffectCommand*>(a.inverse.get());
    REQUIRE(inv != nullptr);
    CHECK_FALSE(inv->GetSettings().replaceAll);
    CheckInverses(f, a);
    CHECK(s.b->GetSetting("E_SLIDER_Z") == "9");
}

TEST_CASE("SettingsChange::IsEmpty", "[seq][commands]") {
    CHECK(SettingsChange{}.IsEmpty());
    SettingsChange all;
    all.replaceAll = true; // an empty complete map clears everything: not empty
    CHECK_FALSE(all.IsEmpty());
    SettingsChange one;
    one.keys["E_X"] = std::nullopt;
    CHECK_FALSE(one.IsEmpty());
}

TEST_CASE("ModifyEffect changes the effect name; the inverse restores it", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    ModifyEffectCommand cmd(Loc(s.a), std::string("Color Wash"), SettingsChange{}, SettingsChange{});
    Applied a = ApplyOk(f, cmd);
    CHECK(s.a->GetEffectName() == "Color Wash");
    CheckInverses(f, a);
    CHECK(s.a->GetEffectName() == "Color Wash");
}

TEST_CASE("ModifyEffect locks through X_Effect_Locked; locked effects refuse strict move and delete", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    SettingsChange lock;
    lock.keys["X_Effect_Locked"] = "True";
    ModifyEffectCommand lockCmd(Loc(s.b), std::nullopt, lock, SettingsChange{});
    Applied a = ApplyOk(f, lockCmd);
    REQUIRE(s.b->IsLocked());

    MoveEffectCommand move(Loc(s.b), 400, 500);
    ExpectRejected(f, move, ApplyMode::Strict, CommandStatus::Locked);
    DeleteEffectCommand del(Loc(s.b));
    ExpectRejected(f, del, ApplyMode::Strict, CommandStatus::Locked);
    CHECK(s.b->GetStartTimeMS() == 200);

    // Unlocking is the inverse of locking and must be possible.
    CommandResult r;
    auto unlockInverse = a.inverse->Apply(f.se, ApplyMode::Strict, &r);
    INFO(r.message);
    REQUIRE(unlockInverse != nullptr);
    CHECK_FALSE(s.b->IsLocked());
    CHECK(f.Describe() == a.before);
    CHECK(move.Validate(f.se).Ok());
}

TEST_CASE("Lenient delete ignores the lock; the inverse restores the locked effect", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    s.b->SetLocked(true);
    SeqObjectId uid = s.b->GetUid();
    DeleteEffectCommand del(Loc(s.b));
    Applied a = ApplyOk(f, del, ApplyMode::Lenient);
    CHECK(s.l->GetEffectFromUid(uid) == nullptr);
    CheckInverses(f, a, ApplyMode::Lenient);
    CHECK(s.l->GetEffectFromUid(uid) == nullptr);
}

TEST_CASE("AddLayer inserts after the named sibling, or first", "[seq][commands]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    EffectLayer* l0 = Layer0(m);
    EffectLayer* l1 = m->AddEffectLayer();
    SeqObjectId l0id = l0->GetUid(), l1id = l1->GetUid();

    SECTION("after a sibling, with name and contents") {
        SeqObjectId newId = SeqIds::Next();
        SeqObjectId effId = SeqIds::Next();
        AddLayerCommand cmd(m->GetUid(), newId, l0id, -1, "Middle", { State("On", 0, 100, "", "", effId) });
        Applied a = ApplyOk(f, cmd);
        REQUIRE(m->GetEffectLayerCount() == 3);
        CHECK(m->GetEffectLayer(0)->GetUid() == l0id);
        EffectLayer* added = m->GetEffectLayer(1);
        CHECK(added->GetUid() == newId);
        CHECK(added->GetLayerName() == "Middle");
        CHECK(added->GetEffectFromUid(effId) != nullptr);
        CHECK(m->GetEffectLayer(2)->GetUid() == l1id);
        CheckInverses(f, a);
        CHECK(m->GetEffectLayer(1)->GetUid() == newId);
    }
    SECTION("first when afterLayer is kNoSeqObjectId") {
        SeqObjectId newId = SeqIds::Next();
        AddLayerCommand cmd(m->GetUid(), newId, kNoSeqObjectId);
        Applied a = ApplyOk(f, cmd);
        REQUIRE(m->GetEffectLayerCount() == 3);
        CHECK(m->GetEffectLayer(0)->GetUid() == newId);
        CHECK(m->GetEffectLayer(1)->GetUid() == l0id);
        CheckInverses(f, a);
    }
    SECTION("after the last layer") {
        SeqObjectId newId = SeqIds::Next();
        AddLayerCommand cmd(m->GetUid(), newId, l1id);
        ApplyOk(f, cmd);
        REQUIRE(m->GetEffectLayerCount() == 3);
        CHECK(m->GetEffectLayer(2)->GetUid() == newId);
    }
    f.RequireValid();
}

TEST_CASE("AddLayer with a missing sibling: strict fails, lenient uses the clamped indexHint", "[seq][commands]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    EffectLayer* l0 = Layer0(m);
    EffectLayer* l1 = m->AddEffectLayer();
    SeqObjectId l0id = l0->GetUid(), l1id = l1->GetUid();

    SeqObjectId newId = SeqIds::Next();
    AddLayerCommand atOne(m->GetUid(), newId, Bogus(), 1);
    ExpectRejected(f, atOne, ApplyMode::Strict, CommandStatus::TargetMissing);

    SECTION("in range") {
        Applied a = ApplyOk(f, atOne, ApplyMode::Lenient);
        REQUIRE(m->GetEffectLayerCount() == 3);
        CHECK(m->GetEffectLayer(0)->GetUid() == l0id);
        CHECK(m->GetEffectLayer(1)->GetUid() == newId);
        CHECK(m->GetEffectLayer(2)->GetUid() == l1id);
        CheckInverses(f, a, ApplyMode::Lenient);
    }
    SECTION("clamped") {
        AddLayerCommand clamped(m->GetUid(), newId, Bogus(), 99);
        ApplyOk(f, clamped, ApplyMode::Lenient);
        REQUIRE(m->GetEffectLayerCount() == 3);
        CHECK(m->GetEffectLayer(2)->GetUid() == newId);
    }
    f.RequireValid();
}

TEST_CASE("RemoveLayer removes a layer; the inverse restores id, name, position and effects", "[seq][commands]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    EffectLayer* l0 = Layer0(m);
    EffectLayer* mid = m->AddEffectLayer();
    EffectLayer* l2 = m->AddEffectLayer();
    mid->SetLayerName("Named");
    Effect* e1 = Add(mid, "Bars", 0, 100, kSettings, kPalette);
    Effect* e2 = Add(mid, "On", 200, 300);
    SeqObjectId midId = mid->GetUid();
    EffectState s1 = NoSel(EffectState::Of(*e1));
    EffectState s2 = NoSel(EffectState::Of(*e2));
    Add(l2, "Off", 0, 50);
    SeqObjectId l0id = l0->GetUid(), l2id = l2->GetUid();

    RemoveLayerCommand cmd(m->GetUid(), midId);
    Applied a = ApplyOk(f, cmd);
    REQUIRE(m->GetEffectLayerCount() == 2);
    CHECK(m->GetEffectLayerFromUid(midId) == nullptr);

    CommandResult r;
    auto inv2 = a.inverse->Apply(f.se, ApplyMode::Strict, &r);
    INFO(r.message);
    REQUIRE(inv2 != nullptr);
    CHECK(f.Describe() == a.before);
    REQUIRE(m->GetEffectLayerCount() == 3);
    CHECK(m->GetEffectLayer(0)->GetUid() == l0id);
    EffectLayer* back = m->GetEffectLayer(1);
    CHECK(back->GetUid() == midId);
    CHECK(back->GetLayerName() == "Named");
    REQUIRE(back->GetEffectCount() == 2);
    REQUIRE(back->GetEffectFromUid(s1.uid) != nullptr);
    REQUIRE(back->GetEffectFromUid(s2.uid) != nullptr);
    CHECK(NoSel(EffectState::Of(*back->GetEffectFromUid(s1.uid))) == s1);
    CHECK(NoSel(EffectState::Of(*back->GetEffectFromUid(s2.uid))) == s2);
    CHECK(m->GetEffectLayer(2)->GetUid() == l2id);
    f.RequireValid();

    REQUIRE(inv2->Apply(f.se, ApplyMode::Strict, &r) != nullptr);
    CHECK(f.Describe() == a.after);
}

TEST_CASE("RemoveLayer refuses an element's last layer", "[seq][commands]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    EffectLayer* only = Layer0(m);
    REQUIRE(m->GetEffectLayerCount() == 1);
    Add(only, "On", 0, 100);
    RemoveLayerCommand cmd(m->GetUid(), only->GetUid());
    for (ApplyMode mode : { ApplyMode::Strict, ApplyMode::Lenient }) {
        std::string before = f.Describe();
        CHECK_FALSE(cmd.Validate(f.se, mode).Ok());
        CommandResult r;
        CHECK(cmd.Apply(f.se, mode, &r) == nullptr);
        CHECK_FALSE(r.Ok());
        CHECK(f.Describe() == before);
        CHECK(m->GetEffectLayerCount() == 1);
    }
}

TEST_CASE("SetLayerName renames and the inverse restores the old name", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    s.l->SetLayerName("Old");
    SetLayerNameCommand cmd(Loc(s.l), "New");
    Applied a = ApplyOk(f, cmd);
    CHECK(s.l->GetLayerName() == "New");
    CheckInverses(f, a);
    CHECK(s.l->GetLayerName() == "New");
    a.inverse->Apply(f.se, ApplyMode::Strict);
    CHECK(s.l->GetLayerName() == "Old");
}

TEST_CASE("ReplaceLayerContents swaps every effect; the inverse restores the originals with their uids", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    SeqObjectId aId = s.a->GetUid(), bId = s.b->GetUid();
    SeqObjectId n1 = SeqIds::Next(), n2 = SeqIds::Next();
    ReplaceLayerContentsCommand cmd(Loc(s.l), { State("Color Wash", 50, 150, "", "", n1), State("Off", 500, 600, "", "", n2) });

    Applied a = ApplyOk(f, cmd);
    REQUIRE(s.l->GetEffectCount() == 2);
    CHECK(s.l->GetEffectFromUid(n1) != nullptr);
    CHECK(s.l->GetEffectFromUid(n2) != nullptr);
    CHECK(s.l->GetEffectFromUid(aId) == nullptr);
    CHECK(s.l->GetEffectFromUid(bId) == nullptr);

    CommandResult r;
    auto inv2 = a.inverse->Apply(f.se, ApplyMode::Strict, &r);
    INFO(r.message);
    REQUIRE(inv2 != nullptr);
    CHECK(f.Describe() == a.before);
    CHECK(s.l->GetEffectFromUid(aId) != nullptr);
    CHECK(s.l->GetEffectFromUid(bId) != nullptr);
    f.RequireValid();
    REQUIRE(inv2->Apply(f.se, ApplyMode::Strict, &r) != nullptr);
    CHECK(f.Describe() == a.after);
}

TEST_CASE("ReplaceLayerContents strict refuses a locked effect or overlapping new effects", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    ReplaceLayerContentsCommand overlapping(Loc(s.l), { State("On", 0, 100), State("Off", 50, 150) });
    ExpectRejected(f, overlapping, ApplyMode::Strict, CommandStatus::Overlap);

    s.a->SetLocked(true);
    ReplaceLayerContentsCommand fine(Loc(s.l), { State("On", 1000, 1100) });
    ExpectRejected(f, fine, ApplyMode::Strict, CommandStatus::Locked);

    Applied a = ApplyOk(f, fine, ApplyMode::Lenient);
    CHECK(s.l->GetEffectCount() == 1);
    CheckInverses(f, a, ApplyMode::Lenient);
}

// ---------------------------------------------------------------------------
// Strict vs lenient
// ---------------------------------------------------------------------------

TEST_CASE("Overlap is rejected in strict mode and allowed in lenient mode", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);

    AddEffectCommand addOver(Loc(s.l), State("On", 50, 150));
    ExpectRejected(f, addOver, ApplyMode::Strict, CommandStatus::Overlap);

    MoveEffectCommand moveOver(Loc(s.b), 80, 250);
    ExpectRejected(f, moveOver, ApplyMode::Strict, CommandStatus::Overlap);
    CHECK(s.b->GetStartTimeMS() == 200);

    Applied a = ApplyOk(f, addOver, ApplyMode::Lenient);
    CHECK(s.l->GetEffectCount() == 3);
    CheckInverses(f, a, ApplyMode::Lenient);
}

TEST_CASE("Missing targets give TargetMissing in both modes", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    SeqObjectId m = s.m->GetUid(), l = s.l->GetUid();
    EffectLocator noEffect{ m, l, Bogus() };
    LayerLocator noLayer{ m, Bogus() };
    LayerLocator noElement{ Bogus(), l };

    std::vector<std::unique_ptr<SequenceCommand>> cmds;
    cmds.push_back(std::make_unique<AddEffectCommand>(noLayer, State("On", 1000, 1100)));
    cmds.push_back(std::make_unique<AddEffectCommand>(noElement, State("On", 1000, 1100)));
    cmds.push_back(std::make_unique<DeleteEffectCommand>(noEffect));
    cmds.push_back(std::make_unique<MoveEffectCommand>(noEffect, 1000, 1100));
    cmds.push_back(std::make_unique<ModifyEffectCommand>(noEffect, std::string("Off"), SettingsChange{}, SettingsChange{}));
    cmds.push_back(std::make_unique<AddLayerCommand>(Bogus(), SeqIds::Next(), kNoSeqObjectId));
    cmds.push_back(std::make_unique<RemoveLayerCommand>(m, Bogus()));
    cmds.push_back(std::make_unique<SetLayerNameCommand>(noLayer, "x"));
    cmds.push_back(std::make_unique<ReplaceLayerContentsCommand>(noLayer, std::vector<EffectState>{}));

    for (auto& c : cmds) {
        INFO(CommandTypeName(c->Type()));
        ExpectRejected(f, *c, ApplyMode::Strict, CommandStatus::TargetMissing);
        ExpectRejected(f, *c, ApplyMode::Lenient, CommandStatus::TargetMissing);
    }
}

TEST_CASE("Malformed commands are Invalid in strict mode", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    AddEffectCommand backwards(Loc(s.l), State("On", 600, 500));
    ExpectRejected(f, backwards, ApplyMode::Strict, CommandStatus::Invalid);
    AddEffectCommand unknownEffect(Loc(s.l), State("No Such Effect", 1000, 1100));
    ExpectRejected(f, unknownEffect, ApplyMode::Strict, CommandStatus::Invalid);
    AddEffectCommand duplicateId(Loc(s.l), State("On", 1000, 1100, "", "", s.a->GetUid()));
    ExpectRejected(f, duplicateId, ApplyMode::Strict, CommandStatus::Invalid);
    MoveEffectCommand badMove(Loc(s.b), 300, 200);
    ExpectRejected(f, badMove, ApplyMode::Strict, CommandStatus::Invalid);
}

TEST_CASE("Validate never mutates, in either mode, whatever the verdict", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    s.l->SetLayerName("Name");
    EffectLayer* l1 = s.m->AddEffectLayer();
    Add(l1, "Off", 0, 50);

    std::vector<std::unique_ptr<SequenceCommand>> cmds;
    cmds.push_back(std::make_unique<AddEffectCommand>(Loc(s.l), State("On", 1000, 1100)));
    cmds.push_back(std::make_unique<AddEffectCommand>(Loc(s.l), State("On", 50, 150))); // overlaps
    cmds.push_back(std::make_unique<DeleteEffectCommand>(Loc(s.a)));
    cmds.push_back(std::make_unique<MoveEffectCommand>(Loc(s.b), 400, 500));
    SettingsChange sc;
    sc.keys["E_SLIDER_A"] = "9";
    cmds.push_back(std::make_unique<ModifyEffectCommand>(Loc(s.b), std::string("On"), sc, SettingsChange::ReplaceWith("")));
    cmds.push_back(std::make_unique<AddLayerCommand>(s.m->GetUid(), SeqIds::Next(), kNoSeqObjectId));
    cmds.push_back(std::make_unique<RemoveLayerCommand>(s.m->GetUid(), l1->GetUid()));
    cmds.push_back(std::make_unique<SetLayerNameCommand>(Loc(s.l), "Other"));
    cmds.push_back(std::make_unique<ReplaceLayerContentsCommand>(Loc(s.l), std::vector<EffectState>{ State("On", 5, 6) }));
    cmds.push_back(std::make_unique<BatchCommand>("b", Cmds(std::make_unique<DeleteEffectCommand>(Loc(s.a)),
                                                            std::make_unique<SetLayerNameCommand>(Loc(s.l), "B"))));

    std::string before = f.Describe();
    for (auto& c : cmds) {
        INFO(CommandTypeName(c->Type()));
        c->Validate(f.se, ApplyMode::Strict);
        c->Validate(f.se, ApplyMode::Lenient);
        CHECK(f.Describe() == before);
    }
    CHECK(cmds[0]->Validate(f.se).Ok());
    CHECK(cmds[1]->Validate(f.se).status == CommandStatus::Overlap);
    CHECK(cmds[1]->Validate(f.se, ApplyMode::Lenient).Ok());
}

TEST_CASE("Apply works without a result out-parameter", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    AddEffectCommand bad(Loc(s.l), State("On", 50, 150));
    CHECK(bad.Apply(f.se) == nullptr);
    AddEffectCommand good(Loc(s.l), State("On", 1000, 1100));
    CHECK(good.Apply(f.se) != nullptr);
}

// ---------------------------------------------------------------------------
// Batch
// ---------------------------------------------------------------------------

TEST_CASE("Batch applies members in order and its inverse undoes them all", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    SeqObjectId newLayer = SeqIds::Next();
    SeqObjectId newEffect = SeqIds::Next();
    // Members 2 and 3 depend on member 1.
    BatchCommand batch("Build", Cmds(std::make_unique<AddLayerCommand>(s.m->GetUid(), newLayer, s.l->GetUid()),
                                     std::make_unique<AddEffectCommand>(LayerLocator{ s.m->GetUid(), newLayer }, State("On", 0, 100, "", "", newEffect)),
                                     std::make_unique<MoveEffectCommand>(EffectLocator{ s.m->GetUid(), newLayer, newEffect }, 500, 700),
                                     std::make_unique<DeleteEffectCommand>(Loc(s.a))));
    CHECK(batch.GetLabel() == "Build");
    CHECK(batch.GetCommands().size() == 4);
    CHECK_FALSE(batch.IsEmpty());

    Applied a = ApplyOk(f, batch);
    REQUIRE(s.m->GetEffectLayerCount() == 2);
    EffectLayer* added = s.m->GetEffectLayer(1);
    CHECK(added->GetUid() == newLayer);
    Effect* e = added->GetEffectFromUid(newEffect);
    REQUIRE(e != nullptr);
    CHECK(e->GetStartTimeMS() == 500);
    CHECK(s.l->GetEffectCount() == 1);
    CheckInverses(f, a);
}

TEST_CASE("Strict batch failure rolls back the members already applied", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    s.l->SetLayerName("Before");
    BatchCommand batch("Doomed", Cmds(std::make_unique<AddEffectCommand>(Loc(s.l), State("On", 1000, 1100)),
                                      std::make_unique<SetLayerNameCommand>(Loc(s.l), "During"),
                                      std::make_unique<AddLayerCommand>(s.m->GetUid(), SeqIds::Next(), kNoSeqObjectId),
                                      std::make_unique<DeleteEffectCommand>(Loc(s.b)),
                                      std::make_unique<AddEffectCommand>(Loc(s.l), State("Off", 1050, 1200)))); // overlaps member 1
    std::string before = f.Describe();
    CommandResult r;
    auto inv = batch.Apply(f.se, ApplyMode::Strict, &r);
    CHECK(inv == nullptr);
    CHECK(r.status == CommandStatus::Overlap);
    CHECK(f.Describe() == before);
    CHECK(s.l->GetLayerName() == "Before");
    CHECK(s.m->GetEffectLayerCount() == 1);
    f.RequireValid();
}

TEST_CASE("Lenient batch skips failing members", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    SeqObjectId x = SeqIds::Next(), y = SeqIds::Next();
    BatchCommand batch("Partial", Cmds(std::make_unique<AddEffectCommand>(Loc(s.l), State("On", 1000, 1100, "", "", x)),
                                       std::make_unique<DeleteEffectCommand>(EffectLocator{ s.m->GetUid(), s.l->GetUid(), Bogus() }),
                                       std::make_unique<AddEffectCommand>(Loc(s.l), State("Off", 2000, 2100, "", "", y))));
    std::string before = f.Describe();
    auto inv = batch.Apply(f.se, ApplyMode::Lenient);
    REQUIRE(inv != nullptr);
    CHECK(s.l->GetEffectFromUid(x) != nullptr);
    CHECK(s.l->GetEffectFromUid(y) != nullptr);
    CHECK(s.l->GetEffectCount() == 4);
    REQUIRE(inv->Apply(f.se, ApplyMode::Lenient) != nullptr);
    CHECK(f.Describe() == before);
}

TEST_CASE("Strict batch with a missing target fails as TargetMissing", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    BatchCommand batch("Missing", Cmds(std::make_unique<SetLayerNameCommand>(Loc(s.l), "x"),
                                       std::make_unique<DeleteEffectCommand>(EffectLocator{ s.m->GetUid(), s.l->GetUid(), Bogus() })));
    std::string before = f.Describe();
    CommandResult r;
    CHECK(batch.Apply(f.se, ApplyMode::Strict, &r) == nullptr);
    CHECK(r.status == CommandStatus::TargetMissing);
    CHECK(f.Describe() == before);
}

// ---------------------------------------------------------------------------
// JSON
// ---------------------------------------------------------------------------

TEST_CASE("Every command type round-trips through JSON", "[seq][commands]") {
    SeqObjectId el = SeqIds::Next(), ly = SeqIds::Next(), ef = SeqIds::Next();
    // An id with the high allocator-prefix bits set must survive (needs all 64 bits).
    SeqObjectId wide = (uint64_t(0xBEEF) << 48) | 12345;
    EffectState st = State("Bars", 10, 20, kSettings, kPalette, wide);
    st.isProtected = true;

    SettingsChange keys;
    keys.keys["E_SLIDER_A"] = "5";
    keys.keys["E_SLIDER_B"] = std::nullopt;
    keys.keys["X_Effect_Locked"] = "True";

    std::vector<std::unique_ptr<SequenceCommand>> cmds;
    cmds.push_back(std::make_unique<AddEffectCommand>(LayerLocator{ el, ly }, st));
    cmds.push_back(std::make_unique<DeleteEffectCommand>(EffectLocator{ el, ly, ef }));
    cmds.push_back(std::make_unique<MoveEffectCommand>(EffectLocator{ el, ly, ef }, 100, 250));
    cmds.push_back(std::make_unique<ModifyEffectCommand>(EffectLocator{ el, ly, ef }, std::string("On"), keys, SettingsChange::ReplaceWith(kPalette)));
    cmds.push_back(std::make_unique<ModifyEffectCommand>(EffectLocator{ el, ly, ef }, std::nullopt, SettingsChange{}, SettingsChange{}));
    cmds.push_back(std::make_unique<AddLayerCommand>(el, ly, wide, 3, "Layer \"q\", with=chars", std::vector<EffectState>{ st, State("On", 30, 40) }));
    cmds.push_back(std::make_unique<AddLayerCommand>(el, ly, kNoSeqObjectId));
    cmds.push_back(std::make_unique<RemoveLayerCommand>(el, ly));
    cmds.push_back(std::make_unique<SetLayerNameCommand>(LayerLocator{ el, ly }, "Name"));
    cmds.push_back(std::make_unique<ReplaceLayerContentsCommand>(LayerLocator{ el, ly }, std::vector<EffectState>{ st }));
    cmds.push_back(std::make_unique<ReplaceLayerContentsCommand>(LayerLocator{ el, ly }, std::vector<EffectState>{}));

    auto inner = std::make_unique<BatchCommand>("Inner", Cmds(std::make_unique<SetLayerNameCommand>(LayerLocator{ el, ly }, "N"),
                                                              std::make_unique<MoveEffectCommand>(EffectLocator{ el, ly, ef }, 1, 2)));
    cmds.push_back(std::make_unique<BatchCommand>("Outer", Cmds(std::make_unique<DeleteEffectCommand>(EffectLocator{ el, ly, ef }),
                                                                std::move(inner),
                                                                std::make_unique<BatchCommand>("Empty"))));
    cmds.push_back(std::make_unique<BatchCommand>("Empty"));

    for (auto& c : cmds) {
        INFO(CommandTypeName(c->Type()));
        CheckJsonRoundTrip(*c);
    }

    auto back = SafeDeserialize(Ser(*cmds[0]));
    auto* add = dynamic_cast<AddEffectCommand*>(back.get());
    REQUIRE(add != nullptr);
    CHECK(add->GetState() == NoSel(st));
    CHECK(add->GetLayer() == LayerLocator{ el, ly });

    auto backBatch = SafeDeserialize(Ser(*cmds[cmds.size() - 2]));
    auto* b = dynamic_cast<BatchCommand*>(backBatch.get());
    REQUIRE(b != nullptr);
    CHECK(b->GetLabel() == "Outer");
    REQUIRE(b->GetCommands().size() == 3);
    CHECK(b->GetCommands()[1]->Type() == CommandType::Batch);
}

TEST_CASE("The selection hint is not serialized", "[seq][commands]") {
    EffectState plain = State("On", 0, 100);
    EffectState sel = plain;
    sel.selected = EFFECT_SELECTED;
    LayerLocator loc{ SeqIds::Next(), SeqIds::Next() };
    AddEffectCommand a(loc, plain), b(loc, sel);
    CHECK(Ser(a) == Ser(b));
    CHECK(a.ToJson() == b.ToJson());
    auto back = SafeDeserialize(Ser(b));
    auto* add = dynamic_cast<AddEffectCommand*>(back.get());
    REQUIRE(add != nullptr);
    CHECK(add->GetState().selected == 0);
}

TEST_CASE("CommandFromJson / DeserializeCommand reject malformed input without throwing", "[seq][commands]") {
    for (const char* text : { "", "not json", "{", "[]", "42", "null", "\"AddEffect\"", "{}",
                              R"({"type":"NoSuchCommand"})", R"({"type":5})", R"({"type":null})" }) {
        INFO(text);
        CHECK(SafeFromJson(text) == nullptr);
    }

    SeqObjectId el = SeqIds::Next(), ly = SeqIds::Next(), ef = SeqIds::Next();
    MoveEffectCommand mv(EffectLocator{ el, ly, ef }, 100, 200);
    json good = Ser(mv);
    REQUIRE(good.is_object());
    REQUIRE(good.contains("type"));
    REQUIRE(SafeDeserialize(good) != nullptr);

    SECTION("missing fields") {
        for (auto it = good.begin(); it != good.end(); ++it) {
            const std::string key = it.key();
            json j = good;
            j.erase(key);
            INFO("without " << key << ": " << j.dump());
            CHECK(SafeDeserialize(j) == nullptr);
        }
    }
    SECTION("wrong types") {
        for (auto it = good.begin(); it != good.end(); ++it) {
            const std::string key = it.key();
            json j = good;
            j[key] = true;
            INFO(j.dump());
            CHECK(SafeDeserialize(j) == nullptr);
        }
    }
    SECTION("a type name that doesn't match the fields") {
        json j = good;
        j["type"] = "AddLayer";
        CHECK(SafeDeserialize(j) == nullptr);
    }
    SECTION("a batch with one bad member") {
        BatchCommand batch("b", Cmds(std::make_unique<MoveEffectCommand>(EffectLocator{ el, ly, ef }, 1, 2)));
        json j = Ser(batch);
        REQUIRE(SafeDeserialize(j) != nullptr);
        bool foundMembers = false;
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it->is_array()) {
                it->push_back(json{ { "type", "NoSuchCommand" } });
                foundMembers = true;
            }
        }
        REQUIRE(foundMembers);
        CHECK(SafeDeserialize(j) == nullptr);
    }
}

TEST_CASE("CommandTypeName names every type distinctly", "[seq][commands]") {
    std::vector<std::string> names;
    for (CommandType t : { CommandType::AddEffect, CommandType::DeleteEffect, CommandType::MoveEffect, CommandType::ModifyEffect,
                           CommandType::AddLayer, CommandType::RemoveLayer, CommandType::SetLayerName,
                           CommandType::ReplaceLayerContents, CommandType::Batch }) {
        const char* n = CommandTypeName(t);
        REQUIRE(n != nullptr);
        CHECK(std::string(n).size() > 0);
        for (auto& other : names) {
            CHECK(other != n);
        }
        names.push_back(n);
    }
}

// ---------------------------------------------------------------------------
// Executor
// ---------------------------------------------------------------------------

TEST_CASE("Execute records one undo step; undo restores and redo re-applies", "[seq][commands][undo]") {
    SeqFixture f;
    ModelSetup s(f);
    REQUIRE_FALSE(f.Undo().CanUndo());
    CountingObserver obs;
    f.Exec().AddObserver(&obs);

    std::string before = f.Describe();
    SeqObjectId uid = s.b->GetUid();
    CommandResult r = f.Exec().Execute(std::make_unique<DeleteEffectCommand>(Loc(s.b)));
    REQUIRE(r.Ok());
    std::string after = f.Describe();
    CHECK(after != before);
    CHECK(obs.calls == 1);
    REQUIRE(obs.appliedTypes.size() == 1);
    CHECK(obs.appliedTypes[0] == CommandType::DeleteEffect);

    REQUIRE(f.Undo().CanUndo());
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    CHECK(s.l->GetEffectFromUid(uid) != nullptr);
    CHECK_FALSE(f.Undo().CanUndo());
    REQUIRE(f.Undo().CanRedo());
    f.Undo().RedoLastStep();
    CHECK(f.Describe() == after);
    f.RequireValid();

    f.Exec().RemoveObserver(&obs);
}

TEST_CASE("Each Execute outside a batch is its own undo step", "[seq][commands][undo]") {
    SeqFixture f;
    ModelSetup s(f);
    std::string s0 = f.Describe();
    REQUIRE(f.Exec().Execute(std::make_unique<MoveEffectCommand>(Loc(s.b), 400, 500)).Ok());
    std::string s1 = f.Describe();
    REQUIRE(f.Exec().Execute(std::make_unique<SetLayerNameCommand>(Loc(s.l), "Renamed")).Ok());
    std::string s2 = f.Describe();
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == s1);
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == s0);
    CHECK_FALSE(f.Undo().CanUndo());
    f.Undo().RedoLastStep();
    f.Undo().RedoLastStep();
    CHECK(f.Describe() == s2);
}

TEST_CASE("A rejected command records no undo step and notifies nobody", "[seq][commands][undo]") {
    SeqFixture f;
    ModelSetup s(f);
    REQUIRE_FALSE(f.Undo().CanUndo());
    CountingObserver obs;
    f.Exec().AddObserver(&obs);
    std::string before = f.Describe();

    CommandResult r = f.Exec().Execute(std::make_unique<AddEffectCommand>(Loc(s.l), State("On", 50, 150)));
    CHECK(r.status == CommandStatus::Overlap); // Execute is strict
    r = f.Exec().Execute(std::make_unique<DeleteEffectCommand>(EffectLocator{ s.m->GetUid(), s.l->GetUid(), Bogus() }));
    CHECK(r.status == CommandStatus::TargetMissing);

    CHECK(f.Describe() == before);
    CHECK(obs.calls == 0);
    CHECK_FALSE(f.Undo().CanUndo());
    f.Exec().RemoveObserver(&obs);
}

TEST_CASE("Nested batches form one undo step and one notification", "[seq][commands][undo]") {
    SeqFixture f;
    ModelSetup s(f);
    CountingObserver obs;
    f.Exec().AddObserver(&obs);
    std::string before = f.Describe();
    SeqObjectId x = SeqIds::Next();

    CHECK_FALSE(f.Exec().InBatch());
    f.Exec().BeginBatch("Outer");
    CHECK(f.Exec().InBatch());
    REQUIRE(f.Exec().Execute(std::make_unique<AddEffectCommand>(Loc(s.l), State("On", 1000, 1100, "", "", x))).Ok());
    CHECK(s.l->GetEffectFromUid(x) != nullptr); // applied immediately
    f.Exec().BeginBatch("Inner");
    REQUIRE(f.Exec().Execute(std::make_unique<MoveEffectCommand>(EffectLocator{ s.m->GetUid(), s.l->GetUid(), x }, 1200, 1300)).Ok());
    REQUIRE(f.Exec().Execute(std::make_unique<SetLayerNameCommand>(Loc(s.l), "Batched")).Ok());
    f.Exec().EndBatch();
    CHECK(f.Exec().InBatch());
    CHECK(obs.calls == 0);
    REQUIRE(f.Exec().Execute(std::make_unique<DeleteEffectCommand>(Loc(s.a))).Ok());
    CHECK(obs.calls == 0);
    f.Exec().EndBatch();
    CHECK_FALSE(f.Exec().InBatch());
    std::string after = f.Describe();
    CHECK(after != before);

    REQUIRE(obs.calls == 1);
    CHECK(obs.appliedTypes[0] == CommandType::Batch);
    auto applied = SafeDeserialize(obs.applied[0]);
    auto* batch = dynamic_cast<BatchCommand*>(applied.get());
    REQUIRE(batch != nullptr);
    CHECK(batch->GetLabel() == "Outer");
    CHECK_FALSE(batch->IsEmpty());

    REQUIRE(f.Undo().CanUndo());
    CHECK(f.Undo().GetUndoString() == "Undo: Outer");
    f.Undo().UndoLastStep();
    CHECK(f.Describe() == before);
    CHECK_FALSE(f.Undo().CanUndo());
    REQUIRE(f.Undo().CanRedo());
    CHECK(f.Undo().GetRedoString() == "Redo: Outer");
    f.Undo().RedoLastStep();
    CHECK(f.Describe() == after);
    f.RequireValid();
    f.Exec().RemoveObserver(&obs);
}

TEST_CASE("RemoveObserver stops notifications to that observer only", "[seq][commands]") {
    SeqFixture f;
    ModelSetup s(f);
    CountingObserver one, two;
    f.Exec().AddObserver(&one);
    f.Exec().AddObserver(&two);
    REQUIRE(f.Exec().Execute(std::make_unique<MoveEffectCommand>(Loc(s.b), 400, 500)).Ok());
    CHECK(one.calls == 1);
    CHECK(two.calls == 1);
    f.Exec().RemoveObserver(&one);
    REQUIRE(f.Exec().Execute(std::make_unique<MoveEffectCommand>(Loc(s.b), 600, 700)).Ok());
    CHECK(one.calls == 1);
    CHECK(two.calls == 2);
    f.Exec().RemoveObserver(&two);
}

// ---------------------------------------------------------------------------
// Replay
// ---------------------------------------------------------------------------

TEST_CASE("A recorded session replays to the same document, ids included", "[seq][commands][undo]") {
    SeqFixture f;
    ModelElement* m = f.Model();
    EffectLayer* l = Layer0(m);
    Add(l, "Off", 5000, 5100);
    TimingElement* t = f.Timing();
    EffectLayer* tl = Layer0(t);
    REQUIRE_FALSE(f.Undo().CanUndo());

    CommandLogRecorder rec;
    f.Exec().AddObserver(&rec);
    std::string start = f.Describe();
    CommandExecutor& ex = f.Exec();
    int steps = 0;
    auto exec = [&](std::unique_ptr<SequenceCommand> c) {
        CommandResult r = ex.Execute(std::move(c));
        INFO(r.message);
        REQUIRE(r.Ok());
    };

    // An effect added without an id: the log must carry the id it got.
    exec(std::make_unique<AddEffectCommand>(Loc(l), State("On", 0, 100, "", "", kNoSeqObjectId)));
    ++steps;
    Effect* first = l->GetEffectStartingAtTime(0);
    REQUIRE(first != nullptr);
    exec(std::make_unique<AddEffectCommand>(Loc(l), State("Bars", 200, 300, kSettings, kPalette)));
    ++steps;
    Effect* bars = l->GetEffectStartingAtTime(200);
    REQUIRE(bars != nullptr);
    SettingsChange sc;
    sc.keys["E_SLIDER_A"] = "42";
    exec(std::make_unique<ModifyEffectCommand>(Loc(bars), std::string("Color Wash"), sc, SettingsChange{}));
    ++steps;
    exec(std::make_unique<MoveEffectCommand>(Loc(first), 100, 150));
    ++steps;

    SeqObjectId newLayer = SeqIds::Next();
    ex.BeginBatch("Layer work");
    exec(std::make_unique<AddLayerCommand>(m->GetUid(), newLayer, l->GetUid(), -1, "Extra"));
    exec(std::make_unique<AddEffectCommand>(LayerLocator{ m->GetUid(), newLayer }, State("On", 0, 1000)));
    ex.BeginBatch("Nested");
    exec(std::make_unique<SetLayerNameCommand>(LayerLocator{ m->GetUid(), newLayer }, "Extra 2"));
    ex.EndBatch();
    ex.EndBatch();
    ++steps;

    exec(std::make_unique<AddEffectCommand>(Loc(tl), State("Verse", 0, 2000)));
    ++steps;
    exec(std::make_unique<DeleteEffectCommand>(Loc(first)));
    ++steps;
    exec(std::make_unique<ReplaceLayerContentsCommand>(Loc(tl), std::vector<EffectState>{ State("Intro", 0, 500), State("Chorus", 500, 900) }));
    ++steps;

    std::string end = f.Describe();
    CHECK(end != start);
    json log = rec.GetLog();
    f.Exec().RemoveObserver(&rec); // whatever undo reports, it must not reach the log being replayed
    REQUIRE(log.is_array());
    CHECK(log.size() == (size_t)steps);

    for (int i = 0; i < steps; ++i) {
        REQUIRE(f.Undo().CanUndo());
        f.Undo().UndoLastStep();
    }
    CHECK_FALSE(f.Undo().CanUndo());
    CHECK(f.Describe() == start);
    f.RequireValid();

    for (const json& entry : log) {
        INFO(entry.dump());
        auto cmd = SafeDeserialize(entry);
        REQUIRE(cmd != nullptr);
        exec(std::move(cmd));
    }
    CHECK(f.Describe() == end);
    f.RequireValid();
}
