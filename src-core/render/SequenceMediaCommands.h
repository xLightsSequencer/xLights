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

// Commands for a sequence's media entries (SequenceMedia): images, SVGs,
// shaders, text files and so on, embedded in the .xsq or referenced on disk.
//
// A media entry is named by its key, the path exactly as effect settings
// store it. Applying a media command re-renders every effect whose settings
// reference an entry whose rendered content changed, and marks the sequence
// changed.
//
// Embedded bytes travel inside the command as the base64 text the .xsq
// stores (MediaEntryState::data), shared rather than copied between the
// command, its inverse and the undo history. Writing files to disk is never
// part of a command: extracting media writes the file as a local side effect
// before the command runs, and undoing the extract embeds the bytes again but
// leaves the file where it is.

#include <memory>
#include <string>
#include <vector>

#include "CommandExecutor.h"
#include "SequenceCommands.h"
#include "SequenceMedia.h"

class CommandExecutor;
class Effect;
class xlImage;

// Creates the entry at state.key, or replaces what it holds: new bytes,
// embedding (reference -> bytes), extracting (bytes -> reference), or a
// different type. The inverse puts back the previous entry, or removes the
// key when there was none. Strict mode refuses to change an existing entry's
// type and requires an embedded state to carry bytes.
class PutMediaCommand : public SequenceCommand {
public:
    explicit PutMediaCommand(MediaEntryState state);
    CommandType Type() const override { return CommandType::PutMedia; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const MediaEntryState& GetState() const { return _state; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    MediaEntryState _state;
};

// Removes the entry at `key`. The inverse puts it back with its content.
class RemoveMediaCommand : public SequenceCommand {
public:
    explicit RemoveMediaCommand(std::string key);
    CommandType Type() const override { return CommandType::RemoveMedia; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const std::string& GetKey() const { return _key; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    std::string _key;
};

// Re-keys the entry at `from` to `to` and repoints the sequence-level face
// definitions that name `from`. Effect settings are not touched: always issue
// it in a Batch with the ModifyEffects that repoint them (BuildRenameMedia).
// With moveEntry false only the face definitions are repointed - used when
// `to` already exists and the entry at `from` is being dropped instead.
// Undo repoints every face reference to `to` back to `from`, including any
// that named `to` before the rename.
class RenameMediaCommand : public SequenceCommand {
public:
    RenameMediaCommand(std::string from, std::string to, bool moveEntry = true);
    CommandType Type() const override { return CommandType::RenameMedia; }
    CommandResult Validate(const SequenceElements& seq, ApplyMode mode = ApplyMode::Strict) const override;
    void Serialize(nlohmann::json& out) const override;
    std::unique_ptr<SequenceCommand> Clone() const override;

    const std::string& GetFrom() const { return _from; }
    const std::string& GetTo() const { return _to; }
    bool GetMoveEntry() const { return _moveEntry; }

protected:
    std::unique_ptr<SequenceCommand> ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) override;

private:
    std::string _from;
    std::string _to;
    bool _moveEntry;
};

// DeserializeCommand hook for the media command types; nullptr for any other
// type or anything malformed.
std::unique_ptr<SequenceCommand> DeserializeMediaCommand(const std::string& type, const nlohmann::json& in);

// ---------------------------------------------------------------------------
// Builders: the commands for one media edit, computed from the current
// document without changing it.
// ---------------------------------------------------------------------------

// Every effect (model, submodel, strand and node layers) with a setting whose
// value is exactly `key`.
std::vector<Effect*> EffectsReferencingMedia(const SequenceElements& seq, const std::string& key);

// Rename `from` to `to` together with every effect setting that names `from`.
// When `to` already holds an entry and mergeIfTaken is set, the entry at
// `from` is removed and its references are pointed at `to`; otherwise an
// occupied `to` gives an empty batch. Empty when from == to.
std::unique_ptr<BatchCommand> BuildRenameMedia(const SequenceElements& seq, const std::string& from, const std::string& to,
                                               bool mergeIfTaken = false, const std::string& label = "Rename Media");

// ---------------------------------------------------------------------------
// Edit-site helpers. Each runs its commands through the sequence's
// CommandExecutor inside a labelled batch, so a call is one undo step - or
// part of the caller's step when the caller already holds a
// ScopedCommandBatch. The changes apply leniently, like the direct edits they
// replace (a locked effect's media reference still follows a rename).
// ---------------------------------------------------------------------------

namespace SeqMediaEdits {

bool Put(SequenceElements& seq, MediaEntryState state, const std::string& label = "Update Media");
bool Remove(SequenceElements& seq, const std::string& key, const std::string& label = "Remove Media");
// Returns true when every reference now names `to`. With recordUndo false
// the change is still applied and reported to observers but gets no undo
// step - for a rename that also moved the file on disk, which undo couldn't
// move back.
bool Rename(SequenceElements& seq, const std::string& from, const std::string& to, bool mergeIfTaken = false,
            const std::string& label = "Rename Media", bool recordUndo = true);

// Embed the bytes the entry currently holds (read from disk if needed).
bool Embed(SequenceElements& seq, const std::string& key);
// Embed a file's bytes under `key`, replacing anything held there.
bool EmbedFile(SequenceElements& seq, const std::string& key, MediaType type, const std::string& sourcePath);
bool EmbedImage(SequenceElements& seq, const std::string& key, const xlImage& image, const std::string& label = "Add Image");
bool EmbedFrames(SequenceElements& seq, const std::string& key, const std::vector<xlImage>& frames, int frameTimeMs,
                 const std::string& label = "Add Image");
// Replace an embedded entry's bytes with its file on disk, keeping it embedded.
bool ReembedFromDisk(SequenceElements& seq, const std::string& key);
// Point `key` at its file on disk (adds the entry, or reloads it).
bool AddFileReference(SequenceElements& seq, const std::string& key, MediaType type, const std::string& subtype = "");

// Make an embedded entry a file reference. When `destPath` is given the bytes
// are first written there (a local side effect undo leaves in place); with
// `newKey` the entry and every reference to it are re-keyed in the same step.
// Returns false, changing nothing, when the entry isn't embedded or the file
// can't be written.
bool Extract(SequenceElements& seq, const std::string& key, const std::string& destPath = "",
             const std::string& newKey = "");

// Text, SVG and shader entries: make an embedded entry a file reference to its
// own key, first writing the bytes to `destPath` when the key doesn't resolve
// to a file on disk (the SequenceMedia::ExtractMediaToDisk rule).
bool ExtractToDisk(SequenceElements& seq, const std::string& key, const std::string& destPath);

// Strip the show/media folder prefix off `path` and repoint every reference
// (see SequenceElements::MakeMediaPathRelative). Returns the key now used.
std::string MakeRelative(SequenceElements& seq, const std::string& path);

// One labelled undo step each. Return the number of entries changed.
int EmbedAll(SequenceElements& seq);
int ExtractAll(SequenceElements& seq);
int RemoveUnreferenced(SequenceElements& seq);

} // namespace SeqMediaEdits
