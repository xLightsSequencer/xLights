/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ZipUtils.h"

#include "../../dependencies/libxlsxwriter/third_party/minizip/unzip.h"

#include <spdlog/spdlog.h>

namespace {

// Read the archive's currently-open entry fully into a string. Assumes the
// caller has already positioned to the entry (GoToFirstFile / LocateFile).
std::optional<std::string> ReadCurrent(unzFile uf) {
    if (unzOpenCurrentFile(uf) != UNZ_OK) {
        return std::nullopt;
    }
    std::string out;
    char buf[16384];
    int read = 0;
    while ((read = unzReadCurrentFile(uf, buf, sizeof(buf))) > 0) {
        out.append(buf, (size_t)read);
    }
    unzCloseCurrentFile(uf);
    if (read < 0) {
        // negative = read error mid-stream; treat as failure.
        return std::nullopt;
    }
    return out;
}

} // namespace

namespace ZipUtils {

std::optional<std::string> ReadEntry(const std::string& zipPath, const std::string& entryName) {
    unzFile uf = unzOpen(zipPath.c_str());
    if (uf == nullptr) {
        spdlog::warn("ZipUtils: could not open archive '{}'", zipPath);
        return std::nullopt;
    }
    std::optional<std::string> result;
    // iCaseSensitivity 0 = OS default; matches how desktop located entries.
    if (unzLocateFile(uf, entryName.c_str(), 0) == UNZ_OK) {
        result = ReadCurrent(uf);
    }
    unzClose(uf);
    return result;
}

std::optional<std::string> ReadFirstEntry(const std::string& zipPath) {
    unzFile uf = unzOpen(zipPath.c_str());
    if (uf == nullptr) {
        spdlog::warn("ZipUtils: could not open archive '{}'", zipPath);
        return std::nullopt;
    }
    std::optional<std::string> result;
    if (unzGoToFirstFile(uf) == UNZ_OK) {
        result = ReadCurrent(uf);
    }
    unzClose(uf);
    return result;
}

std::vector<std::string> ListEntries(const std::string& zipPath) {
    std::vector<std::string> names;
    unzFile uf = unzOpen(zipPath.c_str());
    if (uf == nullptr) {
        return names;
    }
    int ret = unzGoToFirstFile(uf);
    while (ret == UNZ_OK) {
        char entryName[512];
        unz_file_info info;
        if (unzGetCurrentFileInfo(uf, &info, entryName, sizeof(entryName), nullptr, 0, nullptr, 0) == UNZ_OK) {
            names.emplace_back(entryName);
        }
        ret = unzGoToNextFile(uf);
    }
    unzClose(uf);
    return names;
}

} // namespace ZipUtils
