/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xlSlider.h"

xlSlider::xlSlider(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos, const wxSize &size,
                   long style, const wxValidator &validator, const wxString &name)
: wxSlider(parent, id, value, minValue, maxValue, pos, size, style, validator, name)
{

}

xlSlider::~xlSlider()
{
}
