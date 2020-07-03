/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xlColorPicker.h"

BEGIN_EVENT_TABLE(xlColorPicker,wxPanel)
END_EVENT_TABLE()

xlColorPicker::xlColorPicker(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size, long style, const wxString& name)
: mCurrentColor(xlRED)
{
}

xlColorPicker::~xlColorPicker()
{
}
