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

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "SequenceIds.h"

class SequenceElements;
class SequenceCommand;
enum class CommandType : int;
class Effect;
class EffectLayer;
class Element;

enum UNDO_ACTIONS
{
    UNDO_MARKER,   // use this to mark the beginning of a group of actions that should be undone together
    UNDO_EFFECT_ADDED,
    UNDO_EFFECT_DELETED,
    UNDO_EFFECT_MODIFIED,
    UNDO_EFFECT_MOVED,
    UNDO_LAYER_ADDED,
    UNDO_LAYER_REMOVED,
    UNDO_BATCH,    // part of a labelled CommandExecutor batch
};

// Undo history of a sequence. Each recorded change is stored as the command
// that reverses it; undoing applies those commands (in lenient mode) and
// keeps their inverses for redo. Changes are grouped into steps by markers.
// CommandExecutor records into it.
class UndoManager
{
    public:
        explicit UndoManager(SequenceElements* parent);
        virtual ~UndoManager();
    
        void Clear();
        void ClearRedo();
        void RemoveUnusedMarkers();
        bool ChangeCaptured();
        // False when refused because an undo/redo is already running.
        bool UndoLastStep();
        bool RedoLastStep();
        void CreateUndoStep();
        bool CanUndo();
        bool CanRedo();
        // Changes whenever a new undo step is started, so a caller can tell
        // whether an operation recorded one.
        unsigned int GetStepSerial() const { return mStepSerial; }
        void SetCaptureUndo( bool value );
        bool GetCaptureUndo() { return mCaptureUndo; }
        void CancelLastStep();
        std::string GetUndoString();
        std::string GetRedoString();

        // Adds a change to the current step: `inverse` undoes a command of type
        // `appliedType` that has just been applied. CommandExecutor uses this;
        // `label` names a batch (appliedType == Batch).
        void RecordInverse(CommandType appliedType, const std::string& label, std::unique_ptr<SequenceCommand> inverse);

        // Memory-cap support (iPad memory-pressure mitigation).
        // When `maxSteps > 0`, starting a step drops the oldest whole
        // steps until at most `maxSteps` remain. 0
        // disables the cap (the default). The redo list isn't capped
        // separately — it's bounded by the undo history that
        // feeds it.
        void SetMaxSteps(size_t maxSteps) { mMaxSteps = maxSteps; EnforceMaxSteps(); }
        size_t GetMaxSteps() const { return mMaxSteps; }

        // Changes whenever the undo history changes (a step or entry is
        // added or removed, undo, redo, clear). Code that keeps adding to
        // the step it recorded last can compare this to know nothing else
        // has been recorded or undone since.
        uint64_t GetHistorySerial() const { return mHistorySerial; }

        // The document changed outside undo/redo (recorded or not): drops
        // the redo history, which no longer applies to it.
        void DocumentEdited();
        bool IsReplaying() const { return mReplayDepth > 0; }
        // Whether undoing (or redoing) the next step needs the render stopped
        // first, so a host can stop it before the replay, where waiting may
        // still run the event loop.
        bool NextStepNeedsRenderStopped(bool redo);
    protected:
        struct Entry {
            UNDO_ACTIONS kind = UNDO_MARKER;
            std::string label;                      // UNDO_BATCH only
            std::unique_ptr<SequenceCommand> inverse; // null for markers
        };

        void Push(UNDO_ACTIONS kind, std::unique_ptr<SequenceCommand> inverse, const std::string& label = "");
        void ProcessUndoStep(std::vector<Entry> &fromList, std::vector<Entry> &toList);
        bool Replay(std::vector<Entry>& fromList, std::vector<Entry>& toList);
        bool StopRenderIfStepNeeds(const std::vector<Entry>& list);
        static bool StepNeedsRenderStopped(const std::vector<Entry>& list);
        // Drop the oldest whole steps while there are more than
        // `mMaxSteps`. No-op when `mMaxSteps == 0`.
        void EnforceMaxSteps();

    private:
        std::vector<Entry> mUndoSteps;
        std::vector<Entry> mRedoSteps;
        SequenceElements* mParentSequence;
        bool mCaptureUndo;
        size_t mMaxSteps = 0;
        int mReplayDepth = 0;
        uint64_t mHistorySerial = 0;
        unsigned int mStepSerial = 0;

};
