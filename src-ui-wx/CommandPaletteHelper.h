#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/bitmap.h>
#include <wx/colour.h>
#include <string>

#ifdef __WXMAC__
wxBitmap GetSFSymbolBitmap(const std::string& symbolName, int pointSize, const wxColour& color);
std::string GetSFSymbolForEffect(const std::string& effectName);
wxColour GetColorForEffect(const std::string& effectName);
#endif
