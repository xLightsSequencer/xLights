#ifndef XLIGHTSVERSION_H
#define XLIGHTSVERSION_H

// Update these before building a release

#if __has_include("xlights_build_version.h")
#include "xlights_build_version.h"
#else
static const wxString xlights_version_string  = "2020.15";
#endif
static const wxString xlights_qualifier       = ""; // " BETA,ALPHA,PROD";
static const wxString xlights_build_date      = "April 10, 2020";
	
const wxString &GetBitness();
wxString GetDisplayVersionString();

#endif
