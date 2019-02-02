#ifndef XLIGHTSVERSION_H
#define XLIGHTSVERSION_H

// These have been moved here so they can be used in xSchedule as well.

static const wxString xlights_qualifier       = ""; // " BETA,ALPHA,PROD";
static const wxString xlights_version_string  = "2019.9";
static const wxString xlights_build_date      = "February 2, 2019";

const wxString &GetBitness();
wxString GetDisplayVersionString();

#endif
