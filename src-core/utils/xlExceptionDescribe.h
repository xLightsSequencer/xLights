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

// Describes the exception currently in flight.  MUST be called from inside a
// catch handler - it rethrows to recover the type.
//
// Lives in core rather than the wx app base so the iPad, the headless render
// and worker threads all report an exception the same way the desktop does.
// Covers the std hierarchy with demangled type names, pulls the error code and
// both paths out of a filesystem_error, names Cocoa NSExceptions on Apple, and
// distinguishes a foreign unwind (no C++ throw site to go looking for) from an
// unknown C++ type.
std::string DescribeCurrentException();
