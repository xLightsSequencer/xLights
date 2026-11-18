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
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <random>
#include <system_error>
#include <vector>

#include <spdlog/spdlog.h>

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

// A narrow path that the Windows ANSI code page cannot map makes
// std::filesystem::path throw.
std::optional<std::filesystem::path> ToPath(const std::string& s) {
    try {
        return std::filesystem::path(s);
    } catch (const std::exception&) {
        return std::nullopt;
    }
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

bool ShowFileGuard::WriteFileAtomically(const std::string& path, const Writer& writer) {
    ObtainAccessToURL(path, true);
    auto target = ToPath(path);
    if (!target) {
        spdlog::error("ShowFileGuard: cannot write '{}': the path cannot be represented", path);
        return false;
    }
    std::error_code ec;
    // Write the file a symlink points at; renaming over the link would replace it.
    if (std::filesystem::is_symlink(*target, ec)) {
        auto resolved = std::filesystem::canonical(*target, ec);
        if (!ec) {
            target = resolved;
        }
        ec.clear();
    }
    const std::string targetStr = target->string();

    static std::atomic<unsigned> counter{ 0 };
    static const unsigned salt = std::random_device{}();
    std::filesystem::path tmp = target->parent_path() /
                                ("." + target->filename().string() + ".xltmp" + std::to_string(salt) + "_" + std::to_string(counter++));

    bool canCreateTemp = false;
    {
        std::ofstream probe(tmp, std::ios::binary | std::ios::trunc);
        canCreateTemp = probe.is_open();
    }
    if (canCreateTemp) {
        if (writer(tmp.string())) {
            auto st = std::filesystem::status(*target, ec);
            if (!ec && std::filesystem::exists(st)) {
                std::filesystem::permissions(tmp, st.permissions(), std::filesystem::perm_options::replace, ec);
                ec.clear();
            }
            std::filesystem::rename(tmp, *target, ec);
            if (!ec) {
                return true;
            }
            spdlog::warn("ShowFileGuard: could not replace '{}' with the new copy ({}); writing it in place", path, ec.message());
            std::error_code rmEc;
            std::filesystem::remove(tmp, rmEc);
        } else {
            // The temp copy failed part way (disk full, I/O error). Writing in
            // place would most likely fail the same way after truncating the
            // existing file, so leave it alone.
            std::error_code rmEc;
            std::filesystem::remove(tmp, rmEc);
            spdlog::error("ShowFileGuard: writing a new copy of '{}' failed; the existing file was left untouched", path);
            return false;
        }
    } else {
        spdlog::debug("ShowFileGuard: cannot create a temp file beside '{}'; writing it in place", path);
    }

    if (!writer(targetStr)) {
        spdlog::error("ShowFileGuard: writing '{}' failed", path);
        return false;
    }
    return true;
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
