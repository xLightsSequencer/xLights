#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 **************************************************************/


#if defined(__APPLE__)
#if __has_include("ExternalHooksMacOS.h")
#include "osxUtils/ExternalHooksMacOS.h"
#endif
#elif defined(_WIN32)
#if __has_include("ExternalHooksMSW.h")
#include "mswUtils/ExternalHooksMSW.h"
#endif
#else
#if __has_include("ExternalHooksLinux.h")
#include "linuxUtils/ExternalHooksLinux.h"
#endif
#endif

#ifndef __XL_EXTERNAL_HOOKS__

#include <filesystem>
#include <string>
#include <list>
#include <functional>

#define EnableSleepModes()
#define DisableSleepModes()
#define SetThreadQOS(a)

inline bool ObtainAccessToURL(const std::string &path, bool enforceWritable = false) { return true; }
inline bool IsFromAppStore() { return false; }
inline void RunInAutoReleasePool(std::function<void()> &&f) { f(); }

inline bool FileExists(const std::string &s, bool waitForDownload = true) {
    std::error_code ec;
    return std::filesystem::exists(s, ec);
}
inline void MarkNewFileRevision(const std::string &path, int retainMax = 15) {}
inline std::list<std::string> GetFileRevisions(const std::string &path) { return std::list<std::string>(); }
inline std::string GetURLForRevision(const std::string &path, const std::string &rev) { return path; }

#endif
