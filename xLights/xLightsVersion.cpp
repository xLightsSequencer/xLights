/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include "xLightsVersion.h"

static const wxString BITS_32 = "32bit";
static const wxString BITS_64 = "64bit";
static const wxString BITS_UNKNOWN = "";

const wxString &GetBitness()
{
    if (sizeof(size_t) == 4) {
        return BITS_32;
    } else if (sizeof(size_t) == 8) {
        return BITS_64;
    }
    return BITS_UNKNOWN;
}


wxString GetDisplayVersionString() {
#ifndef __WXOSX__
    return xlights_version_string + " " + GetBitness();
#else
    return xlights_version_string;
#endif
}
