/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLGridMetalBridge.h"
#import <UIKit/UIKit.h>
#include "iPadGridPreview.h"
#include "graphics/metal/xlMetalGraphicsContext.h"
#include "graphics/xlGraphicsAccumulators.h"  // xlTexture lives here too
#include "graphics/xlFontInfo.h"
#include "utils/Color.h"
#include "utils/xlImage.h"

#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>

@implementation XLGridMetalBridge {
    std::unique_ptr<iPadGridPreview> _preview;
    xlGraphicsContext* _ctx;  // non-owning; owned by _preview during a frame
    // Per-frame batched accumulators. Allocated on first append,
    // flushed on endFrame (or explicit flush). Keeping them as
    // raw pointers rather than unique_ptrs because the context
    // factory returns raw pointers and we delete after flush.
    xlVertexColorAccumulator* _rectBatch;
    xlVertexColorAccumulator* _lineBatch;
    xlVertexColorAccumulator* _bgBatch;
    // Texture cache keyed by name (effect+bucket). Lifetime tied to
    // the bridge — textures survive across frames so repeated draws
    // only upload once. Deleted in `dealloc` / on name collision.
    std::unordered_map<std::string, xlTexture*> _textures;
    // Font atlas per size (BM-5). `xlFontInfo` holds a black-on-alpha
    // bitmap; we keep a white-on-alpha copy here so `drawTexture` with
    // a color multiplier tints glyphs to arbitrary colors. One texture
    // per size; built lazily on first use of that size.
    std::unordered_map<int, xlTexture*> _fontTextures;
}

- (instancetype)initWithName:(NSString*)name {
    self = [super init];
    if (self) {
        std::string n = name ? std::string([name UTF8String]) : std::string("GridCanvas");
        _preview = std::make_unique<iPadGridPreview>(n);
        _ctx = nullptr;
        _rectBatch = nullptr;
        _lineBatch = nullptr;
        _bgBatch = nullptr;
    }
    return self;
}

- (void)attachLayer:(CAMetalLayer*)layer {
    _preview->SetMetalLayer((__bridge void*)layer);
}

- (void)setDrawableSize:(CGSize)size scale:(CGFloat)scale {
    _preview->SetDrawableSize((int)size.width, (int)size.height, (double)scale);
}

- (BOOL)beginFrame {
    _ctx = _preview->BeginFrame();
    return _ctx != nullptr ? YES : NO;
}

- (void)endFrame {
    // Flush any batches the caller forgot to flush explicitly so
    // their geometry still lands this frame. Cheap no-op if empty.
    [self flushFilledRectBatch];
    [self flushLineBatch];
    [self flushEffectBackgroundBatch];
    _preview->EndFrame(true);
    _ctx = nullptr;
}

- (void)fillRectX:(CGFloat)x
                y:(CGFloat)y
                w:(CGFloat)w
                h:(CGFloat)h
                r:(CGFloat)red
                g:(CGFloat)green
                b:(CGFloat)blue
                a:(CGFloat)alpha {
    if (!_ctx) return;
    xlColor color((uint8_t)(red   * 255),
                  (uint8_t)(green * 255),
                  (uint8_t)(blue  * 255),
                  (uint8_t)(alpha * 255));
    // Ownership: the accumulator is allocated by the context via its
    // factory and freed by us once drawing is committed. Keeping a
    // single rect in its own accumulator is simple; later passes will
    // batch shapes into a shared accumulator for fewer draw calls.
    std::unique_ptr<xlVertexColorAccumulator> va(_ctx->createVertexColorAccumulator());
    va->PreAlloc(6);
    va->AddRectAsTriangles((float)x, (float)y,
                           (float)(x + w), (float)(y + h),
                           color);
    _ctx->drawTriangles(va.get());
}

- (void)lineX1:(CGFloat)x1
            y1:(CGFloat)y1
            x2:(CGFloat)x2
            y2:(CGFloat)y2
            r:(CGFloat)red
            g:(CGFloat)green
            b:(CGFloat)blue
            a:(CGFloat)alpha
            width:(CGFloat)width {
    if (!_ctx) return;
    xlColor color((uint8_t)(red   * 255),
                  (uint8_t)(green * 255),
                  (uint8_t)(blue  * 255),
                  (uint8_t)(alpha * 255));
    std::unique_ptr<xlVertexAccumulator> va(_ctx->createVertexAccumulator());
    va->PreAlloc(2);
    va->AddVertex((float)x1, (float)y1, 0);
    va->AddVertex((float)x2, (float)y2, 0);
    // `width` param unused by xlGraphicsContext's line drawer — Metal
    // line width defaults to 1 px. Widening would need either a
    // stroke-as-triangles path or MSL line-width uniform; both are
    // follow-ups once basic shapes render.
    (void)width;
    _ctx->drawLines(va.get(), color);
}

- (void)fillTriangleX1:(CGFloat)x1
                    y1:(CGFloat)y1
                    x2:(CGFloat)x2
                    y2:(CGFloat)y2
                    x3:(CGFloat)x3
                    y3:(CGFloat)y3
                    r:(CGFloat)red
                    g:(CGFloat)green
                    b:(CGFloat)blue
                    a:(CGFloat)alpha {
    if (!_ctx) return;
    xlColor color((uint8_t)(red   * 255),
                  (uint8_t)(green * 255),
                  (uint8_t)(blue  * 255),
                  (uint8_t)(alpha * 255));
    std::unique_ptr<xlVertexColorAccumulator> va(_ctx->createVertexColorAccumulator());
    va->PreAlloc(3);
    va->AddVertex((float)x1, (float)y1, 0, color);
    va->AddVertex((float)x2, (float)y2, 0, color);
    va->AddVertex((float)x3, (float)y3, 0, color);
    _ctx->drawTriangles(va.get());
}

// MARK: - Batched primitives
//
// `xlVertexColorAccumulator::AddRectAsTriangles` inserts the six
// vertices a filled rect needs. Batching many rects into a single
// accumulator and issuing one `drawTriangles` keeps GPU submission
// count low — otherwise a grid with hundreds of effects sees one
// draw call per rectangle which scales poorly.

- (void)beginFilledRectBatch {
    if (!_ctx) return;
    if (_rectBatch) { delete _rectBatch; _rectBatch = nullptr; }
    _rectBatch = _ctx->createVertexColorAccumulator();
}

- (void)appendFilledRectX:(CGFloat)x
                        y:(CGFloat)y
                        w:(CGFloat)w
                        h:(CGFloat)h
                        r:(CGFloat)red
                        g:(CGFloat)green
                        b:(CGFloat)blue
                        a:(CGFloat)alpha {
    if (!_rectBatch) { [self beginFilledRectBatch]; }
    if (!_rectBatch) return;
    xlColor color((uint8_t)(red   * 255),
                  (uint8_t)(green * 255),
                  (uint8_t)(blue  * 255),
                  (uint8_t)(alpha * 255));
    _rectBatch->AddRectAsTriangles((float)x, (float)y,
                                    (float)(x + w), (float)(y + h),
                                    color);
}

- (void)flushFilledRectBatch {
    if (!_ctx || !_rectBatch) return;
    if (_rectBatch->getCount() > 0) {
        _ctx->drawTriangles(_rectBatch);
    }
    delete _rectBatch;
    _rectBatch = nullptr;
}

- (void)beginLineBatch {
    if (!_ctx) return;
    if (_lineBatch) { delete _lineBatch; _lineBatch = nullptr; }
    _lineBatch = _ctx->createVertexColorAccumulator();
}

- (void)appendLineX1:(CGFloat)x1
                  y1:(CGFloat)y1
                  x2:(CGFloat)x2
                  y2:(CGFloat)y2
                  r:(CGFloat)red
                  g:(CGFloat)green
                  b:(CGFloat)blue
                  a:(CGFloat)alpha {
    if (!_lineBatch) { [self beginLineBatch]; }
    if (!_lineBatch) return;
    xlColor color((uint8_t)(red   * 255),
                  (uint8_t)(green * 255),
                  (uint8_t)(blue  * 255),
                  (uint8_t)(alpha * 255));
    _lineBatch->AddVertex((float)x1, (float)y1, 0, color);
    _lineBatch->AddVertex((float)x2, (float)y2, 0, color);
}

- (void)flushLineBatch {
    if (!_ctx || !_lineBatch) return;
    if (_lineBatch->getCount() > 0) {
        _ctx->drawLines(_lineBatch);
    }
    delete _lineBatch;
    _lineBatch = nullptr;
}

// MARK: - Effect background batch
//
// Desktop builds one accumulator per frame, loops over every visible
// effect calling `RenderableEffect::DrawEffectBackground(e, x1, y1, x2,
// y2, acc, mask, ramps)`, then issues one `drawTriangles`. Follow the
// same shape here: the document bridge owns the per-effect dispatch
// (it needs palette / model / color-mask context), while the bridge
// owns the accumulator + the flush.

- (void)beginEffectBackgroundBatch {
    if (!_ctx) return;
    if (_bgBatch) { delete _bgBatch; _bgBatch = nullptr; }
    _bgBatch = _ctx->createVertexColorAccumulator();
}

- (void)flushEffectBackgroundBatch {
    if (!_ctx || !_bgBatch) return;
    if (_bgBatch->getCount() > 0) {
        _ctx->drawTriangles(_bgBatch);
    }
    delete _bgBatch;
    _bgBatch = nullptr;
}

- (void*)effectBackgroundAccumulator {
    return _bgBatch;
}

// MARK: - Textures
//
// The bridge holds a name-keyed cache of `xlTexture*` so repeated
// draws of the same icon only pay upload cost once. Input bytes
// are BGRA-premultiplied (matches `iconBGRAForEffectNamed:` and
// iOS's default `CGImage` byteOrder32Little + premultipliedFirst).
// `xlImage` stores RGBA, so we swizzle R↔B when copying bytes in.

- (void)ensureTextureNamed:(NSString*)name
                   bgraData:(NSData*)data
                          w:(int)width
                          h:(int)height {
    if (!_ctx || !name || !data || width <= 0 || height <= 0) return;
    std::string key([name UTF8String]);
    if (_textures.find(key) != _textures.end()) return;  // already uploaded

    xlImage image(width, height);
    uint8_t* dst = image.GetData();
    const uint8_t* src = (const uint8_t*)data.bytes;
    size_t bytes = (size_t)width * (size_t)height * 4;
    if (data.length < (NSUInteger)bytes) return;
    // BGRA → RGBA swizzle in place. Alpha stays intact.
    for (size_t i = 0; i < bytes; i += 4) {
        dst[i + 0] = src[i + 2];  // R ← B source
        dst[i + 1] = src[i + 1];  // G
        dst[i + 2] = src[i + 0];  // B ← R source
        dst[i + 3] = src[i + 3];  // A
    }
    xlTexture* tex = _ctx->createTexture(image, key, /* finalize */ true);
    if (tex) {
        _textures[key] = tex;
    }
}

- (void)drawTextureNamed:(NSString*)name
                       x:(CGFloat)x
                       y:(CGFloat)y
                       w:(CGFloat)w
                       h:(CGFloat)h {
    if (!_ctx || !name) return;
    auto it = _textures.find(std::string([name UTF8String]));
    if (it == _textures.end() || !it->second) return;
    // Flush any in-flight batches so geometry order is preserved —
    // otherwise the texture draws before the rect batch that's
    // supposed to sit below it (disabled overlay, fade bars).
    [self flushFilledRectBatch];
    [self flushLineBatch];
    _ctx->drawTexture(it->second,
                      (float)x, (float)y,
                      (float)(x + w), (float)(y + h),
                      /* tx */ 0.0f, /* ty */ 0.0f,
                      /* tx2 */ 1.0f, /* ty2 */ 1.0f,
                      /* smoothScale */ true);
}

- (void)dealloc {
    // Drop cached textures; their MTLTexture retain/release lives
    // inside the xlTexture wrapper.
    for (auto& kv : _textures) {
        delete kv.second;
    }
    _textures.clear();
    for (auto& kv : _fontTextures) {
        delete kv.second;
    }
    _fontTextures.clear();
}

// MARK: - Text rendering
//
// Two paths (BM-5):
//
// - ASCII strings (bulk of text: effect names, ruler labels, timing
//   labels, drag pill) draw through the shared font atlas from
//   `xlFontInfo`. One `xlTexture` per font size holds the glyph
//   sheet; `xlFontInfo::populate` builds textured quads into an
//   `xlVertexTextureAccumulator`, and `drawTexture(..., xlColor)`
//   tints the glyphs to the caller's colour. The atlas we build is
//   white-on-alpha so the colour multiply hits a white source and
//   passes through unchanged.
//
// - Non-ASCII strings (lock glyph 🔒, future emoji) fall through to
//   a per-string CoreText-rendered texture cache — same code path
//   that shipped initially. `xlFontInfo` is ASCII 32–127 only, so
//   this branch is unavoidable for anything outside that range.

static bool _isAsciiPrintable(NSString* text) {
    NSUInteger len = text.length;
    for (NSUInteger i = 0; i < len; i++) {
        unichar c = [text characterAtIndex:i];
        if (c < 0x20 || c > 0x7E) return false;
    }
    return true;
}

- (UIFont*)_gridFontForSize:(CGFloat)size {
    return [UIFont systemFontOfSize:size weight:UIFontWeightRegular];
}

// Build (or return a cached) white-on-alpha atlas texture for the
// given font size. Keyed by size so 9/10/11pt each get their own
// upload.
- (xlTexture*)_fontTextureForSize:(int)size {
    auto it = _fontTextures.find(size);
    if (it != _fontTextures.end()) return it->second;
    if (!_ctx) return nullptr;
    const xlFontInfo& font = xlFontInfo::FindFont(size);
    const xlImage& src = font.getImage();
    if (!src.IsOk()) return nullptr;

    // Invert the RGB channels: the pre-baked atlas stores (0,0,0,a),
    // which would multiply to black no matter the tint colour. We
    // want (255,255,255,a) so `drawTexture(vac, tex, color)` passes
    // the caller's colour through cleanly.
    int w = src.GetWidth();
    int h = src.GetHeight();
    xlImage white(w, h);
    const uint8_t* srcData = src.GetData();
    uint8_t* dstData = white.GetData();
    size_t px = (size_t)w * (size_t)h;
    for (size_t i = 0; i < px; ++i) {
        dstData[i * 4 + 0] = 255;
        dstData[i * 4 + 1] = 255;
        dstData[i * 4 + 2] = 255;
        dstData[i * 4 + 3] = srcData[i * 4 + 3];
    }
    std::string name = "FontAtlasWhite-" + std::to_string(size);
    xlTexture* tex = _ctx->createTexture(white, name, /* finalize */ true);
    if (tex) {
        _fontTextures[size] = tex;
    }
    return tex;
}

- (CGSize)sizeOfText:(NSString*)text fontSize:(CGFloat)fontSize {
    if (!text || text.length == 0) return CGSizeZero;
    if (_isAsciiPrintable(text)) {
        const xlFontInfo& font = xlFontInfo::FindFont((int)fontSize);
        std::string s([text UTF8String]);
        return CGSizeMake(ceil(font.widthOf(s, 1.0f)), (CGFloat)font.getSize());
    }
    UIFont* font = [self _gridFontForSize:fontSize];
    NSDictionary* attrs = @{ NSFontAttributeName: font };
    CGSize s = [text sizeWithAttributes:attrs];
    s.width = ceil(s.width);
    s.height = ceil(s.height);
    return s;
}

- (void)drawText:(NSString*)text
             atX:(CGFloat)x
               y:(CGFloat)y
        fontSize:(CGFloat)fontSize
               r:(CGFloat)red
               g:(CGFloat)green
               b:(CGFloat)blue
               a:(CGFloat)alpha {
    if (!_ctx || !text || text.length == 0) return;

    // Flush any in-flight batches so text lands in its declared
    // Z-order (caller issues geometry, then text).
    [self flushFilledRectBatch];
    [self flushLineBatch];
    [self flushEffectBackgroundBatch];

    if (_isAsciiPrintable(text)) {
        xlTexture* atlas = [self _fontTextureForSize:(int)fontSize];
        if (atlas) {
            const xlFontInfo& font = xlFontInfo::FindFont((int)fontSize);
            std::string s([text UTF8String]);
            // `y` is the top of the text rect; `populate` takes the
            // bottom baseline. Add the font height to convert.
            float yBase = (float)y + (float)font.getSize();
            std::unique_ptr<xlVertexTextureAccumulator> vac(
                _ctx->createVertexTextureAccumulator());
            font.populate(*vac, (float)x, yBase, s, 1.0f);
            if (vac->getCount() > 0) {
                xlColor color((uint8_t)(red   * 255),
                              (uint8_t)(green * 255),
                              (uint8_t)(blue  * 255),
                              (uint8_t)(alpha * 255));
                _ctx->drawTexture(vac.get(), atlas, color);
            }
            return;
        }
        // fall through to per-string path if atlas failed
    }

    // Per-string fallback (non-ASCII / atlas-unavailable). Renders
    // the whole string to a cached CG-backed texture. Keyed by
    // (text, size, color) so different-colour renders don't collide.
    NSString* key = [NSString stringWithFormat:@"__text:%@:%g:%d,%d,%d,%d",
                     text, fontSize,
                     (int)(red * 255), (int)(green * 255),
                     (int)(blue * 255), (int)(alpha * 255)];
    std::string keyStr([key UTF8String]);

    UIFont* uiFont = [self _gridFontForSize:fontSize];
    NSDictionary* attrs = @{ NSFontAttributeName: uiFont };
    CGSize size = [text sizeWithAttributes:attrs];
    size.width = ceil(size.width);
    size.height = ceil(size.height);
    if (size.width <= 0 || size.height <= 0) return;

    auto it = _textures.find(keyStr);
    if (it == _textures.end()) {
        NSDictionary* drawAttrs = @{
            NSFontAttributeName: uiFont,
            NSForegroundColorAttributeName: [UIColor
                colorWithRed:red green:green blue:blue alpha:alpha],
        };
        CGFloat scale = [UIScreen mainScreen].scale;
        if (scale <= 0) scale = 2.0;
        int pxW = (int)ceil(size.width * scale);
        int pxH = (int)ceil(size.height * scale);
        if (pxW <= 0 || pxH <= 0) return;
        std::unique_ptr<uint8_t[]> rgba(new uint8_t[(size_t)pxW * pxH * 4]());
        CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
        CGContextRef cgCtx = CGBitmapContextCreate(
            rgba.get(), pxW, pxH, 8, pxW * 4, cs,
            kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
        CGColorSpaceRelease(cs);
        if (!cgCtx) return;
        UIGraphicsPushContext(cgCtx);
        CGContextTranslateCTM(cgCtx, 0, pxH);
        CGContextScaleCTM(cgCtx, scale, -scale);
        [text drawAtPoint:CGPointMake(0, 0) withAttributes:drawAttrs];
        UIGraphicsPopContext();
        CGContextRelease(cgCtx);

        xlImage img(pxW, pxH);
        std::memcpy(img.GetData(), rgba.get(), (size_t)pxW * pxH * 4);
        xlTexture* tex = _ctx->createTexture(img, keyStr, /* finalize */ true);
        if (!tex) return;
        _textures[keyStr] = tex;
        it = _textures.find(keyStr);
    }

    _ctx->drawTexture(it->second,
                      (float)x, (float)y,
                      (float)(x + size.width),
                      (float)(y + size.height),
                      /* tx */ 0.0f, /* ty */ 0.0f,
                      /* tx2 */ 1.0f, /* ty2 */ 1.0f,
                      /* smoothScale */ true);
}

@end
