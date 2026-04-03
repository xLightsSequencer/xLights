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

#include <functional>
#include <string>

// Core-safe message display functions.
// These always log via spdlog.  If a UI callback has been registered,
// they also show a dialog; otherwise the log is the only output.
//
// The UI layer calls RegisterDisplayCallback() once at startup to wire
// in wxMessageBox (or equivalent).  Core code never includes wx headers.

enum class DisplayMessageLevel {
    Error,
    Warning,
    Info
};

using DisplayMessageCallback = std::function<void(DisplayMessageLevel, const std::string&)>;

// Register the UI-layer callback.  Pass nullptr to unregister.
void RegisterDisplayCallback(DisplayMessageCallback cb);

// Core-safe replacements for the wx-dependent versions in ui/wxUtilities.h.
// These log via spdlog and, if a callback is registered, show a dialog.
void DisplayError(const std::string& err);
void DisplayWarning(const std::string& warn);
void DisplayInfo(const std::string& info);
