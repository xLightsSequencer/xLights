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

// Commands for the sequence-level document state outside effects and
// layers: which models and timing tracks are in the sequence and their order
// in the master view, timing track names and properties, timeline tags,
// sequence face definitions and song structure.
//
// Element visibility, collapsed state, view membership, the active timing
// track and the active song-structure view are per-user state; commands
// never serialize them. An element snapshot carries the timing track's view
// list as a local hint (like EffectState::selected) so an undone delete puts
// the track back into the views it was in.

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "SequenceCommands.h"
#include "SequenceMutationGuard.h"
#include "SongStructureManager.h"

class SequenceElements;
class SequenceFaces;
class SongStructureManager;
class TimingElement;

struct LayerSnapshot {
    SeqObjectId uid = kNoSeqObjectId;
    std::string name;
    std::vector<EffectState> effects;
    bool operator==(const LayerSnapshot&) const = default;
};

// A submodel or strand row of a model element.
struct SubElementSnapshot {
    bool strand = false;
    std::string name;     // submodel name (submodels only)
    int strandIndex = -1; // strands only
    SeqObjectId uid = kNoSeqObjectId;
    std::vector<LayerSnapshot> layers;
    // Strand node layers: every node layer's id, in order, plus the
    // contents of the ones that hold effects (index into nodeLayerUids).
    std::vector<SeqObjectId> nodeLayerUids;
    std::map<int, std::vector<EffectState>> nodeEffects;
    bool operator==(const SubElementSnapshot&) const = default;
};

// Everything needed to re-create a model or timing element in a sequence.
struct ElementSnapshot {
    SeqObjectId uid = kNoSeqObjectId;
    bool timing = false;
    std::string name;
    bool renderDisabled = false;
    // timing only
    std::string subType;
    int fixedTimingMS = 0;
    std::vector<LayerSnapshot> layers;
    std::vector<SubElementSnapshot> subElements; // models only
    std::string views; // local hint only; not serialized

    static ElementSnapshot Of(const Element& e);
    bool SameDocumentState(const ElementSnapshot& o) const;
};

// Adds an element to the sequence, right after `after` in the master view
// (first when `after` is kNoSeqObjectId). If `after` is gone, lenient mode
// falls back to `indexHint` (clamped); strict mode fails with TargetMissing.
// A snapshot uid of kNoSeqObjectId is replaced by a fresh id, as are layer
// ids. A model element with no layers gets one empty layer; its submodel
// and strand rows come from the model in the layout and take the ids and
// contents the snapshot gives them.
class AddElementCommand : public SequenceCommand {
public:
    AddElementCommand(ElementSnapshot state, SeqObjectId after, int indexHint = -1);
    CommandType Type() const override { return CommandType::AddElement; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const ElementSnapshot& GetState() const { return _state; }
    SeqObjectId GetUid() const { return _state.uid; }
    SeqObjectId GetAfter() const { return _after; }
    int GetIndexHint() const { return _indexHint; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    ElementSnapshot _state;
    SeqObjectId _after;
    int _indexHint;
};

// Removes a model or timing element (with every layer, row and effect
// beneath it) from the sequence. Strict mode refuses an element holding a
// locked effect. The inverse re-creates it with its ids, place and contents.
// Like SequenceElements::DeleteElement it also drops a model from the view
// definitions, unless keepInViews is set.
class DeleteElementCommand : public SequenceCommand {
public:
    explicit DeleteElementCommand(SeqObjectId element, bool keepInViews = false);
    CommandType Type() const override { return CommandType::DeleteElement; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    SeqObjectId GetElement() const { return _element; }
    bool GetKeepInViews() const { return _keepInViews; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    SeqObjectId _element;
    bool _keepInViews;
};

// Moves an element in the master view to right after `after` (first when
// kNoSeqObjectId). Missing `after`: lenient uses `indexHint`, strict fails.
class MoveElementCommand : public SequenceCommand {
public:
    MoveElementCommand(SeqObjectId element, SeqObjectId after, int indexHint = -1);
    CommandType Type() const override { return CommandType::MoveElement; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    SeqObjectId GetElement() const { return _element; }
    SeqObjectId GetAfter() const { return _after; }

    // The command equivalent to SequenceElements::MoveSequenceElement(index,
    // dest, MASTER_VIEW), or nullptr when that would not move anything.
    static std::unique_ptr<MoveElementCommand> FromIndices(const SequenceElements& seq, int index, int dest);

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    SeqObjectId _element;
    SeqObjectId _after;
    int _indexHint;
};

// Renames a timing track. With updateReferences, effect settings that name
// the track (Faces, State, Piano, ...) follow the rename; the inverse then
// restores exactly those settings. Strict mode requires a non-empty name no
// other element uses.
class RenameTimingTrackCommand : public SequenceCommand {
public:
    RenameTimingTrackCommand(SeqObjectId element, std::string name, bool updateReferences = true);
    CommandType Type() const override { return CommandType::RenameTimingTrack; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    SeqObjectId GetElement() const { return _element; }
    const std::string& GetName() const { return _name; }
    bool GetUpdateReferences() const { return _updateReferences; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    SeqObjectId _element;
    std::string _name;
    bool _updateReferences;
};

// Sets a timing track's fixed interval (0 makes it a variable, editable
// track; the marks are unchanged either way).
class SetTimingFixedCommand : public SequenceCommand {
public:
    SetTimingFixedCommand(SeqObjectId element, int fixedMS);
    CommandType Type() const override { return CommandType::SetTimingFixed; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    SeqObjectId GetElement() const { return _element; }
    int GetFixedMS() const { return _fixed; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    SeqObjectId _element;
    int _fixed;
};

// Sets timeline tag `tag` (0..SequenceElements::TagCount-1) to a position in
// ms; -1 clears it.
class SetTagCommand : public SequenceCommand {
public:
    SetTagCommand(int tag, int positionMS);
    CommandType Type() const override { return CommandType::SetTag; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    int GetTag() const { return _tag; }
    int GetPosition() const { return _position; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    int _tag;
    int _position;
};

// Sets (or, with std::nullopt, removes) one sequence-level face definition.
class SetSequenceFaceCommand : public SequenceCommand {
public:
    using FaceDefinition = std::map<std::string, std::string>;
    SetSequenceFaceCommand(std::string name, std::optional<FaceDefinition> definition);
    CommandType Type() const override { return CommandType::SetSequenceFace; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const std::string& GetName() const { return _name; }
    const std::optional<FaceDefinition>& GetDefinition() const { return _definition; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    std::string _name;
    std::optional<FaceDefinition> _definition;
};

// Replaces the song structure (every view and its regions). Which view is
// active is per-user and is kept, clamped to the new view count.
class SetSongStructureCommand : public SequenceCommand {
public:
    explicit SetSongStructureCommand(std::vector<SongStructureView> views);
    CommandType Type() const override { return CommandType::SetSongStructure; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const std::vector<SongStructureView>& GetViews() const { return _views; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    std::vector<SongStructureView> _views;
};

// Deserializes the command types declared here; nullptr for anything else
// or anything malformed. Called by DeserializeCommand.
std::unique_ptr<SequenceCommand> DeserializeElementCommand(const nlohmann::json& in);
const char* ElementCommandTypeName(CommandType t);
// The undo/redo menu text for a step made of one of these commands.
const char* ElementCommandUndoLabel(CommandType t);

// Records the elements an operation adds to a sequence as one undoable step
// of AddElementCommands: built directly (so a whole imported timing track,
// marks and all, is one command) and recorded from what exists at the end.
// For operations that only create new elements, such as timing imports.
//
// Edits made inside the scope are exempt from the mutation guard. Debug
// builds report an existing element whose contents changed in the scope,
// since that change would not be recorded. A recorder opened while another
// is active for the same sequence does nothing; the outer one records.
class NewElementsRecorder {
public:
    NewElementsRecorder(SequenceElements& seq, std::string label, bool recordUndo = true);
    ~NewElementsRecorder();
    NewElementsRecorder(const NewElementsRecorder&) = delete;
    NewElementsRecorder& operator=(const NewElementsRecorder&) = delete;

    // The recorder active on this thread for `seq`, if any.
    static NewElementsRecorder* Active(const SequenceElements& seq);

private:
    SequenceElements& _seq;
    std::string _label;
    bool _recordUndo;
    bool _passive;
    NewElementsRecorder* _outer;
    std::map<SeqObjectId, int> _before; // element uid -> change count
    std::optional<SeqMutationGuard::Scope> _scope;
};

// CommandExecutor::BeginBatch / EndBatch for a scope: everything executed
// or recorded while it lives is one undo step and reaches observers as one
// batch.
class ScopedEditBatch {
public:
    ScopedEditBatch(SequenceElements& seq, const std::string& label);
    ~ScopedEditBatch();
    ScopedEditBatch(const ScopedEditBatch&) = delete;
    ScopedEditBatch& operator=(const ScopedEditBatch&) = delete;

private:
    SequenceElements& _seq;
};

// Edit-site helpers for the UI. Like the CommandExecutor edit-site API they
// apply in lenient mode, record the inverse into the caller's current undo
// step (unless recordUndo is false) and notify observers.
namespace ElementEdits {

// Adds a timing track with one empty layer at `index` in the master view
// (-1 appends after the last timing track). Inside a NewElementsRecorder the
// track is created directly and recorded by the recorder.
TimingElement* AddTimingTrack(SequenceElements& seq, int index, const std::string& name, const std::string& subType,
                              bool recordUndo = true);
// Adds a model element with one empty layer at `index` (-1 appends).
Element* AddModel(SequenceElements& seq, int index, const std::string& name, bool recordUndo = true);
// Removes a model or timing element by name.
bool DeleteElement(SequenceElements& seq, const std::string& name, bool recordUndo = true, bool keepInViews = false);
// SequenceElements::MoveSequenceElement. Moves in the master view are
// document edits and go through MoveElementCommand; other views are
// per-user and move directly.
bool MoveElement(SequenceElements& seq, int index, int dest, int view, bool recordUndo = true);
bool RenameTimingTrack(SequenceElements& seq, const std::string& oldName, const std::string& newName, bool recordUndo = true);
bool SetTimingFixed(SequenceElements& seq, TimingElement* timing, int fixedMS, bool recordUndo = true);
bool SetTagPosition(SequenceElements& seq, int tag, int positionMS, bool recordUndo = true);
// Runs `edit` on the sequence faces, then records what changed as
// SetSequenceFaceCommands, which reach observers as one batch and so make
// their own undo step unless an enclosing batch is open. Returns true if
// anything changed.
bool EditSequenceFaces(SequenceElements& seq, const std::function<void(SequenceFaces&)>& edit, bool recordUndo = true);
// Runs `edit` on the song structure, then records the change as a
// SetSongStructureCommand. Returns true if anything changed.
bool EditSongStructure(SequenceElements& seq, const std::function<void(SongStructureManager&)>& edit, bool recordUndo = true);

} // namespace ElementEdits
