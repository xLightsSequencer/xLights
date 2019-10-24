#ifndef XLIGHTSVERSION_H
#define XLIGHTSVERSION_H


#if __has_include("xlights_build_version.h")
#include "xlights_build_version.h"
#else
static const wxString xlights_version_string  = "2019.60";
#endif

// Update these before building a release
static const wxString xlights_qualifier       = ""; // " BETA,ALPHA,PROD";
static const wxString xlights_build_date      = "October 20, 2019";

const wxString &GetBitness();
wxString GetDisplayVersionString();

#endif
