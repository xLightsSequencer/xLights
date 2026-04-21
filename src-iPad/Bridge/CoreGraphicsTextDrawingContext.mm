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

#include <cctype>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// Resolve a wx-supplied font face name to a CoreText family name that
// actually exists on the device. wx writes face names to the .xsq in
// whatever case it feels like ("arial", "Arial", "ARIAL"), and
// CTFontCreateWithName silently substitutes the system fallback when the
// lookup misses — so "arial" quietly becomes Helvetica and the text renders
// in the wrong typeface. Try the caller's string first, then a
// case-insensitive scan of the installed families. Cache the mapping so
// we only enumerate fonts once per process.
static std::string ResolveFontFamily(const std::string& input) {
    static std::mutex cacheMutex;
    static std::unordered_map<std::string, std::string> inputCache;
    static std::unordered_map<std::string, std::string> lowerToCanonical;
    static bool familiesLoaded = false;

    std::lock_guard<std::mutex> lock(cacheMutex);

    if (auto it = inputCache.find(input); it != inputCache.end()) {
        return it->second;
    }

    if (!familiesLoaded) {
        CFArrayRef names = CTFontManagerCopyAvailableFontFamilyNames();
        if (names) {
            CFIndex count = CFArrayGetCount(names);
            for (CFIndex i = 0; i < count; i++) {
                CFStringRef name = (CFStringRef)CFArrayGetValueAtIndex(names, i);
                char buf[256];
                if (CFStringGetCString(name, buf, sizeof(buf), kCFStringEncodingUTF8)) {
                    std::string canonical = buf;
                    std::string lower = canonical;
                    for (char& c : lower) c = (char)std::tolower((unsigned char)c);
                    lowerToCanonical.emplace(std::move(lower), std::move(canonical));
                }
            }
            CFRelease(names);
        }
        familiesLoaded = true;
    }

    std::string lower = input;
    for (char& c : lower) c = (char)std::tolower((unsigned char)c);

    std::string resolved;
    if (auto it = lowerToCanonical.find(lower); it != lowerToCanonical.end()) {
        resolved = it->second;
    } else {
        resolved = "Helvetica";
    }
    inputCache.emplace(input, resolved);
    return resolved;
}

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
        // CGBitmapContext uses bottom-left CG drawing coordinates, but the
        // backing memory is top-to-bottom (row 0 = top visual row). DrawText
        // already converts the caller's y=0-at-top input to CG coords via
        // `_height - y`, so the text lands in memory rows starting at row 0.
        // The interface contract is "row-major, top-to-bottom" — no flip.
        return _pixels.data();
    }

    void SetFont(const TextFontInfo& font, const xlColor& color) override {
        if (_currentFont) {
            CFRelease(_currentFont);
            _currentFont = nullptr;
        }

        std::string canonical = ResolveFontFamily(font.faceName);
        NSString* fontName = [NSString stringWithUTF8String:canonical.c_str()];
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

        // Match wxOSXCG's DoGetTextExtent: ascent + descent + leading from
        // CTLineGetTypographicBounds. TextEffect centers the block using this
        // height via y = (top+bottom+1 - heightText)/2, so any divergence
        // from the value wx returns shifts the whole block.
        CGFloat ascent = 0, descent = 0, leading = 0;
        double w = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
        if (width) *width = w;
        if (height) *height = ascent + descent + leading;

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
            // Text effects render into the pixel buffer that drives the LED
            // grid, so smoothing/AA just produces blurry half-lit pixels. Match
            // desktop behavior (antiAliased=false in wxTextDrawingContext).
            CGContextSetShouldAntialias(_ctx, false);
            CGContextSetShouldSmoothFonts(_ctx, false);
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

        // DrawText's (x, y) anchors the TOP of the glyph box (wxDC semantics
        // — TextEffect passes `y = rect.GetTop()` and advances by heightLine).
        // Pull ascent from the line (same source GetTextExtent reports
        // height from) so positioning and measurement can't disagree.
        CGFloat ascent = 0, descent = 0, leading = 0;
        CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
        CGContextSetTextPosition(_ctx, x, y - ascent);
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

// Parse a wx native-font-info user description. The format is loose — tokens
// can include style keywords, a numeric pixel/point size, a charset (e.g.
// "utf-8"), and the face name in any order. Examples observed in .xsq files:
//   "bold arial 26 utf-8"
//   "Arial 12"
//   "Courier New Bold Italic 10"
// Strategy: case-insensitively classify each whitespace-separated token as
// style / size / charset / face, then stitch the face tokens back together.
static TextFontInfo ParseFontString(const std::string& fontString) {
    TextFontInfo info;
    if (fontString.empty()) return info;

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

    auto lower = [](const std::string& s) {
        std::string out = s;
        for (char& c : out) c = (char)std::tolower((unsigned char)c);
        return out;
    };

    int size = 0;
    std::vector<std::string> faceParts;
    for (const std::string& part : parts) {
        std::string lp = lower(part);
        if (lp == "bold") { info.bold = true; continue; }
        if (lp == "italic" || lp == "oblique") { info.italic = true; continue; }
        if (lp == "light") { info.light = true; continue; }
        if (lp == "underlined") { info.underlined = true; continue; }
        if (lp == "strikethrough") { info.strikethrough = true; continue; }
        if (lp == "regular" || lp == "normal") { continue; }

        // Numeric token → size.
        char* end = nullptr;
        long val = std::strtol(part.c_str(), &end, 10);
        if (end != part.c_str() && *end == '\0' && val > 0) {
            size = (int)val;
            continue;
        }

        // Charset markers like "utf-8", "iso-8859-1", "windows-1252" — contain
        // a digit and a hyphen. wx writes the charset last; discard it so it
        // doesn't pollute the face name.
        bool hasDigit = false, hasHyphen = false;
        for (char c : part) {
            if (std::isdigit((unsigned char)c)) hasDigit = true;
            if (c == '-') hasHyphen = true;
        }
        if (hasDigit && hasHyphen) continue;

        faceParts.push_back(part);
    }

    info.pixelSize = size > 0 ? size : 12;

    std::string faceName;
    for (size_t i = 0; i < faceParts.size(); i++) {
        if (i > 0) faceName += " ";
        faceName += faceParts[i];
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
