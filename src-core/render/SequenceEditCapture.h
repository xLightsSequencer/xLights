#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "SequenceIds.h"
#include "SequenceMutationGuard.h"
#include "../utils/UtilClasses.h"

class SequenceElements;
class SequenceCommand;
class Element;
class EffectLayer;
class MediaCacheEntry;

// Records an edit made by code that changes the sequence directly
// (importers, file conversions, whole-sequence tools) as commands.
//
// The constructor snapshots the sequence and opens an executor batch named
// `label`; Commit() (or the destructor) compares the sequence with the
// snapshot, records the difference into that batch as ordinary commands and
// closes it, so the whole edit is one undo step. It compares:
//   - effects and layers (AddEffect, DeleteEffect, MoveEffect, ModifyEffect,
//     effect symbol links, AddLayer, RemoveLayer, SetLayerName,
//     ReplaceLayerContents),
//   - element render-disable flags and fixed timing intervals,
//   - elements added directly (AddElement, with everything put into them)
//     and elements deleted directly (DeleteElement, recorded as it happens),
//   - sequence face definitions,
//   - embedded media entries added, replaced or removed. File references
//     that appear or go away are render-time registrations and are left out.
//
// While a capture is open, commands for state it compares are folded into
// its result instead of being recorded on their own. Other commands
// (element moves, renames, media commands, ...) are recorded into the batch
// as usual, in order; elements a command adds are compared from then on, and
// elements added directly before it are recorded ahead of it. A capture
// opened inside another one does nothing of its own, and a
// NewElementsRecorder opened inside a capture leaves the recording to it.
//
// Direct edits the capture does not compare (element moves and renames,
// timeline tags, song structure, media embed/extract/rename) still trip the
// mutation guard. Direct changes to an element that is later deleted
// directly are recorded only as far as its layers survive.
class SequenceEditCapture {
public:
    SequenceEditCapture(SequenceElements& seq, std::string label, bool recordUndo = true);
    ~SequenceEditCapture();
    SequenceEditCapture(const SequenceEditCapture&) = delete;
    SequenceEditCapture& operator=(const SequenceEditCapture&) = delete;

    // Records the change now. Later calls (and the destructor) do nothing.
    void Commit();
    // Ends the capture without recording the direct changes (commands
    // recorded into its batch meanwhile stay recorded).
    void Discard();
    // After the capture ends: whether it recorded an undo step.
    bool MadeUndoStep() const { return _madeUndoStep; }

    // A layer with more than this many effects added, removed or changed,
    // making up at least half of it, is recorded as a single
    // ReplaceLayerContents.
    static constexpr size_t kReplaceLayerThreshold = 32;

    struct EffectSnap {
        SeqObjectId uid = kNoSeqObjectId;
        std::string name;
        int startTimeMS = 0;
        int endTimeMS = 0;
        bool isProtected = false;
        SettingsMap settings;
        SettingsMap palette;
        std::string linkedSymbol;
    };
    struct LayerSnap {
        SeqObjectId uid = kNoSeqObjectId;
        std::string name;
        std::vector<EffectSnap> effects;
    };
    struct ElementSnap {
        SeqObjectId uid = kNoSeqObjectId;
        bool renderDisabled = false;
        bool timing = false;
        int fixedTimingMS = 0;
        std::vector<LayerSnap> layers;
        std::vector<LayerSnap> nodeLayers;
    };
    using Snapshot = std::unordered_map<SeqObjectId, ElementSnap>;

    static Snapshot Take(const SequenceElements& seq);
    // The commands that turn `before` into the current state of `seq`, in
    // order, with the command undoing each. Elements only, not media or faces.
    static void Diff(const Snapshot& before, const SequenceElements& seq, std::vector<std::unique_ptr<SequenceCommand>>& applied,
                     std::vector<std::unique_ptr<SequenceCommand>>& inverses);

private:
    friend class CommandExecutor;
    // The executor is about to record `cmd` (already applied) into the
    // batch: records elements added directly so far, ahead of it.
    void BeforeRecording(const SequenceCommand* cmd);
    // `cmd` was recorded into the batch: start comparing what it created,
    // and treat the media and faces it changed as the new baseline.
    void AfterRecording(const SequenceCommand& cmd);
    // A command just applied: elements nothing recorded yet are its own
    // (direct additions were recorded before it ran).
    void NoteCommandAdds();
    // SequenceElements is deleting `el` directly.
    void ElementDeleting(Element* el, bool keepInViews);
    void End(bool record);
    // Elements added directly that nothing has recorded yet, as AddElement
    // commands (master-view order), now treated as known.
    void TakeDirectAdds(const SequenceCommand* except, std::vector<std::unique_ptr<SequenceCommand>>& applied,
                        std::vector<std::unique_ptr<SequenceCommand>>& inverses);
    void NoteElement(Element* el);
    void DiffMedia(std::vector<std::unique_ptr<SequenceCommand>>& applied, std::vector<std::unique_ptr<SequenceCommand>>& inverses);
    void DiffFaces(std::vector<std::unique_ptr<SequenceCommand>>& applied, std::vector<std::unique_ptr<SequenceCommand>>& inverses);

    SequenceElements& _seq;
    std::string _label;
    bool _recordUndo;
    bool _active = false; // the outermost capture on this executor
    bool _open = true;
    bool _madeUndoStep = false;
    bool _flushing = false;
    Snapshot _before;
    std::unordered_set<SeqObjectId> _knownTop; // master-view elements recorded or present at the start
    std::map<std::string, std::shared_ptr<const MediaCacheEntry>> _media;
    std::map<std::string, std::map<std::string, std::string>> _faces;
    std::unique_ptr<SeqMutationGuard::CaptureScope> _scope;
};
