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

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <mutex>
#include <queue>
#include <vector>

// ---------------------------------------------------------------------------
// Generic thread-safe context pool
// ---------------------------------------------------------------------------
template <class CTX>
class ContextPool {
public:
    ContextPool(std::function<CTX*()> alloc) : allocator(alloc) {}
    ~ContextPool() {
        while (!contexts.empty()) {
            CTX* ret = contexts.front();
            delete ret;
            contexts.pop();
        }
    }

    CTX* GetContext() {
        std::unique_lock<std::mutex> locker(lock);
        if (contexts.empty()) {
            locker.unlock();
            return allocator();
        }
        CTX* ret = contexts.front();
        contexts.pop();
        return ret;
    }

    void ReleaseContext(CTX* pctx) {
        std::unique_lock<std::mutex> locker(lock);
        contexts.push(pctx);
    }

private:
    std::mutex lock;
    std::queue<CTX*> contexts;
    std::function<CTX*()> allocator;
};

// ---------------------------------------------------------------------------
// Static state
// ---------------------------------------------------------------------------
static ContextPool<TextDrawingContext>* TEXT_CONTEXT_POOL = nullptr;
static TextDrawingContext::FactoryFn sFactory;
static TextDrawingContext::FontParseFn sTextFontParser;
static TextDrawingContext::FontParseFn sShapeFontParser;

// ---------------------------------------------------------------------------
// Factory registration
// ---------------------------------------------------------------------------
void TextDrawingContext::RegisterFactory(FactoryFn factory,
                                         FontParseFn textFontParser,
                                         FontParseFn shapeFontParser) {
    sFactory = std::move(factory);
    sTextFontParser = std::move(textFontParser);
    sShapeFontParser = std::move(shapeFontParser);
}

// ---------------------------------------------------------------------------
// Context pool management
// ---------------------------------------------------------------------------
void TextDrawingContext::Initialize() {
    if (TEXT_CONTEXT_POOL == nullptr && sFactory) {
        TEXT_CONTEXT_POOL = new ContextPool<TextDrawingContext>([]() {
            return sFactory(10, 10, false);
        });
    }
}

void TextDrawingContext::CleanUp() {
    if (TEXT_CONTEXT_POOL != nullptr) {
        delete TEXT_CONTEXT_POOL;
        TEXT_CONTEXT_POOL = nullptr;
    }
}

TextDrawingContext* TextDrawingContext::GetContext() {
    if (TEXT_CONTEXT_POOL != nullptr) {
        return TEXT_CONTEXT_POOL->GetContext();
    }
    return nullptr;
}

void TextDrawingContext::ReleaseContext(TextDrawingContext* ctx) {
    if (TEXT_CONTEXT_POOL != nullptr) {
        TEXT_CONTEXT_POOL->ReleaseContext(ctx);
    }
}

// ---------------------------------------------------------------------------
// Font parsing dispatch
// ---------------------------------------------------------------------------
TextFontInfo TextDrawingContext::GetTextFont(const std::string& fontString) {
    if (sTextFontParser) {
        return sTextFontParser(fontString);
    }
    return TextFontInfo{};
}

TextFontInfo TextDrawingContext::GetShapeFont(const std::string& fontString) {
    if (sShapeFontParser) {
        return sShapeFontParser(fontString);
    }
    return TextFontInfo{};
}


// ---------------------------------------------------------------------------
// Toolkit-free font descriptor parsing
// ---------------------------------------------------------------------------
// The stored descriptor is a human-readable "face words + style keywords +
// size" string (what wxFont::SetNativeFontInfoUserDesc produces). Parsing it
// here rather than per backend keeps every non-wx backend agreeing on what a
// saved sequence's font meant.
TextFontInfo TextDrawingContext::ParseFontDescriptor(const std::string& fontString,
                                                     const std::string& defaultFaceName) {
    TextFontInfo info;
    if (fontString.empty()) {
        return info;
    }

    auto lower = [](const std::string& s) {
        std::string out = s;
        for (char& c : out) {
            c = (char)std::tolower((unsigned char)c);
        }
        return out;
    };

    // Multi-word families are quoted ("'gill sans' 36 macroman"), so a quoted
    // run is one token and is the family verbatim - splitting it on spaces
    // leaves the quote characters embedded in the face name and the font never
    // matches.
    std::vector<std::string> parts;
    std::string quotedFace;
    std::string current;
    char quote = '\0';
    for (char c : fontString) {
        if (quote != '\0') {
            if (c == quote) {
                quote = '\0';
                quotedFace = current;
                current.clear();
            } else {
                current += c;
            }
            continue;
        }
        if (c == '\'' || c == '"') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
            quote = c;
            continue;
        }
        if (c == ' ' || c == '\t') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    if (parts.empty() && quotedFace.empty()) {
        return info;
    }

    int size = 0;
    std::vector<std::string> faceParts;
    for (size_t idx = 0; idx < parts.size(); idx++) {
        const std::string& part = parts[idx];
        std::string lp = lower(part);

        // "semi bold"/"semi light" is wx's two-word rendering of the SEMIBOLD
        // (600) / SEMILIGHT weight - neither wxFONTWEIGHT_BOLD nor _LIGHT, so
        // it must consume both words without setting either flag. Otherwise
        // "semi" falls through unrecognised into the face name below while
        // "bold" is read on its own and wrongly renders as true Bold.
        if (lp == "semi" && idx + 1 < parts.size()) {
            std::string next = lower(parts[idx + 1]);
            if (next == "bold" || next == "light") {
                idx++;
                continue;
            }
        }

        if (lp == "bold" || lp == "heavy" || lp == "black" || lp == "extrabold" || lp == "ultrabold" || lp == "demibold") {
            info.bold = true;
            continue;
        }
        if (lp == "italic" || lp == "oblique") {
            info.italic = true;
            continue;
        }
        if (lp == "light" || lp == "thin" || lp == "extralight" || lp == "ultralight") {
            info.light = true;
            continue;
        }
        if (lp == "underlined") {
            info.underlined = true;
            continue;
        }
        if (lp == "strikethrough") {
            info.strikethrough = true;
            continue;
        }
        if (lp == "regular" || lp == "normal" || lp == "book" || lp == "medium") {
            continue;
        }
        if (lp == "condensed" || lp == "semicondensed" || lp == "expanded") {
            continue;
        }

        // Size: accept integer ("12"), decimal ("12.0"), and Pango-style
        // "12px" / "12pt". Round decimals; treat all units as pixels — text
        // effects render into pixel-grid buffers, not DPI-scaled surfaces.
        char* end = nullptr;
        double val = std::strtod(part.c_str(), &end);
        if (end != part.c_str() && val > 0) {
            std::string suffix(end);
            std::string suffixLower = lower(suffix);
            if (suffix.empty() || suffixLower == "px" || suffixLower == "pt" || suffixLower == "em") {
                size = (int)std::lround(val);
                continue;
            }
        }

        // Charset markers. Most carry a digit and a hyphen ("utf-8",
        // "iso-8859-1", "windows-1252"), but the Mac ones do not, so they need
        // naming or they end up glued onto the face name.
        if (lp == "macroman" || lp == "macintosh" || lp == "mac" || lp == "default") {
            continue;
        }
        bool hasDigit = false, hasHyphen = false;
        for (char c : part) {
            if (std::isdigit((unsigned char)c)) {
                hasDigit = true;
            }
            if (c == '-') {
                hasHyphen = true;
            }
        }
        if (hasDigit && hasHyphen) {
            continue;
        }

        faceParts.push_back(part);
    }

    // A descriptor with no numeric token is a font wx itself failed to parse
    // as either native-info or user-desc format (both silently fall back to
    // defaults rather than erroring) - the size the sequence was actually
    // saved with is not recoverable from the string. 9 matches wx's own
    // fallback (the classic Windows default UI font, Segoe UI 9pt) rather
    // than this size-grid renderer's otherwise-unrelated default of 12,
    // which is large enough to overflow a small LED matrix's buffer height
    // and produce badly clipped/overlapping glyphs.
    info.pixelSize = size > 0 ? size : 9;

    std::string faceName = quotedFace;
    if (faceName.empty()) {
        for (size_t i = 0; i < faceParts.size(); i++) {
            if (i > 0) {
                faceName += " ";
            }
            faceName += faceParts[i];
        }
    }
    info.faceName = faceName.empty() ? defaultFaceName : faceName;
    info.antiAliased = false;

    return info;
}

TextFontInfo TextDrawingContext::ParseShapeFontDescriptor(const std::string& fontString,
                                                          const std::string& defaultFaceName) {
    TextFontInfo info = ParseFontDescriptor(fontString, defaultFaceName);
    info.pixelSize = 12; // shape effect renders emoji at a reference size and scales
    if (info.faceName == WIN_NATIVE_EMOJI_FONT
        || info.faceName == OSX_NATIVE_EMOJI_FONT
        || info.faceName == LINUX_NATIVE_EMOJI_FONT) {
        info.faceName = NATIVE_EMOJI_FONT;
    }
    info.light = true;
    return info;
}
