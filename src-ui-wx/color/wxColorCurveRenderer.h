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

#include "render/ColorCurve.h"

namespace wxColorCurveRenderer {

wxBitmap GetColorCurveImage(const ColorCurve& cc, int x, int y, bool bars);
wxBitmap GetSolidColourImage(int x, int y, const wxColour& c);

} // namespace wxColorCurveRenderer
