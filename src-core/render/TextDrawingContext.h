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

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

// Windows headers #define DrawText to DrawTextW/DrawTextA — undefine to avoid mangling our method names
#ifdef DrawText
#undef DrawText
#endif

class xlColor;

// Platform emoji font names
constexpr char WIN_NATIVE_EMOJI_FONT[] = "Segoe UI Emoji";
constexpr char OSX_NATIVE_EMOJI_FONT[] = "Apple Color Emoji";
constexpr char LINUX_NATIVE_EMOJI_FONT[] = "Noto Color Emoji";

#ifdef _WIN32
constexpr char NATIVE_EMOJI_FONT[] = "Segoe UI Emoji";
#elif defined(__APPLE__)
constexpr char NATIVE_EMOJI_FONT[] = "Apple Color Emoji";
#else
constexpr char NATIVE_EMOJI_FONT[] = "Noto Color Emoji";
#endif

// Platform-independent font descriptor
struct TextFontInfo {
    std::string faceName;
    int pixelSize = 12;
    bool bold = false;
    bool light = false;
    bool italic = false;
    bool slant = false;
    bool underlined = false;
    bool strikethrough = false;
    bool antiAliased = false;

    bool operator==(const TextFontInfo& o) const {
        return faceName == o.faceName
            && pixelSize == o.pixelSize
            && bold == o.bold
            && light == o.light
            && italic == o.italic
            && slant == o.slant
            && underlined == o.underlined
            && strikethrough == o.strikethrough
            && antiAliased == o.antiAliased;
    }
    bool operator!=(const TextFontInfo& o) const { return !(*this == o); }
};

// Abstract interface for text rendering.
// The wx-based implementation lives in ui/wxTextDrawingContext.
class TextDrawingContext {
public:
    virtual ~TextDrawingContext() = default;

    // Buffer lifecycle
    virtual void ResetSize(int w, int h) = 0;
    virtual size_t GetWidth() const = 0;
    virtual size_t GetHeight() const = 0;
    virtual void Clear() = 0;

    // Finalize rendering and return RGBA pixel data (4 bytes/pixel, row-major, top-to-bottom).
    // Pointer is valid until the next Clear() or ResetSize() call. Caller does NOT own it.
    virtual const uint8_t* FlushAndGetImage(int* width, int* height) = 0;

    // Text operations
    virtual void SetFont(const TextFontInfo& font, const xlColor& color) = 0;
    virtual void DrawText(const std::string& msg, int x, int y, double rotation) = 0;
    virtual void DrawText(const std::string& msg, int x, int y) = 0;
    virtual void GetTextExtent(const std::string& msg, double* width, double* height) = 0;
    virtual void GetTextExtents(const std::string& msg, std::vector<double>& extents) = 0;
    virtual void SetOverlayMode(bool b = true) = 0;

    // Context pool management
    static void Initialize();
    static void CleanUp();
    static TextDrawingContext* GetContext();
    static void ReleaseContext(TextDrawingContext* ctx);

    // Font descriptor parsing from platform font description strings
    static TextFontInfo GetTextFont(const std::string& fontString);
    static TextFontInfo GetShapeFont(const std::string& fontString);

    // Factory registration — called once at startup by the UI layer
    using FactoryFn = std::function<TextDrawingContext*(int, int, bool)>;
    using FontParseFn = std::function<TextFontInfo(const std::string&)>;
    static void RegisterFactory(FactoryFn factory,
                                FontParseFn textFontParser,
                                FontParseFn shapeFontParser);
};
