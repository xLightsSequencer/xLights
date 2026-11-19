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

// Object ids aren't saved in the .xsq, so a session snapshot sends them
// alongside it: CaptureUids walks the host's rows, layers and effects, and
// ApplyUids gives the same ids to the matching objects of a copy loaded from
// that .xsq. The table also holds every effect's exact state, because a
// load doesn't always give back what was saved (load-time settings
// migration, timing-mark settings, timing layer names, a submodel row's
// trailing empty layers), plus each row's flags; ApplyUids puts back
// whatever differs.

#include <string>

#include <nlohmann/json.hpp>

class SequenceElements;

namespace Collab {

nlohmann::json CaptureUids(const SequenceElements& seq);

// False (with `problem` set) when the loaded sequence doesn't have the same
// rows and layers as the one the table came from; ids may then be partly
// applied.
bool ApplyUids(SequenceElements& seq, const nlohmann::json& table, std::string* problem = nullptr);

} // namespace Collab
