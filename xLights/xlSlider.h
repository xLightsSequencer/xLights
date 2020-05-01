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

#include <wx/wx.h>

class xlSlider : public wxSlider
{
    public:

        xlSlider(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize,
                long style=wxSL_HORIZONTAL, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxSliderNameStr);
        virtual ~xlSlider();
};
