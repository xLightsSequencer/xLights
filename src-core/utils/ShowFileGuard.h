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

#include <cstdint>
#include <ctime>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

// Content fingerprint of a file. Size plus a 64-bit content hash: mtime is
// not used because sync clients (Dropbox, iCloud Drive, NAS shares) rewrite
// it, carry over the writer's clock, or round it to two seconds.
struct ShowFileFingerprint {
    bool exists = false;
    uint64_t size = 0;
    uint64_t hash = 0;

    bool operator==(const ShowFileFingerprint&) const = default;
};

// Detects show files (layout, controllers, presets, sequences) that were
// rewritten by someone else -- another person sharing the show folder, or
// another app -- since this process last loaded or saved them, so a save can
// ask instead of silently replacing their work.
//
// Record a file when it is loaded and whenever this process writes it, then
// ask HasChangedOnDisk() before overwriting it.
class ShowFileGuard {
public:
    static ShowFileGuard& Instance();

    static ShowFileFingerprint FingerprintOf(const void* data, size_t len);
    static ShowFileFingerprint FingerprintOf(const std::string& bytes) { return FingerprintOf(bytes.data(), bytes.size()); }
    // nullopt when the file exists but cannot be read.
    static std::optional<ShowFileFingerprint> FingerprintFile(const std::string& path);
    // The file's contents with their fingerprint; empty text and a "missing"
    // fingerprint when it does not exist, nullopt when it cannot be read.
    static std::optional<std::pair<std::string, ShowFileFingerprint>> ReadFile(const std::string& path);

    // The show-wide files several people save into (layout, controllers,
    // effect presets) are merged when someone else changed them, so the
    // guard keeps the text it recorded for them: the base of that merge.
    static bool KeepsBaseText(const std::string& path);

    // Writes the new contents to a temp file, then swaps it in for the target,
    // so a reader (or a sync client) never sees a half-written file. The swap
    // keeps the target's identity as far as each platform allows: on Apple it
    // is a coordinated NSFileManager replace from a same-volume replacement
    // directory outside the show folder (keeps extended attributes, tags,
    // creation date and file versions); on Windows ReplaceFileW (keeps ACLs,
    // attributes, creation time and alternate streams); elsewhere a rename that
    // keeps the mode and, where permitted, the owner. A symlinked target is
    // written through the link, and a hard-linked target is written in place so
    // the link survives. Falls back to writing in place when no temp file can
    // be created (sandbox or folder permissions). A failed write never
    // truncates the existing file unless it came from that in-place fallback.
    static bool WriteFileAtomically(const std::string& path, const std::string& bytes);
    // Same, for large files written by a serializer that streams to a path
    // (e.g. pugi::xml_document::save_file). The writer must create or truncate
    // the path it is given and return false on failure.
    using Writer = std::function<bool(const std::string& writePath)>;
    static bool WriteFileAtomically(const std::string& path, const Writer& writer);

    // Name of a temp file created beside `target` when it has to be. Named so
    // common sync clients skip it (leading ".~", ".tmp" / ".nosync" suffix).
    static std::string TempPathFor(const std::string& target);
    static bool IsTempFileName(const std::string& fileName);
    // Deletes temp files a crashed save left in `dir` (not its subfolders).
    // Only ones older than `minAgeSeconds`, so a save still in progress on
    // another machine sharing the folder is left alone. Returns the count.
    static int RemoveStaleTempFiles(const std::string& dir, int minAgeSeconds = 3600);

    // "<dir>/<stem>_mine_<YYYYMMDD_HHMMSS><ext>", made unique against what is
    // already on disk.
    static std::string MakeCopyPath(const std::string& path);
    static std::string MakeCopyPath(const std::string& path, std::time_t when);

    // Records the file as it is on disk now (missing counts as a state too).
    // Call it before parsing a file being loaded: if the file changes between
    // the two, the next save reports a conflict rather than missing one.
    void RecordFromDisk(const std::string& path);
    // Records contents this process just wrote (or read) without re-reading.
    void Record(const std::string& path, const std::string& bytes);
    void Forget(const std::string& path);
    void Clear();

    bool IsTracked(const std::string& path) const;
    // True when the file is tracked and its contents now differ from what was
    // recorded: rewritten, deleted, or created after it was recorded missing.
    // Untracked or unreadable files report false.
    bool HasChangedOnDisk(const std::string& path) const;
    // What was last recorded for the file; nullopt when it is not tracked.
    std::optional<ShowFileFingerprint> RecordedFingerprint(const std::string& path) const;
    // The text recorded with the fingerprint, for a KeepsBaseText() file.
    // Empty text means the file did not exist. nullopt when not tracked.
    std::optional<std::string> RecordedText(const std::string& path) const;

    // A full rewrite from in-memory state: afterwards the disk holds exactly what
    // this process has, so the new contents are recorded.
    bool SaveFile(const std::string& path, const std::string& bytes);
    bool SaveFile(const std::string& path, const Writer& writer);
    // Tests only: runs in SaveFile(Writer) after the file is in place and
    // before it is recorded, where another machine's write could land.
    void SetAfterSaveHook(std::function<void(const std::string&)> hook) { _afterSave = std::move(hook); }
    // A load-modify-write of part of the file (the rest came from disk just now).
    // An outstanding external change stays flagged so a later full save still asks.
    bool SavePatchedFile(const std::string& path, const std::string& bytes);

private:
    static std::string Key(const std::string& path);

    mutable std::mutex _lock;
    std::function<void(const std::string&)> _afterSave;
    std::map<std::string, ShowFileFingerprint> _recorded;
    std::map<std::string, std::string> _text;
};
