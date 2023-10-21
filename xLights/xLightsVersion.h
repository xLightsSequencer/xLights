#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

 // Update these before building a release

#if __has_include("xlights_build_version.h")
#include "xlights_build_version.h"
#else
static const wxString xlights_version_string  = "2023.17";
#endif
#ifdef DEBUG
static const wxString xlights_qualifier       = " DEBUG";
#else
static const wxString xlights_qualifier       = ""; // " BETA,ALPHA,PROD";
#endif
static const wxString xlights_build_date      = __DATE__;
	
const wxString &GetBitness();
wxString GetDisplayVersionString();

#define WXWIDGETS_VERSION "33"
