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

#include <wx/bitmap.h>
#include <wx/bmpbndl.h>

class BitmapCache {
public:
    static void SetupArtProvider();
    
    static wxBitmapBundle GetPapgayoIcon();
    static wxBitmapBundle GetPapgayoXIcon();
    static wxBitmapBundle GetModelGroupIcon();
    
    static wxBitmapBundle GetLockIcon(bool locked);
    static const wxImage &GetCornerIcon(int position, int size);
    
    static wxBitmapBundle GetFPPIcon();
};


class xlNamedBitmapBundleImpl : public wxBitmapBundleImpl {
public:
    // The vector must not be empty, caller is supposed to have checked for it.
    xlNamedBitmapBundleImpl(const std::string &n, int i, const wxVector<wxBitmap>& b);
    xlNamedBitmapBundleImpl(const std::string &n, const wxSize &sz, const wxVector<wxBitmap>& b);
    virtual ~xlNamedBitmapBundleImpl();

    virtual wxSize GetDefaultSize() const wxOVERRIDE;
    virtual wxSize GetPreferredBitmapSizeAtScale(double scale) const override;
    virtual wxBitmap GetBitmap(const wxSize& size) override;
        
private:
    std::string name;
    wxSize size;
    wxVector<wxBitmap> bitmaps;
    
    wxSize lastSize;
    wxBitmap lastBitmap;
};
