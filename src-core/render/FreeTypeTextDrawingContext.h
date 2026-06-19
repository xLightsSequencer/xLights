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

#include "TextDrawingContext.h"

#include <memory>

class FreeTypeTextDrawingContext : public TextDrawingContext {
public:
    FreeTypeTextDrawingContext(int w, int h, bool aa);
    ~FreeTypeTextDrawingContext() override;

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

    // One-time process registration. Call once on Linux startup.
    static void Register();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
