/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xLightsVersion.h"

static const std::string BITS_32 = "32bit";
static const std::string BITS_64 = "64bit";
static const std::string BITS_UNKNOWN = "";

const std::string &GetBitness()
{
    if (sizeof(size_t) == 4) {
        return BITS_32;
    } else if (sizeof(size_t) == 8) {
        return BITS_64;
    }
    return BITS_UNKNOWN;
}


std::string GetDisplayVersionString() {
#ifndef __WXOSX__
    return xlights_version_string + " " + GetBitness();
#else
    return xlights_version_string;
#endif
}
