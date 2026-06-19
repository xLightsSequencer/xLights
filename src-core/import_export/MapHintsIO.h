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

#include <string>
#include <vector>

// xmaphint files live at <showdir>/maphints/*.xmaphint and contain regex
// hints used by Auto Map to remember per-show prior mappings. Format:
//
//   <MapHints>
//     <Map ToRegex="^Star(s?)(\sGroup)?$" FromModel="Star" ApplyTo="B" />
//     ...
//   </MapHints>
//
// ApplyTo is "B" (both — default), "M" (models only), or "G" (groups only).

struct MapHintEntry {
    std::string toRegex;
    std::string fromModel;
    std::string applyTo;  // "B", "M", "G"
};

// Parse one .xmaphint file. Returns empty vector if the file is missing
// or malformed. Skips entries with empty ToRegex / FromModel.
std::vector<MapHintEntry> LoadMapHintsFile(const std::string& path);

// Scan <showDir>/maphints/*.xmaphint and aggregate every entry across
// every file, in directory-iteration order.
std::vector<MapHintEntry> LoadMapHintsFromShowDir(const std::string& showDir);

// Write entries to a .xmaphint file. Caller is responsible for building
// the entries (typically by walking the import mapping tree). Trailing
// "<!-- Samples -->" block matches the desktop format so users can edit
// the file by hand. Returns false on write failure.
bool WriteMapHintsFile(const std::string& path, const std::vector<MapHintEntry>& entries);
