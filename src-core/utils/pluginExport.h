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

// __declspec(dllexport) is required on Windows so GetProcAddress can find
// exported plugin entry points; on other platforms default visibility must
// be forced since the build may default to hidden visibility.
#ifdef _WIN32
#  define XL_PLUGIN_EXPORT_ATTR __declspec(dllexport)
#else
#  define XL_PLUGIN_EXPORT_ATTR __attribute__((visibility("default")))
#endif
