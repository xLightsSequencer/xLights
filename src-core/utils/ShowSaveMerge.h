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
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ShowFileGuard.h"
#include "ShowFileMerge.h"

class LayoutRenameLog;

// Saving the show-wide files (xlights_rgbeffects.xml, xlights_networks.xml,
// the effect presets) when someone else may have saved them since this
// process loaded them. Shared by the desktop and the iPad; the UI only asks
// the questions.
//
//   Prepare   reads each file as it is on disk and the base the guard kept for
//             it. A file nobody else changed saves as it always did; one that
//             changed is merged (ShowFileMerge): the controllers first, then
//             the layout with the controller renames that merge found.
//   Conflicts lists what both sides changed differently; every one has to be
//             resolved before anything is written.
//   Write     writes every file atomically through the guard, which records
//             the new base. Nothing is written when a file changed on disk
//             again since Prepare (prepare again, AdoptResolutions, retry).
//   Report    what the merge renamed, and the problems the merged files have
//             that the files this process wanted to save did not.
namespace ShowSaveMerge {

using ShowFileMerge::FileKind;

struct FileToSave {
    FileKind kind = FileKind::RgbEffects;
    std::string path;
    // The text this process wants the file to hold.
    std::string mine;
    // False: not written, only consulted. Pass the controllers file this way
    // with a layout save so the layout merge follows controller renames
    // someone else made.
    bool write = true;
};

// While a collaboration session holds the layout (the layout lock, or a
// participant that isn't the host), the layout and controllers files stay
// unwritten with their changes pending. Anything else in the save (the effect
// presets, a separate file) is still written.
bool HeldWithLayout(FileKind kind);
struct HeldSplit {
    // What may still be saved. Layout or controllers files that are only
    // consulted go too: with nothing of theirs written they inform nothing.
    std::vector<FileToSave> writable;
    // Paths of the held files that would have been written.
    std::vector<std::string> held;
};
HeldSplit SplitHeldWithLayout(const std::vector<FileToSave>& files);

// The versions of one file, for Plan::FromVersions.
struct FileVersions {
    FileKind kind = FileKind::RgbEffects;
    std::string path;
    std::string mine;
    // nullopt: the version this process started from is not known.
    std::optional<std::string> base;
    // On disk now; empty when the file does not exist.
    std::string theirs;
    ShowFileFingerprint theirsFingerprint;
    // Whether disk differs from the version this process last loaded or saved.
    bool changedOnDisk = false;
    bool write = true;
};

enum class FileStatus {
    Unchanged, // nobody else changed it: written as given
    Merged,    // someone else changed it: the merged result is written
    NoBase,    // someone else changed it, but its base is unknown: cannot merge
    Failed,    // a version does not parse
};

struct ConflictRef {
    size_t file = 0;
    size_t index = 0;
};

// A conflict in words, for the conflict lists.
struct ConflictText {
    std::string fileName; // "xlights_rgbeffects.xml"
    std::string record;   // "Model 'Arch'"
    std::string what;     // "Brightness: both changed it"
    std::string base;     // the value before either change ("" when none)
    std::string mine;     // "(deleted)" where that side has none
    std::string theirs;
};

struct SaveReport {
    // The files that were merged.
    std::vector<std::string> mergedFiles;
    // "Model 'Arch' was renamed to 'Arch Left' by someone else."
    std::vector<std::string> renames;
    // Problems the merged files have that this process's files did not.
    std::vector<ShowFileMerge::ReferenceIssue> issues;
    // Added by the host after bringing memory up to the merged files: what
    // came in from the other changes, what needs attention, and model check
    // results that are new since before the save.
    std::vector<std::string> changes;
    std::vector<std::string> warnings;
    std::vector<std::string> checks;

    // The merged files and the list of what came in are not on their own
    // worth interrupting for.
    bool HasSomethingToSay() const { return !renames.empty() || !issues.empty() || !warnings.empty() || !checks.empty(); }
    std::string Text() const;
};

// How the open sequence uses a model or group: whether it has a row for it,
// and how many effects that row holds (its own layers, its submodel, strand
// and node rows). The host answers; see ShowMergeApply::SequenceUseOf.
struct SequenceUseInfo {
    bool present = false;
    int effects = 0;
};
using SequenceUse = std::function<SequenceUseInfo(const std::string& name)>;

// What a save does to the open sequence for a model or group the merged
// layout deleted (a decided DeletedWithEffects conflict other than Keep).
struct SequenceChange {
    std::string model;
    int effects = 0;
    bool map = false;   // false: the row and its effects are deleted
    std::string target; // map: the model or group the row becomes
};

enum class WriteResult {
    Written,
    ChangedAgain, // a file changed on disk since Prepare; nothing was written
    Refused,      // not mergeable, or unresolved conflicts
    Failed,       // a write failed (files before it were written)
};

class Plan {
public:
    // `sequenceUse` (optional) says which models and groups the open
    // sequence has effects on. A model or group the merged layout would
    // delete while the sequence has effects on it is a conflict of its own
    // (ConflictKind::DeletedWithEffects): Mine keeps the model (the result
    // holds this process's record of it), Theirs deletes it and its row in
    // the sequence, Map moves the row's effects to another model or group
    // (MapTargets). It is never removed silently.
    static Plan Prepare(const std::vector<FileToSave>& files, const ShowFileMerge::Options& options,
                        const ShowFileGuard& guard, SequenceUse sequenceUse = {});
    static Plan FromVersions(std::vector<FileVersions> files, const ShowFileMerge::Options& options,
                             SequenceUse sequenceUse = {});

    Plan(Plan&&) noexcept;
    Plan& operator=(Plan&&) noexcept;
    ~Plan();

    size_t FileCount() const;
    const std::string& Path(size_t file) const;
    FileKind Kind(size_t file) const;
    bool Writes(size_t file) const;
    FileStatus Status(size_t file) const;
    const std::string& Error(size_t file) const;
    std::optional<size_t> Find(FileKind kind, bool writtenOnly = true) const;

    // Someone else changed a file this plan writes.
    bool ExternalChanges() const;
    // Every file this plan writes can be saved: unchanged, or merged.
    bool CanMerge() const;
    // The written files someone else changed (for the prompt).
    std::vector<std::string> ChangedPaths() const;

    std::vector<ConflictRef> Conflicts() const;
    bool HasConflicts() const { return !Conflicts().empty(); }
    const ShowFileMerge::Conflict& GetConflict(ConflictRef ref) const;
    ConflictText Describe(ConflictRef ref) const;
    // Map needs `mapTarget`, one of MapTargets(), and only applies to a
    // DeletedWithEffects conflict.
    bool Resolve(ConflictRef ref, ShowFileMerge::Resolution resolution, const std::string& mapTarget = {});
    // Where a DeletedWithEffects conflict's effects can go: the models and
    // groups of the merged layout the open sequence has no row for (the same
    // pool the missing-model mapping offers when a sequence is opened).
    std::vector<std::string> MapTargets() const;
    // The decided DeletedWithEffects conflicts that change the open
    // sequence (Theirs and Map). Apply them with
    // ShowMergeApply::ApplySequenceChanges before the layout reaches memory.
    std::vector<SequenceChange> SequenceChanges() const;
    void ResolveAll(ShowFileMerge::Resolution resolution);
    bool AllResolved() const;
    // Carries the resolutions made on an earlier plan for the same files
    // (matched by conflict key) over to this one.
    void AdoptResolutions(const Plan& previous);

    // What Write puts in the file: mine when nobody else changed it, the
    // merged result otherwise.
    std::string Result(size_t file) const;
    const std::string& Mine(size_t file) const;
    bool ResultDiffersFromMine(size_t file) const;

    SaveReport Report() const;

    WriteResult Write(ShowFileGuard& guard, std::string* error = nullptr) const;

private:
    struct Impl;
    explicit Plan(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> _impl;
};

// A save nobody watches (command line, batch render, automation script) never
// overwrites someone else's work and never decides a conflict: it saves when
// nobody else changed the files, or when they merge with no conflicts.
bool CanSaveUnattended(const Plan& plan);

// After a written plan: the rename logs describe this process's version of
// the files relative to their base, and the plan just recorded new bases.
// Clears the model and view object logs when the layout was written, the
// controller log when the controllers were.
void ClearRenameLogs(const Plan& plan, LayoutRenameLog* models, LayoutRenameLog* viewObjects, LayoutRenameLog* controllers);

// The merge of an autosave being restored after someone else saved the real
// file: base is the version the autosave was built on (kept beside it), mine
// the autosave, theirs the file on disk. nullopt when the base is not known.
std::optional<Plan> PrepareAutosaveMerge(FileKind kind, const std::string& realPath, const std::string& backupPath);

// "models" -> "Model", "Controller" -> "Controller", ...
std::string RecordKindName(const std::string& section);
// An attribute value or the XML of a child as a short readable text
// ("line0=1-5, layout=horizontal"); "(none)" for nullopt.
std::string ReadableValue(const std::optional<std::string>& value);

} // namespace ShowSaveMerge
