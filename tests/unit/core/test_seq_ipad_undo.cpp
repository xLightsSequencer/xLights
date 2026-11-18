#include <catch2/catch_test_macros.hpp>

#include "render/CommandExecutor.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceCommands.h"
#include "render/SequenceElementCommands.h"
#include "render/SequenceElements.h"
#include "render/SequenceFaces.h"
#include "render/SequenceMutationGuard.h"
#include "render/SongStructureManager.h"
#include "render/UndoManager.h"

#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// The iPad records its element-level sequence edits (timing tracks, row
// order, tags, sequence faces, song structure) as core undo steps and
// registers each step with Foundation's undo manager, so these tests run
// the exact sequences the XLSequenceDocument bridge runs: every op in its
// own ScopedCommandBatch (or NewElementsRecorder) with undo recorded. Each
// must make exactly one step when it changes something and none when it
// doesn't, since the Swift side registers an undo only when the step
// serial moves, and undo and redo must round-trip the whole state.

namespace {

struct GuardMode {
    SeqMutationGuard::Mode prev;
    explicit GuardMode(SeqMutationGuard::Mode m) : prev(SeqMutationGuard::GetMode()) {
        SeqMutationGuard::SetMode(m);
        SeqMutationGuard::ResetViolationCount();
    }
    ~GuardMode() { SeqMutationGuard::SetMode(prev); }
};

struct Fixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    Fixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
        se.SetFrequency(20); // 50ms frames
        se.SetSequenceEnd(10000);
    }
    ModelElement* Model(const std::string& name) {
        SeqMutationGuard::Scope setup;
        auto* m = dynamic_cast<ModelElement*>(se.AddElement(name, "model", true, false, false, false, false));
        m->AddEffectLayer();
        return m;
    }
    TimingElement* Timing(const std::string& name, int fixedMS = 0) {
        SeqMutationGuard::Scope setup;
        auto* t = dynamic_cast<TimingElement*>(se.AddElement(se.GetNumberOfTimingElements(), name, "timing", true, false, false, false, false));
        t->AddEffectLayer();
        t->SetFixedTiming(fixedMS);
        return t;
    }
    Effect* Add(EffectLayer* l, const std::string& name, int start, int end, const std::string& settings = "") {
        SeqMutationGuard::Scope setup;
        return l->AddEffect(0, name, settings, "", start, end, 0, false);
    }
    UndoManager& Undo() { return se.get_undo_mgr(); }
    unsigned Serial() const { return const_cast<SequenceElements&>(se).get_undo_mgr().GetStepSerial(); }

    // Everything the ops below can change.
    std::string State() const {
        std::ostringstream out;
        out << DescribeSequence(se, true);
        for (int i = 0; i < (int)se.GetNumberOfTimingElements(); ++i) {
            auto* t = const_cast<SequenceElements&>(se).GetTimingElement(i);
            out << "fixed " << t->GetName() << "=" << t->GetFixedTiming() << "\n";
        }
        for (int i = 0; i < SequenceElements::TagCount; ++i) {
            out << "tag" << i << "=" << se.GetTagPosition(i) << "\n";
        }
        const auto& ssm = const_cast<SequenceElements&>(se).GetSongStructureManager();
        for (const auto& v : ssm.GetViews()) {
            out << "view '" << v.name << "' next " << v.nextId << "\n";
            for (const auto& r : v.regions) {
                out << "  " << r.id << " " << r.startTimeMS << "-" << r.endTimeMS << " '" << r.name << "' " << r.colorARGB << "\n";
            }
        }
        for (const auto& [name, def] : const_cast<SequenceElements&>(se).GetSequenceFaces().GetFaces()) {
            out << "face '" << name << "'";
            for (const auto& [k, v] : def) {
                out << " " << k << "=" << v;
            }
            out << "\n";
        }
        return out.str();
    }

    // Runs `op`; requires it to make exactly one undo step named `label`,
    // then undoes it (back to the state before) and redoes it (back to the
    // state after).
    template <typename F>
    void OneStep(const std::string& label, F&& op) {
        std::string before = State();
        unsigned serial = Serial();
        op();
        std::string after = State();
        REQUIRE(after != before);
        REQUIRE(Serial() == serial + 1);
        CHECK(Undo().GetUndoString() == "Undo: " + label);
        Undo().UndoLastStep();
        CHECK(State() == before);
        REQUIRE(Undo().CanRedo());
        Undo().RedoLastStep();
        CHECK(State() == after);
    }

    // Runs `op`; requires it to record nothing.
    template <typename F>
    void NoStep(F&& op) {
        unsigned serial = Serial();
        op();
        CHECK(Serial() == serial);
    }
};

} // namespace

TEST_CASE("iPad timing-track ops are one undo step each", "[seq][elements][undo][ipad]") {
    Fixture f;
    GuardMode mode(SeqMutationGuard::Mode::Count);
    TimingElement* beats = f.Timing("Beats", 50);
    f.Add(beats->GetEffectLayer(0), "", 0, 50);
    f.Add(beats->GetEffectLayer(0), "", 50, 100);
    f.Timing("Lyrics");
    ModelElement* tree = f.Model("Tree");
    f.Add(tree->GetEffectLayer(0), "Faces", 0, 500, "E_CHOICE_Faces_TimingTrack=Lyrics");
    f.Model("House");

    // -makeTimingTrackVariableAtRow:
    f.OneStep("Make Timing Track Variable", [&] {
        ScopedCommandBatch step(f.se, "Make Timing Track Variable");
        ElementEdits::SetTimingFixed(f.se, beats, 0);
    });
    CHECK(beats->GetFixedTiming() == 0);
    f.NoStep([&] {
        ScopedCommandBatch step(f.se, "Make Timing Track Variable");
        ElementEdits::SetTimingFixed(f.se, beats, 0);
    });

    // -renameTimingTrackAtIndex:newName: (effect references follow)
    f.OneStep("Rename Timing Track", [&] {
        ScopedCommandBatch step(f.se, "Rename Timing Track");
        REQUIRE(ElementEdits::RenameTimingTrack(f.se, "Lyrics", "Words"));
    });
    CHECK(tree->GetEffectLayer(0)->GetEffect(0)->GetSetting("E_CHOICE_Faces_TimingTrack") == "Words");

    // -moveTopLevelRowFrom:toBefore: in the master view
    f.OneStep("Move Row", [&] {
        ScopedCommandBatch step(f.se, "Move Row");
        REQUIRE(ElementEdits::MoveElement(f.se, 3, 2, MASTER_VIEW));
    });
    CHECK(f.se.GetElement(2)->GetName() == "House");

    // -deleteTimingTrackAtIndex: takes the marks with it; undo brings them back
    f.OneStep("Delete Timing Track", [&] {
        ScopedCommandBatch step(f.se, "Delete Timing Track");
        REQUIRE(ElementEdits::DeleteElement(f.se, "Beats"));
    });
    CHECK(f.se.GetElement("Beats") == nullptr);
    f.NoStep([&] {
        ScopedCommandBatch step(f.se, "Delete Timing Track");
        CHECK_FALSE(ElementEdits::DeleteElement(f.se, "Beats"));
    });
    CHECK(SeqMutationGuard::ViolationCount() == 0);
}

TEST_CASE("iPad row moves outside the master view record no undo step", "[seq][elements][undo][ipad]") {
    Fixture f;
    f.Model("A");
    f.Model("B");
    f.se.AddView("Mine");
    f.se.PopulateView("A,B", 1);
    f.NoStep([&] {
        ScopedCommandBatch step(f.se, "Move Row");
        REQUIRE(ElementEdits::MoveElement(f.se, 1, 0, 1));
    });
    CHECK(f.se.GetElement(0, 1)->GetName() == "B");
    CHECK_FALSE(f.Undo().CanUndo());
}

TEST_CASE("iPad timing tracks added through NewElementsRecorder undo and redo with their marks", "[seq][elements][undo][ipad]") {
    Fixture f;
    GuardMode mode(SeqMutationGuard::Mode::Count);
    f.Model("Tree");

    // -addLyricTimingTrackNamed:… / -importNotesFromPath:…: marks put in
    // directly inside the recorder.
    f.OneStep("Generate AI Lyrics", [&] {
        NewElementsRecorder recorder(f.se, "Generate AI Lyrics");
        TimingElement* t = ElementEdits::AddTimingTrack(f.se, -1, "AutoGen", "");
        REQUIRE(t != nullptr);
        t->GetEffectLayer(0)->AddEffect(0, "hello", "", "", 0, 400, 0, false);
        t->GetEffectLayer(0)->AddEffect(0, "world", "", "", 400, 900, 0, false);
    });
    REQUIRE(f.se.GetElement("AutoGen") != nullptr);
    CHECK(dynamic_cast<TimingElement*>(f.se.GetElement("AutoGen"))->GetEffectLayer(0)->GetEffectCount() == 2);

    // -addTimingTrackNamed: followed by unrecorded marks (chord, tempo,
    // onset and subdivide generators, tempo/chords in the Add Timing
    // sheet): the step adds an empty track, yet undo removes the marks
    // with it and redo brings them back, because undo snapshots the track
    // as it is when it is deleted.
    std::string before = f.State();
    unsigned serial = f.Serial();
    {
        NewElementsRecorder recorder(f.se, "Add Timing Track");
        REQUIRE(ElementEdits::AddTimingTrack(f.se, -1, "Tempo", "") != nullptr);
    }
    REQUIRE(f.Serial() == serial + 1);
    auto* tempo = dynamic_cast<TimingElement*>(f.se.GetElement("Tempo"));
    REQUIRE(tempo != nullptr);
    for (int ms = 0; ms < 2000; ms += 500) {
        REQUIRE(f.se.GetCommandExecutor().AddEffect(tempo->GetEffectLayer(0), "", "", "", ms, ms + 500, 0, false, false) != nullptr);
    }
    CHECK(f.Serial() == serial + 1);
    std::string after = f.State();
    f.Undo().UndoLastStep();
    CHECK(f.State() == before);
    f.Undo().RedoLastStep();
    CHECK(f.State() == after);
    CHECK(dynamic_cast<TimingElement*>(f.se.GetElement("Tempo"))->GetEffectLayer(0)->GetEffectCount() == 4);

    // An import that adds nothing records nothing.
    f.NoStep([&] { NewElementsRecorder recorder(f.se, "Import Timing"); });
    CHECK(SeqMutationGuard::ViolationCount() == 0);
}

TEST_CASE("iPad tag edits are one undo step each", "[seq][elements][undo][ipad]") {
    Fixture f;
    // -setTagPositionAtIndex:positionMS:
    f.OneStep("Set Tag", [&] {
        ScopedCommandBatch step(f.se, "Set Tag");
        ElementEdits::SetTagPosition(f.se, 3, 1500);
    });
    f.OneStep("Set Tag", [&] {
        ScopedCommandBatch step(f.se, "Set Tag");
        ElementEdits::SetTagPosition(f.se, 5, 2500);
    });
    f.NoStep([&] {
        ScopedCommandBatch step(f.se, "Set Tag");
        ElementEdits::SetTagPosition(f.se, 3, 1500);
    });
    f.OneStep("Clear Tag", [&] {
        ScopedCommandBatch step(f.se, "Clear Tag");
        ElementEdits::SetTagPosition(f.se, 5, -1);
    });
    // -clearAllTags
    f.OneStep("Delete All Tags", [&] {
        ScopedCommandBatch batch(f.se, "Delete All Tags");
        for (int i = 0; i < SequenceElements::TagCount; ++i) {
            ElementEdits::SetTagPosition(f.se, i, -1);
        }
    });
    f.NoStep([&] {
        ScopedCommandBatch batch(f.se, "Delete All Tags");
        for (int i = 0; i < SequenceElements::TagCount; ++i) {
            ElementEdits::SetTagPosition(f.se, i, -1);
        }
    });
}

TEST_CASE("iPad sequence-face edits are one undo step each", "[seq][elements][undo][ipad]") {
    Fixture f;
    GuardMode mode(SeqMutationGuard::Mode::Count);
    ModelElement* tree = f.Model("Tree");
    Effect* faces = f.Add(tree->GetEffectLayer(0), "Faces", 0, 500, "E_CHOICE_Faces_FaceDefinition=Singer");
    using Def = std::map<std::string, std::string>;

    // -addSequenceFace:
    f.OneStep("Sequence Faces", [&] {
        ElementEdits::EditSequenceFaces(f.se, [](SequenceFaces& sf) { sf.SetFace("Singer", Def{ { "ImagePlacement", "Scaled" } }); });
    });
    // -setSequenceFaceImage:… / -setSequenceFacePlacement:…
    f.OneStep("Sequence Faces", [&] {
        ElementEdits::EditSequenceFaces(f.se, [](SequenceFaces& sf) {
            sf.SetFace("Singer", Def{ { "ImagePlacement", "Scaled" }, { "Mouth-AI-EyesOpen", "ai.png" } });
        });
    });
    f.NoStep([&] {
        ElementEdits::EditSequenceFaces(f.se, [](SequenceFaces& sf) {
            sf.SetFace("Singer", Def{ { "ImagePlacement", "Scaled" }, { "Mouth-AI-EyesOpen", "ai.png" } });
        });
    });
    // -renameSequenceFace:to: also repoints the effects that use the face,
    // all in one step.
    f.OneStep("Rename Sequence Face", [&] {
        ScopedCommandBatch batch(f.se, "Rename Sequence Face");
        REQUIRE(ElementEdits::EditSequenceFaces(f.se, [](SequenceFaces& sf) { sf.RenameFace("Singer", "Lead"); }));
        REQUIRE(f.se.GetCommandExecutor().SetEffectSetting(faces, "E_CHOICE_Faces_FaceDefinition", "Lead"));
    });
    CHECK(faces->GetSetting("E_CHOICE_Faces_FaceDefinition") == "Lead");
    // -removeSequenceFace:
    f.OneStep("Sequence Faces", [&] {
        ElementEdits::EditSequenceFaces(f.se, [](SequenceFaces& sf) { sf.RemoveFace("Lead"); });
    });
    CHECK(SeqMutationGuard::ViolationCount() == 0);
}

TEST_CASE("iPad song-structure edits are one undo step each", "[seq][elements][undo][ipad]") {
    Fixture f;
    GuardMode mode(SeqMutationGuard::Mode::Count);
    SongStructureManager& ssm = f.se.GetSongStructureManager();
    auto edit = [&](const char* label, const std::function<void(SongStructureManager&)>& e) {
        ScopedCommandBatch step(f.se, label);
        return ElementEdits::EditSongStructure(f.se, e);
    };

    // -addSongStructureBoundaryAtMS: on a sequence with no views
    f.OneStep("Add Song Boundary", [&] {
        edit("Add Song Boundary", [&](SongStructureManager& m) {
            if (m.GetViewCount() == 0) {
                m.SetActiveView(m.AddView("Default"));
            }
            m.AddBoundary(4000, f.se.GetSequenceEnd());
        });
    });
    REQUIRE(ssm.GetRegionCount() == 2);
    f.OneStep("Add Song Boundary", [&] { edit("Add Song Boundary", [&](SongStructureManager& m) { m.AddBoundary(7000, f.se.GetSequenceEnd()); }); });
    f.OneStep("Move Song Boundary", [&] { edit("Move Song Boundary", [](SongStructureManager& m) { m.MoveBoundary(4000, 3000); }); });
    f.OneStep("Rename Song Region", [&] { edit("Rename Song Region", [](SongStructureManager& m) { m.SetRegionName(1, "Chorus"); }); });
    // the region editor's name + color as one step
    f.OneStep("Edit Song Region", [&] {
        ScopedCommandBatch outer(f.se, "Edit Song Region");
        edit("Rename Song Region", [](SongStructureManager& m) { m.SetRegionName(0, "Intro"); });
        edit("Recolor Song Region", [](SongStructureManager& m) { m.SetRegionColor(0, 0x40FF0000); });
    });
    f.NoStep([&] { edit("Rename Song Region", [](SongStructureManager& m) { m.SetRegionName(1, "Chorus"); }); });
    f.OneStep("Delete Song Boundary", [&] { edit("Delete Song Boundary", [](SongStructureManager& m) { m.DeleteBoundary(7000); }); });
    f.OneStep("Add Song View", [&] { edit("Add Song View", [](SongStructureManager& m) { m.AddView("Alt"); }); });
    f.OneStep("Duplicate Song View", [&] { edit("Duplicate Song View", [](SongStructureManager& m) { m.DuplicateView(0, "Copy"); }); });
    f.OneStep("Rename Song View", [&] { edit("Rename Song View", [](SongStructureManager& m) { m.RenameView(1, "Verse"); }); });
    f.OneStep("Delete Song View", [&] { edit("Delete Song View", [](SongStructureManager& m) { m.DeleteView(2); }); });
    // The active view is per-user: switching it records nothing.
    f.NoStep([&] { edit("Song View", [](SongStructureManager& m) { m.SetActiveView(1); }); });
    CHECK(SeqMutationGuard::ViolationCount() == 0);
}

TEST_CASE("SongStructureLiveEdit records a boundary drag as one undo step", "[seq][elements][undo][ipad]") {
    Fixture f;
    GuardMode mode(SeqMutationGuard::Mode::Count);
    SongStructureManager& ssm = f.se.GetSongStructureManager();
    {
        ScopedCommandBatch step(f.se, "Add Song Boundary");
        ElementEdits::EditSongStructure(f.se, [&](SongStructureManager& m) {
            m.SetActiveView(m.AddView("Default"));
            m.AddBoundary(4000, 10000);
            m.AddBoundary(7000, 10000);
        });
    }
    REQUIRE(ssm.GetRegionCount() == 3);
    std::string before = f.State();
    unsigned serial = f.Serial();

    SECTION("live moves record nothing; Commit records the net move once") {
        {
            SongStructureLiveEdit drag(f.se);
            int at = 4000;
            for (int to : { 4200, 4600, 5100, 5000 }) {
                drag.Apply([&](SongStructureManager& m) { m.MoveBoundary(at, to); });
                at = ssm.GetRegion(1).startTimeMS;
                CHECK(at == to);
                CHECK(f.Serial() == serial);
            }
            // clamped short of the next boundary
            drag.Apply([&](SongStructureManager& m) { m.MoveBoundary(at, 9000); });
            CHECK(ssm.GetRegion(1).startTimeMS == 6999);
            drag.Apply([&](SongStructureManager& m) { m.MoveBoundary(6999, 5500); });
            CHECK(f.Serial() == serial);
            CHECK(drag.Commit("Move Song Boundary"));
            CHECK_FALSE(drag.IsOpen());
        }
        CHECK(f.Serial() == serial + 1);
        CHECK(f.Undo().GetUndoString() == "Undo: Move Song Boundary");
        CHECK(ssm.GetRegion(0).endTimeMS == 5500);
        CHECK(ssm.GetRegion(1).startTimeMS == 5500);
        std::string after = f.State();
        f.Undo().UndoLastStep();
        CHECK(f.State() == before);
        f.Undo().RedoLastStep();
        CHECK(f.State() == after);
        f.Undo().UndoLastStep();
        CHECK(f.State() == before);
        // one step: undoing again reaches the setup step
        CHECK(f.Undo().GetUndoString() == "Undo: Add Song Boundary");
    }
    SECTION("a drag that ends where it started records nothing") {
        SongStructureLiveEdit drag(f.se);
        drag.Apply([&](SongStructureManager& m) { m.MoveBoundary(4000, 4500); });
        drag.Apply([&](SongStructureManager& m) { m.MoveBoundary(4500, 4000); });
        CHECK_FALSE(drag.Commit("Move Song Boundary"));
        CHECK(f.Serial() == serial);
        CHECK(f.State() == before);
    }
    SECTION("Cancel, or destroying it uncommitted, puts the boundary back") {
        {
            SongStructureLiveEdit drag(f.se);
            drag.Apply([&](SongStructureManager& m) { m.MoveBoundary(4000, 4500); });
            CHECK(f.State() != before);
            drag.Cancel();
            CHECK(f.State() == before);
            drag.Apply([&](SongStructureManager& m) { m.MoveBoundary(4000, 4500); });
            CHECK(f.State() == before);
            CHECK_FALSE(drag.Commit("Move Song Boundary"));
        }
        {
            SongStructureLiveEdit drag(f.se);
            drag.Apply([&](SongStructureManager& m) { m.MoveBoundary(7000, 8000); });
        }
        CHECK(f.State() == before);
        CHECK(f.Serial() == serial);
    }
    SECTION("the active view stays the user's") {
        {
            ScopedCommandBatch step(f.se, "Add Song View");
            ElementEdits::EditSongStructure(f.se, [](SongStructureManager& m) { m.AddView("Alt"); });
        }
        ssm.SetActiveView(0);
        SongStructureLiveEdit drag(f.se);
        drag.Apply([&](SongStructureManager& m) { m.MoveBoundary(4000, 4500); });
        CHECK(drag.Commit("Move Song Boundary"));
        CHECK(ssm.GetActiveViewIndex() == 0);
        f.Undo().UndoLastStep();
        CHECK(ssm.GetActiveViewIndex() == 0);
        CHECK(ssm.GetRegion(1).startTimeMS == 4000);
    }
    CHECK(SeqMutationGuard::ViolationCount() == 0);
}
