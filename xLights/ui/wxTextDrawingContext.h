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

#include "../render/TextDrawingContext.h"

#include <memory>
#include <wx/graphics.h>
#include <wx/dcmemory.h>
#include <wx/image.h>

class wxTextDrawingContext : public TextDrawingContext {
public:
    wxTextDrawingContext(int bufferWi, int bufferHt, bool allowShared);
    ~wxTextDrawingContext() override;

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

    // Factory functions for TextDrawingContext::RegisterFactory
    static TextDrawingContext* Create(int w, int h, bool allowShared);
    static TextFontInfo ParseTextFont(const std::string& fontString);
    static TextFontInfo ParseShapeFont(const std::string& fontString);

private:
    std::unique_ptr<wxImage> image;
    std::unique_ptr<wxBitmap> bitmap;
    wxBitmap nullBitmap;
    std::unique_ptr<wxMemoryDC> dc;
    std::unique_ptr<wxGraphicsContext> gc;

    // Font caching state
    TextFontInfo cachedFontInfo;
    xlColor fontColor;
    wxGraphicsFont wxfont;

    // RGBA buffer for FlushAndGetImage
    std::vector<uint8_t> rgbaBuffer;
};
