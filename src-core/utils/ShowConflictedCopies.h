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

#include <ctime>
#include <optional>
#include <string>
#include <vector>

#include "ShowFileGuard.h"
#include "ShowSaveMerge.h"

// The copies cloud sync clients make of a show-wide file (layout, controllers,
// effect presets) when two people saved it at about the same time, and
// merging one back into the file:
//
//   Dropbox      "xlights_rgbeffects (Sam's conflicted copy 2026-11-18).xml"
//   Nextcloud    "xlights_rgbeffects (conflicted copy 2026-11-18 104512).xml"
//   iCloud       "xlights_rgbeffects 2.xml"
//   OneDrive     "xlights_rgbeffects-LAPTOP.xml"
//   Google Drive "xlights_rgbeffects(1).xml", "xlights_rgbeffects (1).xml"
//   Syncthing    "xlights_rgbeffects.sync-conflict-20261118-104512-ABCDEFG.xml"
//
// The merge is the save merge (ShowSaveMerge::Plan) with the copy as "mine"
// and the file on disk as "theirs", so an undecided conflict keeps the file
// and the plan's write checks that the file did not change meanwhile. Its
// base is the version both were built on, when one can be found.
namespace ShowConflictedCopies {

using ShowFileMerge::FileKind;

struct NameMatch {
    FileKind kind = FileKind::RgbEffects;
    std::string realName; // "xlights_rgbeffects.xml"
    std::string client;   // "Dropbox", "iCloud", ...
};

// Whether a file name is a sync client's conflicted copy of one of the
// show-wide files. Names only; Find also checks the contents.
std::optional<NameMatch> MatchFileName(const std::string& fileName);

// Whether `text` is a file of that kind (the layout's <xrgb> root, the
// controllers' <Networks> root, a presets JSON object).
bool ContentIsKind(FileKind kind, const std::string& text);

struct Found {
    FileKind kind = FileKind::RgbEffects;
    std::string client;
    std::string realPath;
    std::string copyPath;
    ShowFileFingerprint fingerprint; // of the copy
    std::time_t modified = 0;        // the copy's modification time
    bool sameAsCurrent = false;      // it holds what the file holds now
};

// The conflicted copies in the show folder itself (not its subfolders) whose
// file they are a copy of exists, whose contents are a file of that kind, and
// that this install was not told to ignore.
std::vector<Found> Find(const std::string& showDir, const std::string& installId);

// "Ignore": remembered per install by the copy's fingerprint, so the same
// copy is not offered again (a different copy with the same name is).
std::string IgnoreListPath(const std::string& showDir, const std::string& installId);
bool Ignore(const std::string& showDir, const std::string& installId, const Found& copy);
bool IsIgnored(const std::string& showDir, const std::string& installId, const ShowFileFingerprint& fp);

enum class BaseSource {
    Autosave, // the version this install's autosave of the file was built on
    Backup,   // a backup of the file older than both versions
    None,     // nothing found: merged two-way, every difference is a conflict
};

struct BaseChoice {
    BaseSource source = BaseSource::None;
    std::string text;
    std::string path; // where it came from
};

// The common version to merge from: this install's autosave base copy of the
// file when there is one and it holds a file of that kind; else the newest
// "<backupRoot>/Backup/*/<file>" whose modification time is older than both
// the file's and the copy's; else none.
BaseChoice ChooseBase(const Found& copy, const std::string& showDir, const std::string& installId, const std::string& backupRoot);

// The merge of the copy into the file. nullopt (with *error) when either
// cannot be read. With no base it runs two-way (ShowFileMerge::Options::twoWay).
std::optional<ShowSaveMerge::Plan> PrepareMerge(const Found& copy, const BaseChoice& base, std::string* error = nullptr);

// "<backupRoot>/Backup/ConflictedCopies"
std::string BackupFolder(const std::string& backupRoot);
// Moves (never deletes) a file into BackupFolder, keeping its name unless
// that is taken. Returns the new path, empty on failure (the file stays).
std::string MoveToBackup(const std::string& path, const std::string& backupRoot);
// Copies the file into BackupFolder as "<stem>_before_merge_<time><ext>".
std::string CopyToBackup(const std::string& path, const std::string& backupRoot);

struct FinishResult {
    bool written = false;
    ShowSaveMerge::WriteResult writeResult = ShowSaveMerge::WriteResult::Refused;
    std::string error;
    std::string keptCurrent; // the file before the merge, in the backup folder
    std::string movedCopy;   // where the conflicted copy went
};
// After every conflict is resolved: keeps the current file in the backup
// folder, writes the merged file atomically through the guard, then moves the
// conflicted copy into the backup folder. Nothing is moved when the write
// fails or the file changed on disk again (prepare again).
FinishResult Finish(const ShowSaveMerge::Plan& plan, const Found& copy, const std::string& backupRoot, ShowFileGuard& guard);

// The show's backup folder setting ("backupDir" in the layout's settings)
// when it names a folder that exists, else the show folder.
std::string BackupRootFor(const std::string& showDir);

// "xlights_rgbeffects (Sam's conflicted copy 2026-11-18).xml (Dropbox)": for lists.
std::string Describe(const Found& copy);

} // namespace ShowConflictedCopies
