#include <catch2/catch_test_macros.hpp>

#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectEditCoalescer.h"
#include "render/EffectLayer.h"
#include "render/EffectSymbol.h"
#include "render/EffectSymbolManager.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceEditCapture.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceFaces.h"
#include "render/SequenceMedia.h"
#include "render/SequenceMediaCommands.h"
#include "render/SequenceMutationGuard.h"
#include "render/UndoManager.h"
#include "utils/xlImage.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// What a SequenceEditCapture records besides effects and layers (elements
// added or deleted directly, media, faces, flags, symbol links), the element
// flag / rename / effect symbol commands, and the symbol edit helpers.

namespace {
struct Fx {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    ModelElement* model = nullptr;
    EffectLayer* layer = nullptr;
    Fx() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
        SeqMutationGuard::Scope setup;
        model = dynamic_cast<ModelElement*>(se.AddElement("Tree", "model", true, false, true, false, false));
        layer = model->GetEffectLayerCount() > 0 ? model->GetEffectLayer(0) : model->AddEffectLayer();
    }
    UndoManager& Undo() { return se.get_undo_mgr(); }
    CommandExecutor& Exec() { return se.GetCommandExecutor(); }
    Effect* Add(EffectLayer* l, const std::string& name, int start, int end, const std::string& settings = "") {
        return Exec().AddEffect(l, name, settings, "", start, end, 0, false, false);
    }
    TimingElement* Timing(const std::string& name, std::vector<std::pair<int, int>> marks = {}) {
        SeqMutationGuard::Scope setup;
        auto* t = dynamic_cast<TimingElement*>(se.AddElement(se.GetNumberOfTimingElements(), name, "timing", true, false, false, false, false));
        EffectLayer* l = t->AddEffectLayer();
        for (auto [s, e] : marks) {
            l->AddEffect(0, "", "", "", s, e, 0, false);
        }
        return t;
    }
};

// The document state these tests touch, beyond DescribeSequence.
std::string State(SequenceElements& se) {
    std::ostringstream out;
    out << DescribeSequence(se);
    for (size_t i = 0; i < se.GetElementCount(); ++i) {
        Element* el = se.GetElement(i);
        out << el->GetName() << " off=" << el->IsRenderDisabledFlag();
        if (auto* te = dynamic_cast<TimingElement*>(el)) {
            out << " fixed=" << te->GetFixedTiming() << " sub=" << te->GetSubType();
        }
        out << "\n";
    }
    for (const auto& [key, entry] : se.GetSequenceMedia().GetEmbeddedEntries()) {
        MediaEntryState s = SequenceMedia::StateOf(key, *entry);
        out << "media " << key << " " << (s.data ? s.data->size() : 0) << "/" << s.frames.size() << "\n";
    }
    for (const auto& [name, def] : se.GetSequenceFaces().GetFaces()) {
        out << "face " << name;
        for (const auto& [k, v] : def) {
            out << " " << k << "=" << v;
        }
        out << "\n";
    }
    for (EffectSymbol* sym : se.GetEffectSymbolManager().GetAllSymbols()) {
        out << "symbol " << sym->GetId() << " '" << sym->GetName() << "' " << sym->GetEffectType() << " {" << sym->GetSettingsAsString()
            << "} {" << sym->GetPalette() << "} linked=" << se.GetEffectSymbolManager().GetLinkedEffectCount(sym->GetId()) << "\n";
    }
    return out.str();
}

struct Recorder : ICommandObserver {
    CommandExecutor& exec;
    std::vector<nlohmann::json> applied;
    explicit Recorder(CommandExecutor& e) : exec(e) { exec.AddObserver(this); }
    ~Recorder() override { exec.RemoveObserver(this); }
    void OnCommandApplied(const SequenceCommand& cmd, const SequenceCommand&) override {
        nlohmann::json j;
        cmd.Serialize(j);
        applied.push_back(j);
    }
    std::vector<std::string> MemberTypes(size_t i) const {
        std::vector<std::string> out;
        if (applied[i].contains("commands")) {
            for (const auto& m : applied[i]["commands"]) {
                out.push_back(m["type"].get<std::string>());
            }
        } else {
            out.push_back(applied[i]["type"].get<std::string>());
        }
        return out;
    }
};

struct GuardCount {
    SeqMutationGuard::Mode prev;
    GuardCount() : prev(SeqMutationGuard::GetMode()) {
        SeqMutationGuard::SetMode(SeqMutationGuard::Mode::Count);
        SeqMutationGuard::ResetViolationCount();
    }
    ~GuardCount() { SeqMutationGuard::SetMode(prev); }
};

// Undo gives `before`, redo `after`; the recorded batch replayed strictly
// onto `before` gives `after`, and its inverse gives `before` again.
void CheckUndoRedoReplay(Fx& f, const nlohmann::json& batch, const std::string& before, const std::string& after) {
    f.Undo().UndoLastStep();
    REQUIRE(State(f.se) == before);
    f.Undo().RedoLastStep();
    REQUIRE(State(f.se) == after);
    f.Undo().UndoLastStep();
    REQUIRE(State(f.se) == before);
    auto cmd = DeserializeCommand(batch);
    REQUIRE(cmd != nullptr);
    CommandResult r;
    auto inverse = cmd->Apply(f.se, ApplyMode::Strict, &r);
    INFO(r.message);
    REQUIRE(inverse != nullptr);
    CHECK(State(f.se) == after);
    REQUIRE(inverse->Apply(f.se, ApplyMode::Strict) != nullptr);
    CHECK(State(f.se) == before);
}

std::string Png(int w) {
    return std::string(w, 'x');
}
} // namespace

TEST_CASE("A capture records elements added directly, with their contents", "[seq][capture][finish][undo]") {
    Fx f;
    f.Add(f.layer, "On", 0, 100);
    f.Undo().Clear();
    std::string before = State(f.se);
    Recorder rec(f.Exec());
    {
        GuardCount guard;
        SequenceEditCapture capture(f.se, "Import Effects");
        // an importer adding a timing track and a model the sequence lacked
        auto* t = dynamic_cast<TimingElement*>(f.se.AddElement(0, "Lyrics", "timing", true, false, false, false, false));
        EffectLayer* tl = t->AddEffectLayer();
        tl->AddEffect(0, "la", "", "", 0, 500, 0, false);
        tl->AddEffect(0, "la", "", "", 500, 900, 0, false);
        Element* m = f.se.AddElement("Arch", "model", true, false, false, false, false);
        if (m->GetEffectLayerCount() == 0) {
            m->AddEffectLayer();
        }
        m->GetEffectLayer(0)->AddEffect(0, "On", "", "", 0, 50, 0, false);
        f.layer->AddEffect(0, "Bars", "", "", 200, 300, 0, false);
        CHECK(SeqMutationGuard::ViolationCount() == 0);
    }
    std::string after = State(f.se);
    REQUIRE(after != before);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "AddElement", "AddElement", "AddEffect" });
    CHECK(f.Undo().GetUndoString() == "Undo: Import Effects");
    CheckUndoRedoReplay(f, rec.applied[0], before, after);
    CHECK(f.se.GetElement("Lyrics") == nullptr);
    CHECK(f.se.GetElement("Arch") == nullptr);
}

TEST_CASE("Elements added directly are recorded ahead of a later command that uses them", "[seq][capture][finish][undo]") {
    Fx f;
    f.Undo().Clear();
    std::string before = State(f.se);
    Recorder rec(f.Exec());
    {
        GuardCount guard;
        SequenceEditCapture capture(f.se, "Import Timing");
        auto* t = dynamic_cast<TimingElement*>(f.se.AddElement(0, "Beats", "timing", true, false, false, false, false));
        t->AddEffectLayer()->AddEffect(0, "", "", "", 0, 250, 0, false);
        // a command that names the new track
        REQUIRE(ElementEdits::RenameTimingTrack(f.se, "Beats", "Beats 2"));
        f.se.GetElement("Beats 2")->GetEffectLayer(0)->AddEffect(0, "", "", "", 250, 500, 0, false);
        CHECK(SeqMutationGuard::ViolationCount() == 0);
    }
    std::string after = State(f.se);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "AddElement", "RenameTimingTrack", "AddEffect" });
    CheckUndoRedoReplay(f, rec.applied[0], before, after);
}

TEST_CASE("A capture records elements deleted directly, restoring their original contents", "[seq][capture][finish][undo]") {
    Fx f;
    TimingElement* t = f.Timing("New Timing", { { 0, 100 }, { 100, 200 } });
    f.Add(f.layer, "On", 0, 100);
    f.Undo().Clear();
    std::string before = State(f.se);
    Recorder rec(f.Exec());
    {
        GuardCount guard;
        SequenceEditCapture capture(f.se, "Import Effects");
        // changed first, then deleted: undo must bring back the original marks
        t->GetEffectLayer(0)->DeleteAllEffects();
        f.se.DeleteElement("New Timing");
        f.layer->AddEffect(0, "Bars", "", "", 300, 400, 0, false);
        CHECK(SeqMutationGuard::ViolationCount() == 0);
    }
    std::string after = State(f.se);
    REQUIRE(f.se.GetElement("New Timing") == nullptr);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "DeleteElement", "AddEffect" });
    CheckUndoRedoReplay(f, rec.applied[0], before, after);
}

TEST_CASE("A capture records embedded media and sequence faces changed directly", "[seq][capture][finish][undo]") {
    Fx f;
    xlImage img(2, 2);
    {
        SeqMutationGuard::Scope setup;
        f.se.GetSequenceMedia().AddEmbeddedImage("old.png", img);
        f.se.GetSequenceFaces().SetFace("Gone", { { "Type", "Matrix" }, { "Mouth-AI", "old.png" } });
        f.se.GetSequenceFaces().SetFace("Kept", { { "Type", "Matrix" } });
    }
    f.Undo().Clear();
    std::string before = State(f.se);
    Recorder rec(f.Exec());
    {
        GuardCount guard;
        SequenceEditCapture capture(f.se, "Import Effects");
        SequenceMedia& media = f.se.GetSequenceMedia();
        xlImage big(4, 4);
        media.AddEmbeddedImage("new.png", big);
        media.RemoveMedia("old.png");
        // a render-time registration of a file reference is not a document change
        media.GetImage("not-embedded.png");
        f.se.GetSequenceFaces().RemoveFace("Gone");
        f.se.GetSequenceFaces().SetFace("Kept", { { "Type", "Matrix" }, { "Mouth-AI", "new.png" } });
        f.se.GetSequenceFaces().SetFace("Added", { { "Type", "Rendered" } });
        f.layer->AddEffect(0, "Pictures", "E_TEXTCTRL_Pictures_Filename=new.png", "", 0, 100, 0, false);
        CHECK(SeqMutationGuard::ViolationCount() == 0);
    }
    std::string after = State(f.se);
    REQUIRE(after != before);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "RemoveMedia", "PutMedia", "SetSequenceFace", "SetSequenceFace", "SetSequenceFace", "AddEffect" });
    CheckUndoRedoReplay(f, rec.applied[0], before, after);
    CHECK(f.se.GetSequenceMedia().HasImage("old.png"));
    CHECK_FALSE(f.se.GetSequenceMedia().GetEntryState("new.png").has_value());
}

TEST_CASE("A media command inside a capture is recorded once, in order", "[seq][capture][finish]") {
    Fx f;
    f.Undo().Clear();
    std::string before = State(f.se);
    Recorder rec(f.Exec());
    {
        GuardCount guard;
        SequenceEditCapture capture(f.se, "Import Effects");
        auto state = MediaEntryState::EmbeddedImage("cmd.png", xlImage(2, 2));
        REQUIRE(state.has_value());
        REQUIRE(SeqMediaEdits::Put(f.se, *state));
        f.se.GetSequenceMedia().AddEmbeddedImage("direct.png", xlImage(3, 3));
        CHECK(SeqMutationGuard::ViolationCount() == 0);
    }
    std::string after = State(f.se);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "PutMedia", "PutMedia" });
    CHECK(rec.applied[0]["commands"][0]["key"] == "cmd.png");
    CHECK(rec.applied[0]["commands"][1]["key"] == "direct.png");
    CheckUndoRedoReplay(f, rec.applied[0], before, after);
}

TEST_CASE("A capture records element flags and effect symbol links", "[seq][capture][finish][undo]") {
    Fx f;
    TimingElement* t = f.Timing("Fixed");
    {
        SeqMutationGuard::Scope setup;
        t->SetFixedTiming(50);
    }
    Effect* a = f.Add(f.layer, "On", 0, 100, "E_TEXTCTRL_Eff_On_Start=10");
    Effect* b = f.Add(f.layer, "On", 200, 300);
    std::string sym = SymbolEdits::Create(f.se, "Sym", a, true);
    REQUIRE_FALSE(sym.empty());
    f.Undo().Clear();
    std::string before = State(f.se);
    Recorder rec(f.Exec());
    {
        GuardCount guard;
        SequenceEditCapture capture(f.se, "Bulk");
        f.model->SetRenderDisabled(true);
        t->SetFixedTiming(0);
        a->UnlinkFromSymbol();
        b->LinkToSymbol(sym);
        CHECK(SeqMutationGuard::ViolationCount() == 0);
    }
    std::string after = State(f.se);
    REQUIRE(after != before);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.MemberTypes(0) ==
          std::vector<std::string>{ "SetTimingFixed", "SetElementRenderDisabled", "SetEffectSymbolLink", "ModifyEffect", "SetEffectSymbolLink" });
    CheckUndoRedoReplay(f, rec.applied[0], before, after);
    CHECK(a->GetLinkedSymbolId() == sym);
    CHECK_FALSE(b->IsLinkedToSymbol());
}

TEST_CASE("A NewElementsRecorder inside a capture leaves the recording to it", "[seq][capture][finish][undo]") {
    Fx f;
    f.Undo().Clear();
    std::string before = State(f.se);
    Recorder rec(f.Exec());
    {
        GuardCount guard;
        SequenceEditCapture capture(f.se, "Import Timing");
        {
            NewElementsRecorder recorder(f.se, "Import Timing");
            TimingElement* t = ElementEdits::AddTimingTrack(f.se, -1, "Words", "");
            REQUIRE(t != nullptr);
            t->GetEffectLayer(0)->AddEffect(0, "hi", "", "", 0, 100, 0, false);
        }
        f.layer->AddEffect(0, "On", "", "", 0, 100, 0, false);
        CHECK(SeqMutationGuard::ViolationCount() == 0);
    }
    std::string after = State(f.se);
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.MemberTypes(0) == std::vector<std::string>{ "AddElement", "AddEffect", "AddEffect" });
    CheckUndoRedoReplay(f, rec.applied[0], before, after);
    CHECK(f.se.GetElement("Words") == nullptr);
}

TEST_CASE("Direct edits a capture does not compare still trip the guard", "[seq][capture][finish][guard]") {
#ifndef NDEBUG
    Fx f;
    f.Timing("A");
    f.Timing("B");
    GuardCount guard;
    SequenceEditCapture capture(f.se, "Import");
    f.se.MoveSequenceElement(0, 2, MASTER_VIEW);
    CHECK(SeqMutationGuard::ViolationCount() == 1);
    f.se.GetElement("A")->SetName("C");
    CHECK(SeqMutationGuard::ViolationCount() == 2);
    f.se.SetTagPosition(0, 100);
    CHECK(SeqMutationGuard::ViolationCount() == 3);
    capture.Discard();
#endif
}

// ---------------------------------------------------------------------------
// Element flag and rename commands
// ---------------------------------------------------------------------------

TEST_CASE("SetElementRenderDisabled is one undoable step and round-trips", "[seq][finish][commands]") {
    Fx f;
    f.Undo().Clear();
    Recorder rec(f.Exec());
    f.Undo().CreateUndoStep();
    REQUIRE(ElementEdits::SetRenderDisabled(f.se, f.model, true));
    CHECK(f.model->IsRenderDisabled());
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.applied[0]["type"] == "SetElementRenderDisabled");
    CHECK_FALSE(ElementEdits::SetRenderDisabled(f.se, f.model, true)); // no change, nothing recorded
    CHECK(rec.applied.size() == 1);
    f.Undo().UndoLastStep();
    CHECK_FALSE(f.model->IsRenderDisabled());
    f.Undo().RedoLastStep();
    CHECK(f.model->IsRenderDisabled());

    auto cmd = DeserializeCommand(rec.applied[0]);
    REQUIRE(cmd != nullptr);
    nlohmann::json again;
    cmd->Serialize(again);
    CHECK(again == rec.applied[0]);
    CHECK_FALSE(SetElementRenderDisabledCommand(12345, true).Validate(f.se).Ok());
}

TEST_CASE("A model rename follows into the sequence without an undo step", "[seq][finish][commands]") {
    Fx f;
    f.Undo().Clear();
    Recorder rec(f.Exec());
    f.se.RenameModelInViews("Tree", "Oak");
    CHECK(f.model->GetName() == "Oak");
    CHECK(f.se.GetElement("Oak") == f.model);
    CHECK_FALSE(f.Undo().CanUndo());
    REQUIRE(rec.applied.size() == 1);
    CHECK(rec.applied[0]["type"] == "RenameModelElement");
    CHECK(rec.applied[0]["name"] == "Oak");
    auto cmd = DeserializeCommand(rec.applied[0]);
    REQUIRE(cmd != nullptr);
    nlohmann::json again;
    cmd->Serialize(again);
    CHECK(again == rec.applied[0]);
    auto inverse = RenameModelElementCommand(f.model->GetUid(), "Tree").Clone()->Apply(f.se, ApplyMode::Strict);
    REQUIRE(inverse != nullptr);
    CHECK(f.model->GetName() == "Tree");
    REQUIRE(inverse->Apply(f.se, ApplyMode::Strict) != nullptr);
    CHECK(f.model->GetName() == "Oak");
    // strict refuses a name another element already has
    f.Timing("Taken");
    CHECK_FALSE(RenameModelElementCommand(f.model->GetUid(), "Taken").Validate(f.se).Ok());
}

// ---------------------------------------------------------------------------
// Effect symbols
// ---------------------------------------------------------------------------

TEST_CASE("Creating a symbol and linking to it are undoable, replayable steps", "[seq][finish][symbols]") {
    Fx f;
    Effect* a = f.Add(f.layer, "On", 0, 100, "E_TEXTCTRL_Eff_On_Start=10");
    Effect* b = f.Add(f.layer, "Bars", 200, 300);
    f.Undo().Clear();
    std::string start = State(f.se);
    Recorder rec(f.Exec());

    std::string id = SymbolEdits::Create(f.se, "Sym", a);
    REQUIRE_FALSE(id.empty());
    CHECK(a->GetLinkedSymbolId() == id);
    CHECK(SymbolEdits::Create(f.se, "Sym", b).empty()); // name taken
    std::string created = State(f.se);

    REQUIRE(SymbolEdits::Link(f.se, { b }, id) == 1);
    CHECK(b->GetLinkedSymbolId() == id);
    CHECK(b->GetEffectName() == "On");
    CHECK(b->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "10");
    std::string linked = State(f.se);
    REQUIRE(rec.applied.size() == 2);
    CHECK(rec.MemberTypes(1) == std::vector<std::string>{ "ModifyEffect", "SetEffectSymbolLink" });

    f.Undo().UndoLastStep();
    CHECK(State(f.se) == created);
    CHECK_FALSE(b->IsLinkedToSymbol());
    f.Undo().UndoLastStep();
    CHECK(State(f.se) == start);
    CHECK_FALSE(a->IsLinkedToSymbol());
    CHECK(f.se.GetEffectSymbolManager().GetSymbolCount() == 0);

    // replaying the log reproduces both steps
    for (const auto& j : rec.applied) {
        auto cmd = DeserializeCommand(j);
        REQUIRE(cmd != nullptr);
        REQUIRE(cmd->Apply(f.se, ApplyMode::Strict) != nullptr);
    }
    CHECK(State(f.se) == linked);
}

TEST_CASE("Changing a linked effect still updates its symbol and siblings, and undo puts them back", "[seq][finish][symbols]") {
    Fx f;
    Effect* a = f.Add(f.layer, "On", 0, 100, "E_TEXTCTRL_Eff_On_Start=10");
    Effect* b = f.Add(f.layer, "Bars", 200, 300);
    std::string id = SymbolEdits::Create(f.se, "Sym", a);
    REQUIRE(SymbolEdits::Link(f.se, { b }, id) == 1);
    std::string before = State(f.se);
    f.Undo().CreateUndoStep();
    REQUIRE(f.Exec().SetEffectSetting(a, "E_TEXTCTRL_Eff_On_Start", std::string("40")));
    CHECK(b->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "40");
    f.Undo().UndoLastStep();
    CHECK(State(f.se) == before);
}

TEST_CASE("Unlink, rename, delete and convert-all are undoable", "[seq][finish][symbols]") {
    Fx f;
    Effect* a = f.Add(f.layer, "On", 0, 100);
    Effect* b = f.Add(f.layer, "On", 200, 300);
    std::string id = SymbolEdits::Create(f.se, "Sym", a);
    REQUIRE(SymbolEdits::Link(f.se, { b }, id) == 1);
    std::string id2 = SymbolEdits::Create(f.se, "Other", f.Add(f.layer, "Bars", 400, 500));
    f.Undo().Clear();
    std::string start = State(f.se);

    REQUIRE(SymbolEdits::Unlink(f.se, { b }) == 1);
    CHECK_FALSE(b->IsLinkedToSymbol());
    f.Undo().UndoLastStep();
    CHECK(State(f.se) == start);

    REQUIRE(SymbolEdits::Rename(f.se, id, "Renamed"));
    CHECK(f.se.GetEffectSymbolManager().GetSymbol(id)->GetName() == "Renamed");
    f.Undo().UndoLastStep();
    CHECK(State(f.se) == start);

    REQUIRE(SymbolEdits::Delete(f.se, id));
    CHECK_FALSE(a->IsLinkedToSymbol());
    CHECK_FALSE(b->IsLinkedToSymbol());
    CHECK(f.se.GetEffectSymbolManager().GetSymbol(id) == nullptr);
    f.Undo().UndoLastStep();
    CHECK(State(f.se) == start);
    CHECK(a->GetLinkedSymbolId() == id);
    CHECK(b->GetLinkedSymbolId() == id);

    auto [effects, symbols] = SymbolEdits::ConvertAllToEffects(f.se);
    CHECK(effects == 3);
    CHECK(symbols == 2);
    CHECK(f.se.GetEffectSymbolManager().GetSymbolCount() == 0);
    f.Undo().UndoLastStep();
    CHECK(State(f.se) == start);
    CHECK_FALSE(id2.empty());
}

TEST_CASE("Undoing the delete of a linked effect links it again", "[seq][finish][symbols]") {
    Fx f;
    Effect* a = f.Add(f.layer, "On", 0, 100);
    std::string id = SymbolEdits::Create(f.se, "Sym", a);
    SeqObjectId uid = a->GetUid();
    f.Undo().CreateUndoStep();
    REQUIRE(f.Exec().DeleteEffect(a));
    f.Undo().UndoLastStep();
    Effect* back = f.layer->GetEffectFromUid(uid);
    REQUIRE(back != nullptr);
    CHECK(back->GetLinkedSymbolId() == id);
    CHECK(f.se.GetEffectSymbolManager().GetLinkedEffectCount(id) == 1);
}

TEST_CASE("Symbol commands round-trip through JSON and refuse bad targets", "[seq][finish][symbols]") {
    Fx f;
    Effect* a = f.Add(f.layer, "On", 0, 100);
    SetEffectSymbolCommand put("sym_9", EffectSymbolDef{ "N", "On", "E_X=1", "C_BUTTON_Palette1=#FF0000" });
    SetEffectSymbolCommand del("sym_9", std::nullopt);
    SetEffectSymbolLinkCommand link(LocatorOf(a), "sym_9");
    for (const SequenceCommand* c : std::vector<const SequenceCommand*>{ &put, &del, &link }) {
        nlohmann::json j;
        c->Serialize(j);
        auto d = DeserializeCommand(j);
        REQUIRE(d != nullptr);
        nlohmann::json again;
        d->Serialize(again);
        CHECK(again == j);
    }
    CHECK_FALSE(link.Validate(f.se).Ok()); // no such symbol yet
    CHECK_FALSE(del.Validate(f.se).Ok());
    REQUIRE(put.Clone()->Apply(f.se) != nullptr);
    CHECK(link.Validate(f.se).Ok());
    CHECK_FALSE(SetEffectSymbolCommand("sym_10", EffectSymbolDef{ "N", "On", "", "" }).Validate(f.se).Ok()); // name taken
}

// ---------------------------------------------------------------------------
// Uid lookups
// ---------------------------------------------------------------------------

TEST_CASE("Uid lookups stay correct as elements and effects come and go", "[seq][finish][uid]") {
    Fx f;
    TimingElement* t = f.Timing("T");
    SeqObjectId tree = f.model->GetUid();
    SeqObjectId tid = t->GetUid();
    CHECK(f.se.GetElementFromUid(tree) == f.model);
    CHECK(f.se.GetElementFromUid(tid) == t);
    SubModelElement* strand = f.model->GetStrand(0, true);
    REQUIRE(strand != nullptr);
    CHECK(f.se.GetElementFromUid(strand->GetUid()) == strand);

    {
        SeqMutationGuard::Scope setup;
        f.se.MoveSequenceElement(1, 0, MASTER_VIEW);
        CHECK(f.se.GetElementFromUid(tree) == f.model);
        f.se.DeleteElement("T");
        CHECK(f.se.GetElementFromUid(tid) == nullptr);
        Element* t2 = f.se.AddElement(0, "T2", "timing", true, false, false, false, false);
        CHECK(f.se.GetElementFromUid(t2->GetUid()) == t2);
        // an id changed behind the index's back is still found, the old one not
        SeqObjectId old = t2->GetUid();
        t2->SetUid(old + 1000000);
        CHECK(f.se.GetElementFromUid(old) == nullptr);
        CHECK(f.se.GetElementFromUid(old + 1000000) == t2);
    }

    // many effects on one layer: the effect index is used
    std::vector<Effect*> effects;
    for (int i = 0; i < 100; ++i) {
        effects.push_back(f.Add(f.layer, "On", i * 10, i * 10 + 5));
    }
    for (Effect* e : effects) {
        CHECK(f.layer->GetEffectFromUid(e->GetUid()) == e);
    }
    SeqObjectId gone = effects[50]->GetUid();
    REQUIRE(f.Exec().DeleteEffect(effects[50], false));
    CHECK(f.layer->GetEffectFromUid(gone) == nullptr);
    CHECK(f.layer->GetEffectFromUid(effects[51]->GetUid()) == effects[51]);
    SeqObjectId before = effects[10]->GetUid();
    effects[10]->SetUid(before + 1000000);
    CHECK(f.layer->GetEffectFromUid(before) == nullptr);
    CHECK(f.layer->GetEffectFromUid(before + 1000000) == effects[10]);

    f.se.Clear();
    CHECK(f.se.GetElementFromUid(tree) == nullptr);
}

TEST_CASE("An effect panel change can join the undo step that embedded its media", "[seq][finish][undo]") {
    Fx f;
    Effect* e = f.Add(f.layer, "Pictures", 0, 100, "E_TEXTCTRL_Pictures_Filename=old.png");
    f.Undo().Clear();
    std::string before = State(f.se);
    EffectEditCoalescer coalescer;
    REQUIRE(SeqMediaEdits::EmbedImage(f.se, "new.png", xlImage(2, 2), "Add AI Image"));
    coalescer.JoinNextToCurrentStep(f.se);
    SettingsChange change;
    change.keys["E_TEXTCTRL_Pictures_Filename"] = std::string("new.png");
    REQUIRE(coalescer.Apply(f.se, e, std::nullopt, change, SettingsChange{}));
    f.Undo().UndoLastStep();
    CHECK(State(f.se) == before);
    CHECK_FALSE(f.Undo().CanUndo());

    // Without the join they are two steps.
    f.Undo().RedoLastStep();
    EffectEditCoalescer other;
    REQUIRE(SeqMediaEdits::EmbedImage(f.se, "third.png", xlImage(2, 2), "Add AI Image"));
    change.keys["E_TEXTCTRL_Pictures_Filename"] = std::string("third.png");
    REQUIRE(other.Apply(f.se, e, std::nullopt, change, SettingsChange{}));
    f.Undo().UndoLastStep();
    CHECK(e->GetSettings().Get("E_TEXTCTRL_Pictures_Filename", "") == "new.png");
    CHECK(f.se.GetSequenceMedia().GetEntryState("third.png").has_value());
}

// Undo of a sparse import on a big sequence: one modify per element, so
// every undo entry looks up an element, a layer and an effect by id.
TEST_CASE("Benchmark: undo of a sparse import on a big sequence", "[.perf][seq][uid]") {
    Fx f;
    constexpr int kElements = 3000;
    constexpr int kEffects = 200;
    {
        SeqMutationGuard::Scope setup;
        for (int i = 0; i < kElements; ++i) {
            Element* el = f.se.AddElement("M" + std::to_string(i), "model", true, false, false, false, false);
            EffectLayer* l = el->GetEffectLayerCount() > 0 ? el->GetEffectLayer(0) : el->AddEffectLayer();
            for (int k = 0; k < kEffects; ++k) {
                l->AddEffect(0, "On", "", "", k * 100, k * 100 + 50, 0, false);
            }
        }
    }
    f.Undo().Clear();
    {
        SequenceEditCapture capture(f.se, "Import Effects");
        for (int i = 0; i < kElements; ++i) {
            EffectLayer* l = f.se.GetElement("M" + std::to_string(i))->GetEffectLayer(0);
            l->GetEffect(kEffects - 1)->SetSetting("E_TEXTCTRL_Eff_On_Start", "5");
        }
    }
    auto t0 = std::chrono::steady_clock::now();
    f.Undo().UndoLastStep();
    auto t1 = std::chrono::steady_clock::now();
    f.Undo().RedoLastStep();
    auto t2 = std::chrono::steady_clock::now();
    WARN("undo " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms, redo "
                 << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms for " << kElements
                 << " modifies over " << kElements << " elements x " << kEffects << " effects");
    CHECK(f.se.GetElement("M7")->GetEffectLayer(0)->GetEffect(kEffects - 1)->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "5");
}
