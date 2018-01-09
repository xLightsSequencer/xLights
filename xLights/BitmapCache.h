//
//  BitmapCache.h
//  xLights
//

#ifndef xLights_BitmapCache_h
#define xLights_BitmapCache_h

#include "wx/bitmap.h"

class BitmapCache {
public:

    static const wxBitmap &GetPapgayoIcon(wxString &toolTip, int size, bool exact);
    static const wxBitmap &GetPapgayoXIcon(wxString &toolTip, int size, bool exact);
    static const wxBitmap &GetModelGroupIcon(wxString &toolTip, int size, bool exact);
    static const wxBitmap &GetCornerIcon(int position, wxString &toolTip, int size, bool exact);
    static const wxBitmap &GetLockIcon(bool locked);
};


#endif
