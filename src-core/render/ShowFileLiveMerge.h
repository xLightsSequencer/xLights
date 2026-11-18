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

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "render/ShowMergeApply.h"
#include "utils/ShowFileGuard.h"
#include "utils/ShowSaveMerge.h"

struct ShowPresenceEntry;
class xLightsShowContext;

// Bringing someone else's changes to the shared show files (layout,
// controllers, effect presets) into the open show while it is open, as soon
// as ShowFileWatcher reports them, instead of only when this process saves.
//
// It is the save merge (ShowSaveMerge::Plan) without the write:
//   base   - what the guard recorded at load or the last save
//   mine   - what memory holds: the host's serialization when it has unsaved
//            edits to the file, otherwise the base itself (so the result is
//            theirs exactly)
//   theirs - the file on disk now
// Apply() brings memory up to the merged result and then records theirs as
// the file's new base: memory is their file plus this process's unsaved edits
// on top, so the next save merges against what they wrote, finds nothing new
// on disk and writes the union without asking. When the merge has conflicts
// nothing is applied until every one is decided; until then a save goes
// through the normal save merge (the base was not moved).
namespace ShowFileLiveMerge {

using ShowSaveMerge::FileKind;

struct LocalFile {
    FileKind kind = FileKind::RgbEffects;
    std::string path;
    // This process has unsaved edits to the file: `mine` holds them.
    // Otherwise `mine` is ignored and the recorded base is used.
    bool edited = false;
    std::string mine;
};

using Reader = std::function<std::optional<std::pair<std::string, ShowFileFingerprint>>(const std::string& path)>;

class Incoming {
public:
    // Reads every file from disk (with `reader`, ShowFileGuard::ReadFile when
    // not given) and merges it with its recorded base and `mine`.
    static Incoming Prepare(const std::vector<LocalFile>& files, const ShowFileMerge::Options& options,
                            const ShowFileGuard& guard, ShowSaveMerge::SequenceUse sequenceUse = {}, const Reader& reader = {});

    Incoming(Incoming&&) noexcept;
    Incoming& operator=(Incoming&&) noexcept;
    ~Incoming();

    ShowSaveMerge::Plan& Plan() { return _plan; }
    const ShowSaveMerge::Plan& Plan() const { return _plan; }

    // Some file differs on disk from its recorded base.
    bool HasChanges() const;
    // The changed files merge (their base is known and every version parses).
    bool CanMerge() const { return _plan.CanMerge(); }
    // Every conflict is decided (or there are none): Apply can run.
    bool CanApply() const { return _plan.CanMerge() && _plan.AllResolved(); }
    bool NeedsDecisions() const { return _plan.CanMerge() && !_plan.AllResolved(); }
    size_t Decisions() const { return _plan.Conflicts().size(); }
    // Records (models, groups, controllers, presets...) the other side
    // added, removed or changed since the base.
    size_t ChangedItems() const;
    // The files that changed on disk, and their kinds.
    std::vector<std::string> ChangedPaths() const;
    std::vector<FileKind> ChangedKinds() const;
    // What was read from disk for a file of the plan.
    const std::string& Theirs(size_t file) const { return _theirs[file]; }
    bool Edited(size_t file) const { return _edited[file]; }
    bool Changed(size_t file) const { return _changed[file]; }

    // The disk still holds what Prepare read (nothing changed again since).
    bool StillOnDisk(const Reader& reader = {}) const;
    void AdoptResolutions(const Incoming& previous) { _plan.AdoptResolutions(previous._plan); }

private:
    Incoming(ShowSaveMerge::Plan plan);
    ShowSaveMerge::Plan _plan;
    std::vector<std::string> _theirs;
    std::vector<std::string> _base;
    std::vector<ShowFileFingerprint> _theirsFingerprint;
    std::vector<bool> _changed;
    std::vector<bool> _edited;
};

struct Applied {
    ShowMergeApply::LayoutChanges layout;
    bool layoutApplied = false;
    bool controllersReloaded = false;
    bool presetsReloaded = false;
    // Effects dropped by a decided "map" of a deleted model's row.
    int droppedEffects = 0;
    // The parsed merged layout (layoutApplied), for host-held sections
    // (layout.otherSections).
    std::string layoutResult;

    std::vector<std::string> Describe() const;
};

// Brings memory up to the merged result (the open sequence's side of decided
// deletions first, then the layout, the controllers and the presets) and
// records what was read from disk as each changed file's new base. The caller
// has stopped rendering (AbortRender returned true) and checked CanApply().
Applied Apply(xLightsShowContext& ctx, const Incoming& incoming, ShowFileGuard& guard);

// Records (by layout id, else name) that differ between two versions of a file.
size_t CountChangedItems(FileKind kind, const std::string& before, const std::string& after);

// Who most likely wrote the change, from the fresh presence entries of the
// other installs sharing the show folder: the only one there is, or the only
// one editing that kind of file. Empty when it cannot be told.
std::string WhoChanged(const std::vector<ShowPresenceEntry>& freshOthers, const std::vector<FileKind>& kinds);

// "Layout updated by Sam on Office-PC — 3 items changed, 1 needs your decision".
// An empty `who` reads "someone else".
std::string BannerText(const std::string& who, const std::vector<FileKind>& kinds, size_t changedItems, size_t decisions);

} // namespace ShowFileLiveMerge
