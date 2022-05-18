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
    static void SetupArtProvider();
    
    static const wxBitmapBundle &GetPapgayoIcon();
    static const wxBitmapBundle &GetPapgayoXIcon();
    static const wxBitmapBundle &GetModelGroupIcon();
    
    static const wxBitmapBundle &GetLockIcon(bool locked);
    static const wxImage &GetCornerIcon(int position, int size);
};
