// Author:      Joachim Buermann (adapted for xLights)
// Copyright:   (c) 2010 Joachim Buermann

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "serial.h"

#include "./utils/spdlog_macros.h"

#ifdef __WXMSW__
# include "serial_win32.cpp"
#elif defined __WXOSX__
// # include "serial_osx.cpp"
#else
# include "serial_posix.cpp"
#endif
