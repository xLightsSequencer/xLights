/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowPresence.h"

#include "ExternalHooks.h"
#include "ShowAutosave.h"
#include "ShowFileGuard.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

namespace {

std::optional<std::filesystem::path> ToPath(const std::string& s) {
    try {
        return std::filesystem::path(s);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::string JoinPath(const std::string& dir, const std::string& name) {
    if (dir.empty()) {
        return name;
    }
    char last = dir.back();
    if (last == '/' || last == '\\') {
        return dir + name;
    }
    return dir + "/" + name;
}

std::optional<std::string> ReadSmallFile(const std::filesystem::path& p) {
    std::ifstream ifs(p, std::ios::binary);
    if (!ifs.is_open()) {
        return std::nullopt;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    if (ifs.bad()) {
        return std::nullopt;
    }
    return ss.str();
}

std::string Lower(std::string s) {
    for (auto& c : s) {
        c = (char)std::tolower((unsigned char)c);
    }
    return s;
}

std::string NormalizedForCompare(const std::string& s) {
    std::string t = s;
    std::replace(t.begin(), t.end(), '\\', '/');
    auto p = ToPath(t);
    if (!p) {
        return Lower(t);
    }
    return Lower(p->lexically_normal().generic_string());
}

// Seconds since the file was last modified, measured on this machine's clock.
std::optional<int64_t> FileAgeSeconds(const std::filesystem::path& p, int64_t now) {
    std::error_code ec;
    auto ft = std::filesystem::last_write_time(p, ec);
    if (ec) {
        return std::nullopt;
    }
    auto sys = std::chrono::time_point_cast<std::chrono::seconds>(ft - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    return now - (int64_t)sys.time_since_epoch().count();
}

} // namespace

std::string ShowPresenceEntry::DisplayName() const {
    if (userName.empty()) {
        return deviceName.empty() ? std::string("Someone") : deviceName;
    }
    if (deviceName.empty()) {
        return userName;
    }
    return userName + " on " + deviceName;
}

namespace ShowPresence {

int64_t Now() {
    return (int64_t)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string ThisPlatform() {
#if defined(__APPLE__) && TARGET_OS_IPHONE
    return "iPad";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(_WIN32)
    return "Windows";
#else
    return "Linux";
#endif
}

std::string PresenceDir(const std::string& showDir) {
    return JoinPath(ShowAutosave::ShowStateDir(showDir), "presence");
}

std::string PresencePath(const std::string& showDir, const std::string& installId) {
    return JoinPath(PresenceDir(showDir), installId + ".json");
}

std::string RelativeSequencePath(const std::string& showDir, const std::string& sequencePath) {
    if (sequencePath.empty()) {
        return std::string();
    }
    auto seq = ToPath(sequencePath);
    auto show = ToPath(showDir);
    if (!seq) {
        return sequencePath;
    }
    std::filesystem::path seqN = seq->lexically_normal();
    // A sequence opened from its autosave is the same sequence.
    if (Lower(seqN.extension().string()) == ".xbkp") {
        seqN.replace_extension(".xsq");
    }
    if (show && !showDir.empty()) {
        std::filesystem::path showN = show->lexically_normal();
        // "a/b/" normalizes to "a/b/" with an empty filename; drop it so the
        // relative path does not start with "..".
        if (!showN.has_filename()) {
            showN = showN.parent_path();
        }
        std::filesystem::path rel = seqN.lexically_relative(showN);
        if (!rel.empty() && *rel.begin() != "..") {
            return rel.generic_string();
        }
    }
    return seqN.generic_string();
}

bool SameSequence(const std::string& a, const std::string& b) {
    if (a.empty() || b.empty()) {
        return false;
    }
    return NormalizedForCompare(a) == NormalizedForCompare(b);
}

std::string ToJson(const ShowPresenceEntry& e) {
    nlohmann::json j;
    j["version"] = 1;
    j["installId"] = e.installId;
    j["userName"] = e.userName;
    j["device"] = e.deviceName;
    j["platform"] = e.platform;
    j["appVersion"] = e.appVersion;
    j["sequence"] = e.sequence;
    j["editingLayout"] = e.editingLayout;
    j["editingControllers"] = e.editingControllers;
    j["heartbeat"] = e.heartbeat;
    return j.dump(2);
}

std::optional<ShowPresenceEntry> FromJson(const std::string& json) {
    nlohmann::json j = nlohmann::json::parse(json, nullptr, false);
    if (j.is_discarded() || !j.is_object() || !j.contains("installId") || !j.contains("heartbeat")) {
        return std::nullopt;
    }
    try {
        ShowPresenceEntry e;
        e.installId = j.value("installId", std::string());
        e.userName = j.value("userName", std::string());
        e.deviceName = j.value("device", std::string());
        e.platform = j.value("platform", std::string());
        e.appVersion = j.value("appVersion", std::string());
        e.sequence = j.value("sequence", std::string());
        e.editingLayout = j.value("editingLayout", false);
        e.editingControllers = j.value("editingControllers", false);
        e.heartbeat = j.value("heartbeat", (int64_t)0);
        return e;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::vector<ShowPresenceEntry> ReadEntries(const std::string& showDir) {
    std::vector<ShowPresenceEntry> result;
    if (showDir.empty()) {
        return result;
    }
    auto dir = ToPath(PresenceDir(showDir));
    if (!dir) {
        return result;
    }
    std::error_code ec;
    if (!std::filesystem::is_directory(*dir, ec)) {
        return result;
    }
    std::filesystem::directory_iterator it(*dir, ec), end;
    for (; !ec && it != end; it.increment(ec)) {
        const std::filesystem::path p = it->path();
        const std::string name = p.filename().string();
        // An evicted iCloud file shows up as ".<name>.icloud". Start its
        // download so the next read sees it; don't wait for it.
        if (name.size() > 8 && name[0] == '.' && name.ends_with(".json.icloud")) {
            FileExists((p.parent_path() / name.substr(1, name.size() - 8)).string(), false);
            continue;
        }
        if (p.extension() != ".json" || !ShowAutosave::IsValidInstallId(p.stem().string())) {
            continue;
        }
        auto text = ReadSmallFile(p);
        if (!text) {
            continue;
        }
        auto e = FromJson(*text);
        if (!e || Lower(e->installId) != Lower(p.stem().string())) {
            continue;
        }
        result.push_back(std::move(*e));
    }
    return result;
}

int64_t AgeSeconds(const ShowPresenceEntry& e, int64_t now) {
    return std::max<int64_t>(0, now - e.heartbeat);
}

bool IsFresh(const ShowPresenceEntry& e, int64_t now, int64_t staleAfter) {
    // A clock running ahead would otherwise keep an entry fresh long after the
    // app stopped; beyond a plausible skew the heartbeat is not trusted at all.
    if (e.heartbeat > now + kMaxFutureSeconds) {
        return false;
    }
    return now - e.heartbeat <= staleAfter;
}

std::vector<ShowPresenceEntry> FreshOthers(const std::vector<ShowPresenceEntry>& entries, const std::string& myInstallId, int64_t now) {
    std::vector<ShowPresenceEntry> result;
    const std::string me = Lower(myInstallId);
    for (const auto& e : entries) {
        if (Lower(e.installId) == me || !IsFresh(e, now)) {
            continue;
        }
        result.push_back(e);
    }
    return result;
}

std::vector<ShowPresenceEntry> FreshOthers(const std::string& showDir, const std::string& myInstallId, int64_t now) {
    return FreshOthers(ReadEntries(showDir), myInstallId, now);
}

std::vector<ShowPresenceEntry> WhoHasSequenceOpen(const std::string& showDir, const std::string& sequencePath,
                                                  const std::string& myInstallId, int64_t now) {
    std::vector<ShowPresenceEntry> result;
    const std::string mine = RelativeSequencePath(showDir, sequencePath);
    if (mine.empty()) {
        return result;
    }
    for (auto& e : FreshOthers(showDir, myInstallId, now)) {
        if (SameSequence(e.sequence, mine)) {
            result.push_back(std::move(e));
        }
    }
    return result;
}

int RemoveVeryStale(const std::string& showDir, const std::string& myInstallId, int64_t now, int64_t maxAge) {
    if (showDir.empty()) {
        return 0;
    }
    auto dir = ToPath(PresenceDir(showDir));
    if (!dir) {
        return 0;
    }
    std::error_code ec;
    if (!std::filesystem::is_directory(*dir, ec)) {
        return 0;
    }
    const std::string me = Lower(myInstallId);
    std::vector<std::filesystem::path> doomed;
    std::filesystem::directory_iterator it(*dir, ec), end;
    for (; !ec && it != end; it.increment(ec)) {
        const std::filesystem::path p = it->path();
        const std::string stem = p.stem().string();
        if (p.extension() != ".json" || !ShowAutosave::IsValidInstallId(stem) || Lower(stem) == me) {
            continue;
        }
        // Both clocks have to agree it is old: the heartbeat comes from the
        // writer's clock, the modification time from wherever the file was
        // last written or synced.
        auto fileAge = FileAgeSeconds(p, now);
        if (!fileAge || *fileAge <= maxAge) {
            continue;
        }
        auto text = ReadSmallFile(p);
        if (text) {
            auto e = FromJson(*text);
            if (e && e->heartbeat <= now + kMaxFutureSeconds && now - e->heartbeat <= maxAge) {
                continue;
            }
        }
        doomed.push_back(p);
    }
    int removed = 0;
    for (const auto& p : doomed) {
        std::error_code rec;
        if (std::filesystem::remove(p, rec)) {
            ++removed;
        }
    }
    return removed;
}

std::string LastSeenText(int64_t ageSeconds) {
    if (ageSeconds < 60) {
        return "just now";
    }
    return std::to_string(ageSeconds / 60) + " min ago";
}

std::string SequenceOpenWarning(const std::vector<ShowPresenceEntry>& holders, int64_t now) {
    if (holders.empty()) {
        return std::string();
    }
    const auto& h = holders.front();
    std::string text;
    if (!h.userName.empty() && !h.deviceName.empty()) {
        text = h.userName + " has this sequence open on " + h.deviceName;
    } else {
        text = h.DisplayName() + " has this sequence open";
    }
    text += " (last seen " + LastSeenText(AgeSeconds(h, now)) + ").";
    if (holders.size() == 2) {
        text += " " + holders[1].DisplayName() + " has it open too.";
    } else if (holders.size() > 2) {
        text += " " + std::to_string(holders.size() - 1) + " others have it open too.";
    }
    text += " Opening it too can lose one of your changes.";
    return text;
}

std::string EditingHint(const std::vector<ShowPresenceEntry>& others) {
    std::string text;
    for (const auto& e : others) {
        const char* what = nullptr;
        if (e.editingLayout && e.editingControllers) {
            what = "the layout and controllers";
        } else if (e.editingLayout) {
            what = "the layout";
        } else if (e.editingControllers) {
            what = "the controllers";
        } else {
            continue;
        }
        if (!text.empty()) {
            text += "; ";
        }
        text += e.DisplayName() + " is editing " + what;
    }
    return text;
}

} // namespace ShowPresence

ShowPresencePublisher& ShowPresencePublisher::Instance() {
    static ShowPresencePublisher instance;
    return instance;
}

void ShowPresencePublisher::SetIdentity(const std::string& installId, const std::string& userName, const std::string& deviceName,
                                        const std::string& platform, const std::string& appVersion) {
    std::lock_guard<std::mutex> l(_lock);
    if (installId != _installId) {
        RemoveOwnFileLocked();
    }
    _installId = installId;
    _userName = userName;
    _deviceName = deviceName;
    _platform = platform;
    _appVersion = appVersion;
    _written = false;
}

std::string ShowPresencePublisher::InstallId() const {
    std::lock_guard<std::mutex> l(_lock);
    return _installId;
}

void ShowPresencePublisher::Enable(bool enable) {
    std::lock_guard<std::mutex> l(_lock);
    if (!enable) {
        RemoveOwnFileLocked();
        _showDir.clear();
    }
    _enabled = enable;
}

bool ShowPresencePublisher::IsEnabled() const {
    std::lock_guard<std::mutex> l(_lock);
    return _enabled;
}

int ShowPresencePublisher::WriteCount() const {
    std::lock_guard<std::mutex> l(_lock);
    return _writeCount;
}

void ShowPresencePublisher::RemoveOwnFileLocked() {
    if (!_showDir.empty() && ShowAutosave::IsValidInstallId(_installId)) {
        const std::string path = ShowPresence::PresencePath(_showDir, _installId);
        if (auto p = ToPath(path)) {
            std::error_code ec;
            std::filesystem::remove(*p, ec);
        }
    }
    _written = false;
}

void ShowPresencePublisher::Clear() {
    std::lock_guard<std::mutex> l(_lock);
    RemoveOwnFileLocked();
    _showDir.clear();
}

bool ShowPresencePublisher::Update(const std::string& showDir, const ShowPresenceState& state, int64_t now) {
    std::lock_guard<std::mutex> l(_lock);
    if (!_enabled || !ShowAutosave::IsValidInstallId(_installId)) {
        return false;
    }
    if (showDir != _showDir) {
        RemoveOwnFileLocked();
        _showDir = showDir;
        if (!showDir.empty()) {
            int removed = ShowPresence::RemoveVeryStale(showDir, _installId, now);
            if (removed > 0) {
                spdlog::info("ShowPresence: removed {} presence files not updated in a long time.", removed);
            }
        }
    }
    if (showDir.empty()) {
        return false;
    }
    const std::string sequence = ShowPresence::RelativeSequencePath(showDir, state.sequencePath);
    const bool unchanged = _written && sequence == _lastSequence && state.editingLayout == _lastLayout &&
                           state.editingControllers == _lastControllers;
    // A clock that went backwards rewrites rather than waiting it out.
    if (unchanged && now >= _lastWrite && now - _lastWrite < ShowPresence::kHeartbeatSeconds) {
        return false;
    }

    ShowPresenceEntry e;
    e.installId = _installId;
    e.userName = _userName;
    e.deviceName = _deviceName;
    e.platform = _platform;
    e.appVersion = _appVersion;
    e.sequence = sequence;
    e.editingLayout = state.editingLayout;
    e.editingControllers = state.editingControllers;
    e.heartbeat = now;

    const std::string dir = ShowPresence::PresenceDir(showDir);
    auto dirPath = ToPath(dir);
    if (!dirPath) {
        return false;
    }
    std::error_code ec;
    std::filesystem::create_directories(*dirPath, ec);
    if (!ShowFileGuard::WriteFileAtomically(ShowPresence::PresencePath(showDir, _installId), ShowPresence::ToJson(e))) {
        spdlog::debug("ShowPresence: could not write the presence file in {}", dir);
        // Try again on the next update rather than after a full heartbeat.
        _written = false;
        return false;
    }
    _written = true;
    _lastWrite = now;
    _lastSequence = sequence;
    _lastLayout = state.editingLayout;
    _lastControllers = state.editingControllers;
    ++_writeCount;
    return true;
}
