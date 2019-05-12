#ifndef XLIGHTSVERSION_H
#define XLIGHTSVERSION_H

// Update these before building a release
static const wxString xlights_qualifier       = ""; // " BETA,ALPHA,PROD";
static const wxString xlights_version_string  = "2019.25";
static const wxString xlights_build_date      = "May 11, 2019";

const wxString &GetBitness();
wxString GetDisplayVersionString();

#endif