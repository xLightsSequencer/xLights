/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Compiles the vendored RtMidi with the platform's native MIDI API selected
// here, so no build system has to pass RtMidi's API defines.
#if defined(_WIN32)
#define __WINDOWS_MM__
#elif defined(__APPLE__)
#define __MACOSX_CORE__
#else
#define __LINUX_ALSA__
#endif

#include "../../dependencies/rtmidi/RtMidi.cpp"
