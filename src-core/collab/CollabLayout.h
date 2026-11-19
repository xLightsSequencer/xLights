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

// Editing the layout (models, groups, view objects, views, controllers...)
// together in a live session.
//
// The layout travels as the two show-wide files, xlights_rgbeffects.xml and
// xlights_networks.xml, compared record by record (layout id, else name), so
// no editor needs per-property code:
//   - every peer keeps the session layout: the files as of the last layout
//     commit it applied. Its memory is the session layout plus its own edits
//     that are not committed yet;
//   - when the app reports a layout change and no edit is in progress, the
//     peer serializes memory and proposes the records that differ from the
//     session layout (a LayoutDelta) against the rev it had;
//   - the host merges the proposal three-way (base: the session layout at that
//     rev, mine: base plus the delta, theirs: the layout now) with
//     ShowFileMerge, so changes to different attributes of one model both
//     survive and the later proposal wins on the same attribute, and commits
//     the records that changed as one delta in the shared rev stream;
//   - every peer applies a committed delta to its session layout and brings
//     memory to it, keeping its own uncommitted edits on top.
// Renames and deletes carry their cascades because the editing app already
// applied them in memory (group members, chained start channels, ...) and the
// merge rewrites references to renamed records on every side.

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace Collab {

// The show-wide layout files, as text.
struct LayoutFiles {
    std::string rgbEffects; // xlights_rgbeffects.xml
    std::string networks;   // xlights_networks.xml
    bool operator==(const LayoutFiles&) const = default;
};

namespace LayoutDelta {

// The records that differ from `from` to `to` (added, changed with their
// whole new XML, removed, reordered; whole sections where the file has no
// records), by the merge's rules: layout ids, attribute order, whitespace and
// number formatting are not changes. Null when nothing differs. Never throws.
nlohmann::json Diff(const LayoutFiles& from, const LayoutFiles& to);

// `from` with `delta` applied. Apply(a, Diff(a, b)) holds the same records as
// b (ShowFileMerge::RecordsEqual on every record and section). nullopt when
// the delta is malformed or does not fit `from` (a changed record it lacks).
std::optional<LayoutFiles> Apply(const LayoutFiles& from, const nlohmann::json& delta);

// The record ids / names a delta touches, for presence and notices
// ("Model 'Arch'"). Empty for a null delta.
std::vector<std::string> Describe(const nlohmann::json& delta);

// A hash of the layout that is equal for equal records (the same rules as
// Diff), for drift checks.
uint64_t Hash(const LayoutFiles& files);

// No record differs (Diff is null) and every record carries the same layout
// id in both. Memory loaded from `a` can then stand in for `b`: later deltas
// name records by id, so ids that differ would read as other records.
bool Equivalent(const LayoutFiles& a, const LayoutFiles& b);

} // namespace LayoutDelta

// The three-way merge of a layout proposal (ShowFileMerge, networks first so
// models follow renamed controllers), with the session's conflict policy.
namespace LayoutMerge {

enum class Policy {
    // A proposal (mine) merged onto the session (theirs): the later proposal
    // wins every conflict, except that a record they deleted stays deleted
    // (listed in `dropped`) and a record both added refuses the whole
    // proposal (Reason::NameTaken).
    Proposal,
    // Bringing this peer's uncommitted edits (mine) onto a commit (theirs):
    // the commit wins every conflict.
    Theirs,
};

struct Outcome {
    bool ok = false;
    std::string reason; // Reason::NameTaken / Reason::Invalid when !ok
    std::string text;   // readable, when !ok
    LayoutFiles result;
    std::vector<std::string> dropped; // records of mine that theirs deleted (Proposal)
};

Outcome Merge(const LayoutFiles& base, const LayoutFiles& mine, const LayoutFiles& theirs, Policy policy);

} // namespace LayoutMerge

// What applying a layout commit did, for the app's refresh and notices.
struct LayoutApplied {
    bool layoutChanged = false;      // rgbeffects records were replaced
    bool controllersChanged = false; // networks were reloaded
    std::vector<std::string> modelsChanged; // names after the change
    std::vector<std::string> modelsRemoved;
    std::vector<std::pair<std::string, std::string>> modelsRenamed; // old, new
};

// The app side of live layout editing. ICollabApp derives from it; every
// method has a default that keeps the layout out of the session (the layout
// then stays locked as before).
class ICollabLayout {
public:
    virtual ~ICollabLayout() = default;

    // The app can serialize and apply its layout (both hooks below work).
    virtual bool SupportsLiveLayout() { return false; }

    // Changes whenever memory's layout or controllers may have changed (any
    // edit, including the app's own undo). Cheap; read on every tick.
    virtual uint64_t LayoutGeneration() { return 0; }

    // An edit is under way and memory is not at a resting state: a drag or
    // handle move with the button down, a modal layout editor open, a text
    // field with uncommitted typing. Proposals wait until it is false.
    virtual bool LayoutEditInProgress() { return false; }

    // The layout files as memory holds them now (the same text a save would
    // write). False when they cannot be produced right now (try later).
    virtual bool SerializeLayout(LayoutFiles& /*out*/) { return false; }

    // Brings memory from `current` (what SerializeLayout gives now) to
    // `target`: stops rendering first (AbortRender is best-effort; return
    // false when it did not stop, and the controller retries), rebuilds only
    // the records that differ (ShowMergeApply::ApplyLayout,
    // ReloadControllers), queues the re-render and redraw work a local edit
    // would, and refreshes the layout UI. Applying must leave the open
    // sequence identical on every peer (renamed rows renamed, deleted models'
    // rows removed by the same rule everywhere).
    virtual bool ApplyLayout(const LayoutFiles& /*current*/, const LayoutFiles& /*target*/, LayoutApplied& /*out*/) { return false; }

    // Someone's layout selection or lock changed (redraw outlines, badges and
    // read-only state). Read them from CollabController.
    virtual void OnLayoutPresenceChanged() {}
    // A layout proposal of this peer was not committed (or only partly: the
    // record was deleted or renamed onto a taken name meanwhile). `text` is
    // readable ("Arch 1 was deleted by Sam; your change to it was dropped").
    virtual void OnLayoutRejected(const std::string& /*text*/) {}

    // While a live-layout session runs the layout is editable but the
    // sequence settings (timing, media, ...) are not: forward this to
    // xLightsShowContext::SetSequenceSettingsLock, whose
    // RefusesSequenceSettingsWrite the sequence settings editors check.
    virtual void SetSequenceSettingsLock(bool /*locked*/, const std::string& /*reason*/) {}
};

// A layout object a participant is editing in a modal editor (custom model
// grid, submodels, faces, states, DMX setup, dimming curve): others treat it
// as read-only until it is released.
struct LayoutLockInfo {
    std::string object; // layout id, else the name
    std::string name;   // for display
    std::string what;   // "submodels", "faces", ... for "Sam is editing Arch 1's submodels"
    bool operator==(const LayoutLockInfo&) const = default;
};

} // namespace Collab
