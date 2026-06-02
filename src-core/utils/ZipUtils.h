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

// ZipUtils — small wx-free minizip wrapper for reading entries out of a
// zip archive into memory. Consolidates the raw minizip usage that
// otherwise gets hand-rolled per call site (SequencePackage, FPP, and
// the LSP timing importer). Read-only for now; extend as needed.

#include <optional>
#include <string>
#include <vector>

namespace ZipUtils {

// Read a single archive entry into memory. `ReadEntry` matches `entryName`
// (default OS case sensitivity); `ReadFirstEntry` returns the first entry.
// Returns std::nullopt if the archive can't be opened or the entry isn't
// found / can't be read. Never throws.
std::optional<std::string> ReadEntry(const std::string& zipPath, const std::string& entryName);
std::optional<std::string> ReadFirstEntry(const std::string& zipPath);

// List every entry name in the archive (empty vector on open failure).
std::vector<std::string> ListEntries(const std::string& zipPath);

} // namespace ZipUtils
