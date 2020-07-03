/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xlLockButton.h"
#include <wx/settings.h>

xlSizedBitmapButton::xlSizedBitmapButton(wxWindow *parent, wxWindowID id, const wxBitmap &bitmap,
                           const wxPoint &pos, const wxSize &size,
                           long style, const wxValidator &validator,
                           const wxString &name)
: wxBitmapButton(parent, id, bitmap, pos, bitmap.IsOk() ? bitmap.GetScaledSize() : size, style, validator, name)
{
}

xlSizedBitmapButton::~xlSizedBitmapButton()
{
}


xlLockButton::xlLockButton(wxWindow *parent, wxWindowID id, const wxBitmap &bitmap,
                           const wxPoint &pos, const wxSize &size,
                           long style, const wxValidator &validator,
                           const wxString &name)
: xlSizedBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT));
    SetToolTip(_("Lock/Unlock. If Locked then a \"Create Random Effects\" will NOT change this value."));
}

xlLockButton::~xlLockButton()
{
}
