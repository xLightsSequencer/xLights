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

    // Writes a temp file beside the target and renames it over the target, so a
    // reader (or a sync client) never sees a half-written file. Falls back to
    // writing in place when the temp file cannot be created (sandbox or folder
    // permissions). A failed write never truncates the existing file unless it
    // came from that in-place fallback.
    static bool WriteFileAtomically(const std::string& path, const std::string& bytes);
    // Same, for large files written by a serializer that streams to a path
    // (e.g. pugi::xml_document::save_file). The writer must create or truncate
    // the path it is given and return false on failure.
    using Writer = std::function<bool(const std::string& writePath)>;
    static bool WriteFileAtomically(const std::string& path, const Writer& writer);

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

    // A full rewrite from in-memory state: afterwards the disk holds exactly what
    // this process has, so the new contents are recorded.
    bool SaveFile(const std::string& path, const std::string& bytes);
    bool SaveFile(const std::string& path, const Writer& writer);
    // A load-modify-write of part of the file (the rest came from disk just now).
    // An outstanding external change stays flagged so a later full save still asks.
    bool SavePatchedFile(const std::string& path, const std::string& bytes);

private:
    static std::string Key(const std::string& path);

    mutable std::mutex _lock;
    std::map<std::string, ShowFileFingerprint> _recorded;
};
