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

// What session code needs to know about a SequenceCommand beyond applying
// it: what it touches (for undo conflicts and stale edits), whether it can be
// applied optimistically, and the host's normalisations.

#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "render/SequenceCommands.h"

class SequenceElements;

namespace Collab {

// The top-level (master view) element a row uid belongs to: the model for
// a submodel or strand, the uid itself otherwise or when it isn't found.
SeqObjectId TopLevelOf(const SequenceElements& seq, SeqObjectId elementUid);

// Footprint items name the pieces of document state a command touched, e.g.
// "e12.t" (effect 12's times), "e12.s.E_SLIDER_X" (one setting key), "e12.*"
// (anything about effect 12), "l5.*", "x3.o" (row 3's position), "m.<key>"
// (a media entry). An item ending in ".*" is recorded for every touch of
// that object; "...x" for its creation or removal.
//
// Items a committed command touched. `applied` is the command as applied
// (so ids it allocated are filled in) and `inverse` what Apply returned;
// `seq` is the document right after applying it.
std::set<std::string> TouchedItems(const SequenceElements& seq, const SequenceCommand& applied, const SequenceCommand& inverse);

// Items that, if someone else touched them after rev R, mean `cmd` (an undo
// or redo of R) would overwrite or resurrect their work.
std::set<std::string> ConflictItems(const SequenceElements& seq, const SequenceCommand& cmd);

// The type-change item for an effect: present in TouchedItems when a commit
// changed that effect's type.
std::string TypeItem(SeqObjectId effect);

// True for the edits a client applies before the host commits them: a
// ModifyEffect that changes individual settings / palette keys and not the
// effect type.
bool IsOptimistic(const SequenceCommand& cmd);

// A ModifyEffect that replaces whole maps but not the type, rewritten as the
// key changes it amounts to against the effect's current state. nullptr when
// `cmd` isn't such a command, or when it would change nothing.
std::unique_ptr<SequenceCommand> AsKeyChanges(const SequenceElements& seq, const SequenceCommand& cmd);

// Effects whose settings or palette keys `cmd` (or any member) writes
// without also setting the effect type.
std::vector<SeqObjectId> KeyWriteTargets(const SequenceCommand& cmd);

// The host's idempotence rule: deleting an effect, layer or row that is
// already gone is a no-op. Returns `cmd` without those members (an empty
// Batch when nothing is left), or nullptr if nothing was removed. A delete of
// something an earlier member of the same command creates is kept.
std::unique_ptr<SequenceCommand> WithoutRedundantDeletes(const SequenceElements& seq, const SequenceCommand& cmd);

bool IsNoOp(const SequenceCommand& cmd);

// The order of the effects in every layer `cmd` names (effect order isn't
// document state, but the .xsq is written in it). A strict batch that fails
// part way rolls back through inverses, which can re-sort a layer; Restore
// puts the order (and position ids) back.
class LayerOrderSnapshot {
public:
    LayerOrderSnapshot(const SequenceElements& seq, const SequenceCommand& cmd);
    void Restore(const SequenceElements& seq) const;

private:
    struct Saved {
        LayerLocator layer;
        std::vector<std::pair<SeqObjectId, int>> effects; // uid, position id
    };
    std::vector<Saved> _layers;
};

// Every object id in a document, kept current command by command, so the
// host can refuse a command that would give two objects the same id.
class UidRegistry {
public:
    void Reset(const SequenceElements& seq);
    // Records what `applied` (with its `inverse`) created and removed. False,
    // changing nothing, when it created an id that was already in use.
    bool Commit(const SequenceCommand& applied, const SequenceCommand& inverse, SeqObjectId* duplicate = nullptr);
    bool operator==(const std::unordered_set<SeqObjectId>& o) const { return _ids == o; }
    size_t Size() const { return _ids.size(); }

    static std::unordered_set<SeqObjectId> AllUids(const SequenceElements& seq);

private:
    std::unordered_set<SeqObjectId> _ids;
};

// Every serialized command contains no per-user state: true when the JSON
// has none of the local-only fields (selection and so on).
bool HasNoLocalState(const nlohmann::json& serialized);

} // namespace Collab
