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

#include <wx/colour.h>
#include "../Color.h"

inline wxColour xlColorToWxColour(const xlColor& c) {
    return wxColour(c.red, c.green, c.blue, c.alpha);
}

inline xlColor wxColourToXlColor(const wxColour& c) {
    return xlColor(c.Red(), c.Green(), c.Blue());
}
