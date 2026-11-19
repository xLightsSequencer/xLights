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

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ShowFileGuard.h"

// Notices, while a show is open, that someone else (another person sharing
// the show folder, or another app) rewrote one of the shared show files, so
// the host can bring their changes in right away instead of only at save.
//
// The host calls Poll() on a timer. A file is reported when
//   - its contents (fingerprint, not mtime) differ from what the guard
//     recorded: what this process last loaded or wrote, so its own saves are
//     never reported;
//   - it has not changed again for the debounce interval (sync clients write
//     in bursts, and a layout save rewrites several files one after another);
//     the files that changed are reported together;
//   - it was not already reported with these contents.
// A missing or unreadable file is not reported: a sync client may be
// replacing it, or iCloud may not have downloaded it yet.
//
// A cheap stamp (size and modification time) decides when the contents are
// read; they are also read every kFullCheckMs, and after Poke(). On Apple a
// file presenter on the show folder pokes the watcher when a coordinated
// writer (iCloud Drive, another app, the Files app) changes something there,
// and the contents are read through NSFileCoordinator so a half-synced file is
// never fingerprinted. Elsewhere polling is all there is.
class ShowFileWatcher {
public:
    struct Stamp {
        bool exists = false;
        uint64_t size = 0;
        int64_t mtime = 0;
        bool operator==(const Stamp&) const = default;
    };
    using Clock = std::function<int64_t()>; // milliseconds, monotonic
    using StampFunction = std::function<Stamp(const std::string& path)>;
    // nullopt while the file cannot be read (yet).
    using FingerprintFunction = std::function<std::optional<ShowFileFingerprint>(const std::string& path)>;

    static constexpr int64_t kDefaultDebounceMs = 2000;
    static constexpr int64_t kFullCheckMs = 15000;

    explicit ShowFileWatcher(const ShowFileGuard& guard);
    ~ShowFileWatcher();
    ShowFileWatcher(const ShowFileWatcher&) = delete;
    ShowFileWatcher& operator=(const ShowFileWatcher&) = delete;

    // Tests replace the clock, the stamp and the fingerprint.
    void SetClock(Clock clock) { _clock = std::move(clock); }
    void SetStampFunction(StampFunction f) { _stamp = std::move(f); }
    void SetFingerprintFunction(FingerprintFunction f) { _fingerprint = std::move(f); }
    void SetDebounce(int64_t ms) { _debounceMs = ms; }
    // Whether Watch() registers the platform notifications (on by default;
    // tests turn it off).
    void SetPlatformNotifications(bool enable) { _usePlatform = enable; }

    // The files to watch, all in one folder (the show folder). Watching a
    // different set starts over; the same set again changes nothing.
    void Watch(const std::vector<std::string>& paths);
    void Stop();
    bool IsWatching() const { return !_files.empty(); }
    const std::vector<std::string>& Paths() const { return _paths; }

    // Something in the folder may have changed: read the contents on the
    // next Poll. Safe from any thread.
    void Poke() { _poked->store(true); }

    // The files that changed, once per change (see the class comment).
    std::vector<std::string> Poll();
    std::vector<std::string> Poll(int64_t nowMs);

    // Poll with the file I/O (stamps, coordinated reads) on a worker thread,
    // for hosts that poll from the UI thread: a show folder on a stalled
    // network share or behind an iCloud writer can block for a long time.
    // StartBackgroundPoll returns false while the previous one is still
    // running. TakeBackgroundPoll, called later on the same thread as every
    // other method, returns nullopt until the worker is done, then the
    // changed files exactly as Poll would have. A Watch/Stop in between
    // discards that result.
    bool StartBackgroundPoll();
    bool StartBackgroundPoll(int64_t nowMs);
    std::optional<std::vector<std::string>> TakeBackgroundPoll();
    bool BackgroundPollRunning() const { return _job != nullptr; }
    // Whether the file's contents matched what the guard recorded the last
    // time a poll read them; nullopt when no poll has read it since Watch.
    // Lets a host check an outstanding change without reading the file itself.
    std::optional<bool> MatchedRecordedAtLastRead(const std::string& path) const;
    // The contents (and fingerprint) the last poll read, read the same way as
    // CoordinatedRead; nullopt when no poll has read the file since Watch, or
    // when tests replaced the fingerprint function. A merge prepared from
    // these needs no file I/O on the calling thread; if the file changed
    // since, the next poll reports it again.
    std::optional<std::pair<std::string, ShowFileFingerprint>> LastReadContents(const std::string& path) const;

    // Report the file again the next time it is found changed (the host
    // could not act on the report).
    void Rearm(const std::string& path);
    void RearmAll();

    // The stamp and fingerprint used outside tests.
    static Stamp StampOf(const std::string& path);
    static std::optional<ShowFileFingerprint> CoordinatedFingerprint(const std::string& path);
    // The file's contents for a merge, read the same way (coordinated on
    // Apple). Like ShowFileGuard::ReadFile: empty text and a "missing"
    // fingerprint when it does not exist, nullopt when it cannot be read.
    static std::optional<std::pair<std::string, ShowFileFingerprint>> CoordinatedRead(const std::string& path);

private:
    struct FileState {
        Stamp stamp;
        bool stampKnown = false;
        int64_t lastRead = 0;
        std::optional<ShowFileFingerprint> candidate;
        int64_t candidateSince = 0;
        std::optional<ShowFileFingerprint> reported;
        std::optional<bool> matchedRecorded;
        std::optional<std::pair<std::string, ShowFileFingerprint>> contents;
    };

    // One file's I/O for a poll, gathered without touching _files.
    struct Probe {
        std::string path;
        // From _files and the guard when the poll started.
        bool tracked = false;
        bool stampKnown = false;
        Stamp priorStamp;
        bool hasCandidate = false;
        int64_t lastRead = 0;
        // Gathered.
        Stamp stamp;
        bool read = false;
        std::optional<ShowFileFingerprint> fingerprint;
        std::optional<std::string> text;
    };
    struct PollJob {
        int64_t nowMs = 0;
        bool poked = false;
        uint64_t generation = 0;
        std::vector<Probe> probes;
        std::atomic<bool> done{ false };
    };
    std::shared_ptr<PollJob> BeginPoll(int64_t nowMs);
    static void Gather(PollJob& job, const StampFunction& stamp, const FingerprintFunction& fingerprint);
    std::vector<std::string> FinishPoll(const PollJob& job);

    int64_t Now() const;
    void StartPlatform(const std::string& folder);
    void StopPlatform();

    const ShowFileGuard& _guard;
    Clock _clock;
    StampFunction _stamp;
    FingerprintFunction _fingerprint;
    int64_t _debounceMs = kDefaultDebounceMs;
    bool _usePlatform = true;
    std::vector<std::string> _paths;
    std::map<std::string, FileState> _files;
    uint64_t _generation = 0;
    std::shared_ptr<PollJob> _job;
    // Shared with the platform callback, which may outlive a Stop() briefly.
    std::shared_ptr<std::atomic<bool>> _poked = std::make_shared<std::atomic<bool>>(false);
    void* _platform = nullptr;
};
