/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PropertyGridHelpers.h"

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "Color.h"
#include "ui/shared/utils/wxUtilities.h"

bool wxDropPatternProperty::ValidateValue(wxVariant& value, wxPGValidationInfo& validationInfo) const {
    for (auto c : value.GetString()) {
        if ((c < '0' || c > '9') && c != ',' && c != '-')
            return false;
    }
    return true;
}

static const char* NODE_TYPE_VALUES[] = {
    "RGB Nodes", "RBG Nodes", "GBR Nodes", "GRB Nodes",
    "BRG Nodes", "BGR Nodes", "Node Single Color", "3 Channel RGB",
    "4 Channel RGBW", "4 Channel WRGB", "Strobes", "Single Color",
    "Single Color Intensity", "Superstring", "WRGB Nodes", "WRBG Nodes",
    "WGBR Nodes", "WGRB Nodes", "WBRG Nodes", "WBGR Nodes", "RGBW Nodes",
    "RBGW Nodes", "GBRW Nodes", "GRBW Nodes", "BRGW Nodes", "BGRW Nodes", "RGBWW Nodes"
};
wxArrayString NODE_TYPES(27, NODE_TYPE_VALUES);

static const char* RGBW_HANDLING_VALUES[] = { "R=G=B -> W", "RGB Only", "White Only", "Advanced", "White On All" };
wxArrayString RGBW_HANDLING(5, RGBW_HANDLING_VALUES);

static const char* PIXEL_STYLES_VALUES[] = { "Square", "Smooth", "Solid Circle", "Blended Circle" };
wxArrayString PIXEL_STYLES(4, PIXEL_STYLES_VALUES);

static const char* CONTROLLER_DIRECTION_VALUES[] = { "Forward", "Reverse" };
wxArrayString CONTROLLER_DIRECTION(2, CONTROLLER_DIRECTION_VALUES);

const std::string CLICK_TO_EDIT("--Click To Edit--");

wxString GetColorString(wxPGProperty* p, xlColor& xc) {
    wxString tp = "Single Color Custom";
    if (p != nullptr) {
        wxColour c;
        c << p->GetValue();
        if (c == *wxRED) {
            tp = "Single Color Red";
        } else if (c == *wxBLUE) {
            tp = "Single Color Blue";
        } else if (c == *wxGREEN) {
            tp = "Single Color Green";
        } else if (c == *wxWHITE) {
            tp = "Single Color White";
        } else {
            xc = wxColourToXlColor(c);
        }
    }
    return tp;
}
