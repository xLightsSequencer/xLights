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

namespace pugi {
class xml_document;
}

// Strips the identifying values out of a show's XML while leaving it loadable.
//
// Opening a submitted crash report as a show folder in the debugger is the most
// useful thing the desktop reports carry, and the iPad's automatic upload cannot
// attach the show because nothing there shows the user what is leaving. A
// redacted copy is attachable: it keeps everything the loader needs - structure,
// model/group/submodel names, every cross-reference between them - and rewrites
// only what points at a person, a machine or a network.
//
// What is deliberately NOT touched:
//  - Relative paths. They are relative to the show folder, so they name nothing
//    outside it, and rewriting them would stop media resolving.
//  - "Model/Submodel" references, which look like paths and are not. Rewriting
//    one silently breaks the group that names it, which defeats the point.
//  - Model, group, submodel, view and controller names - the report is
//    unreadable without them, and they are the user's vocabulary, not identity.
namespace ShowRedactor {

// Stands in for the directory part of every absolute path. A fixed token rather
// than the id itself: the real id is already in <settings><ShowGUID>, and a
// constant is what lets a server recognise a value as derived.
inline constexpr char const* PATH_TOKEN = "${ShowGUID}";

struct Stats {
    int paths = 0;     // absolute paths rewritten
    int addresses = 0; // IPs / hostnames replaced
};

// Both operate in place on an already-loaded document and are safe to call on a
// document of the wrong kind (they simply find nothing to do).
Stats RedactRgbEffects(pugi::xml_document& doc);
Stats RedactNetworks(pugi::xml_document& doc);

// Load `srcPath`, redact by kind, write `destPath`. Returns false if the source
// could not be read or the destination could not be written.
bool RedactFileToFile(std::string const& srcPath, std::string const& destPath, bool isNetworksFile, Stats* stats = nullptr);

} // namespace ShowRedactor
