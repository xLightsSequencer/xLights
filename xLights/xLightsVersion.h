#ifndef XLIGHTSVERSION_H
#define XLIGHTSVERSION_H

// Update these before building a release
static const wxString xlights_qualifier       = ""; // " BETA,ALPHA,PROD";
static const wxString xlights_version_string  = "2019.60";
static const wxString xlights_build_date      = "October 20, 2019";

const wxString &GetBitness();
wxString GetDisplayVersionString();

#endif