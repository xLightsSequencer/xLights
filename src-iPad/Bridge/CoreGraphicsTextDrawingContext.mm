/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// CoreGraphics-based TextDrawingContext for iOS.
// Uses CoreText + CGBitmapContext to render text into RGBA pixel buffers.

#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>

#include "render/TextDrawingContext.h"
#include "utils/Color.h"

#include <cstring>
#include <vector>

class CoreGraphicsTextDrawingContext : public TextDrawingContext {
public:
    CoreGraphicsTextDrawingContext(int w, int h, bool /*aa*/)
        : _width(w), _height(h) {
        _pixels.resize(w * h * 4, 0);
        createContext();
    }

    ~CoreGraphicsTextDrawingContext() override {
        releaseContext();
        if (_currentFont) CFRelease(_currentFont);
    }

    void ResetSize(int w, int h) override {
        if (w == _width && h == _height) return;
        releaseContext();
        _width = w;
        _height = h;
        _pixels.resize(w * h * 4, 0);
        createContext();
    }

    size_t GetWidth() const override { return _width; }
    size_t GetHeight() const override { return _height; }

    void Clear() override {
        if (_ctx) {
            CGContextClearRect(_ctx, CGRectMake(0, 0, _width, _height));
        }
        std::memset(_pixels.data(), 0, _pixels.size());
    }

    const uint8_t* FlushAndGetImage(int* width, int* height) override {
        if (_ctx) CGContextFlush(_ctx);
        if (width) *width = _width;
        if (height) *height = _height;
        // CGBitmapContext renders bottom-up; flip vertically for xLights (top-down)
        int rowBytes = _width * 4;
        std::vector<uint8_t> tmp(rowBytes);
        for (int y = 0; y < _height / 2; y++) {
            int topOff = y * rowBytes;
            int botOff = (_height - 1 - y) * rowBytes;
            std::memcpy(tmp.data(), &_pixels[topOff], rowBytes);
            std::memcpy(&_pixels[topOff], &_pixels[botOff], rowBytes);
            std::memcpy(&_pixels[botOff], tmp.data(), rowBytes);
        }
        return _pixels.data();
    }

    void SetFont(const TextFontInfo& font, const xlColor& color) override {
        if (_currentFont) {
            CFRelease(_currentFont);
            _currentFont = nullptr;
        }

        NSString* fontName = [NSString stringWithUTF8String:font.faceName.c_str()];
        CGFloat size = font.pixelSize > 0 ? font.pixelSize : 12;

        CTFontSymbolicTraits traits = 0;
        if (font.bold) traits |= kCTFontBoldTrait;
        if (font.italic || font.slant) traits |= kCTFontItalicTrait;

        CTFontRef baseFont = CTFontCreateWithName((__bridge CFStringRef)fontName, size, nullptr);
        if (!baseFont) {
            baseFont = CTFontCreateWithName(CFSTR("Helvetica"), size, nullptr);
        }

        if (traits != 0) {
            CTFontRef traited = CTFontCreateCopyWithSymbolicTraits(baseFont, size, nullptr, traits, traits);
            if (traited) {
                CFRelease(baseFont);
                baseFont = traited;
            }
        }

        _currentFont = baseFont;
        _fontColor[0] = color.red / 255.0;
        _fontColor[1] = color.green / 255.0;
        _fontColor[2] = color.blue / 255.0;
        _fontColor[3] = 1.0;
    }

    void DrawText(const std::string& msg, int x, int y, double rotation) override {
        if (!_ctx || !_currentFont || msg.empty()) return;

        CGContextSaveGState(_ctx);
        CGContextTranslateCTM(_ctx, x, _height - y);
        if (rotation != 0) {
            CGContextRotateCTM(_ctx, -rotation * M_PI / 180.0);
        }

        drawString(msg, 0, 0);
        CGContextRestoreGState(_ctx);
    }

    void DrawText(const std::string& msg, int x, int y) override {
        DrawText(msg, x, y, 0);
    }

    void GetTextExtent(const std::string& msg, double* width, double* height) override {
        if (!_currentFont || msg.empty()) {
            if (width) *width = 0;
            if (height) *height = 0;
            return;
        }

        NSString* str = [NSString stringWithUTF8String:msg.c_str()];
        if (!str) str = @"";

        NSDictionary* attrs = @{
            (__bridge NSString*)kCTFontAttributeName: (__bridge id)_currentFont
        };
        NSAttributedString* attrStr = [[NSAttributedString alloc] initWithString:str attributes:attrs];
        CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attrStr);

        CGRect bounds = CTLineGetBoundsWithOptions(line, 0);
        if (width) *width = bounds.size.width;
        if (height) *height = bounds.size.height;

        CFRelease(line);
    }

    void GetTextExtents(const std::string& msg, std::vector<double>& extents) override {
        extents.clear();
        if (!_currentFont || msg.empty()) return;

        NSString* str = [NSString stringWithUTF8String:msg.c_str()];
        if (!str) return;

        NSDictionary* attrs = @{
            (__bridge NSString*)kCTFontAttributeName: (__bridge id)_currentFont
        };

        for (NSUInteger i = 0; i < str.length; i++) {
            NSString* sub = [str substringToIndex:i + 1];
            NSAttributedString* attrStr = [[NSAttributedString alloc] initWithString:sub attributes:attrs];
            CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attrStr);
            CGRect bounds = CTLineGetBoundsWithOptions(line, 0);
            extents.push_back(bounds.size.width);
            CFRelease(line);
        }
    }

    void SetOverlayMode(bool /*b*/) override {
        // Not needed for basic text rendering
    }

private:
    void createContext() {
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        _ctx = CGBitmapContextCreate(
            _pixels.data(), _width, _height, 8, _width * 4,
            colorSpace, kCGImageAlphaPremultipliedLast);
        CGColorSpaceRelease(colorSpace);

        if (_ctx) {
            CGContextSetShouldAntialias(_ctx, true);
            CGContextSetShouldSmoothFonts(_ctx, true);
        }
    }

    void releaseContext() {
        if (_ctx) {
            CGContextRelease(_ctx);
            _ctx = nullptr;
        }
    }

    void drawString(const std::string& msg, CGFloat x, CGFloat y) {
        NSString* str = [NSString stringWithUTF8String:msg.c_str()];
        if (!str) return;

        CGColorRef color = CGColorCreateSRGB(_fontColor[0], _fontColor[1], _fontColor[2], _fontColor[3]);
        NSDictionary* attrs = @{
            (__bridge NSString*)kCTFontAttributeName: (__bridge id)_currentFont,
            (__bridge NSString*)kCTForegroundColorAttributeName: (__bridge id)color
        };
        NSAttributedString* attrStr = [[NSAttributedString alloc] initWithString:str attributes:attrs];
        CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)attrStr);

        CGContextSetTextPosition(_ctx, x, y - CTFontGetDescent(_currentFont));
        CTLineDraw(line, _ctx);

        CFRelease(line);
        CGColorRelease(color);
    }

    int _width = 0;
    int _height = 0;
    CGContextRef _ctx = nullptr;
    CTFontRef _currentFont = nullptr;
    CGFloat _fontColor[4] = {1, 1, 1, 1};
    std::vector<uint8_t> _pixels;
};

// Parse wx-style font description: "FaceName [Bold] [Italic] [Light] Size"
// e.g. "Arial 12", "Arial Bold 14", "Courier New Bold Italic 10"
static TextFontInfo ParseFontString(const std::string& fontString) {
    TextFontInfo info;
    if (fontString.empty()) return info;

    // Split on spaces
    std::vector<std::string> parts;
    std::string current;
    for (char c : fontString) {
        if (c == ' ' || c == '\t') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) parts.push_back(current);
    if (parts.empty()) return info;

    // Last part should be the size (number)
    int size = 12;
    if (!parts.empty()) {
        char* end = nullptr;
        long val = std::strtol(parts.back().c_str(), &end, 10);
        if (end != parts.back().c_str() && *end == '\0' && val > 0) {
            size = (int)val;
            parts.pop_back();
        }
    }
    info.pixelSize = size;

    // Check for style keywords from the end
    while (!parts.empty()) {
        std::string& last = parts.back();
        if (last == "Bold" || last == "bold") {
            info.bold = true;
            parts.pop_back();
        } else if (last == "Italic" || last == "italic") {
            info.italic = true;
            parts.pop_back();
        } else if (last == "Light" || last == "light") {
            info.light = true;
            parts.pop_back();
        } else if (last == "Underlined" || last == "underlined") {
            info.underlined = true;
            parts.pop_back();
        } else if (last == "Strikethrough" || last == "strikethrough") {
            info.strikethrough = true;
            parts.pop_back();
        } else {
            break;
        }
    }

    // Remaining parts are the face name
    std::string faceName;
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) faceName += " ";
        faceName += parts[i];
    }
    info.faceName = faceName.empty() ? "Helvetica" : faceName;

    return info;
}

// Registration — called from XLiPadInit
void RegisterCoreGraphicsTextDrawingContext() {
    TextDrawingContext::RegisterFactory(
        // Factory
        [](int w, int h, bool aa) -> TextDrawingContext* {
            return new CoreGraphicsTextDrawingContext(w, h, aa);
        },
        // Text font parser
        ParseFontString,
        // Shape font parser (same format)
        ParseFontString
    );
    TextDrawingContext::Initialize();
}
