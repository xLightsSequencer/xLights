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

// Serializable, invertible edits to a sequence's effects and layers.
//
// Every command names its targets by stable id (SequenceIds.h), never by
// position, so a command stays meaningful after other edits shift things
// around. Applying a command returns its inverse; applying the inverse
// restores the previous state, including object ids.
//
// Commands only touch document state. Per-user view state (selection,
// collapsed rows, ...) is never read or written, with one exception: an
// added effect can carry a local selection hint, which is not serialized.

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "SequenceIds.h"

class SequenceElements;
class Element;
class EffectLayer;
class Effect;

enum class CommandType : int {
    AddEffect,
    DeleteEffect,
    MoveEffect,
    ModifyEffect,
    AddLayer,
    RemoveLayer,
    SetLayerName,
    ReplaceLayerContents,
    Batch,
    // Sequence media (SequenceMediaCommands.h)
    PutMedia,
    RemoveMedia,
    RenameMedia,
    // Elements, timing tracks, tags, faces, song structure (SequenceElementCommands.h)
    AddElement,
    DeleteElement,
    MoveElement,
    RenameTimingTrack,
    SetTimingFixed,
    SetTag,
    SetSequenceFace,
    SetSongStructure
};

const char* CommandTypeName(CommandType t);

// `element` is the layer's owning element or any ancestor of it that
// SequenceElements::GetElementFromUid can find (a model, submodel, strand or
// timing element); the layer is searched for beneath it.
struct LayerLocator {
    SeqObjectId element = kNoSeqObjectId;
    SeqObjectId layer = kNoSeqObjectId;
    bool operator==(const LayerLocator&) const = default;
};

struct EffectLocator {
    SeqObjectId element = kNoSeqObjectId;
    SeqObjectId layer = kNoSeqObjectId;
    SeqObjectId effect = kNoSeqObjectId;
    LayerLocator Layer() const { return { element, layer }; }
    bool operator==(const EffectLocator&) const = default;
};

// Everything needed to re-create an effect.
struct EffectState {
    SeqObjectId uid = kNoSeqObjectId;
    std::string name;
    std::string settings; // Effect::GetSettingsAsString() form
    std::string palette;  // Effect::GetPaletteAsString() form
    int startTimeMS = 0;
    int endTimeMS = 0;
    bool isProtected = false;
    int selected = 0; // local hint only; not serialized

    // Selection is view state and is not captured (selected stays 0).
    static EffectState Of(const Effect& e);
    bool operator==(const EffectState&) const = default;
};

// Key -> new value; std::nullopt removes the key.
using KeyChanges = std::map<std::string, std::optional<std::string>>;

// A change to an effect's settings or palette map. With replaceAll, `keys`
// is the complete new map (every value set) and any key not listed is
// removed; otherwise only the listed keys change.
struct SettingsChange {
    bool replaceAll = false;
    KeyChanges keys;

    bool IsEmpty() const { return !replaceAll && keys.empty(); }
    // Complete-map change parsed from a settings / palette string.
    static SettingsChange ReplaceWith(const std::string& serialized);
    bool operator==(const SettingsChange&) const = default;
};

enum class CommandStatus {
    Ok,
    TargetMissing, // an element, layer or effect named by the command is gone
    Overlap,       // would make effects on one layer overlap
    Locked,        // would move, change or delete a locked effect
    Invalid        // malformed (bad times, duplicate id, unknown effect,
                   // removing an element's last layer, ...)
};

struct CommandResult {
    CommandStatus status = CommandStatus::Ok;
    std::string message;
    bool Ok() const { return status == CommandStatus::Ok; }
    static CommandResult Success() { return {}; }
    static CommandResult Fail(CommandStatus s, std::string msg) { return { s, std::move(msg) }; }
};

// Strict mode enforces the document invariants (no overlap, locked effects
// untouched) and is what new edits use. Effects that only touch (one ends
// where the next starts) do not overlap. A change to a locked effect is
// allowed in strict mode only if it removes the lock. Lenient mode only requires that the
// targets exist; undo replays its recorded inverses in lenient mode so it
// behaves exactly as it always has.
enum class ApplyMode { Strict, Lenient };

class SequenceCommand {
public:
    virtual ~SequenceCommand() = default;

    virtual CommandType Type() const = 0;

    // Would Apply succeed? Never mutates.
    virtual CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const = 0;

    // Validates, then applies. Returns the inverse, or nullptr (with
    // `result`, if given, saying why) when nothing was applied.
    std::unique_ptr<SequenceCommand> Apply(SequenceElements& seq, ApplyMode mode = ApplyMode::Strict,
                                           CommandResult* result = nullptr);

    virtual void Serialize(nlohmann::json& out) const = 0;
    virtual std::unique_ptr<SequenceCommand> Clone() const = 0;

    std::string ToJson() const;

protected:
    virtual std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) = 0;
};

// Returns nullptr for anything malformed; never throws.
std::unique_ptr<SequenceCommand> DeserializeCommand(const nlohmann::json& in);
std::unique_ptr<SequenceCommand> CommandFromJson(const std::string& text);

// ---------------------------------------------------------------------------
// Commands
// ---------------------------------------------------------------------------

class AddEffectCommand : public SequenceCommand {
public:
    // A state.uid of kNoSeqObjectId is replaced by a freshly allocated id
    // here, so the command (and its serialized form) names the real id.
    AddEffectCommand(LayerLocator layer, EffectState state);
    CommandType Type() const override { return CommandType::AddEffect; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const LayerLocator& GetLayer() const { return _layer; }
    const EffectState& GetState() const { return _state; }
    EffectLocator Target() const { return { _layer.element, _layer.layer, _state.uid }; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    LayerLocator _layer;
    EffectState _state;
};

class DeleteEffectCommand : public SequenceCommand {
public:
    explicit DeleteEffectCommand(EffectLocator target);
    CommandType Type() const override { return CommandType::DeleteEffect; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const EffectLocator& Target() const { return _target; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    EffectLocator _target;
};

// Changes start/end within the effect's layer. Like the existing drag code,
// this does not re-sort the layer. Lenient mode also moves a locked effect
// (keeping it locked).
class MoveEffectCommand : public SequenceCommand {
public:
    MoveEffectCommand(EffectLocator target, int startTimeMS, int endTimeMS);
    CommandType Type() const override { return CommandType::MoveEffect; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const EffectLocator& Target() const { return _target; }
    int GetStartTimeMS() const { return _start; }
    int GetEndTimeMS() const { return _end; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    EffectLocator _target;
    int _start;
    int _end;
};

// Changes an effect's type, settings and/or palette in one step. Lock and
// render-disable are settings keys (X_Effect_Locked, X_Effect_RenderDisabled),
// so they change through here too. The inverse always lists keys
// individually (never replaceAll), so it restores exactly what changed.
class ModifyEffectCommand : public SequenceCommand {
public:
    ModifyEffectCommand(EffectLocator target, std::optional<std::string> effectName, SettingsChange settings, SettingsChange palette);
    CommandType Type() const override { return CommandType::ModifyEffect; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const EffectLocator& Target() const { return _target; }
    const std::optional<std::string>& GetEffectName() const { return _name; }
    const SettingsChange& GetSettings() const { return _settings; }
    const SettingsChange& GetPalette() const { return _palette; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    EffectLocator _target;
    std::optional<std::string> _name;
    SettingsChange _settings;
    SettingsChange _palette;
};

// Inserts a layer directly under `element` (its owning element), right after
// the sibling `afterLayer`, or first when afterLayer is kNoSeqObjectId. If the
// sibling is gone, lenient mode falls back to `indexHint` (clamped); strict
// mode fails with TargetMissing. The layer can be created with a name and
// contents. A layerUid of kNoSeqObjectId is replaced by a fresh id.
class AddLayerCommand : public SequenceCommand {
public:
    AddLayerCommand(SeqObjectId element, SeqObjectId layerUid, SeqObjectId afterLayer, int indexHint = -1,
                    std::string name = "", std::vector<EffectState> effects = {});
    CommandType Type() const override { return CommandType::AddLayer; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    SeqObjectId GetElement() const { return _element; }
    SeqObjectId GetLayer() const { return _layer; }
    SeqObjectId GetAfterLayer() const { return _after; }
    int GetIndexHint() const { return _indexHint; }
    const std::string& GetName() const { return _name; }
    const std::vector<EffectState>& GetEffects() const { return _effects; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    SeqObjectId _element;
    SeqObjectId _layer;
    SeqObjectId _after;
    int _indexHint;
    std::string _name;
    std::vector<EffectState> _effects;
};

// Removes a layer (and its effects) from its owning element. An element's last
// layer is never removed. The inverse re-creates the layer with its id, name,
// position and effects.
class RemoveLayerCommand : public SequenceCommand {
public:
    RemoveLayerCommand(SeqObjectId element, SeqObjectId layer);
    CommandType Type() const override { return CommandType::RemoveLayer; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    SeqObjectId GetElement() const { return _element; }
    SeqObjectId GetLayer() const { return _layer; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    SeqObjectId _element;
    SeqObjectId _layer;
};

class SetLayerNameCommand : public SequenceCommand {
public:
    SetLayerNameCommand(LayerLocator layer, std::string name);
    CommandType Type() const override { return CommandType::SetLayerName; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const LayerLocator& GetLayer() const { return _layer; }
    const std::string& GetName() const { return _name; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    LayerLocator _layer;
    std::string _name;
};

// Replaces every effect on a layer (for bulk operations such as imports).
// Strict mode fails if the layer holds a locked effect or the new effects
// overlap each other.
class ReplaceLayerContentsCommand : public SequenceCommand {
public:
    ReplaceLayerContentsCommand(LayerLocator layer, std::vector<EffectState> effects);
    CommandType Type() const override { return CommandType::ReplaceLayerContents; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const LayerLocator& GetLayer() const { return _layer; }
    const std::vector<EffectState>& GetEffects() const { return _effects; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    LayerLocator _layer;
    std::vector<EffectState> _effects;
};

// Applies its commands in order; the inverse applies their inverses in
// reverse. In strict mode a failing member rolls back the ones already
// applied, so the batch is all-or-nothing, and the result carries that
// member's status. In lenient mode failing members are skipped and the
// inverse covers the members that applied (possibly none).
class BatchCommand : public SequenceCommand {
public:
    explicit BatchCommand(std::string label, std::vector<std::unique_ptr<SequenceCommand>> commands = {});
    CommandType Type() const override { return CommandType::Batch; }
    // Checks each member against the current document; members that depend
    // on earlier members of the same batch may report a missing target here
    // and still apply.
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    void Add(std::unique_ptr<SequenceCommand> cmd) { _commands.push_back(std::move(cmd)); }
    const std::string& GetLabel() const { return _label; }
    const std::vector<std::unique_ptr<SequenceCommand>>& GetCommands() const { return _commands; }
    bool IsEmpty() const { return _commands.empty(); }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    std::string _label;
    std::vector<std::unique_ptr<SequenceCommand>> _commands;
};

// ---------------------------------------------------------------------------
// Lookup helpers shared by the commands and their callers.
// ---------------------------------------------------------------------------

EffectLayer* FindLayer(const SequenceElements& seq, const LayerLocator& loc);
Effect* FindEffect(const SequenceElements& seq, const EffectLocator& loc);
LayerLocator LocatorOf(const EffectLayer* layer);
EffectLocator LocatorOf(const Effect* effect);

// A canonical text description of the document state the commands change:
// every element, layer and effect in order, with times, settings and
// palettes, optionally with ids. Two sequences that describe the same are
// equal as far as commands are concerned.
std::string DescribeSequence(const SequenceElements& seq, bool includeIds = true);
