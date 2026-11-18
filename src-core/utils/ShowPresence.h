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
#include <mutex>
#include <optional>
#include <string>
#include <vector>

// Advisory presence for show folders that several people (or devices) share.
//
// Each running app keeps "<show>/.xlights/presence/<install-id>.json" up to
// date: who it is, which sequence it has open and whether it has unsaved
// layout or controller edits, plus a heartbeat time. Other installs read those
// files to warn before opening a sequence someone else has open, and to show
// a hint while someone else is editing the layout. Sync latency and clock skew
// make all of this advisory: nothing is locked, and an entry whose heartbeat is
// old (or implausibly far in the future) is ignored.
struct ShowPresenceEntry {
    std::string installId;
    std::string userName;    // may be empty (the iPad has no user name)
    std::string deviceName;
    std::string platform;    // "macOS", "Windows", "Linux", "iPad"
    std::string appVersion;
    // The open sequence, relative to the show folder with '/' separators
    // (absolute when it is outside the show folder). Empty when none is open.
    std::string sequence;
    bool editingLayout = false;
    bool editingControllers = false;
    int64_t heartbeat = 0;   // seconds since the epoch (UTC)

    // "Sam on Office-PC", or just the device when there is no user name.
    std::string DisplayName() const;
};

// What this install is doing right now.
struct ShowPresenceState {
    std::string sequencePath;  // absolute path of the open sequence, or empty
    bool editingLayout = false;
    bool editingControllers = false;
};

namespace ShowPresence {

constexpr int64_t kHeartbeatSeconds = 60;          // rewrite at least this often while open
constexpr int64_t kStaleSeconds = 5 * 60;          // older heartbeats are ignored (allows for sync latency)
constexpr int64_t kMaxFutureSeconds = 10 * 60;     // a heartbeat further ahead than this is not trusted
constexpr int64_t kCleanupSeconds = 7 * 24 * 3600; // other installs' files older than this are removed

int64_t Now();
// "macOS", "Windows", "Linux" or "iPad" for this build.
std::string ThisPlatform();

// "<show>/.xlights/presence"
std::string PresenceDir(const std::string& showDir);
// "<show>/.xlights/presence/<installId>.json"
std::string PresencePath(const std::string& showDir, const std::string& installId);

// The form stored in an entry's `sequence` field. A sequence backup (.xbkp)
// counts as its .xsq.
std::string RelativeSequencePath(const std::string& showDir, const std::string& sequencePath);
// Whether two stored sequence paths name the same sequence (ignores case,
// separators and "." / ".." segments).
bool SameSequence(const std::string& a, const std::string& b);

std::string ToJson(const ShowPresenceEntry& entry);
std::optional<ShowPresenceEntry> FromJson(const std::string& json);

// Every readable entry in the show's presence folder, fresh or not. Only files
// named "<install-id>.json" whose content carries the same id count.
std::vector<ShowPresenceEntry> ReadEntries(const std::string& showDir);

// Seconds since the heartbeat; 0 for a heartbeat in the (near) future.
int64_t AgeSeconds(const ShowPresenceEntry& entry, int64_t now);
bool IsFresh(const ShowPresenceEntry& entry, int64_t now, int64_t staleAfter = kStaleSeconds);

// Fresh entries of installs other than myInstallId.
std::vector<ShowPresenceEntry> FreshOthers(const std::vector<ShowPresenceEntry>& entries, const std::string& myInstallId, int64_t now);
std::vector<ShowPresenceEntry> FreshOthers(const std::string& showDir, const std::string& myInstallId, int64_t now);
// Fresh entries of other installs that have sequencePath open.
std::vector<ShowPresenceEntry> WhoHasSequenceOpen(const std::string& showDir, const std::string& sequencePath,
                                                  const std::string& myInstallId, int64_t now);

// Removes other installs' presence files whose heartbeat and modification
// time are both older than maxAge. Leaves this install's file, fresh entries
// and anything that is not a presence file alone. Returns the count removed.
int RemoveVeryStale(const std::string& showDir, const std::string& myInstallId, int64_t now, int64_t maxAge = kCleanupSeconds);

// "just now", "1 min ago", "12 min ago"
std::string LastSeenText(int64_t ageSeconds);
// The warning shown before opening a sequence others have open. Empty when
// holders is empty.
std::string SequenceOpenWarning(const std::vector<ShowPresenceEntry>& holders, int64_t now);
// "Sam on Office-PC is editing the layout", or empty when nobody in others is
// editing the layout or controllers.
std::string EditingHint(const std::vector<ShowPresenceEntry>& others);

} // namespace ShowPresence

// Keeps this install's presence file up to date. Disabled until Enable() is
// called, so a headless or command line run never writes one.
class ShowPresencePublisher {
public:
    static ShowPresencePublisher& Instance();

    void SetIdentity(const std::string& installId, const std::string& userName, const std::string& deviceName,
                     const std::string& platform, const std::string& appVersion);
    std::string InstallId() const;
    void Enable(bool enable);
    bool IsEnabled() const;

    // Writes the presence file for showDir when the state (or show folder)
    // changed since the last write, or when the last write is older than the
    // heartbeat interval. Switching show folders removes the file from the old
    // one. An empty showDir is the same as Clear(). Returns true when it wrote.
    bool Update(const std::string& showDir, const ShowPresenceState& state, int64_t now);
    bool Update(const std::string& showDir, const ShowPresenceState& state) { return Update(showDir, state, ShowPresence::Now()); }
    // Removes this install's presence file (show closed, app quitting).
    void Clear();

    int WriteCount() const;

private:
    mutable std::mutex _lock;
    std::string _installId;
    std::string _userName;
    std::string _deviceName;
    std::string _platform;
    std::string _appVersion;
    bool _enabled = false;

    std::string _showDir;
    bool _written = false;
    int64_t _lastWrite = 0;
    std::string _lastSequence;
    bool _lastLayout = false;
    bool _lastControllers = false;
    int _writeCount = 0;

    void RemoveOwnFileLocked();
};
