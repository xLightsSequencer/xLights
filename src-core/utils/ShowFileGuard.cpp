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

#include "ExternalHooks.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <random>
#include <system_error>
#include <vector>

#include <spdlog/spdlog.h>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <thread>
#elif !defined(__APPLE__)
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace {

// Word-at-a-time multiply/xor hash. Change detection only, not security; the
// size is compared as well.
class ContentHasher {
public:
    void Add(const void* data, size_t len) {
        const unsigned char* p = static_cast<const unsigned char*>(data);
        _size += len;
        if (_carryLen > 0) {
            while (_carryLen < 8 && len > 0) {
                _carry[_carryLen++] = *p++;
                --len;
            }
            if (_carryLen < 8) {
                return;
            }
            Mix(Load(_carry));
            _carryLen = 0;
        }
        while (len >= 8) {
            Mix(Load(p));
            p += 8;
            len -= 8;
        }
        while (len > 0) {
            _carry[_carryLen++] = *p++;
            --len;
        }
    }

    ShowFileFingerprint Finish() {
        if (_carryLen > 0) {
            unsigned char tail[8] = { 0 };
            std::memcpy(tail, _carry, _carryLen);
            Mix(Load(tail));
        }
        Mix(_size);
        ShowFileFingerprint fp;
        fp.exists = true;
        fp.size = _size;
        fp.hash = _h;
        return fp;
    }

private:
    static uint64_t Load(const unsigned char* p) {
        uint64_t w = 0;
        for (int i = 7; i >= 0; --i) {
            w = (w << 8) | p[i];
        }
        return w;
    }
    void Mix(uint64_t w) {
        _h ^= w;
        _h *= 0x9E3779B97F4A7C15ULL;
        _h ^= _h >> 31;
    }

    uint64_t _h = 0xCBF29CE484222325ULL;
    uint64_t _size = 0;
    unsigned char _carry[8] = { 0 };
    size_t _carryLen = 0;
};

#ifdef _WIN32
std::wstring Widen(const std::string& s) {
    if (s.empty()) {
        return std::wstring();
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n > 0 ? n : 0, L'\0');
    if (n > 0) {
        MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    }
    return w;
}
#endif

// Show paths are UTF-8 (the Windows app runs with the UTF-8 code page); build
// the path from wide characters there so a name the ANSI code page cannot map
// doesn't throw or get mangled.
std::optional<std::filesystem::path> ToPath(const std::string& s) {
    try {
#ifdef _WIN32
        return std::filesystem::path(Widen(s));
#else
        return std::filesystem::path(s);
#endif
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::string ToUtf8(const std::filesystem::path& p) {
    auto u8 = p.u8string();
    return std::string(u8.begin(), u8.end());
}

bool WriteStream(const std::filesystem::path& p, const std::string& bytes, bool& opened) {
    std::ofstream ofs(p, std::ios::binary | std::ios::trunc);
    opened = ofs.is_open();
    if (!opened) {
        return false;
    }
    ofs.write(bytes.data(), (std::streamsize)bytes.size());
    ofs.flush();
    bool ok = ofs.good();
    ofs.close();
    return ok && !ofs.fail();
}

} // namespace

ShowFileGuard& ShowFileGuard::Instance() {
    static ShowFileGuard guard;
    return guard;
}

ShowFileFingerprint ShowFileGuard::FingerprintOf(const void* data, size_t len) {
    ContentHasher h;
    h.Add(data, len);
    return h.Finish();
}

std::optional<ShowFileFingerprint> ShowFileGuard::FingerprintFile(const std::string& path) {
    // FileExists, not std::filesystem: on macOS it brings down a file iCloud
    // evicted, which would otherwise read as missing.
    if (path.empty() || !FileExists(path)) {
        return ShowFileFingerprint{};
    }
    auto p = ToPath(path);
    if (!p) {
        return std::nullopt;
    }
    std::ifstream ifs(*p, std::ios::binary);
    if (!ifs.is_open()) {
        return std::nullopt;
    }
    ContentHasher h;
    std::vector<char> buf(1 << 20);
    while (ifs) {
        ifs.read(buf.data(), (std::streamsize)buf.size());
        std::streamsize n = ifs.gcount();
        if (n > 0) {
            h.Add(buf.data(), (size_t)n);
        }
    }
    if (ifs.bad()) {
        return std::nullopt;
    }
    return h.Finish();
}

bool ShowFileGuard::WriteFileAtomically(const std::string& path, const std::string& bytes) {
    return WriteFileAtomically(path, [&bytes](const std::string& writePath) {
        auto p = ToPath(writePath);
        bool opened = false;
        return p && WriteStream(*p, bytes, opened);
    });
}

namespace {

constexpr const char* kTempPrefix = ".~";
constexpr const char* kTempTag = ".xl";
#ifdef __APPLE__
// iCloud Drive never uploads a name ending in ".nosync".
constexpr const char* kTempSuffix = ".nosync";
#else
constexpr const char* kTempSuffix = ".tmp";
#endif
constexpr size_t kTempIdLen = 16;

bool EndsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool CanCreate(const std::filesystem::path& p) {
    std::ofstream probe(p, std::ios::binary | std::ios::trunc);
    return probe.is_open();
}

void RemoveQuietly(const std::filesystem::path& p) {
    std::error_code ec;
    std::filesystem::remove(p, ec);
}

// Moves the finished temp file over the target, keeping as much of the
// target's identity (metadata, permissions, owner) as the platform allows.
bool ReplaceTarget(const std::filesystem::path& tmp, const std::filesystem::path& target, std::string& error) {
    std::error_code ec;
    const bool targetExists = std::filesystem::exists(target, ec);
#if defined(__APPLE__)
    if (targetExists) {
        if (ReplaceFileKeepingAttributes(ToUtf8(target), ToUtf8(tmp), error)) {
            return true;
        }
        if (!std::filesystem::exists(tmp, ec)) {
            // Nothing left to move; the replace either finished or lost the copy.
            return std::filesystem::exists(target, ec);
        }
        spdlog::warn("ShowFileGuard: could not replace '{}' keeping its metadata ({}); renaming over it", ToUtf8(target), error);
    }
    std::filesystem::rename(tmp, target, ec);
    if (ec) {
        error = ec.message();
        return false;
    }
    return true;
#elif defined(_WIN32)
    const std::wstring wt = target.wstring();
    const std::wstring wr = tmp.wstring();
    if (targetExists) {
        // ReplaceFileW keeps the target's ACLs, attributes, creation time and
        // alternate data streams. Antivirus and sync clients briefly open new
        // files without delete sharing, so retry a few times.
        DWORD err = 0;
        for (int attempt = 0; attempt < 5; ++attempt) {
            if (ReplaceFileW(wt.c_str(), wr.c_str(), nullptr, REPLACEFILE_IGNORE_MERGE_ERRORS | REPLACEFILE_IGNORE_ACL_ERRORS, nullptr, nullptr)) {
                return true;
            }
            err = GetLastError();
            if (err != ERROR_SHARING_VIOLATION && err != ERROR_ACCESS_DENIED && err != ERROR_LOCK_VIOLATION) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (!std::filesystem::exists(tmp, ec)) {
            error = "ReplaceFileW error " + std::to_string(err);
            return std::filesystem::exists(target, ec);
        }
        // ERROR_UNABLE_TO_MOVE_REPLACEMENT means the target is already gone and
        // the new copy is intact; other failures (a file system without
        // ReplaceFileW support) leave both in place. A plain move finishes both.
        spdlog::warn("ShowFileGuard: ReplaceFileW failed for '{}' (error {}); moving the new copy over it", ToUtf8(target), (unsigned)err);
    }
    if (MoveFileExW(wr.c_str(), wt.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        return true;
    }
    error = "MoveFileExW error " + std::to_string(GetLastError());
    return false;
#else
    if (targetExists) {
        struct stat st;
        const std::string t = target.string();
        const std::string r = tmp.string();
        if (::stat(t.c_str(), &st) == 0) {
            ::chmod(r.c_str(), st.st_mode & 07777);
            // Only root (or the owner, for the group) may do this; when it is
            // not allowed the file ends up owned by whoever saved it.
            if (::chown(r.c_str(), st.st_uid, st.st_gid) != 0) {
                (void)!::chown(r.c_str(), (uid_t)-1, st.st_gid);
            }
        }
    }
    std::filesystem::rename(tmp, target, ec);
    if (ec) {
        error = ec.message();
        return false;
    }
    return true;
#endif
}

enum class TempWrite { Replaced,
                       WriteFailed,
                       Unavailable };

TempWrite WriteThroughTemp(const std::filesystem::path& tmp, const std::filesystem::path& target, const std::string& displayPath, const ShowFileGuard::Writer& writer) {
    if (!CanCreate(tmp)) {
        return TempWrite::Unavailable;
    }
    if (!writer(ToUtf8(tmp))) {
        // The temp copy failed part way (disk full, I/O error). Writing in
        // place would most likely fail the same way after truncating the
        // existing file, so leave it alone.
        RemoveQuietly(tmp);
        spdlog::error("ShowFileGuard: writing a new copy of '{}' failed; the existing file was left untouched", displayPath);
        return TempWrite::WriteFailed;
    }
    std::string error;
    if (ReplaceTarget(tmp, target, error)) {
        return TempWrite::Replaced;
    }
    spdlog::warn("ShowFileGuard: could not replace '{}' with the new copy ({}); writing it in place", displayPath, error);
    RemoveQuietly(tmp);
    return TempWrite::Unavailable;
}

// `tempDir`, when set, is a same-volume directory outside the target's folder
// to stage the new copy in; otherwise (or if it can't be used) the temp goes
// beside the target.
bool WriteResolvedTarget(const std::filesystem::path& target, const std::string& tempDir, const std::string& displayPath, const ShowFileGuard::Writer& writer) {
    std::error_code ec;
    // A rename gives the target a new file, which would cut it loose from its
    // other hard links. Rewrite the shared file instead.
    auto links = std::filesystem::hard_link_count(target, ec);
    if (ec || links <= 1) {
        if (!tempDir.empty()) {
            if (auto dir = ToPath(tempDir)) {
                switch (WriteThroughTemp(*dir / target.filename(), target, displayPath, writer)) {
                case TempWrite::Replaced:
                    return true;
                case TempWrite::WriteFailed:
                    return false;
                case TempWrite::Unavailable:
                    break;
                }
            }
        }
        if (auto tmp = ToPath(ShowFileGuard::TempPathFor(ToUtf8(target)))) {
            switch (WriteThroughTemp(*tmp, target, displayPath, writer)) {
            case TempWrite::Replaced:
                return true;
            case TempWrite::WriteFailed:
                return false;
            case TempWrite::Unavailable:
                break;
            }
        }
        spdlog::debug("ShowFileGuard: cannot stage a new copy of '{}'; writing it in place", displayPath);
    }

    if (!writer(ToUtf8(target))) {
        spdlog::error("ShowFileGuard: writing '{}' failed", displayPath);
        return false;
    }
    return true;
}

} // namespace

std::string ShowFileGuard::TempPathFor(const std::string& target) {
    static std::atomic<uint32_t> counter{ 0 };
    static const uint32_t salt = std::random_device{}();
    char id[kTempIdLen + 1];
    std::snprintf(id, sizeof(id), "%08x%08x", (unsigned)salt, (unsigned)counter++);

    auto p = ToPath(target);
    if (!p) {
        return target + kTempTag + id + kTempSuffix;
    }
    auto name = ToPath(kTempPrefix + ToUtf8(p->filename()) + kTempTag + id + kTempSuffix);
    if (!name) {
        return target + kTempTag + id + kTempSuffix;
    }
    return ToUtf8(p->parent_path() / *name);
}

bool ShowFileGuard::IsTempFileName(const std::string& fileName) {
    // ".~" <name> ".xl" <16 hex> (".tmp" | ".nosync"); both suffixes are
    // recognised so a show folder shared between platforms is swept everywhere.
    if (fileName.compare(0, 2, kTempPrefix) != 0) {
        return false;
    }
    std::string rest;
    if (EndsWith(fileName, ".tmp")) {
        rest = fileName.substr(0, fileName.size() - 4);
    } else if (EndsWith(fileName, ".nosync")) {
        rest = fileName.substr(0, fileName.size() - 7);
    } else {
        return false;
    }
    const size_t tagLen = std::strlen(kTempTag);
    if (rest.size() < 2 + 1 + tagLen + kTempIdLen) {
        return false;
    }
    const size_t idStart = rest.size() - kTempIdLen;
    if (rest.compare(idStart - tagLen, tagLen, kTempTag) != 0) {
        return false;
    }
    for (size_t i = idStart; i < rest.size(); ++i) {
        char c = rest[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }
    return true;
}

int ShowFileGuard::RemoveStaleTempFiles(const std::string& dir, int minAgeSeconds) {
    auto d = ToPath(dir);
    if (!d || dir.empty()) {
        return 0;
    }
    std::error_code ec;
    std::filesystem::directory_iterator it(*d, ec);
    if (ec) {
        return 0;
    }
    const auto cutoff = std::filesystem::file_time_type::clock::now() - std::chrono::seconds(minAgeSeconds);
    int removed = 0;
    for (; it != std::filesystem::directory_iterator(); it.increment(ec)) {
        if (ec) {
            break;
        }
        const auto& entry = *it;
        std::error_code e2;
        if (!entry.is_regular_file(e2) || !IsTempFileName(ToUtf8(entry.path().filename()))) {
            continue;
        }
        auto when = entry.last_write_time(e2);
        if (e2 || when > cutoff) {
            continue;
        }
        if (std::filesystem::remove(entry.path(), e2)) {
            spdlog::info("ShowFileGuard: removed '{}' left behind by an interrupted save", ToUtf8(entry.path()));
            ++removed;
        }
    }
    return removed;
}

bool ShowFileGuard::WriteFileAtomically(const std::string& path, const Writer& writer) {
    ObtainAccessToURL(path, true);
    auto target = ToPath(path);
    if (!target) {
        spdlog::error("ShowFileGuard: cannot write '{}': the path cannot be represented", path);
        return false;
    }
    std::error_code ec;
    // Write the file a symlink points at; replacing the link would cut it loose.
    if (std::filesystem::is_symlink(*target, ec)) {
        auto resolved = std::filesystem::canonical(*target, ec);
        if (!ec) {
            target = resolved;
        }
        ec.clear();
    }

#ifdef __APPLE__
    return CoordinatedFileReplace(ToUtf8(*target), [&](const std::string& coordinated, const std::string& tempDir) {
        auto dest = ToPath(coordinated);
        return WriteResolvedTarget(dest ? *dest : *target, tempDir, path, writer);
    });
#else
    return WriteResolvedTarget(*target, std::string(), path, writer);
#endif
}

std::string ShowFileGuard::MakeCopyPath(const std::string& path) {
    return MakeCopyPath(path, std::time(nullptr));
}

std::string ShowFileGuard::MakeCopyPath(const std::string& path, std::time_t when) {
    std::tm tmv{};
#ifdef _WIN32
    localtime_s(&tmv, &when);
#else
    localtime_r(&when, &tmv);
#endif
    char stamp[32];
    std::strftime(stamp, sizeof(stamp), "%Y%m%d_%H%M%S", &tmv);

    auto p = ToPath(path);
    if (!p) {
        return path + "_mine_" + stamp;
    }
    std::filesystem::path dir = p->parent_path();
    std::string stem = p->stem().string();
    std::string ext = p->extension().string();
    std::string base = stem + "_mine_" + stamp;
    std::filesystem::path candidate = dir / (base + ext);
    for (int n = 2; FileExists(candidate.string(), false) && n < 1000; ++n) {
        candidate = dir / (base + "_" + std::to_string(n) + ext);
    }
    return candidate.string();
}

std::string ShowFileGuard::Key(const std::string& path) {
    auto p = ToPath(path);
    if (!p) {
        return path;
    }
    return p->lexically_normal().generic_string();
}

void ShowFileGuard::RecordFromDisk(const std::string& path) {
    if (path.empty()) {
        return;
    }
    auto fp = FingerprintFile(path);
    std::lock_guard<std::mutex> l(_lock);
    if (fp) {
        _recorded[Key(path)] = *fp;
    } else {
        // Can't read it, so nothing to compare a later save against.
        _recorded.erase(Key(path));
    }
}

void ShowFileGuard::Record(const std::string& path, const std::string& bytes) {
    if (path.empty()) {
        return;
    }
    auto fp = FingerprintOf(bytes);
    std::lock_guard<std::mutex> l(_lock);
    _recorded[Key(path)] = fp;
}

void ShowFileGuard::Forget(const std::string& path) {
    std::lock_guard<std::mutex> l(_lock);
    _recorded.erase(Key(path));
}

void ShowFileGuard::Clear() {
    std::lock_guard<std::mutex> l(_lock);
    _recorded.clear();
}

bool ShowFileGuard::IsTracked(const std::string& path) const {
    std::lock_guard<std::mutex> l(_lock);
    return _recorded.find(Key(path)) != _recorded.end();
}

std::optional<ShowFileFingerprint> ShowFileGuard::RecordedFingerprint(const std::string& path) const {
    std::lock_guard<std::mutex> l(_lock);
    auto it = _recorded.find(Key(path));
    if (it == _recorded.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool ShowFileGuard::HasChangedOnDisk(const std::string& path) const {
    ShowFileFingerprint recorded;
    {
        std::lock_guard<std::mutex> l(_lock);
        auto it = _recorded.find(Key(path));
        if (it == _recorded.end()) {
            return false;
        }
        recorded = it->second;
    }
    auto now = FingerprintFile(path);
    if (!now) {
        spdlog::warn("ShowFileGuard: cannot read '{}' to check it for changes", path);
        return false;
    }
    if (!(*now == recorded)) {
        spdlog::info("ShowFileGuard: '{}' changed on disk since it was loaded or saved (was {} bytes{}, now {} bytes{})",
                     path, recorded.size, recorded.exists ? "" : ", missing", now->size, now->exists ? "" : ", missing");
        return true;
    }
    return false;
}

bool ShowFileGuard::SaveFile(const std::string& path, const std::string& bytes) {
    if (!WriteFileAtomically(path, bytes)) {
        return false;
    }
    Record(path, bytes);
    return true;
}

bool ShowFileGuard::SaveFile(const std::string& path, const Writer& writer) {
    if (!WriteFileAtomically(path, writer)) {
        return false;
    }
    RecordFromDisk(path);
    return true;
}

bool ShowFileGuard::SavePatchedFile(const std::string& path, const std::string& bytes) {
    bool const hadExternalChange = HasChangedOnDisk(path);
    if (!WriteFileAtomically(path, bytes)) {
        return false;
    }
    if (!hadExternalChange) {
        Record(path, bytes);
    }
    return true;
}
