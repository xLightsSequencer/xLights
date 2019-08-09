#ifndef XLIGHTSVERSION_H
#define XLIGHTSVERSION_H

// Update these before building a release
static const wxString xlights_qualifier       = ""; // " BETA,ALPHA,PROD";
static const wxString xlights_version_string  = "2019.39";
static const wxString xlights_build_date      = "August 9, 2019";

const wxString &GetBitness();
wxString GetDisplayVersionString();

#endif