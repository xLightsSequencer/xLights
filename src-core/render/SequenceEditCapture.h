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
#include <unordered_map>
#include <vector>

#include "SequenceIds.h"
#include "SequenceMutationGuard.h"
#include "../utils/UtilClasses.h"

class SequenceElements;
class SequenceCommand;
class Element;
class EffectLayer;

// Records an edit made by code that changes effects and layers directly
// (importers, file conversions, whole-sequence tools) as commands.
//
// The constructor snapshots every effect and layer of the sequence and opens
// an executor batch named `label`; Commit() (or the destructor) compares the
// sequence with the snapshot, records the difference into that batch as
// ordinary commands (AddEffect, DeleteEffect, MoveEffect, ModifyEffect,
// AddLayer, RemoveLayer, SetLayerName, ReplaceLayerContents) and closes it,
// so the whole edit is one undo step.
//
// While a capture is open, effect and layer commands the executor applies
// are folded into the capture's result instead of being recorded on their
// own. Other commands (elements, media, ...) are recorded into the batch as
// usual, and an element they add is compared from that point on, so effects
// put into a new element are recorded too. A capture opened inside another
// one does nothing of its own.
//
// Changes made directly to an element that is then removed by a command are
// not recorded. Elements added or removed directly, and node layers that
// appear, are not recorded either.
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
        std::vector<LayerSnap> layers;
        std::vector<LayerSnap> nodeLayers;
    };
    using Snapshot = std::unordered_map<SeqObjectId, ElementSnap>;

    static Snapshot Take(const SequenceElements& seq);
    // The commands that turn `before` into the current state of `seq`, in
    // order, with the command undoing each.
    static void Diff(const Snapshot& before, const SequenceElements& seq, std::vector<std::unique_ptr<SequenceCommand>>& applied,
                     std::vector<std::unique_ptr<SequenceCommand>>& inverses);

private:
    friend class CommandExecutor;
    // Starts comparing elements that didn't exist when the snapshot was taken.
    void NoteNewElements();
    void End(bool record);

    SequenceElements& _seq;
    std::string _label;
    bool _recordUndo;
    bool _active = false; // the outermost capture on this executor
    bool _open = true;
    bool _madeUndoStep = false;
    Snapshot _before;
    std::unique_ptr<SeqMutationGuard::EffectEditScope> _scope;
};
