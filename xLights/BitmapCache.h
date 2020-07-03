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

#include "wx/bitmap.h"

class BitmapCache {
public:

    static const wxBitmap &GetPapgayoIcon(wxString &toolTip, int size, bool exact);
    static const wxBitmap &GetPapgayoXIcon(wxString &toolTip, int size, bool exact);
    static const wxBitmap &GetModelGroupIcon(wxString &toolTip, int size, bool exact);
    static const wxBitmap &GetCornerIcon(int position, wxString &toolTip, int size, bool exact);
    static const wxBitmap &GetLockIcon(bool locked);
};
