/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#ifdef __APPLE__
#include <fcntl.h>
#include <sys/clonefile.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "CollabFiles.h"

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace Collab::Files {

namespace Test {
std::atomic<bool> noClone{ false };
std::function<void()> beforeCopy;
namespace {
std::atomic<size_t> s_inFlight{ 0 };
}
size_t CopiesInFlight() {
    return s_inFlight.load();
}
} // namespace Test

FILE* OpenForCopy(const std::string& path) {
#ifdef _WIN32
    // Deleting or replacing an open file needs FILE_SHARE_DELETE, which fopen
    // doesn't ask for.
    HANDLE h = CreateFileW(fs::path(path).wstring().c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        return nullptr;
    }
    int fd = _open_osfhandle((intptr_t)h, _O_RDONLY | _O_BINARY);
    if (fd < 0) {
        CloseHandle(h);
        return nullptr;
    }
    FILE* f = _fdopen(fd, "rb");
    if (f == nullptr) {
        _close(fd);
    }
    return f;
#else
    return fopen(path.c_str(), "rb");
#endif
}

namespace {

#ifdef __APPLE__
// A clone is instant, but only works on the same APFS volume.
bool Clone(FILE* src, const std::string& dest, uint64_t& size) {
    if (Test::noClone.load()) {
        return false;
    }
    if (fclonefileat(fileno(src), AT_FDCWD, dest.c_str(), 0) != 0) {
        return false;
    }
    struct stat st {};
    if (stat(dest.c_str(), &st) != 0) {
        std::error_code ec;
        fs::remove(dest, ec);
        return false;
    }
    size = (uint64_t)st.st_size;
    return true;
}
#endif

bool Copy(FILE* src, const std::string& dest, uint64_t& size) {
#ifdef __APPLE__
    if (Clone(src, dest, size)) {
        return true;
    }
#endif
    size = 0;
    FILE* out = fopen(dest.c_str(), "wb");
    if (out == nullptr) {
        return false;
    }
    std::vector<char> buf(1u << 20);
    bool ok = true;
    while (true) {
        size_t n = fread(buf.data(), 1, buf.size(), src);
        if (n == 0) {
            ok = ferror(src) == 0;
            break;
        }
        if (fwrite(buf.data(), 1, n, out) != n) {
            ok = false;
            break;
        }
        size += n;
    }
    return fclose(out) == 0 && ok;
}

} // namespace

void FileCopier::Add(Job j) {
    ++Test::s_inFlight;
    bool start = false;
    {
        std::lock_guard<std::mutex> l(_m);
        _jobs.push_back(std::move(j));
        if (!_running) {
            _running = true;
            start = true;
        }
    }
    if (start) {
        std::shared_ptr<FileCopier> self = shared_from_this();
        std::thread([self] { self->Run(); }).detach();
    }
}

void FileCopier::Run() {
    while (true) {
        Job j;
        {
            std::lock_guard<std::mutex> l(_m);
            if (_jobs.empty()) {
                _running = false;
                return;
            }
            j = std::move(_jobs.front());
            _jobs.pop_front();
        }
        if (Test::beforeCopy) {
            Test::beforeCopy();
        }
        uint64_t size = 0;
        bool ok = Copy(j.src, j.dest, size);
        fclose(j.src);
        if (!ok) {
            std::error_code ec;
            fs::remove(j.dest, ec);
        }
        j.done(ok, size);
        --Test::s_inFlight;
    }
}

IncomingBlob::Result IncomingBlob::Start(const std::string& path, BlobId id, uint64_t size, uint64_t maxSize) {
    if (_active || size > maxSize) {
        return Result::Bad;
    }
    _active = true;
    _id = id;
    _size = size;
    _got = 0;
    _path = path;
    _error.clear();
    _file = fopen(path.c_str(), "wb");
    if (_file == nullptr) {
        DiskFailed("couldn't create");
    }
    return Result::Ok;
}

IncomingBlob::Result IncomingBlob::Data(BlobId id, std::string_view bytes) {
    if (!_active || id != _id || _got + bytes.size() > _size) {
        return Result::Bad;
    }
    if (_file != nullptr && fwrite(bytes.data(), 1, bytes.size(), _file) != bytes.size()) {
        DiskFailed("couldn't write");
    }
    _got += bytes.size();
    return Result::Ok;
}

IncomingBlob::Result IncomingBlob::End(BlobId id, std::string& path, std::string& why) {
    if (!_active || id != _id || _got != _size) {
        return Result::Bad;
    }
    if (_file != nullptr && fclose(_file) != 0) {
        _file = nullptr;
        DiskFailed("couldn't finish writing");
    }
    _file = nullptr;
    _active = false;
    if (!_error.empty()) {
        why = _error;
        return Result::Failed;
    }
    path = _path;
    return Result::Received;
}

void IncomingBlob::Abort() {
    if (_file != nullptr) {
        fclose(_file);
        _file = nullptr;
        std::error_code ec;
        fs::remove(_path, ec);
    }
    _active = false;
}

void IncomingBlob::DiskFailed(const std::string& what) {
    if (!_error.empty()) {
        return;
    }
    int e = errno;
    _error = "Couldn't save a file from the session (" + what + " " + _path + ": " + std::strerror(e) + ").";
    spdlog::error("collab: {}", _error);
    if (_file != nullptr) {
        fclose(_file);
        _file = nullptr;
    }
    std::error_code ec;
    fs::remove(_path, ec);
}

} // namespace Collab::Files
