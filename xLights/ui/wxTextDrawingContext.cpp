/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "wxTextDrawingContext.h"
#include "wxUtilities.h"

#include <wx/wx.h>
#include <wx/font.h>
#include <wx/graphics.h>

#include <cmath>
#include <map>
#include <mutex>
#include <string>

#include <log.h>

// ---------------------------------------------------------------------------
// Font map (thread-safe)
// ---------------------------------------------------------------------------
static std::mutex FONT_MAP_LOCK;
static std::map<std::string, TextFontInfo> FONT_MAP_TXT;
static std::map<std::string, TextFontInfo> FONT_MAP_SHP;

class FontMapLock {
    std::unique_lock<std::mutex> lk;
public:
    FontMapLock() : lk(FONT_MAP_LOCK) {}
};

// ---------------------------------------------------------------------------
// wx helper
// ---------------------------------------------------------------------------
inline double DegToRad(double deg) { return (deg * M_PI) / 180.0; }

inline void unshare(wxObject& o) {
    if (o.GetRefData() != nullptr) {
        o.UnShare();
    }
}
inline void unshare(const wxObject& o2) {
    wxObject* o = (wxObject*)&o2;
    if (o->GetRefData() != nullptr) {
        o->UnShare();
    }
}

// Convert TextFontInfo to wxFontInfo for internal wx use
static wxFontInfo ToWxFontInfo(const TextFontInfo& fi) {
    wxFontInfo wfi(wxSize(0, fi.pixelSize));
    if (!fi.faceName.empty()) {
        wfi.FaceName(fi.faceName);
    }
    if (fi.bold) wfi.Bold();
    if (fi.light) wfi.Light();
    if (fi.italic) wfi.Italic();
    if (fi.slant) wfi.Slant();
    if (fi.underlined) wfi.Underlined();
    if (fi.strikethrough) wfi.Strikethrough();
    wfi.AntiAliased(fi.antiAliased);
    return wfi;
}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------
TextDrawingContext* wxTextDrawingContext::Create(int w, int h, bool allowShared) {
    return new wxTextDrawingContext(w, h, allowShared);
}

// ---------------------------------------------------------------------------
// Font parsing
// ---------------------------------------------------------------------------
TextFontInfo wxTextDrawingContext::ParseTextFont(const std::string& FontString) {
    FontMapLock locker;
    auto it = FONT_MAP_TXT.find(FontString);
    if (it != FONT_MAP_TXT.end()) {
        return it->second;
    }

    TextFontInfo info;
    if (!FontString.empty()) {
        spdlog::debug("Loading font {}.", FontString.c_str());
        wxFont font(FontString);
        font.SetNativeFontInfoUserDesc(FontString);

        // Use GetPointSize() and treat it as pixel height — intentional.
        // "Arial 8" should be 8 pixels high, not DPI-scaled.
        info.pixelSize = font.GetPointSize();
        info.faceName = font.GetFaceName().ToStdString();
        info.bold = (font.GetWeight() == wxFONTWEIGHT_BOLD);
        info.light = (font.GetWeight() == wxFONTWEIGHT_LIGHT);
        info.italic = (font.GetStyle() == wxFONTSTYLE_ITALIC);
        info.slant = (font.GetStyle() == wxFONTSTYLE_SLANT);
        info.underlined = font.GetUnderlined();
        info.strikethrough = font.GetStrikethrough();
        info.antiAliased = false;
        spdlog::debug("    Added to font map.");
    } else {
        info.pixelSize = 12;
        info.antiAliased = false;
    }
    FONT_MAP_TXT[FontString] = info;
    return info;
}

TextFontInfo wxTextDrawingContext::ParseShapeFont(const std::string& fontString) {
    FontMapLock locker;
    auto it = FONT_MAP_SHP.find(fontString);
    if (it != FONT_MAP_SHP.end()) {
        return it->second;
    }

    wxFont ff(fontString);
    ff.SetNativeFontInfoUserDesc(fontString);

    TextFontInfo info;
    info.pixelSize = 12;
    std::string face = ff.GetFaceName().ToStdString();
    if (face == WIN_NATIVE_EMOJI_FONT || face == OSX_NATIVE_EMOJI_FONT || face == LINUX_NATIVE_EMOJI_FONT) {
        info.faceName = NATIVE_EMOJI_FONT;
    } else {
        info.faceName = face;
    }
    info.light = true;
    info.antiAliased = false;
    FONT_MAP_SHP[fontString] = info;
    return info;
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
wxTextDrawingContext::wxTextDrawingContext(int BufferWi, int BufferHt, bool allowShared)
    : nullBitmap(wxNullBitmap)
{
#ifdef LINUX
    allowShared = true;
#endif

    unshare(nullBitmap);
    image = std::make_unique<wxImage>(BufferWi > 0 ? BufferWi : 1, BufferHt > 0 ? BufferHt : 1);
    image->SetAlpha();
    for (int x = 0; x < BufferWi; x++) {
        for (int y = 0; y < BufferHt; y++) {
            image->SetAlpha(x, y, wxIMAGE_ALPHA_TRANSPARENT);
        }
    }
    bitmap = std::make_unique<wxBitmap>(*image);
    dc = std::make_unique<wxMemoryDC>(*bitmap);

    if (!allowShared) {
        FontMapLock lk;
        wxFont font(*wxITALIC_FONT);
        unshare(font);
        dc->SetFont(font);

        wxBrush brush(*wxYELLOW_BRUSH);
        unshare(brush);
        dc->SetBrush(brush);
        dc->SetBackground(brush);

        wxPen pen(*wxGREEN_PEN);
        unshare(pen);
        dc->SetPen(pen);

        unshare(dc->GetBrush());
        unshare(dc->GetBackground());
        unshare(dc->GetFont());
        unshare(dc->GetPen());
        unshare(dc->GetTextForeground());
        unshare(dc->GetTextBackground());
#ifndef LINUX
        wxColor c(12, 25, 3);
        unshare(c);
        dc->SetTextBackground(c);

        wxColor c2(0, 35, 5);
        unshare(c2);
        dc->SetTextForeground(c2);
#endif
    }

    dc->SelectObject(nullBitmap);
    bitmap.reset();
}

wxTextDrawingContext::~wxTextDrawingContext() {
    gc.reset();
    if (dc) dc->SelectObject(nullBitmap);
}

// ---------------------------------------------------------------------------
// Buffer lifecycle
// ---------------------------------------------------------------------------
void wxTextDrawingContext::ResetSize(int BufferWi, int BufferHt) {
    bitmap.reset();
    image = std::make_unique<wxImage>(BufferWi > 0 ? BufferWi : 1, BufferHt > 0 ? BufferHt : 1);
    image->SetAlpha();
    for (int x = 0; x < BufferWi; x++) {
        for (int y = 0; y < BufferHt; y++) {
            image->SetAlpha(x, y, wxIMAGE_ALPHA_TRANSPARENT);
        }
    }
}

size_t wxTextDrawingContext::GetWidth() const {
    if (gc != nullptr) {
        wxDouble w = 0, h = 0;
        gc->GetSize(&w, &h);
        return w;
    } else if (image != nullptr) {
        return image->GetWidth();
    }
    return 0;
}

size_t wxTextDrawingContext::GetHeight() const {
    if (gc != nullptr) {
        wxDouble w = 0, h = 0;
        gc->GetSize(&w, &h);
        return h;
    } else if (image != nullptr) {
        return image->GetHeight();
    }
    return 0;
}

void wxTextDrawingContext::Clear() {
    gc.reset();

    if (dc) {
        dc->SelectObject(nullBitmap);
        bitmap.reset();
        image->Clear();
        image->SetAlpha();
        memset(image->GetAlpha(), wxIMAGE_ALPHA_TRANSPARENT, image->GetWidth() * image->GetHeight());
        bitmap = std::make_unique<wxBitmap>(*image, 32);
        dc->SelectObject(*bitmap);
    }

#ifdef __WXMSW__
    static std::mutex singleThreadContextCreate;
    {
        std::unique_lock<std::mutex> locker(singleThreadContextCreate);
        gc.reset(wxGraphicsRenderer::GetDirect2DRenderer()->CreateContext(*dc));
        if (gc) {
            gc->SetPen(wxPen());
        }
    }
#else
    gc.reset(wxGraphicsContext::Create(*dc));
#endif

    if (gc == nullptr) {
        spdlog::error("wxTextDrawingContext DC creation failed.");
        return;
    }

    gc->SetAntialiasMode(wxANTIALIAS_NONE);
    gc->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_FAST);
    gc->SetCompositionMode(wxCompositionMode::wxCOMPOSITION_SOURCE);
}

const uint8_t* wxTextDrawingContext::FlushAndGetImage(int* width, int* height) {
    if (gc) {
        gc->Flush();
        gc.reset();
    }
    dc->SelectObject(nullBitmap);
    *image = bitmap->ConvertToImage();
    dc->SelectObject(*bitmap);

    int w = image->GetWidth();
    int h = image->GetHeight();
    *width = w;
    *height = h;

    // Convert separate RGB + alpha to interleaved RGBA
    unsigned char* rgb = image->GetData();
    bool hasAlpha = image->HasAlpha();
    unsigned char* alpha = hasAlpha ? image->GetAlpha() : nullptr;

    size_t pixelCount = (size_t)w * h;
    rgbaBuffer.resize(pixelCount * 4);

    for (size_t i = 0; i < pixelCount; i++) {
        rgbaBuffer[i * 4 + 0] = rgb[i * 3 + 0];
        rgbaBuffer[i * 4 + 1] = rgb[i * 3 + 1];
        rgbaBuffer[i * 4 + 2] = rgb[i * 3 + 2];
        rgbaBuffer[i * 4 + 3] = alpha ? alpha[i] : 255;
    }

    return rgbaBuffer.data();
}

void wxTextDrawingContext::SetOverlayMode(bool b) {
    gc->SetCompositionMode(b ? wxCompositionMode::wxCOMPOSITION_OVER : wxCompositionMode::wxCOMPOSITION_SOURCE);
}

// ---------------------------------------------------------------------------
// Font / Text
// ---------------------------------------------------------------------------
void wxTextDrawingContext::SetFont(const TextFontInfo& fi, const xlColor& color) {
    wxFontInfo wfi = ToWxFontInfo(fi);

    FontMapLock lk;
    if (gc != nullptr) {
        if (fi != cachedFontInfo || color != fontColor) {
            int style = wxFONTFLAG_NOT_ANTIALIASED;
            if (fi.bold) style |= wxFONTFLAG_BOLD;
            if (fi.light) style |= wxFONTFLAG_LIGHT;
            if (fi.italic) style |= wxFONTFLAG_ITALIC;
            if (fi.slant) style |= wxFONTFLAG_SLANT;
            if (fi.underlined) style |= wxFONTFLAG_UNDERLINED;
            if (fi.strikethrough) style |= wxFONTFLAG_STRIKETHROUGH;

            wxfont = gc->CreateFont(fi.pixelSize, fi.faceName, style, xlColorToWxColour(color));

#ifdef __WXMSW__
            if (wxfont.IsNull()) {
                wxfont = gc->CreateFont(fi.pixelSize, "segoe ui", style, xlColorToWxColour(color));
            }
#endif

            cachedFontInfo = fi;
            fontColor = color;
        }
        gc->SetFont(wxfont);
    } else {
        wxFont f(wfi);
#ifdef __WXMSW__
        {
            wxString s = f.GetNativeFontInfoDesc();
            s.Replace(";2;", ";3;", false);
            f.SetNativeFontInfo(s);
        }
#endif
        dc->SetFont(f);
        dc->SetTextForeground(xlColorToWxColour(color));
    }
}

void wxTextDrawingContext::DrawText(const std::string& msg, int x, int y, double rotation) {
    wxString wmsg(msg);
    if (gc != nullptr) {
        gc->DrawText(wmsg, x, y, DegToRad(rotation));
    } else {
        dc->DrawRotatedText(wmsg, x, y, rotation);
    }
}

void wxTextDrawingContext::DrawText(const std::string& msg, int x, int y) {
    wxString wmsg(msg);
    if (gc != nullptr) {
        gc->DrawText(wmsg, x, y);
    } else {
        dc->DrawText(wmsg, x, y);
    }
}

void wxTextDrawingContext::GetTextExtent(const std::string& msg, double* width, double* height) {
    wxString wmsg(msg);
    if (gc != nullptr) {
        gc->GetTextExtent(wmsg, width, height);
    } else {
        wxSize size = dc->GetTextExtent(wmsg);
        *width = size.GetWidth();
        *height = size.GetHeight();
    }
}

void wxTextDrawingContext::GetTextExtents(const std::string& msg, std::vector<double>& extents) {
    wxString wmsg(msg);
    if (gc != nullptr) {
        wxArrayDouble wxExtents;
        gc->GetPartialTextExtents(wmsg, wxExtents);
        extents.resize(wxExtents.size());
        for (size_t x = 0; x < wxExtents.size(); x++) {
            extents[x] = wxExtents[x];
        }
        return;
    }
    wxArrayInt sizes;
    dc->GetPartialTextExtents(wmsg, sizes);
    extents.resize(sizes.size());
    for (size_t x = 0; x < sizes.size(); x++) {
        extents[x] = sizes[x];
    }
}
