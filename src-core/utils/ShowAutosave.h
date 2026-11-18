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

#include "ShowFileGuard.h"

#include <cstdint>
#include <ctime>
#include <optional>
#include <string>

class ShowFileGuard;

// Autosave backups for show folders that several people (or devices) share.
//
// Layout autosaves (rgbeffects, presets) go to a folder of their own per app
// install, "<show>/.xlights/autosave/<install-id>/", so two people working in
// the same show folder no longer overwrite each other's crash recovery.
// Sequence autosaves stay beside the sequence ("<seq>.xbkp"), where people
// open them by hand.
//
// Every autosave gets a sidecar "<backup>.json" saying which install wrote it,
// when, and which version of the real file it was based on. That decides
// whether it is offered back at load (only this install's, only when it
// differs from the real file) and whether restoring it would discard changes
// someone else saved since.
struct ShowAutosaveInfo {
    std::string installId;  // empty: written for an autosave from an older version
    std::string deviceName;
    std::string appVersion;
    int64_t savedAt = 0;    // seconds since the epoch
    // The real file as this install last loaded or saved it when the autosave
    // was written: what the autosaved state was built on.
    ShowFileFingerprint base;
    // The autosave file itself. A sidecar that no longer matches its backup
    // (an older version rewrote the backup) is ignored.
    ShowFileFingerprint backup;
    bool declined = false;  // the user said No; don't offer it again
};

enum class AutosaveOffer {
    None,           // nothing to offer
    Offer,          // this install's autosave; the real file is as it was based on
    OfferConflict,  // this install's autosave, but the real file was changed since (someone else saved)
    OfferLegacy,    // no usable sidecar (older version): may belong to someone else
    Stale,          // this install's autosave holds what the real file already has; remove it
};

struct AutosaveCheck {
    AutosaveOffer offer = AutosaveOffer::None;
    bool hasInfo = false;     // info came from a sidecar that matches the backup
    ShowAutosaveInfo info;
    int64_t backupTime = 0;   // modification time of the backup (seconds since the epoch)
};

namespace ShowAutosave {

// The identity of this app install. Set once at startup; the id is a random
// UUID the app keeps in its own settings (see NewInstallId).
void SetIdentity(const std::string& installId, const std::string& deviceName, const std::string& appVersion);
std::string InstallId();
std::string DeviceName();
std::string NewInstallId();
// Returns the id stored in settings, or a new one when it is missing or malformed
// (the caller stores it back).
std::string ValidInstallIdOr(const std::string& stored);
bool IsValidInstallId(const std::string& id);

// "<show>/.xlights"
std::string ShowStateDir(const std::string& showDir);
// "<show>/.xlights/autosave/<installId>"
std::string LayoutAutosaveDir(const std::string& showDir, const std::string& installId);
// The layout autosave for a show file such as "xlights_rgbeffects.xml" or
// "xlights_effectpresets.json": "<LayoutAutosaveDir>/<stem>.xbkp" (".jbkp" for
// a .json file).
std::string LayoutAutosavePath(const std::string& showDir, const std::string& installId, const std::string& realFileName);
std::string LayoutAutosavePath(const std::string& showDir, const std::string& realFileName);
// The file older versions autosave to, in the show folder itself.
// LayoutAutosavePath for this install, creating its folder. Empty when this
// install has no id yet or the folder cannot be created.
std::string PrepareLayoutAutosavePath(const std::string& showDir, const std::string& realFileName);
std::string LegacyLayoutAutosavePath(const std::string& showDir, const std::string& realFileName);
// "<backup>.json"
std::string SidecarPath(const std::string& backupPath);

// True for a directory name that show-folder walks (backup, packaging, media
// search) should skip.
bool IsShowStateDirName(const std::string& name);

std::string ToJson(const ShowAutosaveInfo& info);
std::optional<ShowAutosaveInfo> FromJson(const std::string& json);
std::optional<ShowAutosaveInfo> ReadSidecar(const std::string& backupPath);

// After writing an autosave: records who wrote it and the version of the real
// file it was based on. Uses this install's identity.
bool WriteSidecar(const std::string& backupPath, const ShowFileFingerprint& base);
bool WriteSidecar(const std::string& backupPath, const ShowFileFingerprint& base, const ShowAutosaveInfo& identity);
// The base to record for an autosave of realPath: what the guard recorded for
// it, or the file as it is now when it is not tracked.
ShowFileFingerprint BaseFor(const std::string& realPath, const ShowFileGuard& guard);

// Whether to offer backupPath back for realPath, from the point of view of
// install myInstallId.
AutosaveCheck Check(const std::string& realPath, const std::string& backupPath, const std::string& myInstallId);
AutosaveCheck Check(const std::string& realPath, const std::string& backupPath);

// Writes the backup's contents over realPath through the guard (an atomic
// write, then recorded so the next save compares against it). When
// keepCurrentAsCopy and the real file exists, it is first copied beside itself
// ("<stem>_before_restore_<time><ext>") and that path is returned in
// keptCopyPath. Removes the backup and its sidecar on success.
bool Restore(const std::string& realPath, const std::string& backupPath, bool keepCurrentAsCopy,
             ShowFileGuard& guard, std::string* keptCopyPath = nullptr);
// Writes the backup's contents to ShowFileGuard::MakeCopyPath(realPath) and
// removes the backup and its sidecar. Empty on failure.
std::string SaveAsCopy(const std::string& realPath, const std::string& backupPath);
// Marks the backup as declined so it is not offered again until a newer
// autosave replaces it. A backup with no sidecar gets one (with no install id).
bool Decline(const std::string& backupPath);
// Removes the backup and its sidecar.
void Remove(const std::string& backupPath);
// Removes the backup and its sidecar only when this install wrote them: for
// cleaning up after a save, without deleting someone else's autosave of the
// same sequence or an older version's.
bool RemoveIfMine(const std::string& backupPath);

// "<dir>/<stem>_<tag>_<YYYYMMDD_HHMMSS><ext>", made unique against what is on disk.
std::string MakeTaggedCopyPath(const std::string& path, const std::string& tag, std::time_t when);

} // namespace ShowAutosave
