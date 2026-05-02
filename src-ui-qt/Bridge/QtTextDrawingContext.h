#pragma once

#include "../../src-core/render/TextDrawingContext.h"

#include <QFont>
#include <QImage>

#include <memory>
#include <string>
#include <vector>

// Qt-based TextDrawingContext — uses QImage + QPainter for text rendering.
// Registered at app startup so the Text effect can render without wxWidgets.
class QtTextDrawingContext : public TextDrawingContext {
public:
    QtTextDrawingContext(int w, int h);
    ~QtTextDrawingContext() override = default;

    // Buffer lifecycle
    void   ResetSize(int w, int h) override;
    size_t GetWidth()  const override { return size_t(_image.width()); }
    size_t GetHeight() const override { return size_t(_image.height()); }
    void   Clear() override;
    const uint8_t* FlushAndGetImage(int* width, int* height) override;

    // Text operations
    void SetFont(const TextFontInfo& font, const xlColor& color) override;
    void DrawText(const std::string& msg, int x, int y, double rotation) override;
    void DrawText(const std::string& msg, int x, int y) override;
    void GetTextExtent(const std::string& msg, double* width, double* height) override;
    void GetTextExtents(const std::string& msg, std::vector<double>& extents) override;
    void SetOverlayMode(bool b = true) override { _overlayMode = b; }

    // Factory and font-parsing functions for TextDrawingContext::RegisterFactory
    static TextDrawingContext* Create(int w, int h, bool /*allowShared*/);
    static TextFontInfo ParseTextFont(const std::string& fontString);
    static TextFontInfo ParseShapeFont(const std::string& fontString);

private:
    QImage              _image;
    QFont               _font;
    QColor              _color  { Qt::white };
    bool                _overlayMode { false };
    std::vector<uint8_t> _rgba;        // converted RGBA for FlushAndGetImage

    static QFont ToQFont(const TextFontInfo& fi);
};
