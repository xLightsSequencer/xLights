#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Windows text rendering straight onto Direct2D + DirectWrite, with no toolkit
// in between. The wx path this replaces cannot avoid two structural costs on
// Windows, both of which show up as the Text effect being roughly an order of
// magnitude dearer here than on the CoreGraphics path:
//
//   * wx's D2D backend has no way to clear a surface. It accepts only
//     wxCOMPOSITION_DEST and wxCOMPOSITION_OVER (every other mode returns
//     false), never calls ID2D1RenderTarget::Clear, and ClearRectangle is an
//     empty stub in the common code. So a fresh transparent surface can only be
//     had by reallocating the bitmap.
//   * wx binds a DC render target and issues exactly one BeginDraw/EndDraw per
//     context lifetime, and a DC render target only publishes pixels into the
//     bound GDI bitmap on EndDraw. Reading the result therefore means destroying
//     the context - and creating one is expensive and serialised on a
//     process-global mutex.
//
// Owning the D2D objects removes both: Clear() is one native call, and the
// render target lives for the life of the context with a BeginDraw/EndDraw pair
// per flush. Rendering to a WIC bitmap also means the pixels can be read by
// locking it, instead of a GDI SelectObject dance plus a full-buffer copy and
// an RGB/alpha de-interleave on every render.
//
// Per-instance thread-safe: each context owns its own WIC bitmap, render target
// and brush. The D2D factory is created multi-threaded and the DirectWrite
// factory shared, so the process-wide objects are safe to use concurrently. The
// TextDrawingContext pool hands out one context per render thread.

#include "TextDrawingContext.h"

#include <memory>

// If windows.h has already been seen, DrawText is a macro for DrawTextW and
// would rename this class's DrawText overrides. Drop it before declaring them.
#ifdef DrawText
#undef DrawText
#endif

class D2DTextDrawingContext : public TextDrawingContext {
public:
    D2DTextDrawingContext(int w, int h, bool allowShared);
    ~D2DTextDrawingContext() override;

    void ResetSize(int w, int h) override;
    size_t GetWidth() const override;
    size_t GetHeight() const override;
    void Clear() override;
    const uint8_t* FlushAndGetImage(int* width, int* height) override;

    void SetFont(const TextFontInfo& font, const xlColor& color) override;
    void DrawText(const std::string& msg, int x, int y, double rotation) override;
    void DrawText(const std::string& msg, int x, int y) override;
    void GetTextExtent(const std::string& msg, double* width, double* height) override;
    void GetTextExtents(const std::string& msg, std::vector<double>& extents) override;
    void SetOverlayMode(bool b = true) override;

    static TextDrawingContext* Create(int w, int h, bool allowShared);
    static TextFontInfo ParseTextFont(const std::string& fontString);
    static TextFontInfo ParseShapeFont(const std::string& fontString);

    // One-time process registration. Call once on Windows startup. Returns
    // false if Direct2D/DirectWrite/WIC could not be brought up, in which case
    // the caller should register a different backend.
    static bool Register();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
