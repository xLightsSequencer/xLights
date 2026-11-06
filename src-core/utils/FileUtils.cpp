/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "FileUtils.h"

#include "ExternalHooks.h"

#include <log.h>

#include <map>
#include <mutex>

#include <filesystem>
#include <chrono>
#include <ctime>
#include <cstdlib>
namespace FileUtils
{

std::optional<long long> GetFileModTimeTicks(const std::string& path)
{
    std::error_code ec;
    auto ftime = std::filesystem::last_write_time(path, ec);
    if (ec) {
        return std::nullopt;
    }

    // Portable file_time_type -> time_t conversion
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
    return static_cast<long long>(std::chrono::system_clock::to_time_t(sctp));
}

bool NeedsBaseFileUpdate(const std::string& path, const std::string& syncedTicks, const std::string& mergeDescription)
{
    auto baseTicks = GetFileModTimeTicks(path);
    if (!baseTicks) {
        return true;
    }

    if (syncedTicks.empty()) {
        return true;
    }

    char* end = nullptr;
    long long synced = std::strtoll(syncedTicks.c_str(), &end, 10);
    if (end == syncedTicks.c_str() || *end != '\0') {
        return true;
    }

    if (*baseTicks > synced) {
        return true;
    }

    spdlog::info("Base folder file '{}' unchanged since last sync (base mtime epoch={}, synced checkpoint epoch={}) -- skipping {}.", path, *baseTicks, synced, mergeDescription);
    return false;
}

// Length of the valid UTF-8 sequence starting at s[i], or 0 if none does.
// Rejects overlongs, surrogates and > U+10FFFF, as MultiByteToWideChar does.
static size_t ValidUTF8SequenceLength(const std::string& s, size_t i) {
    const auto at = [&](size_t k) { return static_cast<unsigned char>(s[k]); };
    const unsigned char c = at(i);
    if (c < 0x80) return 1;
    size_t len;
    unsigned char lo = 0x80, hi = 0xBF;
    if (c >= 0xC2 && c <= 0xDF) {
        len = 2;
    } else if (c >= 0xE0 && c <= 0xEF) {
        len = 3;
        if (c == 0xE0) lo = 0xA0;
        if (c == 0xED) hi = 0x9F;
    } else if (c >= 0xF0 && c <= 0xF4) {
        len = 4;
        if (c == 0xF0) lo = 0x90;
        if (c == 0xF4) hi = 0x8F;
    } else {
        return 0;
    }
    if (i + len > s.size()) return 0;
    if (at(i + 1) < lo || at(i + 1) > hi) return 0;
    for (size_t k = 2; k < len; ++k) {
        if (at(i + k) < 0x80 || at(i + k) > 0xBF) return 0;
    }
    return len;
}

bool IsValidUTF8(const std::string& s) {
    for (size_t i = 0; i < s.size();) {
        size_t const len = ValidUTF8SequenceLength(s, i);
        if (len == 0) return false;
        i += len;
    }
    return true;
}

static void AppendUTF8(std::string& out, char32_t cp) {
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

// Bytes 0x80-0xFF. Undefined Windows-1252 slots map to the C1 control, as Windows does.
static const char16_t CP1252_HIGH[128] = {
    0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
    0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

static const char16_t CP437_HIGH[128] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7, 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9, 0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA, 0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, 0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, 0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4, 0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248, 0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

std::string ToValidUTF8(const std::string& s, LegacyEncoding enc) {
    if (IsValidUTF8(s)) return s;
    const char16_t* table = (enc == LegacyEncoding::CP437) ? CP437_HIGH : CP1252_HIGH;
    std::string out;
    out.reserve(s.size() + s.size() / 2);
    for (size_t i = 0; i < s.size();) {
        size_t const len = ValidUTF8SequenceLength(s, i);
        if (len > 0) {
            out.append(s, i, len);
            i += len;
        } else {
            AppendUTF8(out, table[static_cast<unsigned char>(s[i]) - 0x80]);
            ++i;
        }
    }
    return out;
}

// ---- FileUtils::FixFile and related functions ----

static std::list<std::string> _fixFileSearchDirs;
static std::string _fixFileShowDir;
static std::recursive_mutex _fixFileMutex;
static std::vector<std::string> _fixFileNonExistent;
static std::map<std::string, std::string> _fixFileMap;
static std::map<std::string, bool> _fileExistsMap;

// The resolved-path cache is keyed on the path as stored, and stored paths are
// relative wherever possible, so entries are only meaningful for the directories
// they were resolved against.
static void ClearFixFileCaches() {
    _fixFileMap.clear();
    _fixFileNonExistent.clear();
    _fileExistsMap.clear();
}

void SetFixFileShowDir(const std::string& showDir) {
    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
    if (_fixFileShowDir != showDir) {
        ClearFixFileCaches();
    }
    _fixFileShowDir = showDir;
}

void SetFixFileDirectories(const std::list<std::string>& dirs) {
    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
    if (_fixFileSearchDirs != dirs) {
        ClearFixFileCaches();
    }
    _fixFileSearchDirs = dirs;
}

void ClearNonExistentFiles() {
    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
    _fixFileNonExistent.clear();
    _fileExistsMap.clear();
}

std::string GetFilenameFromPath(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

bool IsAbsoluteOrRootedPath(const std::string& path) {
    if (path.empty()) return false;
    if (path[0] == '/' || path[0] == '\\') return true;
    // "H:\...", "H:/..." and drive-relative "H:..." are all anchored to a drive
    if (path.size() >= 2 && path[1] == ':' &&
        ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z'))) return true;
    return std::filesystem::path(path).is_absolute();
}

// Get directory components from a path, splitting on both / and backslash
static std::vector<std::string> GetPathComponents(const std::string& path) {
    std::vector<std::string> components;
    std::string current;
    for (char c : path) {
        if (c == '/' || c == '\\') {
            if (!current.empty()) {
                components.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    // Don't include the filename — only directory components
    return components;
}

// Check if a file exists in a directory with the given filename
static bool doesFileExist(const std::string& dir, const std::string& filename, std::string& resultPath) {
    if (filename.empty()) return false;
    std::filesystem::path p = std::filesystem::path(dir) / filename;
    std::string candidate = p.string();
    if (FileExists(candidate, false)) {
        spdlog::debug("File location fixed: {} -> {}", filename, candidate);
        resultPath = candidate;
        return true;
    }
    return false;
}

// The search-dir list is replaced wholesale by SetFixFileDirectories() while
// parallel model loads are resolving paths, so callers that walk it outside
// _fixFileMutex must walk a private copy - iterating the shared list is a UAF.
static std::list<std::string> SnapshotSearchDirs() {
    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
    return _fixFileSearchDirs;
}

static std::string SnapshotShowDir() {
    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
    return _fixFileShowDir;
}

// Search dir + append, then all search directories
static bool doesFileExistInDirs(const std::list<std::string>& searchDirs,
                                const std::string& baseDir, const std::string& append,
                                const std::string& filename, std::string& resultPath) {
    std::string searchDir = baseDir;
    if (!append.empty()) {
        searchDir = (std::filesystem::path(baseDir) / append).string();
    }
    if (doesFileExist(searchDir, filename, resultPath)) return true;
    for (const auto& fd : searchDirs) {
        std::string sd = fd;
        if (!append.empty()) {
            sd = (std::filesystem::path(fd) / append).string();
        }
        if (doesFileExist(sd, filename, resultPath)) return true;
    }
    return false;
}

// Searches the installation's bundled resources/meshobjects tree
static std::string FindBundledMeshObjectFile(const std::string& filename) {
    std::string resDir = GetResourcesDir();
    if (resDir.empty()) return {};
    std::error_code ec;
    std::string meshRoot = (std::filesystem::path(resDir) / "meshobjects").string();
    std::filesystem::recursive_directory_iterator dirIt(meshRoot, ec);
    for (; !ec && dirIt != std::filesystem::recursive_directory_iterator(); dirIt.increment(ec)) {
        if (dirIt->is_regular_file(ec) && dirIt->path().filename() == filename) {
            return dirIt->path().string();
        }
    }
    return {};
}

static std::string FixFileImpl(const std::string& showDir, const std::string& file);

std::string FixFile(const std::string& showDir, const std::string& file) {
    if (file.empty()) return file;
    if (!IsValidUTF8(file)) {
        return FixFile(showDir, ToValidUTF8(file));
    }
    // Every probe below builds a std::filesystem::path, which throws on Windows
    // for names the narrow conversion can't round-trip; an unresolvable name
    // must not take the app down.
    try {
        return FixFileImpl(showDir, file);
    } catch (const std::exception& e) {
        spdlog::warn("FixFile: could not resolve '{}': {}", file, e.what());
        return file;
    }
}

static std::string FixFileImpl(const std::string& showDir, const std::string& file) {
    // A bare relative path is show/media relative by construction, so it must be
    // resolved against those directories rather than the process CWD, which is
    // arbitrary and could bind to an unrelated same-named file.
    const bool rooted = IsAbsoluteOrRootedPath(file);
    if (rooted && FileExists(file, false)) return file;
    auto meshPos = file.find("/meshobjects/");
    if (meshPos != std::string::npos) {
        return GetResourcesDir() + file.substr(meshPos);
    }

    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
    if (showDir != _fixFileShowDir && !showDir.empty() && _fixFileShowDir.empty()) {
        _fixFileShowDir = showDir;
    }

    // Check cache
    auto it = _fixFileMap.find(file);
    if (it != _fixFileMap.end()) return it->second;

    if (std::find(_fixFileNonExistent.begin(), _fixFileNonExistent.end(), file) != _fixFileNonExistent.end()) {
        return file;
    }

    std::string sd = showDir.empty() ? _fixFileShowDir : showDir;
    const std::list<std::string> searchDirs = _fixFileSearchDirs;
    lock.unlock();

    spdlog::debug("File not found ... attempting to fix location ({}) : {}", sd, file);

    // Extract filename using both Unix and Windows separators
    std::string filename = GetFilenameFromPath(file);
    std::string resultPath;

    // Relative paths (saved for portability) resolve against the show dir and
    // media dirs before any filename-based searching
    if (!rooted && !sd.empty()) {
        std::string append;
        for (const auto& comp : GetPathComponents(file)) {
            if (!append.empty()) append += std::filesystem::path::preferred_separator;
            append += comp;
        }
        if (doesFileExistInDirs(searchDirs, sd, append, filename, resultPath)) {
            lock.lock();
            _fixFileMap[file] = resultPath;
            return resultPath;
        }
    }

    // Nothing under the show or media dirs matched, so fall back to the CWD the
    // early-out above skipped for relative paths
    if (!rooted && FileExists(file, false)) return file;

    // Search show dir and search dirs for the file directly
    if (doesFileExistInDirs(searchDirs, sd, "", filename, resultPath)) {
        lock.lock();
        _fixFileMap[file] = resultPath;
        return resultPath;
    }

    // Search subdirectories of the show dir
    {
        std::error_code ec;
        if (std::filesystem::is_directory(sd, ec)) {
            for (const auto& entry : std::filesystem::directory_iterator(sd, ec)) {
                if (entry.is_directory()) {
                    std::string folderName = entry.path().filename().string();
                    std::string folderLower = folderName;
                    std::transform(folderLower.begin(), folderLower.end(), folderLower.begin(), ::tolower);
                    if (folderLower != "backup") {
                        if (doesFileExist(entry.path().string(), filename, resultPath)) {
                            lock.lock();
                            _fixFileMap[file] = resultPath;
                            return resultPath;
                        }
                    }
                }
            }
        }
    }

    // Try to match directory structure from the file path
    std::string sdLower = std::filesystem::path(sd).filename().string();
    std::transform(sdLower.begin(), sdLower.end(), sdLower.begin(), ::tolower);

    auto components = GetPathComponents(file);

    // Forward search: find the show folder name in the path components and use everything after it
    std::string appendPath;
    bool appending = false;
    for (const auto& comp : components) {
        std::string compLower = comp;
        std::transform(compLower.begin(), compLower.end(), compLower.begin(), ::tolower);
        if (compLower == sdLower) {
            appending = true;
        } else if (appending) {
            if (!appendPath.empty()) appendPath += std::filesystem::path::preferred_separator;
            appendPath += comp;
        }
    }
    if (!appendPath.empty()) {
        if (doesFileExistInDirs(searchDirs, sd, appendPath, filename, resultPath)) {
            lock.lock();
            _fixFileMap[file] = resultPath;
            return resultPath;
        }
    }

    // Check if file contains the show folder name and try the relative portion
    std::string fileLower = file;
    std::transform(fileLower.begin(), fileLower.end(), fileLower.begin(), ::tolower);
    auto sdPos = fileLower.find(sdLower);
    if (sdPos != std::string::npos) {
        size_t offset = sdPos + sdLower.length();
        std::string relative = file.substr(offset);
        if (FileExists(relative, false)) {
            lock.lock();
            _fixFileMap[file] = relative;
            return relative;
        }
    }

    // Reverse search: traverse up from the end of the file's directory components
    for (int x = (int)components.size() - 1; x >= 0; x--) {
        std::string revPath;
        for (int y = x; y < (int)components.size(); y++) {
            if (!revPath.empty()) revPath += std::filesystem::path::preferred_separator;
            revPath += components[y];
        }
        if (doesFileExistInDirs(searchDirs, sd, revPath, filename, resultPath)) {
            lock.lock();
            _fixFileMap[file] = resultPath;
            return resultPath;
        }
    }

    // Last resort: try with the last directory component of the file path as a subdirectory
    if (showDir.empty() && !components.empty()) {
        std::string lastDir = sd + std::string(1, std::filesystem::path::preferred_separator) + components.back();
        return FileUtils::FixFile(lastDir, file);
    }

    {
        std::string ext = std::filesystem::path(filename).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".obj" || ext == ".mtl") {
            std::string resolved = FindBundledMeshObjectFile(filename);
            if (!resolved.empty()) {
                lock.lock();
                _fixFileMap[file] = resolved;
                return resolved;
            }
        }
    }

    spdlog::debug("   could not find a fixed file location for : {}", file);
    spdlog::debug("   We will not look for this file again until a new sequence is loaded.");
    lock.lock();
    _fixFileNonExistent.push_back(file);
    return file;
}

bool CachedFileExists(const std::string& file) {
    if (file.empty()) {
        return false;
    }
    {
        std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
        auto it = _fileExistsMap.find(file);
        if (it != _fileExistsMap.end()) {
            return it->second;
        }
    }
    // Probe outside the lock — on macOS this can be a slow FileProvider XPC.
    bool exists = ::FileExists(file, false);
    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
    _fileExistsMap.emplace(file, exists);
    return exists;
}

std::string MakeRelativeFile(const std::string& file) {
    if (file.empty()) return {};
    if (!std::filesystem::path(file).is_absolute()) return {};

    // Normalize separators to /
    std::string f = file;
    std::replace(f.begin(), f.end(), '\\', '/');

    auto stripPrefix = [&](std::string base) -> std::string {
        std::replace(base.begin(), base.end(), '\\', '/');
        std::string baseCmp = base;
        std::string fCmp = f;
#ifdef _WIN32
        std::transform(baseCmp.begin(), baseCmp.end(), baseCmp.begin(), ::tolower);
        std::transform(fCmp.begin(), fCmp.end(), fCmp.begin(), ::tolower);
#endif
        if (!baseCmp.empty() && baseCmp.back() != '/') baseCmp += '/';
        if (fCmp.substr(0, baseCmp.size()) == baseCmp)
            return f.substr(baseCmp.size());
        return {};
    };

    std::string rel = stripPrefix(SnapshotShowDir());
    if (!rel.empty()) return rel;

    for (const auto& dir : SnapshotSearchDirs()) {
        rel = stripPrefix(dir);
        if (!rel.empty()) return rel;
    }

    return {};
}

std::string MakeRelativeFileOrOriginal(const std::string& file) {
    std::string rel = MakeRelativeFile(file);
    return rel.empty() ? file : rel;
}

bool IsFileInShowDir(const std::string& showDir, const std::string& filename) {
    std::string sd = showDir.empty() ? SnapshotShowDir() : showDir;
    if (sd.empty()) return false;
    std::string fixedFile = FileUtils::FixFile(sd, filename);
    const std::list<std::string> searchDirs = SnapshotSearchDirs();

#ifdef _WIN32
    std::string fixedLower = fixedFile;
    std::transform(fixedLower.begin(), fixedLower.end(), fixedLower.begin(), ::tolower);
    std::string sdLower = sd;
    std::transform(sdLower.begin(), sdLower.end(), sdLower.begin(), ::tolower);
    if (fixedLower.substr(0, sdLower.size()) == sdLower) return true;
    for (auto d : searchDirs) {
        std::transform(d.begin(), d.end(), d.begin(), ::tolower);
        if (fixedLower.substr(0, d.size()) == d) return true;
    }
#else
    if (fixedFile.substr(0, sd.size()) == sd) return true;
    for (const auto& d : searchDirs) {
        if (fixedFile.substr(0, d.size()) == d) return true;
    }
#endif
    return false;
}

std::string FixEffectFileParameter(const std::string& paramname, const std::string& parametervalue, const std::string& showDir) {
    auto startparamname = parametervalue.find(paramname);
    if (startparamname == std::string::npos) return parametervalue;
    auto endparamname = parametervalue.find("=", startparamname);
    if (endparamname == std::string::npos) return parametervalue;
    auto startvalue = endparamname + 1;
    auto endvalue = parametervalue.find(",", startvalue);
    if (endvalue == std::string::npos) endvalue = parametervalue.size();
    std::string file = parametervalue.substr(startvalue, endvalue - startvalue);
    std::string newfile = FileUtils::FixFile(showDir, file);
    return parametervalue.substr(0, startvalue) + newfile + parametervalue.substr(endvalue);
}

bool DeleteDirectory(std::string directory) {
    spdlog::debug("  Processing directory: {}.", directory);
    std::error_code ec;
    if (!std::filesystem::exists(directory, ec)) {
        spdlog::error("  Thats odd ... the directory cannot be found: {}.", directory);
        return false;
    }
    std::filesystem::remove_all(directory, ec);
    if (ec) {
        spdlog::error("  Could not delete folder {}: {}.", directory, ec.message());
        return false;
    }
    return true;
}

static std::string _resourcesDir;
std::string GetResourcesDir() {
    return _resourcesDir;
}
void SetResourcesDir(const std::string& dir) {
    _resourcesDir = dir;
}

std::string GetEffectMetadataDirectory() {
    static std::string cachedDir;
    if (!cachedDir.empty()) return cachedDir;

    std::string resDir = GetResourcesDir();
    if (resDir.empty()) return "";

    std::error_code ec;
    auto tryDir = [&](const std::string& dir) -> bool {
        if (std::filesystem::is_directory(std::filesystem::path(dir), ec)) {
            cachedDir = dir;
            return true;
        }
        return false;
    };

    // The fallbacks let dev builds find resources/effectmetadata in the source
    // tree without a post-build copy step.
    if (tryDir(resDir + "/effectmetadata")) return cachedDir;
#ifdef _WIN32
    if (tryDir(resDir + "/../../../resources/effectmetadata")) return cachedDir;
#endif
#ifdef __linux__
    if (tryDir(resDir + "/../resources/effectmetadata")) return cachedDir;
#endif
    return "";
}

};
