/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SequenceMediaCommands.h"

#include <algorithm>
#include <unordered_set>

#include "CommandExecutor.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "RenderContext.h"
#include "SequenceElements.h"
#include "SequenceFaces.h"

using nlohmann::json;

namespace {

std::unique_ptr<SequenceCommand> Fail(CommandResult* out, CommandResult r) {
    if (out != nullptr) {
        *out = std::move(r);
    }
    return nullptr;
}

std::unique_ptr<SequenceCommand> Done(CommandResult* out, std::unique_ptr<SequenceCommand> inverse) {
    if (out != nullptr) {
        *out = CommandResult::Success();
    }
    return inverse;
}

// Re-render what draws from these keys and mark the sequence changed.
void InvalidateMediaUsers(SequenceElements& seq, const std::string& key) {
    seq.IncrementChangeCount(nullptr);
    for (Effect* e : EffectsReferencingMedia(seq, key)) {
        e->IncrementChangeCount();
    }
}

json WriteState(const MediaEntryState& s) {
    json j{ { "key", s.key }, { "mediaType", MediaEntryState::TypeName(s.type) }, { "embedded", s.embedded } };
    if (!s.subtype.empty()) {
        j["subtype"] = s.subtype;
    }
    if (s.data && !s.data->empty()) {
        j["data"] = *s.data;
    }
    if (!s.frames.empty()) {
        json frames = json::array();
        for (const auto& f : s.frames) {
            frames.push_back(json{ { "time", f.timeMS }, { "data", f.data ? *f.data : std::string() } });
        }
        j["frames"] = std::move(frames);
    }
    return j;
}

bool GetString(const json& j, const char* key, std::string& out, bool required = true) {
    auto it = j.find(key);
    if (it == j.end()) {
        return !required;
    }
    if (!it->is_string()) {
        return false;
    }
    out = it->get<std::string>();
    return true;
}

bool GetBool(const json& j, const char* key, bool& out) {
    auto it = j.find(key);
    if (it == j.end() || !it->is_boolean()) {
        return false;
    }
    out = it->get<bool>();
    return true;
}

bool ReadState(const json& j, MediaEntryState& s) {
    std::string typeName, data;
    if (!GetString(j, "key", s.key) || s.key.empty() || !GetString(j, "mediaType", typeName) ||
        !MediaEntryState::TypeFromName(typeName, s.type) || !GetBool(j, "embedded", s.embedded) ||
        !GetString(j, "subtype", s.subtype, false) || !GetString(j, "data", data, false)) {
        return false;
    }
    if (!data.empty()) {
        s.data = std::make_shared<const std::string>(std::move(data));
    }
    if (auto it = j.find("frames"); it != j.end()) {
        if (!it->is_array()) {
            return false;
        }
        for (const auto& f : *it) {
            if (!f.is_object()) {
                return false;
            }
            auto t = f.find("time");
            std::string fd;
            if (t == f.end() || !t->is_number_integer() || !GetString(f, "data", fd)) {
                return false;
            }
            s.frames.push_back({ t->get<int>(), std::make_shared<const std::string>(std::move(fd)) });
        }
    }
    return true;
}

} // namespace

// ---------------------------------------------------------------------------
// PutMedia
// ---------------------------------------------------------------------------

PutMediaCommand::PutMediaCommand(MediaEntryState state) :
    _state(std::move(state)) {
}

CommandResult PutMediaCommand::Validate(const SequenceElements& seq, ApplyMode mode) const {
    if (_state.key.empty()) {
        return CommandResult::Fail(CommandStatus::Invalid, "no media key");
    }
    if (_state.embedded && !MediaEntryState::IsEmbeddableType(_state.type)) {
        return CommandResult::Fail(CommandStatus::Invalid, "media type can't be embedded");
    }
    if (mode == ApplyMode::Strict) {
        if (_state.embedded && !_state.HasPayload()) {
            return CommandResult::Fail(CommandStatus::Invalid, "embedded media without data");
        }
        auto cur = seq.GetSequenceMedia().GetEntryState(_state.key);
        if (cur && cur->type != _state.type) {
            return CommandResult::Fail(CommandStatus::Invalid, "media key already holds another type");
        }
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> PutMediaCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Fail(result, r);
    }
    SequenceMedia& media = seq.GetSequenceMedia();
    std::optional<MediaEntryState> prior = media.GetEntryState(_state.key);
    bool changed = false;
    if (!media.PutEntry(_state, &changed)) {
        return Fail(result, CommandResult::Fail(CommandStatus::Invalid, "media entry not stored"));
    }
    if (changed) {
        InvalidateMediaUsers(seq, _state.key);
    } else {
        seq.IncrementChangeCount(nullptr);
    }
    if (prior) {
        return Done(result, std::make_unique<PutMediaCommand>(std::move(*prior)));
    }
    return Done(result, std::make_unique<RemoveMediaCommand>(_state.key));
}

void PutMediaCommand::Serialize(json& out) const {
    out = WriteState(_state);
    out["type"] = CommandTypeName(Type());
}

std::unique_ptr<SequenceCommand> PutMediaCommand::Clone() const {
    return std::make_unique<PutMediaCommand>(*this);
}

// ---------------------------------------------------------------------------
// RemoveMedia
// ---------------------------------------------------------------------------

RemoveMediaCommand::RemoveMediaCommand(std::string key) :
    _key(std::move(key)) {
}

CommandResult RemoveMediaCommand::Validate(const SequenceElements& seq, ApplyMode) const {
    if (_key.empty()) {
        return CommandResult::Fail(CommandStatus::Invalid, "no media key");
    }
    if (!seq.GetSequenceMedia().HasMedia(_key)) {
        return CommandResult::Fail(CommandStatus::TargetMissing, "media entry not found");
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> RemoveMediaCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Fail(result, r);
    }
    SequenceMedia& media = seq.GetSequenceMedia();
    std::optional<MediaEntryState> prior = media.GetEntryState(_key);
    media.RemoveMedia(_key);
    InvalidateMediaUsers(seq, _key);
    return Done(result, std::make_unique<PutMediaCommand>(std::move(*prior)));
}

void RemoveMediaCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "key", _key } };
}

std::unique_ptr<SequenceCommand> RemoveMediaCommand::Clone() const {
    return std::make_unique<RemoveMediaCommand>(*this);
}

// ---------------------------------------------------------------------------
// RenameMedia
// ---------------------------------------------------------------------------

RenameMediaCommand::RenameMediaCommand(std::string from, std::string to, bool moveEntry) :
    _from(std::move(from)), _to(std::move(to)), _moveEntry(moveEntry) {
}

CommandResult RenameMediaCommand::Validate(const SequenceElements& seq, ApplyMode) const {
    if (_from.empty() || _to.empty() || _from == _to) {
        return CommandResult::Fail(CommandStatus::Invalid, "bad media rename");
    }
    if (_moveEntry) {
        const SequenceMedia& media = seq.GetSequenceMedia();
        if (!media.HasMedia(_from)) {
            return CommandResult::Fail(CommandStatus::TargetMissing, "media entry not found");
        }
        if (media.HasMedia(_to)) {
            return CommandResult::Fail(CommandStatus::Invalid, "media key already in use");
        }
    }
    return CommandResult::Success();
}

std::unique_ptr<SequenceCommand> RenameMediaCommand::ApplyImpl(SequenceElements& seq, ApplyMode mode, CommandResult* result) {
    CommandResult r = Validate(seq, mode);
    if (!r.Ok()) {
        return Fail(result, r);
    }
    if (_moveEntry && !seq.GetSequenceMedia().RenameMedia(_from, _to)) {
        return Fail(result, CommandResult::Fail(CommandStatus::Invalid, "media entry not renamed"));
    }
    const auto facePaths = seq.GetSequenceFaces().GetImagePaths();
    if (std::find(facePaths.begin(), facePaths.end(), _from) != facePaths.end()) {
        // Faces effects hold references into the face definitions while rendering.
        if (RenderContext* rc = seq.GetRenderContext()) {
            rc->AbortRender();
        }
        seq.GetSequenceFaces().RewriteImagePath(_from, _to);
    }
    // Effects follow through the ModifyEffects batched with this command,
    // which re-render them.
    seq.IncrementChangeCount(nullptr);
    return Done(result, std::make_unique<RenameMediaCommand>(_to, _from, _moveEntry));
}

void RenameMediaCommand::Serialize(json& out) const {
    out = json{ { "type", CommandTypeName(Type()) }, { "from", _from }, { "to", _to }, { "moveEntry", _moveEntry } };
}

std::unique_ptr<SequenceCommand> RenameMediaCommand::Clone() const {
    return std::make_unique<RenameMediaCommand>(*this);
}

std::unique_ptr<SequenceCommand> DeserializeMediaCommand(const std::string& type, const json& in) {
    if (type == "PutMedia") {
        MediaEntryState s;
        if (!ReadState(in, s)) {
            return nullptr;
        }
        return std::make_unique<PutMediaCommand>(std::move(s));
    }
    if (type == "RemoveMedia") {
        std::string key;
        if (!GetString(in, "key", key) || key.empty()) {
            return nullptr;
        }
        return std::make_unique<RemoveMediaCommand>(std::move(key));
    }
    if (type == "RenameMedia") {
        std::string from, to;
        bool move = true;
        if (!GetString(in, "from", from) || !GetString(in, "to", to) || !GetBool(in, "moveEntry", move) || from.empty() ||
            to.empty()) {
            return nullptr;
        }
        return std::make_unique<RenameMediaCommand>(std::move(from), std::move(to), move);
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Builders
// ---------------------------------------------------------------------------

std::vector<Effect*> EffectsReferencingMedia(const SequenceElements& seq, const std::string& key) {
    std::vector<Effect*> out;
    if (key.empty()) {
        return out;
    }
    seq.ForEachModelEffectLayer([&](EffectLayer* layer, const std::string&) {
        for (int k = 0; k < layer->GetEffectCount(); ++k) {
            Effect* eff = layer->GetEffect(k);
            for (const auto& kv : eff->GetSettings()) {
                if (kv.second == key) {
                    out.push_back(eff);
                    break;
                }
            }
        }
    });
    return out;
}

std::unique_ptr<BatchCommand> BuildRenameMedia(const SequenceElements& seq, const std::string& from, const std::string& to,
                                               bool mergeIfTaken, const std::string& label) {
    auto batch = std::make_unique<BatchCommand>(label);
    if (from.empty() || to.empty() || from == to) {
        return batch;
    }
    const SequenceMedia& media = seq.GetSequenceMedia();
    bool hasFrom = media.HasMedia(from);
    if (media.HasMedia(to)) {
        if (!mergeIfTaken) {
            return batch;
        }
        if (hasFrom) {
            batch->Add(std::make_unique<RemoveMediaCommand>(from));
        }
        batch->Add(std::make_unique<RenameMediaCommand>(from, to, false));
    } else {
        batch->Add(std::make_unique<RenameMediaCommand>(from, to, hasFrom));
    }
    for (Effect* eff : EffectsReferencingMedia(seq, from)) {
        SettingsChange change;
        for (const auto& kv : eff->GetSettings()) {
            if (kv.second == from) {
                change.keys[kv.first] = to;
            }
        }
        batch->Add(std::make_unique<ModifyEffectCommand>(LocatorOf(eff), std::nullopt, std::move(change), SettingsChange{}));
    }
    return batch;
}

// ---------------------------------------------------------------------------
// Edit-site helpers
// ---------------------------------------------------------------------------

namespace SeqMediaEdits {

namespace {
bool Run(SequenceElements& seq, std::unique_ptr<SequenceCommand> cmd, bool recordUndo = true) {
    return seq.GetCommandExecutor().ExecuteInStep(std::move(cmd), recordUndo).Ok();
}
} // namespace

bool Put(SequenceElements& seq, MediaEntryState state, const std::string& label) {
    ScopedCommandBatch batch(seq, label);
    return Run(seq, std::make_unique<PutMediaCommand>(std::move(state)));
}

bool Remove(SequenceElements& seq, const std::string& key, const std::string& label) {
    if (!seq.GetSequenceMedia().HasMedia(key)) {
        return false;
    }
    ScopedCommandBatch batch(seq, label);
    return Run(seq, std::make_unique<RemoveMediaCommand>(key));
}

bool Rename(SequenceElements& seq, const std::string& from, const std::string& to, bool mergeIfTaken, const std::string& label,
            bool recordUndo) {
    if (from == to) {
        return true;
    }
    auto cmds = BuildRenameMedia(seq, from, to, mergeIfTaken, label);
    if (cmds->IsEmpty()) {
        return false;
    }
    ScopedCommandBatch batch(seq, label);
    return Run(seq, std::move(cmds), recordUndo);
}

bool Embed(SequenceElements& seq, const std::string& key) {
    SequenceMedia& media = seq.GetSequenceMedia();
    auto cur = media.GetEntryState(key);
    if (!cur) {
        return false;
    }
    if (cur->embedded) {
        return true;
    }
    auto state = media.GetEmbeddableState(key);
    return state && Put(seq, std::move(*state), "Embed Media");
}

bool EmbedFile(SequenceElements& seq, const std::string& key, MediaType type, const std::string& sourcePath) {
    auto state = MediaEntryState::EmbeddedFromFile(key, type, sourcePath);
    return state && Put(seq, std::move(*state), "Embed Media");
}

bool EmbedImage(SequenceElements& seq, const std::string& key, const xlImage& image, const std::string& label) {
    auto state = MediaEntryState::EmbeddedImage(key, image);
    return state && Put(seq, std::move(*state), label);
}

bool EmbedFrames(SequenceElements& seq, const std::string& key, const std::vector<xlImage>& frames, int frameTimeMs,
                 const std::string& label) {
    auto state = MediaEntryState::EmbeddedFrames(key, frames, frameTimeMs);
    return state && Put(seq, std::move(*state), label);
}

bool ReembedFromDisk(SequenceElements& seq, const std::string& key) {
    SequenceMedia& media = seq.GetSequenceMedia();
    auto cur = media.GetEntryState(key);
    if (!cur || !cur->embedded) {
        return false;
    }
    auto state = media.ReadEmbeddableStateFromDisk(key);
    return state && Put(seq, std::move(*state), "Re-embed Media from Disk");
}

bool AddFileReference(SequenceElements& seq, const std::string& key, MediaType type, const std::string& subtype) {
    if (key.empty()) {
        return false;
    }
    return Put(seq, MediaEntryState::FileReference(key, type, subtype), "Add Media");
}

bool Extract(SequenceElements& seq, const std::string& key, const std::string& destPath, const std::string& newKey) {
    SequenceMedia& media = seq.GetSequenceMedia();
    auto cur = media.GetEntryState(key);
    if (!cur || !cur->embedded) {
        return false;
    }
    if (!destPath.empty() && !media.WriteEmbeddedToFile(key, destPath)) {
        return false;
    }
    const std::string& target = newKey.empty() ? key : newKey;
    ScopedCommandBatch batch(seq, "Extract Media");
    if (target != key && !Rename(seq, key, target, true)) {
        return false;
    }
    return Run(seq, std::make_unique<PutMediaCommand>(MediaEntryState::FileReference(target, cur->type, cur->subtype)));
}

bool ExtractToDisk(SequenceElements& seq, const std::string& key, const std::string& destPath) {
    if (SequenceMedia::ExistsOnDisk(key)) {
        return Extract(seq, key);
    }
    return !destPath.empty() && Extract(seq, key, destPath);
}

std::string MakeRelative(SequenceElements& seq, const std::string& path) {
    if (path.empty() || seq.GetRenderContext() == nullptr) {
        return path;
    }
    const std::string key = seq.GetRenderContext()->MakeRelativePath(path);
    if (key.empty() || key == path) {
        return path;
    }
    // Two entries from different folders that share a filename keep their
    // original paths rather than collapsing into one.
    const SequenceMedia& media = seq.GetSequenceMedia();
    if (!media.HasMedia(path) || media.HasMedia(key)) {
        return path;
    }
    return Rename(seq, path, key, false, "Make Media Path Relative") ? key : path;
}

int EmbedAll(SequenceElements& seq) {
    SequenceMedia& media = seq.GetSequenceMedia();
    ScopedCommandBatch batch(seq, "Embed All Media");
    int count = 0;
    for (const auto& [key, type] : media.GetAllMediaPaths()) {
        if (!MediaEntryState::IsEmbeddableType(type)) {
            continue;
        }
        auto cur = media.GetEntryState(key);
        if (!cur || cur->embedded) {
            continue;
        }
        auto state = media.GetEmbeddableState(key);
        if (state && Run(seq, std::make_unique<PutMediaCommand>(std::move(*state)))) {
            ++count;
        }
    }
    return count;
}

int ExtractAll(SequenceElements& seq) {
    SequenceMedia& media = seq.GetSequenceMedia();
    ScopedCommandBatch batch(seq, "Extract All Media");
    int count = 0;
    for (const auto& [key, type] : media.GetAllMediaPaths()) {
        auto cur = media.GetEntryState(key);
        if (!cur || !cur->embedded) {
            continue;
        }
        if (Run(seq, std::make_unique<PutMediaCommand>(MediaEntryState::FileReference(key, cur->type, cur->subtype)))) {
            ++count;
        }
    }
    return count;
}

int RemoveUnreferenced(SequenceElements& seq) {
    SequenceMedia& media = seq.GetSequenceMedia();
    std::unordered_set<std::string> used = seq.GetEffectSettingValues();
    for (const auto& p : seq.GetSequenceFaces().GetImagePaths()) {
        used.insert(p);
    }
    ScopedCommandBatch batch(seq, "Remove Unused Media");
    int removed = 0;
    for (const auto& p : media.GetAllMediaPaths()) {
        if (used.count(p.first) == 0 && !media.IsUsedByMetadata(p.first) &&
            Run(seq, std::make_unique<RemoveMediaCommand>(p.first))) {
            ++removed;
        }
    }
    return removed;
}

} // namespace SeqMediaEdits
