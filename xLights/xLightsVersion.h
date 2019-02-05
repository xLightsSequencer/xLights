#ifndef XLIGHTSVERSION_H
#define XLIGHTSVERSION_H

// These have been moved here so they can be used in xSchedule as well.

static const wxString xlights_qualifier       = ""; // " BETA,ALPHA,PROD";
static const wxString xlights_version_string  = "2019.10";
static const wxString xlights_build_date      = "February 5, 2019";

const wxString &GetBitness();
wxString GetDisplayVersionString();

#endif
