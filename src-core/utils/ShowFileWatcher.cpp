/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowFileWatcher.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <system_error>

namespace {

std::optional<std::filesystem::path> PathOf(const std::string& s) {
    try {
        return std::filesystem::path(std::u8string(s.begin(), s.end()));
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

} // namespace

ShowFileWatcher::ShowFileWatcher(const ShowFileGuard& guard) :
    _guard(guard) {
}

ShowFileWatcher::~ShowFileWatcher() {
    StopPlatform();
}

int64_t ShowFileWatcher::Now() const {
    if (_clock) {
        return _clock();
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void ShowFileWatcher::Watch(const std::vector<std::string>& paths) {
    if (paths == _paths) {
        return;
    }
    Stop();
    _paths = paths;
    for (const auto& p : _paths) {
        _files[p] = FileState{};
    }
    if (_usePlatform && !_paths.empty()) {
        if (auto p = PathOf(_paths.front())) {
            auto u8 = p->parent_path().u8string();
            StartPlatform(std::string(u8.begin(), u8.end()));
        }
    }
}

void ShowFileWatcher::Stop() {
    StopPlatform();
    _paths.clear();
    _files.clear();
    _poked->store(false);
}

void ShowFileWatcher::Rearm(const std::string& path) {
    auto it = _files.find(path);
    if (it != _files.end()) {
        it->second.reported.reset();
        it->second.stampKnown = false;
    }
}

void ShowFileWatcher::RearmAll() {
    for (auto& [path, st] : _files) {
        st.reported.reset();
        st.stampKnown = false;
    }
}

std::vector<std::string> ShowFileWatcher::Poll() {
    return Poll(Now());
}

std::vector<std::string> ShowFileWatcher::Poll(int64_t nowMs) {
    const bool poked = _poked->exchange(false);
    for (const auto& path : _paths) {
        FileState& st = _files[path];
        const Stamp stamp = _stamp ? _stamp(path) : StampOf(path);
        const bool stampChanged = !st.stampKnown || !(stamp == st.stamp);
        st.stamp = stamp;
        st.stampKnown = true;
        if (!_guard.IsTracked(path)) {
            st.candidate.reset();
            continue;
        }
        if (!stamp.exists) {
            continue;
        }
        const bool look = stampChanged || poked || st.candidate.has_value() || nowMs - st.lastRead >= kFullCheckMs;
        if (!look) {
            continue;
        }
        auto fp = _fingerprint ? _fingerprint(path) : CoordinatedFingerprint(path);
        if (!fp || !fp->exists) {
            // Not readable yet: try again on the next poll.
            continue;
        }
        st.lastRead = nowMs;
        auto recorded = _guard.RecordedFingerprint(path);
        if (recorded && *recorded == *fp) {
            // What this process last loaded or wrote (or it changed back).
            st.candidate.reset();
            st.reported.reset();
            continue;
        }
        if (st.reported && *st.reported == *fp) {
            st.candidate.reset();
            continue;
        }
        if (!st.candidate || !(*st.candidate == *fp)) {
            st.candidate = fp;
            st.candidateSince = nowMs;
        }
    }

    int64_t latest = -1;
    for (const auto& path : _paths) {
        const FileState& st = _files[path];
        if (st.candidate) {
            latest = std::max(latest, st.candidateSince);
        }
    }
    std::vector<std::string> out;
    if (latest < 0 || nowMs - latest < _debounceMs) {
        return out;
    }
    for (const auto& path : _paths) {
        FileState& st = _files[path];
        if (st.candidate) {
            st.reported = st.candidate;
            st.candidate.reset();
            out.push_back(path);
        }
    }
    return out;
}

ShowFileWatcher::Stamp ShowFileWatcher::StampOf(const std::string& path) {
    Stamp s;
    auto p = PathOf(path);
    if (!p) {
        return s;
    }
    std::error_code ec;
    auto st = std::filesystem::status(*p, ec);
    if (ec || !std::filesystem::is_regular_file(st)) {
        return s;
    }
    s.exists = true;
    auto size = std::filesystem::file_size(*p, ec);
    s.size = ec ? 0 : (uint64_t)size;
    auto t = std::filesystem::last_write_time(*p, ec);
    s.mtime = ec ? 0 : (int64_t)t.time_since_epoch().count();
    return s;
}

#ifndef __APPLE__

void ShowFileWatcher::StartPlatform(const std::string&) {
}

void ShowFileWatcher::StopPlatform() {
}

std::optional<ShowFileFingerprint> ShowFileWatcher::CoordinatedFingerprint(const std::string& path) {
    return ShowFileGuard::FingerprintFile(path);
}

std::optional<std::pair<std::string, ShowFileFingerprint>> ShowFileWatcher::CoordinatedRead(const std::string& path) {
    return ShowFileGuard::ReadFile(path);
}

#endif
