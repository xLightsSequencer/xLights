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

 // Update these before building a release

#if __has_include("xlights_build_version.h")
#include "xlights_build_version.h"
#else
static const std::string xlights_version_string  = "2026.08.1";
#endif
#ifdef DEBUG
static const std::string xlights_qualifier       = " DEBUG";
#else
static const std::string xlights_qualifier       = ""; // " BETA,ALPHA,PROD";
#endif
extern const std::string xlights_build_date;

inline std::string GetDisplayVersionString() { return xlights_version_string; }

#define WXWIDGETS_VERSION "33"
