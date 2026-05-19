/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Portable TextDrawingContext implementation using FreeType + HarfBuzz +
// Fontconfig. Used on Linux to replace wxTextDrawingContext, freeing
// TextEffect and ShapeEffect to render on the render-thread pool instead of
// being forced to the main thread by wxGTK/Pango's lack of off-thread safety.
//
// The class is per-instance thread-safe: each context owns its own
// FT_Library, FT_Face cache, and HarfBuzz font handles. Fontconfig is only
// touched at face-load time, behind a process-wide mutex, and results are
// cached. The TextDrawingContext pool already hands out one context per
// render thread, so render threads never share these instances.

#include "FreeTypeTextDrawingContext.h"

#ifdef LINUX

#include "Color.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

#include <hb.h>
#include <hb-ft.h>

#include <fontconfig/fontconfig.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <log.h>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

// ---------------------------------------------------------------------------
// Fontconfig: face name → font file path
// ---------------------------------------------------------------------------
struct FontFileQuery {
    std::string family;
    bool bold = false;
    bool italic = false;
    bool light = false;

    bool operator==(const FontFileQuery& o) const {
        return family == o.family && bold == o.bold && italic == o.italic && light == o.light;
    }
};

struct FontFileQueryHash {
    size_t operator()(const FontFileQuery& q) const noexcept {
        size_t h = std::hash<std::string>{}(q.family);
        h ^= (q.bold ? 0x9e3779b9 : 0);
        h ^= (q.italic ? 0x85ebca6b : 0);
        h ^= (q.light ? 0xc2b2ae35 : 0);
        return h;
    }
};

static std::mutex sFcMutex;
static std::unordered_map<FontFileQuery, std::string, FontFileQueryHash> sFcCache;
static bool sFcInitOk = false;
static bool sFcInitTried = false;

static void EnsureFcInit() {
    if (sFcInitTried) return;
    sFcInitTried = true;
    if (FcInit()) {
        sFcInitOk = true;
    } else {
        spdlog::error("FreeTypeTextDrawingContext: FcInit() failed; font lookup will fall back to defaults.");
    }
}

// Resolve a face description to a full filesystem path. Returns "" on failure.
static std::string ResolveFontPath(const FontFileQuery& q) {
    std::lock_guard<std::mutex> lock(sFcMutex);
    EnsureFcInit();

    auto it = sFcCache.find(q);
    if (it != sFcCache.end()) return it->second;

    std::string result;
    if (sFcInitOk) {
        FcPattern* pat = FcPatternCreate();
        if (pat) {
            if (!q.family.empty()) {
                FcPatternAddString(pat, FC_FAMILY, (const FcChar8*)q.family.c_str());
            }
            int weight = FC_WEIGHT_NORMAL;
            if (q.bold) weight = FC_WEIGHT_BOLD;
            else if (q.light) weight = FC_WEIGHT_LIGHT;
            FcPatternAddInteger(pat, FC_WEIGHT, weight);
            FcPatternAddInteger(pat, FC_SLANT, q.italic ? FC_SLANT_ITALIC : FC_SLANT_ROMAN);

            FcConfigSubstitute(nullptr, pat, FcMatchPattern);
            FcDefaultSubstitute(pat);

            FcResult fcRes = FcResultNoMatch;
            FcPattern* match = FcFontMatch(nullptr, pat, &fcRes);
            if (match) {
                FcChar8* file = nullptr;
                if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch && file) {
                    result = (const char*)file;
                }
                FcPatternDestroy(match);
            }
            FcPatternDestroy(pat);
        }
    }

    sFcCache.emplace(q, result);
    return result;
}

// ---------------------------------------------------------------------------
// UTF-8 decoding
// ---------------------------------------------------------------------------
struct CodePoint {
    uint32_t value = 0;
    uint32_t byteStart = 0; // index in source UTF-8 string
    uint32_t byteLen = 0;
};

static std::vector<CodePoint> DecodeUTF8(const std::string& s) {
    std::vector<CodePoint> out;
    out.reserve(s.size());
    const uint8_t* p = (const uint8_t*)s.data();
    size_t i = 0, n = s.size();
    while (i < n) {
        CodePoint cp;
        cp.byteStart = (uint32_t)i;
        uint8_t b = p[i];
        if (b < 0x80) {
            cp.value = b;
            cp.byteLen = 1;
        } else if ((b & 0xE0) == 0xC0 && i + 1 < n) {
            cp.value = ((b & 0x1F) << 6) | (p[i + 1] & 0x3F);
            cp.byteLen = 2;
        } else if ((b & 0xF0) == 0xE0 && i + 2 < n) {
            cp.value = ((b & 0x0F) << 12) | ((p[i + 1] & 0x3F) << 6) | (p[i + 2] & 0x3F);
            cp.byteLen = 3;
        } else if ((b & 0xF8) == 0xF0 && i + 3 < n) {
            cp.value = ((b & 0x07) << 18) | ((p[i + 1] & 0x3F) << 12) | ((p[i + 2] & 0x3F) << 6) | (p[i + 3] & 0x3F);
            cp.byteLen = 4;
        } else {
            cp.value = 0xFFFD;
            cp.byteLen = 1;
        }
        i += cp.byteLen;
        out.push_back(cp);
    }
    return out;
}

static bool IsEmojiCodepoint(uint32_t cp) {
    // Coarse classifier: anything obviously emoji-flavored should fall through
    // to the emoji font. Includes Fitzpatrick skin-tone modifiers, ZWJ, regional
    // indicators, and the main emoji blocks. Tightening this is fine — false
    // positives just route through the emoji font's cmap, and if the emoji
    // font doesn't have it, it falls through again to DejaVu.
    if (cp == 0x200D) return true;                              // ZWJ
    if (cp == 0xFE0F || cp == 0xFE0E) return true;              // VS16 / VS15
    if (cp >= 0x1F1E6 && cp <= 0x1F1FF) return true;            // Regional indicators
    if (cp >= 0x1F3FB && cp <= 0x1F3FF) return true;            // Skin tones
    if (cp >= 0x1F300 && cp <= 0x1FAFF) return true;            // Misc symbols & pictographs / emoji blocks
    if (cp >= 0x2600 && cp <= 0x27BF) return true;              // Misc symbols + dingbats
    if (cp == 0x2934 || cp == 0x2935) return true;
    if (cp >= 0x2B00 && cp <= 0x2BFF) return true;
    return false;
}

// ---------------------------------------------------------------------------
// Per-context state
// ---------------------------------------------------------------------------
struct LoadedFace {
    FT_Face face = nullptr;
    hb_font_t* hbFont = nullptr;
    int pixelSize = 0;
    bool hasColor = false;
};

} // anon namespace

struct FreeTypeTextDrawingContext::Impl {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels; // RGBA, top-to-bottom

    FT_Library lib = nullptr;

    // Faces opened in this context. Key: "<path>|<size>". The value's
    // FT_Face is at the requested pixel size and has FT_LOAD_COLOR applied.
    struct FaceCacheEntry {
        FT_Face face = nullptr;
        hb_font_t* hbFont = nullptr;
        bool hasColor = false;
    };
    std::unordered_map<std::string, FaceCacheEntry> faceCache;

    // Currently selected text font (from SetFont)
    TextFontInfo currentFont;
    xlColor currentColor{255, 255, 255};
    int currentPixelSize = 12; // resolved size used for rendering

    // Cache of fallback-resolved faces for SetFont's primary font
    LoadedFace primary;       // user-selected face
    LoadedFace emojiFallback; // Noto Color Emoji
    LoadedFace genericFallback; // DejaVu Sans

    Impl(int w, int h) {
        FT_Error err = FT_Init_FreeType(&lib);
        if (err) {
            spdlog::error("FreeTypeTextDrawingContext: FT_Init_FreeType failed (code {}).", (int)err);
            lib = nullptr;
        }
        Resize(w, h);
    }

    ~Impl() {
        for (auto& [k, e] : faceCache) {
            if (e.hbFont) hb_font_destroy(e.hbFont);
            if (e.face) FT_Done_Face(e.face);
        }
        if (lib) FT_Done_FreeType(lib);
    }

    void Resize(int w, int h) {
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        width = w;
        height = h;
        pixels.assign((size_t)w * h * 4, 0);
    }

    void Clear() {
        std::fill(pixels.begin(), pixels.end(), (uint8_t)0);
    }

    // Acquire a face at a specific pixel size. Returns null on failure.
    LoadedFace LoadFace(const std::string& family, bool bold, bool italic, bool light, int pixelSize) {
        if (!lib) return {};
        if (pixelSize < 1) pixelSize = 12;

        FontFileQuery q{family, bold, italic, light};
        std::string path = ResolveFontPath(q);
        if (path.empty()) {
            spdlog::debug("FreeTypeTextDrawingContext: fontconfig returned no match for '{}'.", family);
            return {};
        }

        std::string key = path + "|" + std::to_string(pixelSize);
        auto it = faceCache.find(key);
        if (it != faceCache.end()) {
            LoadedFace lf;
            lf.face = it->second.face;
            lf.hbFont = it->second.hbFont;
            lf.pixelSize = pixelSize;
            lf.hasColor = it->second.hasColor;
            return lf;
        }

        FT_Face face = nullptr;
        FT_Error err = FT_New_Face(lib, path.c_str(), 0, &face);
        if (err || !face) {
            spdlog::warn("FreeTypeTextDrawingContext: FT_New_Face('{}') failed (code {}).", path, (int)err);
            return {};
        }

        bool hasColor = (face->face_flags & FT_FACE_FLAG_COLOR) != 0;

        // Color bitmap fonts (CBDT) have fixed strike sizes — pick the closest.
        if (hasColor && face->num_fixed_sizes > 0) {
            int best = 0;
            int bestDelta = std::abs((int)face->available_sizes[0].y_ppem / 64 - pixelSize);
            for (int i = 1; i < face->num_fixed_sizes; i++) {
                int d = std::abs((int)face->available_sizes[i].y_ppem / 64 - pixelSize);
                if (d < bestDelta) { bestDelta = d; best = i; }
            }
            FT_Select_Size(face, best);
        } else {
            FT_Set_Pixel_Sizes(face, 0, pixelSize);
        }

        hb_font_t* hbFont = hb_ft_font_create_referenced(face);
        if (hbFont) {
            hb_ft_font_set_funcs(hbFont);
        }

        FaceCacheEntry e;
        e.face = face;
        e.hbFont = hbFont;
        e.hasColor = hasColor;
        faceCache[key] = e;

        LoadedFace lf;
        lf.face = face;
        lf.hbFont = hbFont;
        lf.pixelSize = pixelSize;
        lf.hasColor = hasColor;
        return lf;
    }

    void EnsureFallbacks(int pixelSize) {
        // Primary
        if (primary.face == nullptr || primary.pixelSize != pixelSize) {
            primary = LoadFace(currentFont.faceName, currentFont.bold, currentFont.italic || currentFont.slant, currentFont.light, pixelSize);
            if (primary.face == nullptr) {
                primary = LoadFace("DejaVu Sans", currentFont.bold, currentFont.italic || currentFont.slant, currentFont.light, pixelSize);
            }
        }
        // Emoji
        if (emojiFallback.face == nullptr || emojiFallback.pixelSize != pixelSize) {
            emojiFallback = LoadFace("Noto Color Emoji", false, false, false, pixelSize);
            if (emojiFallback.face == nullptr) {
                emojiFallback = LoadFace("Noto Emoji", false, false, false, pixelSize);
            }
        }
        // Generic last resort (different from primary if possible)
        if (genericFallback.face == nullptr || genericFallback.pixelSize != pixelSize) {
            genericFallback = LoadFace("DejaVu Sans", false, false, false, pixelSize);
            if (genericFallback.face == nullptr) {
                genericFallback = LoadFace("Liberation Sans", false, false, false, pixelSize);
            }
        }
    }

    // Pick the best face for a single codepoint among the available chain.
    // Returns nullptr if none can render it (caller should still try shaping
    // with primary so a notdef glyph is emitted).
    LoadedFace ChooseFaceFor(uint32_t cp) {
        // For known emoji codepoints, prefer the emoji font even if the
        // primary face has a fallback ASCII glyph (e.g. Noto Sans has a
        // monochrome tofu for many emoji that we'd rather see as color).
        if (IsEmojiCodepoint(cp)) {
            if (emojiFallback.face && FT_Get_Char_Index(emojiFallback.face, cp) != 0) {
                return emojiFallback;
            }
        }
        if (primary.face && FT_Get_Char_Index(primary.face, cp) != 0) {
            return primary;
        }
        if (emojiFallback.face && FT_Get_Char_Index(emojiFallback.face, cp) != 0) {
            return emojiFallback;
        }
        if (genericFallback.face && FT_Get_Char_Index(genericFallback.face, cp) != 0) {
            return genericFallback;
        }
        // Nothing has this codepoint — return primary so HarfBuzz emits notdef.
        return primary;
    }

    struct ShapedGlyph {
        FT_Face face = nullptr;
        bool faceHasColor = false;
        uint32_t glyphIndex = 0;
        // Position in 26.6 fixed-point pixels (HarfBuzz returns design units
        // that match FT pixel size since hb-ft sets the scale from FT_Face).
        int xOffset = 0;
        int yOffset = 0;
        int xAdvance = 0;
        int yAdvance = 0;
    };

    // Shape a contiguous run with one face.
    void ShapeRun(LoadedFace& face,
                  const std::vector<CodePoint>& cps,
                  size_t firstIdx, size_t lastIdxExclusive,
                  std::vector<ShapedGlyph>& outGlyphs) const {
        if (face.hbFont == nullptr || lastIdxExclusive <= firstIdx) return;

        hb_buffer_t* buf = hb_buffer_create();
        hb_buffer_set_content_type(buf, HB_BUFFER_CONTENT_TYPE_UNICODE);
        for (size_t i = firstIdx; i < lastIdxExclusive; i++) {
            hb_buffer_add(buf, cps[i].value, (unsigned)i);
        }
        hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
        hb_buffer_guess_segment_properties(buf);

        hb_shape(face.hbFont, buf, nullptr, 0);

        unsigned int n = 0;
        hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buf, &n);
        hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buf, &n);

        outGlyphs.reserve(outGlyphs.size() + n);
        for (unsigned i = 0; i < n; i++) {
            ShapedGlyph g;
            g.face = face.face;
            g.faceHasColor = face.hasColor;
            g.glyphIndex = infos[i].codepoint; // HarfBuzz's "codepoint" is the resolved glyph index
            g.xOffset = positions[i].x_offset;
            g.yOffset = positions[i].y_offset;
            g.xAdvance = positions[i].x_advance;
            g.yAdvance = positions[i].y_advance;
            // CBDT/CBLC color bitmap fonts (Noto Color Emoji on Linux) sometimes
            // come back from hb-ft with x_advance=0 because the advance metric
            // lives in the embedded bitmap strike and hb_font_funcs's default
            // path queries scalable outline metrics. Fall back to FT's own
            // advance from the glyph slot, which is correct for both bitmap
            // and outline glyphs.
            if (g.xAdvance == 0 && face.face) {
                FT_Error e = FT_Load_Glyph(face.face, g.glyphIndex, FT_LOAD_DEFAULT | FT_LOAD_COLOR);
                if (!e) {
                    g.xAdvance = (int)face.face->glyph->advance.x;
                    if (g.yAdvance == 0) g.yAdvance = (int)face.face->glyph->advance.y;
                }
            }
            outGlyphs.push_back(g);
        }
        hb_buffer_destroy(buf);
    }

    // Shape an entire string, partitioning into runs by which fallback face
    // owns each codepoint.
    std::vector<ShapedGlyph> ShapeString(const std::string& s) {
        std::vector<ShapedGlyph> out;
        if (s.empty()) return out;

        EnsureFallbacks(currentPixelSize);

        std::vector<CodePoint> cps = DecodeUTF8(s);
        if (cps.empty()) return out;

        size_t runStart = 0;
        LoadedFace runFace = ChooseFaceFor(cps[0].value);
        for (size_t i = 1; i <= cps.size(); i++) {
            LoadedFace nextFace = (i < cps.size()) ? ChooseFaceFor(cps[i].value) : LoadedFace{};
            // Treat ZWJ and variation selectors as continuation of current run.
            bool continueRun = (i < cps.size())
                && (cps[i].value == 0x200D || cps[i].value == 0xFE0F || cps[i].value == 0xFE0E
                    || (cps[i].value >= 0x1F3FB && cps[i].value <= 0x1F3FF));
            if (continueRun) continue;

            if (i == cps.size() || nextFace.face != runFace.face) {
                if (runFace.face) {
                    ShapeRun(runFace, cps, runStart, i, out);
                }
                runStart = i;
                if (i < cps.size()) runFace = nextFace;
            }
        }
        return out;
    }

    // Composite an alpha glyph (FT_PIXEL_MODE_GRAY) tinted by currentColor
    // into the RGBA buffer. (x,y) is the top-left of the bitmap in target
    // pixel coordinates.
    void BlitGray(int x, int y, FT_Bitmap& bmp) {
        if (bmp.buffer == nullptr) return;
        const uint8_t cr = currentColor.red;
        const uint8_t cg = currentColor.green;
        const uint8_t cb = currentColor.blue;

        for (unsigned int row = 0; row < bmp.rows; row++) {
            int dstY = y + (int)row;
            if (dstY < 0 || dstY >= height) continue;
            const uint8_t* src = bmp.buffer + row * bmp.pitch;
            uint8_t* dst = pixels.data() + (size_t)(dstY * width + x) * 4;
            for (unsigned int col = 0; col < bmp.width; col++) {
                int dstX = x + (int)col;
                if (dstX < 0 || dstX >= width) { dst += 4; continue; }
                uint8_t a = src[col];
                if (a == 0) { dst += 4; continue; }
                // Source-over composite: out = src + dst*(1-srcA)
                uint8_t srcR = (uint8_t)((cr * a + 127) / 255);
                uint8_t srcG = (uint8_t)((cg * a + 127) / 255);
                uint8_t srcB = (uint8_t)((cb * a + 127) / 255);
                uint8_t inv = 255 - a;
                dst[0] = (uint8_t)(srcR + (dst[0] * inv + 127) / 255);
                dst[1] = (uint8_t)(srcG + (dst[1] * inv + 127) / 255);
                dst[2] = (uint8_t)(srcB + (dst[2] * inv + 127) / 255);
                dst[3] = (uint8_t)(a + (dst[3] * inv + 127) / 255);
                dst += 4;
            }
        }
    }

    // Composite a color BGRA glyph (FT_PIXEL_MODE_BGRA, premultiplied) into
    // the RGBA buffer. Used for color emoji.
    void BlitBGRA(int x, int y, FT_Bitmap& bmp) {
        if (bmp.buffer == nullptr) return;
        for (unsigned int row = 0; row < bmp.rows; row++) {
            int dstY = y + (int)row;
            if (dstY < 0 || dstY >= height) continue;
            const uint8_t* src = bmp.buffer + row * bmp.pitch;
            uint8_t* dst = pixels.data() + (size_t)(dstY * width + x) * 4;
            for (unsigned int col = 0; col < bmp.width; col++) {
                int dstX = x + (int)col;
                if (dstX < 0 || dstX >= width) { dst += 4; src += 4; continue; }
                uint8_t b = src[0];
                uint8_t g = src[1];
                uint8_t r = src[2];
                uint8_t a = src[3];
                if (a == 0) { dst += 4; src += 4; continue; }
                uint8_t inv = 255 - a;
                // Source already premultiplied per FT_PIXEL_MODE_BGRA contract.
                dst[0] = (uint8_t)(r + (dst[0] * inv + 127) / 255);
                dst[1] = (uint8_t)(g + (dst[1] * inv + 127) / 255);
                dst[2] = (uint8_t)(b + (dst[2] * inv + 127) / 255);
                dst[3] = (uint8_t)(a + (dst[3] * inv + 127) / 255);
                dst += 4;
                src += 4;
            }
        }
    }

    // Draw at unrotated origin (penX, penY). penY anchors the TOP of the
    // typographic block (matching wxDC::DrawText semantics, which is what
    // TextEffect was written against).
    void DrawShapedAtOrigin(double penX, double penY, std::vector<ShapedGlyph>& glyphs) {
        if (glyphs.empty()) return;
        // Convert pen-top-of-line to pen-baseline by adding ascent of the
        // primary face. TextEffect computes line height via GetTextExtent
        // (ascent+descent+leading from us), so positioning must use the same
        // ascender we report.
        double baselineY = penY;
        if (primary.face) {
            FT_Face f = primary.face;
            double ascent;
            if (f->size && f->size->metrics.ascender != 0) {
                ascent = f->size->metrics.ascender / 64.0;
            } else {
                ascent = (f->ascender * (double)currentPixelSize) / (double)f->units_per_EM;
            }
            baselineY += ascent;
        }

        double cursorX = penX;
        double cursorY = baselineY;
        for (auto& g : glyphs) {
            if (!g.face) continue;

            // Render at 1bpp (FT_LOAD_TARGET_MONO) for outline fonts to match
            // wxTextDrawingContext's AA-off behavior — text effects render
            // into LED-grid buffers where AA produces blurry half-lit pixels.
            // Color bitmap glyphs ignore the target hint and come back as
            // their pre-rasterized BGRA — that's the right behavior for emoji.
            FT_Int32 loadFlags = FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_TARGET_MONO | FT_LOAD_MONOCHROME | FT_LOAD_NO_BITMAP;
            if (g.faceHasColor) {
                loadFlags = FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_COLOR;
            }
            FT_Error err = FT_Load_Glyph(g.face, g.glyphIndex, loadFlags);
            if (err) {
                cursorX += g.xAdvance / 64.0;
                cursorY += g.yAdvance / 64.0;
                continue;
            }
            FT_GlyphSlot slot = g.face->glyph;
            FT_Bitmap& bmp = slot->bitmap;

            int penPixX = (int)std::lround(cursorX + g.xOffset / 64.0 + slot->bitmap_left);
            int penPixY = (int)std::lround(cursorY + g.yOffset / 64.0 - slot->bitmap_top);

            if (bmp.pixel_mode == FT_PIXEL_MODE_BGRA) {
                BlitBGRA(penPixX, penPixY, bmp);
            } else if (bmp.pixel_mode == FT_PIXEL_MODE_GRAY) {
                BlitGray(penPixX, penPixY, bmp);
            } else if (bmp.pixel_mode == FT_PIXEL_MODE_MONO) {
                // Expand 1-bit into the gray path.
                std::vector<uint8_t> expanded((size_t)bmp.width * bmp.rows, 0);
                for (unsigned int row = 0; row < bmp.rows; row++) {
                    const uint8_t* src = bmp.buffer + row * bmp.pitch;
                    uint8_t* dst = expanded.data() + row * bmp.width;
                    for (unsigned int col = 0; col < bmp.width; col++) {
                        if (src[col >> 3] & (0x80 >> (col & 7))) dst[col] = 255;
                    }
                }
                FT_Bitmap fake = bmp;
                fake.pixel_mode = FT_PIXEL_MODE_GRAY;
                fake.buffer = expanded.data();
                fake.pitch = (int)bmp.width;
                BlitGray(penPixX, penPixY, fake);
            }

            cursorX += g.xAdvance / 64.0;
            cursorY += g.yAdvance / 64.0;
        }
    }

    // Rotation applied to glyph outlines via FT_Set_Transform, plus rotated
    // pen-positioning. Color bitmap glyphs ignore FT_Set_Transform (FT only
    // rotates outlines), so rotated emoji come out upright but at rotated
    // positions — acceptable since rotated text effects with emoji are an
    // edge case.
    void DrawShapedRotated(double penX, double penY, double degrees, std::vector<ShapedGlyph>& glyphs) {
        double rad = degrees * M_PI / 180.0;
        // Note: TextEffect's rotation argument is "degrees clockwise" in
        // wxGraphicsContext::DrawText terms (positive = clockwise visually
        // because y grows downward). Match that here.
        double cosA = std::cos(rad);
        double sinA = std::sin(rad);

        // Convert top-of-line origin to baseline along the rotated y axis.
        double ascent = 0.0;
        if (primary.face) {
            FT_Face f = primary.face;
            if (f->size && f->size->metrics.ascender != 0) {
                ascent = f->size->metrics.ascender / 64.0;
            } else {
                ascent = (f->ascender * (double)currentPixelSize) / (double)f->units_per_EM;
            }
        }
        double baselineX = penX + ascent * sinA;
        double baselineY = penY + ascent * cosA;

        FT_Matrix mat;
        // FT_Matrix uses 16.16 fixed point. Glyph y is up-positive in FT,
        // so to rotate clockwise on a y-down screen we use the same matrix
        // as standard rotation in math axes.
        mat.xx = (FT_Fixed)(cosA * 0x10000L);
        mat.xy = (FT_Fixed)(-sinA * 0x10000L);
        mat.yx = (FT_Fixed)(sinA * 0x10000L);
        mat.yy = (FT_Fixed)(cosA * 0x10000L);

        double cursorX = baselineX;
        double cursorY = baselineY;
        for (auto& g : glyphs) {
            if (!g.face) continue;

            FT_Set_Transform(g.face, &mat, nullptr);

            FT_Int32 loadFlags = FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_TARGET_MONO | FT_LOAD_MONOCHROME | FT_LOAD_NO_BITMAP;
            if (g.faceHasColor) {
                loadFlags = FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_COLOR;
            }
            FT_Error err = FT_Load_Glyph(g.face, g.glyphIndex, loadFlags);
            if (err) {
                FT_Set_Transform(g.face, nullptr, nullptr);
                cursorX += (g.xAdvance / 64.0) * cosA;
                cursorY += (g.xAdvance / 64.0) * sinA;
                continue;
            }
            FT_GlyphSlot slot = g.face->glyph;
            FT_Bitmap& bmp = slot->bitmap;

            int penPixX = (int)std::lround(cursorX + slot->bitmap_left);
            int penPixY = (int)std::lround(cursorY - slot->bitmap_top);

            if (bmp.pixel_mode == FT_PIXEL_MODE_BGRA) {
                BlitBGRA(penPixX, penPixY, bmp);
            } else if (bmp.pixel_mode == FT_PIXEL_MODE_GRAY) {
                BlitGray(penPixX, penPixY, bmp);
            } else if (bmp.pixel_mode == FT_PIXEL_MODE_MONO) {
                std::vector<uint8_t> expanded((size_t)bmp.width * bmp.rows, 0);
                for (unsigned int row = 0; row < bmp.rows; row++) {
                    const uint8_t* src = bmp.buffer + row * bmp.pitch;
                    uint8_t* dst = expanded.data() + row * bmp.width;
                    for (unsigned int col = 0; col < bmp.width; col++) {
                        if (src[col >> 3] & (0x80 >> (col & 7))) dst[col] = 255;
                    }
                }
                FT_Bitmap fake = bmp;
                fake.pixel_mode = FT_PIXEL_MODE_GRAY;
                fake.buffer = expanded.data();
                fake.pitch = (int)bmp.width;
                BlitGray(penPixX, penPixY, fake);
            }

            FT_Set_Transform(g.face, nullptr, nullptr);

            double advancePx = g.xAdvance / 64.0;
            cursorX += advancePx * cosA;
            cursorY += advancePx * sinA;
        }
    }
};

// ---------------------------------------------------------------------------
// Public class
// ---------------------------------------------------------------------------
FreeTypeTextDrawingContext::FreeTypeTextDrawingContext(int w, int h, bool /*aa*/)
    : impl(std::make_unique<Impl>(w, h)) {}

FreeTypeTextDrawingContext::~FreeTypeTextDrawingContext() = default;

void FreeTypeTextDrawingContext::ResetSize(int w, int h) {
    impl->Resize(w, h);
}

size_t FreeTypeTextDrawingContext::GetWidth() const { return impl->width; }
size_t FreeTypeTextDrawingContext::GetHeight() const { return impl->height; }

void FreeTypeTextDrawingContext::Clear() {
    impl->Clear();
}

const uint8_t* FreeTypeTextDrawingContext::FlushAndGetImage(int* width, int* height) {
    if (width) *width = impl->width;
    if (height) *height = impl->height;
    return impl->pixels.data();
}

void FreeTypeTextDrawingContext::SetFont(const TextFontInfo& font, const xlColor& color) {
    bool sizeChanged = font.pixelSize != impl->currentPixelSize;
    bool faceChanged = font.faceName != impl->currentFont.faceName
                       || font.bold != impl->currentFont.bold
                       || font.italic != impl->currentFont.italic
                       || font.slant != impl->currentFont.slant
                       || font.light != impl->currentFont.light;
    impl->currentFont = font;
    impl->currentColor = color;
    impl->currentPixelSize = font.pixelSize > 0 ? font.pixelSize : 12;
    if (sizeChanged || faceChanged) {
        impl->primary = LoadedFace{};
        impl->emojiFallback = LoadedFace{};
        impl->genericFallback = LoadedFace{};
    }
}

void FreeTypeTextDrawingContext::DrawText(const std::string& msg, int x, int y, double rotation) {
    auto glyphs = impl->ShapeString(msg);
    if (rotation == 0.0) {
        impl->DrawShapedAtOrigin(x, y, glyphs);
    } else {
        impl->DrawShapedRotated(x, y, rotation, glyphs);
    }
}

void FreeTypeTextDrawingContext::DrawText(const std::string& msg, int x, int y) {
    DrawText(msg, x, y, 0.0);
}

void FreeTypeTextDrawingContext::GetTextExtent(const std::string& msg, double* width, double* height) {
    impl->EnsureFallbacks(impl->currentPixelSize);
    auto glyphs = impl->ShapeString(msg);
    double w = 0;
    for (auto& g : glyphs) w += g.xAdvance / 64.0;

    double h = 0;
    if (impl->primary.face) {
        FT_Face f = impl->primary.face;
        if (f->size && (f->size->metrics.ascender != 0 || f->size->metrics.descender != 0)) {
            // Match wx's text extent: ascent + descent + line gap.
            h = (f->size->metrics.ascender - f->size->metrics.descender) / 64.0;
            // Pango/CoreText include leading; FT's height includes line-gap.
            double height_metric = f->size->metrics.height / 64.0;
            if (height_metric > h) h = height_metric;
        } else if (f->units_per_EM > 0) {
            h = ((f->ascender - f->descender) * (double)impl->currentPixelSize) / f->units_per_EM;
        } else {
            h = impl->currentPixelSize;
        }
    } else {
        h = impl->currentPixelSize;
    }
    if (width) *width = w;
    if (height) *height = h;
}

void FreeTypeTextDrawingContext::GetTextExtents(const std::string& msg, std::vector<double>& extents) {
    extents.clear();
    if (msg.empty()) return;
    // Per-character partial extents — mirrors wx's GetPartialTextExtents,
    // which reports cumulative width up through each character. Shape the
    // full string once and accumulate advances; map glyph cluster back to
    // character index. For non-ligatured Latin text this is the same as
    // shaping each prefix, but cheaper.
    auto cps = DecodeUTF8(msg);
    extents.resize(cps.size(), 0.0);
    auto glyphs = impl->ShapeString(msg);

    // Approximation: HarfBuzz output preserves cluster info per-glyph; we'd
    // need it to map glyphs → input characters precisely. For our use case
    // (TextEffect's per-character coloring), shaping each prefix yields the
    // same widths and is what wx does internally. Go with the cumulative
    // advance approach: split runs at each codepoint boundary by re-shaping.
    // This is O(n^2) on length but text effect strings are short.
    double cumulative = 0;
    for (size_t i = 0; i < cps.size(); i++) {
        std::string upTo = msg.substr(0, cps[i].byteStart + cps[i].byteLen);
        auto gg = impl->ShapeString(upTo);
        double w = 0;
        for (auto& g : gg) w += g.xAdvance / 64.0;
        extents[i] = w;
        cumulative = w;
    }
    (void)cumulative;
}

void FreeTypeTextDrawingContext::SetOverlayMode(bool /*b*/) {
    // Not needed: BlitGray/BlitBGRA always do source-over composition.
}

// ---------------------------------------------------------------------------
// Factory + font parsing
// ---------------------------------------------------------------------------
TextDrawingContext* FreeTypeTextDrawingContext::Create(int w, int h, bool aa) {
    return new FreeTypeTextDrawingContext(w, h, aa);
}

// Parse a wx native-font-info user description. Format is loose — tokens can
// include style keywords (bold/italic/oblique/light), a numeric pixel size,
// a charset (e.g. "utf-8"), and the face name in any order. Examples:
//   "bold arial 26 utf-8"     (wxOSX/wxMSW style)
//   "Arial 12"                (wxGTK/Pango style)
//   "DejaVu Sans Bold 14"     (wxGTK/Pango style)
TextFontInfo FreeTypeTextDrawingContext::ParseTextFont(const std::string& fontString) {
    TextFontInfo info;
    if (fontString.empty()) return info;

    auto lower = [](const std::string& s) {
        std::string out = s;
        for (char& c : out) c = (char)std::tolower((unsigned char)c);
        return out;
    };

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

    int size = 0;
    std::vector<std::string> faceParts;
    for (const std::string& part : parts) {
        std::string lp = lower(part);
        if (lp == "bold" || lp == "heavy" || lp == "black" || lp == "extrabold" || lp == "ultrabold" || lp == "demibold") {
            info.bold = true; continue;
        }
        if (lp == "italic" || lp == "oblique") { info.italic = true; continue; }
        if (lp == "light" || lp == "thin" || lp == "extralight" || lp == "ultralight") {
            info.light = true; continue;
        }
        if (lp == "underlined") { info.underlined = true; continue; }
        if (lp == "strikethrough") { info.strikethrough = true; continue; }
        if (lp == "regular" || lp == "normal" || lp == "book" || lp == "medium") continue;
        if (lp == "condensed" || lp == "semicondensed" || lp == "expanded") continue;

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

        // Charset markers like "utf-8", "iso-8859-1", "windows-1252" — contain
        // a digit and a hyphen. Discard so they don't pollute the face name.
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
    info.faceName = faceName.empty() ? "DejaVu Sans" : faceName;
    info.antiAliased = false;

    return info;
}

TextFontInfo FreeTypeTextDrawingContext::ParseShapeFont(const std::string& fontString) {
    TextFontInfo info = ParseTextFont(fontString);
    info.pixelSize = 12; // shape effect renders emoji at a reference size and scales
    if (info.faceName == WIN_NATIVE_EMOJI_FONT
        || info.faceName == OSX_NATIVE_EMOJI_FONT
        || info.faceName == LINUX_NATIVE_EMOJI_FONT) {
        info.faceName = NATIVE_EMOJI_FONT;
    }
    info.light = true;
    return info;
}

void FreeTypeTextDrawingContext::Register() {
    TextDrawingContext::RegisterFactory(&FreeTypeTextDrawingContext::Create,
                                        &FreeTypeTextDrawingContext::ParseTextFont,
                                        &FreeTypeTextDrawingContext::ParseShapeFont);
    TextDrawingContext::Initialize();
}

#else // !LINUX

// Non-Linux platforms don't link FreeType/HarfBuzz/Fontconfig. Provide
// no-op stubs so the symbols exist if anything references them. xLightsMain
// only calls Register() from inside #ifdef LINUX, so these are not exercised
// on Apple/Windows builds, but the empty TU keeps the file compiling cleanly
// when picked up by the macOS Xcode auto-discovery for src-core/.

struct FreeTypeTextDrawingContext::Impl {};

FreeTypeTextDrawingContext::FreeTypeTextDrawingContext(int, int, bool) {}
FreeTypeTextDrawingContext::~FreeTypeTextDrawingContext() = default;
void FreeTypeTextDrawingContext::ResetSize(int, int) {}
size_t FreeTypeTextDrawingContext::GetWidth() const { return 0; }
size_t FreeTypeTextDrawingContext::GetHeight() const { return 0; }
void FreeTypeTextDrawingContext::Clear() {}
const uint8_t* FreeTypeTextDrawingContext::FlushAndGetImage(int*, int*) { return nullptr; }
void FreeTypeTextDrawingContext::SetFont(const TextFontInfo&, const xlColor&) {}
void FreeTypeTextDrawingContext::DrawText(const std::string&, int, int, double) {}
void FreeTypeTextDrawingContext::DrawText(const std::string&, int, int) {}
void FreeTypeTextDrawingContext::GetTextExtent(const std::string&, double*, double*) {}
void FreeTypeTextDrawingContext::GetTextExtents(const std::string&, std::vector<double>&) {}
void FreeTypeTextDrawingContext::SetOverlayMode(bool) {}
TextDrawingContext* FreeTypeTextDrawingContext::Create(int, int, bool) { return nullptr; }
TextFontInfo FreeTypeTextDrawingContext::ParseTextFont(const std::string&) { return {}; }
TextFontInfo FreeTypeTextDrawingContext::ParseShapeFont(const std::string&) { return {}; }
void FreeTypeTextDrawingContext::Register() {}

#endif // LINUX
