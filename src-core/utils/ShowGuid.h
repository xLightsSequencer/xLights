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

// The show id lives in <settings><ShowGUID> inside a show folder's
// xlights_rgbeffects.xml, so it identifies the show rather than the machine or
// the user, and one show opened from Mac, Windows and the iPad counts once.
//
// That placement means it is copied whenever the folder is. Setting up a base
// show directory by copying an existing show - the usual way people do it -
// therefore produces two different shows carrying one id, and every report from
// either lands in the same bucket. Only reading lives here: the two clients
// persist rgbeffects in opposite ways (desktop rebuilds it from in-memory
// settings on save, the iPad writes the file directly), so each regenerates
// through its own path.
namespace ShowGuid {

// <settings><ShowGUID> from `showDir`'s rgbeffects file. Empty when the folder
// has no id, or cannot be read at all - callers must treat those the same, as
// "nothing to compare against" rather than "matches".
std::string ReadFromShowFolder(std::string const& showDir);

} // namespace ShowGuid
