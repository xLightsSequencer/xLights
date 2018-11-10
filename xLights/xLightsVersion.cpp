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
