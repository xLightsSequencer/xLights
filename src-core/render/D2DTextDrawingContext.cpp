/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "D2DTextDrawingContext.h"

// Parsing needs no Windows API, so it is defined unconditionally below; only the
// rendering half is Windows-only.
#ifdef _WIN32

#include "Color.h"

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

// windows.h (pulled in by the D2D headers) defines DrawText as DrawTextW, which
// would rename this class's own DrawText overrides out from under it.
#undef DrawText

#include <algorithm>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>

#include <log.h>

#ifdef _MSC_VER
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")
#endif

namespace {

// Process-wide factories. Direct2D is created multi-threaded so contexts on
// different render threads can draw concurrently; DirectWrite's shared factory
// is documented as thread-safe.
struct D2DGlobals {
    ID2D1Factory* d2d = nullptr;
    IDWriteFactory* dwrite = nullptr;
    IDWriteGdiInterop* gdiInterop = nullptr;
    IWICImagingFactory* wic = nullptr;
    bool ok = false;
};

D2DGlobals& Globals() {
    static D2DGlobals g;
    return g;
}

std::once_flag sInitOnce;

void InitGlobals() {
    D2DGlobals& g = Globals();

    // The render threads that use this never call CoInitialize themselves, and
    // WIC needs COM up. Multithreaded apartment; a repeat call on an already
    // initialised thread returns S_FALSE, which is fine.
    HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (hr != S_OK && hr != S_FALSE && hr != RPC_E_CHANGED_MODE) {
        spdlog::warn("D2DTextDrawingContext: CoInitializeEx failed (0x{:08x})", (unsigned)hr);
    }

    D2D1_FACTORY_OPTIONS opts = {};
    opts.debugLevel = D2D1_DEBUG_LEVEL_NONE;
    hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory),
                             &opts, reinterpret_cast<void**>(&g.d2d));
    if (FAILED(hr) || g.d2d == nullptr) {
        spdlog::error("D2DTextDrawingContext: D2D1CreateFactory failed (0x{:08x})", (unsigned)hr);
        return;
    }

    hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                               reinterpret_cast<IUnknown**>(&g.dwrite));
    if (FAILED(hr) || g.dwrite == nullptr) {
        spdlog::error("D2DTextDrawingContext: DWriteCreateFactory failed (0x{:08x})", (unsigned)hr);
        return;
    }

    // Used to resolve fonts the way the wx path did, through the GDI font
    // mapper; not fatal if it is unavailable, SetFont just falls back.
    if (FAILED(g.dwrite->GetGdiInterop(&g.gdiInterop))) {
        g.gdiInterop = nullptr;
        spdlog::warn("D2DTextDrawingContext: GetGdiInterop failed; font matching may differ from earlier releases");
    }

    hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                            IID_PPV_ARGS(&g.wic));
    if (FAILED(hr) || g.wic == nullptr) {
        spdlog::error("D2DTextDrawingContext: WIC factory creation failed (0x{:08x})", (unsigned)hr);
        return;
    }

    g.ok = true;
    spdlog::info("Direct2D text rendering enabled");
}

std::wstring Widen(const std::string& utf8) {
    if (utf8.empty()) {
        return std::wstring();
    }
    int need = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
    if (need <= 0) {
        return std::wstring();
    }
    std::wstring out((size_t)need, L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &out[0], need);
    return out;
}

template <class T>
void Release(T*& p) {
    if (p != nullptr) {
        p->Release();
        p = nullptr;
    }
}

} // namespace

struct D2DTextDrawingContext::Impl {
    IWICBitmap* bitmap = nullptr;
    ID2D1RenderTarget* rt = nullptr;
    ID2D1SolidColorBrush* brush = nullptr;
    IDWriteTextFormat* format = nullptr;

    int width = 0;
    int height = 0;
    bool drawing = false;      // a BeginDraw is outstanding
    bool overlay = false;

    TextFontInfo cachedFont;
    xlColor cachedColor{ 0, 0, 0, 0 };
    bool haveFont = false;

    std::vector<uint8_t> rgba;

    ~Impl() {
        EndDrawIfNeeded();
        Release(format);
        Release(brush);
        Release(rt);
        Release(bitmap);
    }

    void EndDrawIfNeeded() {
        if (drawing && rt != nullptr) {
            rt->EndDraw();
            drawing = false;
        }
    }

    void BeginDrawIfNeeded() {
        if (!drawing && rt != nullptr) {
            rt->BeginDraw();
            drawing = true;
        }
    }

    bool Build(int w, int h) {
        EndDrawIfNeeded();
        Release(format);
        Release(brush);
        Release(rt);
        Release(bitmap);
        haveFont = false;

        width = w > 0 ? w : 1;
        height = h > 0 ? h : 1;

        D2DGlobals& g = Globals();
        if (!g.ok) {
            return false;
        }

        // Premultiplied BGRA is the only 32bpp alpha format a D2D render target
        // will take; FlushAndGetImage un-premultiplies on the way out.
        HRESULT hr = g.wic->CreateBitmap((UINT)width, (UINT)height, GUID_WICPixelFormat32bppPBGRA,
                                         WICBitmapCacheOnLoad, &bitmap);
        if (FAILED(hr) || bitmap == nullptr) {
            spdlog::error("D2DTextDrawingContext: CreateBitmap {}x{} failed (0x{:08x})", width, height, (unsigned)hr);
            return false;
        }

        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
        hr = g.d2d->CreateWicBitmapRenderTarget(bitmap, &props, &rt);
        if (FAILED(hr) || rt == nullptr) {
            spdlog::error("D2DTextDrawingContext: CreateWicBitmapRenderTarget failed (0x{:08x})", (unsigned)hr);
            Release(bitmap);
            return false;
        }

        // The effects ask for un-antialiased text (TextFontInfo::antiAliased is
        // false and the wx path set wxANTIALIAS_NONE), and these buffers are
        // pixel grids where a blurred glyph edge just muddies a node.
        rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        rt->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

        hr = rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush);
        if (FAILED(hr) || brush == nullptr) {
            spdlog::error("D2DTextDrawingContext: CreateSolidColorBrush failed (0x{:08x})", (unsigned)hr);
            Release(rt);
            Release(bitmap);
            return false;
        }

        BeginDrawIfNeeded();
        return true;
    }

    IDWriteTextLayout* MakeLayout(const std::string& msg) const {
        if (format == nullptr) {
            return nullptr;
        }
        std::wstring w = Widen(msg);
        IDWriteTextLayout* layout = nullptr;
        // A generous layout box: the effects position text themselves and expect
        // no wrapping or clipping from the measurement.
        HRESULT hr = Globals().dwrite->CreateTextLayout(w.c_str(), (UINT32)w.size(), format,
                                                        (FLOAT)(width * 4 + 4096),
                                                        (FLOAT)(height * 4 + 4096), &layout);
        if (FAILED(hr) || layout == nullptr) {
            return nullptr;
        }
        layout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        return layout;
    }
};

D2DTextDrawingContext::D2DTextDrawingContext(int w, int h, bool /*allowShared*/)
    : impl(std::make_unique<Impl>()) {
    std::call_once(sInitOnce, InitGlobals);
    impl->Build(w, h);
}

D2DTextDrawingContext::~D2DTextDrawingContext() = default;

void D2DTextDrawingContext::ResetSize(int w, int h) {
    if (w == impl->width && h == impl->height) {
        return;
    }
    impl->Build(w, h);
}

size_t D2DTextDrawingContext::GetWidth() const {
    return (size_t)impl->width;
}

size_t D2DTextDrawingContext::GetHeight() const {
    return (size_t)impl->height;
}

void D2DTextDrawingContext::Clear() {
    if (impl->rt == nullptr) {
        return;
    }
    impl->BeginDrawIfNeeded();
    impl->rt->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
}

const uint8_t* D2DTextDrawingContext::FlushAndGetImage(int* width, int* height) {
    const int w = impl->width;
    const int h = impl->height;
    *width = w;
    *height = h;
    impl->rgba.assign((size_t)w * h * 4, 0);
    if (impl->rt == nullptr || impl->bitmap == nullptr) {
        return impl->rgba.data();
    }

    // A WIC bitmap render target publishes on EndDraw; reopen straight after so
    // the context stays usable without being rebuilt.
    if (impl->drawing) {
        HRESULT hr = impl->rt->EndDraw();
        impl->drawing = false;
        if (FAILED(hr)) {
            spdlog::error("D2DTextDrawingContext: EndDraw failed (0x{:08x})", (unsigned)hr);
            impl->BeginDrawIfNeeded();
            return impl->rgba.data();
        }
    }

    WICRect rect{ 0, 0, w, h };
    IWICBitmapLock* lock = nullptr;
    HRESULT hr = impl->bitmap->Lock(&rect, WICBitmapLockRead, &lock);
    if (SUCCEEDED(hr) && lock != nullptr) {
        UINT stride = 0;
        UINT bufSize = 0;
        BYTE* src = nullptr;
        if (SUCCEEDED(lock->GetStride(&stride)) && SUCCEEDED(lock->GetDataPointer(&bufSize, &src)) && src != nullptr) {
            uint8_t* dst = impl->rgba.data();
            for (int y = 0; y < h; y++) {
                const BYTE* srow = src + (size_t)y * stride;
                uint8_t* drow = dst + (size_t)y * w * 4;
                for (int x = 0; x < w; x++) {
                    // Source is premultiplied BGRA; the effects consume straight
                    // RGBA, so undo the premultiply.
                    const uint8_t b = srow[x * 4 + 0];
                    const uint8_t g = srow[x * 4 + 1];
                    const uint8_t r = srow[x * 4 + 2];
                    const uint8_t a = srow[x * 4 + 3];
                    if (a == 0) {
                        drow[x * 4 + 0] = 0;
                        drow[x * 4 + 1] = 0;
                        drow[x * 4 + 2] = 0;
                        drow[x * 4 + 3] = 0;
                    } else if (a == 255) {
                        drow[x * 4 + 0] = r;
                        drow[x * 4 + 1] = g;
                        drow[x * 4 + 2] = b;
                        drow[x * 4 + 3] = 255;
                    } else {
                        drow[x * 4 + 0] = (uint8_t)std::min(255, (r * 255 + a / 2) / a);
                        drow[x * 4 + 1] = (uint8_t)std::min(255, (g * 255 + a / 2) / a);
                        drow[x * 4 + 2] = (uint8_t)std::min(255, (b * 255 + a / 2) / a);
                        drow[x * 4 + 3] = a;
                    }
                }
            }
        }
        lock->Release();
    } else {
        spdlog::error("D2DTextDrawingContext: WIC Lock failed (0x{:08x})", (unsigned)hr);
    }

    impl->BeginDrawIfNeeded();
    return impl->rgba.data();
}

void D2DTextDrawingContext::SetFont(const TextFontInfo& fi, const xlColor& color) {
    if (impl->rt == nullptr) {
        return;
    }
    if (impl->haveFont && fi == impl->cachedFont && color == impl->cachedColor) {
        return;
    }

    Release(impl->format);

    LONG weight = 400; // FW_NORMAL
    if (fi.bold) {
        weight = 700; // FW_BOLD
    } else if (fi.light) {
        weight = 300; // FW_LIGHT
    }
    const bool italic = fi.italic || fi.slant;
    const FLOAT size = (FLOAT)(fi.pixelSize > 0 ? fi.pixelSize : 12);
    const std::wstring face = Widen(fi.faceName);

    D2DGlobals& g = Globals();

    // Resolve the font the way the wx path did: hand a LOGFONT to the GDI font
    // mapper via IDWriteGdiInterop, then build the format from the *resolved*
    // font's canonical family name, weight and style rather than the requested
    // ones. Going straight to CreateTextFormat instead picks a different
    // physical font in three cases that matter for existing sequences: a stored
    // name that is a subfamily rather than a family ("Arial Black", "Segoe UI
    // Semibold"), a name the system does not have (GDI substitutes a near match,
    // DirectWrite does not), and bold/italic on a family with no such real face
    // (GDI synthesises, DirectWrite picks the nearest instead).
    if (g.gdiInterop != nullptr && !face.empty()) {
        LOGFONTW lf = {};
        lf.lfHeight = -(LONG)std::lround(size); // negative: em height, matching wx
        lf.lfWeight = weight;
        lf.lfItalic = italic ? TRUE : FALSE;
        lf.lfUnderline = fi.underlined ? TRUE : FALSE;
        lf.lfStrikeOut = fi.strikethrough ? TRUE : FALSE;
        lf.lfCharSet = DEFAULT_CHARSET;
        lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
        lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        lf.lfQuality = fi.antiAliased ? DEFAULT_QUALITY : NONANTIALIASED_QUALITY;
        lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
        const size_t n = std::min<size_t>(face.size(), LF_FACESIZE - 1);
        std::copy_n(face.begin(), n, lf.lfFaceName);
        lf.lfFaceName[n] = L'\0';

        IDWriteFont* font = nullptr;
        if (SUCCEEDED(g.gdiInterop->CreateFontFromLOGFONT(&lf, &font)) && font != nullptr) {
            IDWriteFontFamily* family = nullptr;
            if (SUCCEEDED(font->GetFontFamily(&family)) && family != nullptr) {
                IDWriteLocalizedStrings* names = nullptr;
                if (SUCCEEDED(family->GetFamilyNames(&names)) && names != nullptr) {
                    UINT32 len = 0;
                    if (SUCCEEDED(names->GetStringLength(0, &len))) {
                        std::wstring familyName(len + 1, L'\0');
                        if (SUCCEEDED(names->GetString(0, &familyName[0], len + 1))) {
                            familyName.resize(len);
                            g.dwrite->CreateTextFormat(familyName.c_str(), nullptr,
                                                       font->GetWeight(), font->GetStyle(),
                                                       font->GetStretch(), size, L"",
                                                       &impl->format);
                        }
                    }
                    names->Release();
                }
                family->Release();
            }
            font->Release();
        }
    }

    if (impl->format == nullptr) {
        // No GDI interop, or a font it could not map (private/non-TrueType).
        DWRITE_FONT_WEIGHT dwWeight = (DWRITE_FONT_WEIGHT)weight;
        DWRITE_FONT_STYLE dwStyle = italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
        const std::wstring family = face.empty() ? L"Segoe UI" : face;
        HRESULT hr = g.dwrite->CreateTextFormat(family.c_str(), nullptr, dwWeight, dwStyle,
                                                DWRITE_FONT_STRETCH_NORMAL, size, L"", &impl->format);
        if (FAILED(hr) || impl->format == nullptr) {
            hr = g.dwrite->CreateTextFormat(L"Segoe UI", nullptr, dwWeight, dwStyle,
                                            DWRITE_FONT_STRETCH_NORMAL, size, L"", &impl->format);
            if (FAILED(hr) || impl->format == nullptr) {
                spdlog::error("D2DTextDrawingContext: CreateTextFormat failed for '{}' (0x{:08x})", fi.faceName, (unsigned)hr);
                return;
            }
        }
    }

    impl->brush->SetColor(D2D1::ColorF(color.red / 255.0f, color.green / 255.0f,
                                       color.blue / 255.0f, color.alpha / 255.0f));

    impl->cachedFont = fi;
    impl->cachedColor = color;
    impl->haveFont = true;
}

void D2DTextDrawingContext::DrawText(const std::string& msg, int x, int y) {
    DrawText(msg, x, y, 0.0);
}

void D2DTextDrawingContext::DrawText(const std::string& msg, int x, int y, double rotation) {
    if (impl->rt == nullptr || impl->format == nullptr || msg.empty()) {
        return;
    }
    impl->BeginDrawIfNeeded();

    IDWriteTextLayout* layout = impl->MakeLayout(msg);
    if (layout == nullptr) {
        return;
    }

    // The interface passes degrees and means counter-clockwise about (x,y);
    // in a y-down space that is a negative D2D rotation.
    const bool rotated = std::fabs(rotation) > 0.0001;
    if (rotated) {
        impl->rt->SetTransform(D2D1::Matrix3x2F::Rotation((FLOAT)-rotation, D2D1::Point2F((FLOAT)x, (FLOAT)y)));
    }
    impl->rt->DrawTextLayout(D2D1::Point2F((FLOAT)x, (FLOAT)y), layout, impl->brush,
                             D2D1_DRAW_TEXT_OPTIONS_NONE);
    if (rotated) {
        impl->rt->SetTransform(D2D1::Matrix3x2F::Identity());
    }
    layout->Release();
}

void D2DTextDrawingContext::GetTextExtent(const std::string& msg, double* width, double* height) {
    *width = 0;
    *height = 0;
    IDWriteTextLayout* layout = impl->MakeLayout(msg);
    if (layout == nullptr) {
        return;
    }
    DWRITE_TEXT_METRICS tm = {};
    if (SUCCEEDED(layout->GetMetrics(&tm))) {
        *width = tm.widthIncludingTrailingWhitespace;
        *height = tm.height;
    }
    layout->Release();
}

void D2DTextDrawingContext::GetTextExtents(const std::string& msg, std::vector<double>& extents) {
    extents.clear();
    const std::wstring w = Widen(msg);
    if (w.empty()) {
        return;
    }
    IDWriteTextLayout* layout = impl->MakeLayout(msg);
    if (layout == nullptr) {
        extents.assign(w.size(), 0.0);
        return;
    }

    // Cumulative advance after each UTF-16 unit, which is what the callers use
    // to place per-character effects. Cluster metrics give advances per cluster;
    // spread a multi-unit cluster's advance across its units so the vector stays
    // one entry per unit.
    UINT32 count = 0;
    layout->GetClusterMetrics(nullptr, 0, &count);
    std::vector<DWRITE_CLUSTER_METRICS> clusters(count);
    double running = 0;
    if (count > 0 && SUCCEEDED(layout->GetClusterMetrics(clusters.data(), count, &count))) {
        for (UINT32 c = 0; c < count; c++) {
            const UINT16 units = clusters[c].length > 0 ? clusters[c].length : 1;
            const double per = clusters[c].width / units;
            for (UINT16 u = 0; u < units; u++) {
                running += per;
                extents.push_back(running);
            }
        }
    }
    extents.resize(w.size(), running);
    layout->Release();
}

void D2DTextDrawingContext::SetOverlayMode(bool b) {
    // A D2D 1.0 render target always composites source-over, which is what the
    // overlay case wants. The non-overlay case wants a replace, but the surface
    // is cleared before each render so drawing over transparent is equivalent.
    impl->overlay = b;
}

TextDrawingContext* D2DTextDrawingContext::Create(int w, int h, bool allowShared) {
    return new D2DTextDrawingContext(w, h, allowShared);
}

bool D2DTextDrawingContext::Register() {
    std::call_once(sInitOnce, InitGlobals);
    if (!Globals().ok) {
        return false;
    }
    TextDrawingContext::RegisterFactory(&D2DTextDrawingContext::Create,
                                        &D2DTextDrawingContext::ParseTextFont,
                                        &D2DTextDrawingContext::ParseShapeFont);
    TextDrawingContext::Initialize();
    return true;
}

#else // !_WIN32

// Non-Windows builds pick this file up (macOS auto-discovers src-core/), so keep
// it compiling as an empty implementation.
struct D2DTextDrawingContext::Impl {};

D2DTextDrawingContext::D2DTextDrawingContext(int, int, bool) {}
D2DTextDrawingContext::~D2DTextDrawingContext() = default;
void D2DTextDrawingContext::ResetSize(int, int) {}
size_t D2DTextDrawingContext::GetWidth() const { return 0; }
size_t D2DTextDrawingContext::GetHeight() const { return 0; }
void D2DTextDrawingContext::Clear() {}
const uint8_t* D2DTextDrawingContext::FlushAndGetImage(int*, int*) { return nullptr; }
void D2DTextDrawingContext::SetFont(const TextFontInfo&, const xlColor&) {}
void D2DTextDrawingContext::DrawText(const std::string&, int, int, double) {}
void D2DTextDrawingContext::DrawText(const std::string&, int, int) {}
void D2DTextDrawingContext::GetTextExtent(const std::string&, double*, double*) {}
void D2DTextDrawingContext::GetTextExtents(const std::string&, std::vector<double>&) {}
void D2DTextDrawingContext::SetOverlayMode(bool) {}
TextDrawingContext* D2DTextDrawingContext::Create(int, int, bool) { return nullptr; }
bool D2DTextDrawingContext::Register() { return false; }

#endif // _WIN32

// Descriptor parsing needs no Windows API; both platforms share the toolkit-free
// implementation so a saved font string means the same thing everywhere.
TextFontInfo D2DTextDrawingContext::ParseTextFont(const std::string& fontString) {
    return TextDrawingContext::ParseFontDescriptor(fontString, "Segoe UI");
}

TextFontInfo D2DTextDrawingContext::ParseShapeFont(const std::string& fontString) {
    return TextDrawingContext::ParseShapeFontDescriptor(fontString, "Segoe UI");
}
