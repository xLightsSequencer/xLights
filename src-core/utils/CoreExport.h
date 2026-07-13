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

// On Windows, a plugin DLL loaded via LoadLibrary does NOT automatically see
// symbols exported by the host .exe - unlike dlopen on Linux/macOS, which
// sees the executable's global symbol table by default. Any class a plugin
// subclasses or calls non-inline methods on must be explicitly exported from
// xLights.exe (producing an import .lib alongside it) for plugin projects to
// link against.
//
// XLIGHTS_EXE_BUILD is defined only by the main xLights.exe project
// (Xlights.vcxproj), never by plugin projects, so the exe exports these
// symbols and plugins import them against its generated Xlights.lib.
#ifdef _WIN32
#  ifdef XLIGHTS_EXE_BUILD
#    define XLCORE_API __declspec(dllexport)
#  else
#    define XLCORE_API __declspec(dllimport)
#  endif
#else
// Linux/macOS executables already expose their symbols to dlopen'd plugins
// (given -rdynamic / default two-level namespace behavior), so no annotation
// is needed there.
#  define XLCORE_API
#endif
