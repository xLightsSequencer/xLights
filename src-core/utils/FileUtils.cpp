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
namespace FileUtils
{
// ---- FileUtils::FixFile and related functions ----

static std::list<std::string> _fixFileSearchDirs;
static std::string _fixFileShowDir;
static std::recursive_mutex _fixFileMutex;
static std::vector<std::string> _fixFileNonExistent;
static std::map<std::string, std::string> _fixFileMap;

void SetFixFileShowDir(const std::string& showDir) {
    _fixFileShowDir = showDir;
}

void SetFixFileDirectories(const std::list<std::string>& dirs) {
    _fixFileSearchDirs = dirs;
}

void ClearNonExistentFiles() {
    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);
    _fixFileNonExistent.clear();
}

// Get just the filename from a path, handling both / and backslash separators
static std::string GetFilenameFromPath(const std::string& path) {
    auto pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
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

// Search dir + append, then all search directories
static bool doesFileExistInDirs(const std::string& baseDir, const std::string& append,
                                const std::string& filename, std::string& resultPath) {
    std::string searchDir = baseDir;
    if (!append.empty()) {
        searchDir = (std::filesystem::path(baseDir) / append).string();
    }
    if (doesFileExist(searchDir, filename, resultPath)) return true;
    for (const auto& fd : _fixFileSearchDirs) {
        std::string sd = fd;
        if (!append.empty()) {
            sd = (std::filesystem::path(fd) / append).string();
        }
        if (doesFileExist(sd, filename, resultPath)) return true;
    }
    return false;
}

std::string FileUtils::FixFile(const std::string& showDir, const std::string& file) {
    if (showDir != _fixFileShowDir && !showDir.empty() && _fixFileShowDir.empty()) {
        _fixFileShowDir = showDir;
    }

    if (file.empty()) return file;

    if (FileExists(file, false)) return file;

    // Handle meshobjects special case
    auto meshPos = file.find("/meshobjects/");
    if (meshPos != std::string::npos) {
        return GetResourcesDir() + file.substr(meshPos);
    }

    std::unique_lock<std::recursive_mutex> lock(_fixFileMutex);

    // Check cache
    auto it = _fixFileMap.find(file);
    if (it != _fixFileMap.end()) return it->second;

    if (std::find(_fixFileNonExistent.begin(), _fixFileNonExistent.end(), file) != _fixFileNonExistent.end()) {
        return file;
    }

    std::string sd = showDir.empty() ? _fixFileShowDir : showDir;
    lock.unlock();

    spdlog::debug("File not found ... attempting to fix location ({}) : {}", sd, file);

    // Extract filename using both Unix and Windows separators
    std::string filename = GetFilenameFromPath(file);
    std::string resultPath;

    // Search show dir and search dirs for the file directly
    if (doesFileExistInDirs(sd, "", filename, resultPath)) {
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
        if (doesFileExistInDirs(sd, appendPath, filename, resultPath)) {
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
        if (doesFileExistInDirs(sd, revPath, filename, resultPath)) {
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

    spdlog::debug("   could not find a fixed file location for : {}", file);
    spdlog::debug("   We will not look for this file again until a new sequence is loaded.");
    lock.lock();
    _fixFileNonExistent.push_back(file);
    return file;
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

    std::string rel = stripPrefix(_fixFileShowDir);
    if (!rel.empty()) return rel;

    for (const auto& dir : _fixFileSearchDirs) {
        rel = stripPrefix(dir);
        if (!rel.empty()) return rel;
    }

    return {};
}

bool IsFileInShowDir(const std::string& showDir, const std::string& filename) {
    std::string sd = showDir.empty() ? _fixFileShowDir : showDir;
    if (sd.empty()) return false;
    std::string fixedFile = FileUtils::FixFile(sd, filename);

#ifdef _WIN32
    std::string fixedLower = fixedFile;
    std::transform(fixedLower.begin(), fixedLower.end(), fixedLower.begin(), ::tolower);
    std::string sdLower = sd;
    std::transform(sdLower.begin(), sdLower.end(), sdLower.begin(), ::tolower);
    if (fixedLower.substr(0, sdLower.size()) == sdLower) return true;
    for (auto d : _fixFileSearchDirs) {
        std::transform(d.begin(), d.end(), d.begin(), ::tolower);
        if (fixedLower.substr(0, d.size()) == d) return true;
    }
#else
    if (fixedFile.substr(0, sd.size()) == sd) return true;
    for (const auto& d : _fixFileSearchDirs) {
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

};