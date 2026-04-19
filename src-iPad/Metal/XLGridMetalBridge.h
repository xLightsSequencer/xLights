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

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

// ObjC bridge that Swift uses to drive 2D Metal rendering for one of
// the grid canvases (model effects, timing effects, or top chrome).
// Owns an `iPadGridPreview` underneath and exposes a small batch-
// oriented draw API. All coordinates are in logical pixel space with
// (0,0) at top-left — matches the Core Graphics convention the
// current Swift-side drawing code uses, so porting is mechanical.

@interface XLGridMetalBridge : NSObject

- (instancetype)initWithName:(NSString*)name;
- (void)attachLayer:(CAMetalLayer*)layer;
- (void)setDrawableSize:(CGSize)size scale:(CGFloat)scale;

// Frame lifecycle. `beginFrame` returns NO if the canvas isn't
// ready yet (no layer attached, zero drawable size, etc.). When it
// returns YES the caller must eventually call `endFrame`.
- (BOOL)beginFrame;
- (void)endFrame;

// Filled rectangle (top-left origin). RGBA 0..1.
- (void)fillRectX:(CGFloat)x
                y:(CGFloat)y
                w:(CGFloat)w
                h:(CGFloat)h
                r:(CGFloat)r
                g:(CGFloat)g
                b:(CGFloat)b
                a:(CGFloat)a;

// Single-line segment in point space. Stroke colour in RGBA 0..1.
- (void)lineX1:(CGFloat)x1
            y1:(CGFloat)y1
            x2:(CGFloat)x2
            y2:(CGFloat)y2
            r:(CGFloat)r
            g:(CGFloat)g
            b:(CGFloat)b
            a:(CGFloat)a
            width:(CGFloat)width;

// Filled triangle — used for fade-handle diamonds (as two tris) and
// any other non-axis-aligned shape. RGBA 0..1.
- (void)fillTriangleX1:(CGFloat)x1
                    y1:(CGFloat)y1
                    x2:(CGFloat)x2
                    y2:(CGFloat)y2
                    x3:(CGFloat)x3
                    y3:(CGFloat)y3
                    r:(CGFloat)r
                    g:(CGFloat)g
                    b:(CGFloat)b
                    a:(CGFloat)a;

// Batched rectangle API. Beats per-rect `fillRectX:...` for grids
// with hundreds of shapes — one accumulator, one `drawTriangles`
// call at flush time. `beginFilledRectBatch` resets the batch;
// `appendFilledRect...` adds to it; `flushFilledRectBatch` issues
// the draw and clears the accumulator. Safe to call with no
// appended rects (flush becomes a no-op).
- (void)beginFilledRectBatch;
- (void)appendFilledRectX:(CGFloat)x
                        y:(CGFloat)y
                        w:(CGFloat)w
                        h:(CGFloat)h
                        r:(CGFloat)r
                        g:(CGFloat)g
                        b:(CGFloat)b
                        a:(CGFloat)a;
- (void)flushFilledRectBatch;

// Batched line API — sibling of the rect batch, used for effect
// brackets + centerlines + timing-mark verticals (all single-pixel
// strokes of the same logical width). Per-vertex colour lets a
// single flush mix brackets of several colours (selected vs
// unselected vs locked vs disabled) without separate batches.
- (void)beginLineBatch;
- (void)appendLineX1:(CGFloat)x1
                  y1:(CGFloat)y1
                  x2:(CGFloat)x2
                  y2:(CGFloat)y2
                  r:(CGFloat)r
                  g:(CGFloat)g
                  b:(CGFloat)b
                  a:(CGFloat)a;
- (void)flushLineBatch;

// Effect-background batch. Mirrors how desktop `EffectsGrid::DrawEffects`
// shares one `xlVertexColorAccumulator` across every visible effect's
// `RenderableEffect::DrawEffectBackground` call and issues a single
// triangle draw at flush time. `effectBackgroundAccumulator` returns
// the raw `xlVertexColorAccumulator*` (as a `void*`) between
// begin/flush so ObjC++ callers (the document bridge) can pass it by
// reference to `DrawEffectBackground`. Between frames the accumulator
// is null — callers must wrap their background passes in begin/flush.
- (void)beginEffectBackgroundBatch;
- (void)flushEffectBackgroundBatch;
- (void*)effectBackgroundAccumulator;

// Upload a named texture (BGRA-premultiplied, 4 bytes per pixel)
// into the bridge's texture cache. Cheap no-op if a texture with
// the same `name` has already been uploaded — names encode
// (effect, bucket) and are stable across frames.
- (void)ensureTextureNamed:(NSString*)name
                   bgraData:(NSData*)data
                          w:(int)w
                          h:(int)h;

// Draw the named cached texture at the given rect. Silently skips
// if `name` isn't in the cache (call `ensureTextureNamed:` first).
- (void)drawTextureNamed:(NSString*)name
                       x:(CGFloat)x
                       y:(CGFloat)y
                       w:(CGFloat)w
                       h:(CGFloat)h;

// Text rendering — renders `text` to a cached texture (keyed on
// text + fontSize + rgba) the first time it's seen, then draws
// that texture at (x, y). The rendered bitmap is sized via
// `sizeOfText:fontSize:` so callers can lay out surrounding
// geometry before drawing. Colour is baked into the texture —
// different colours of the same string are cached separately.
- (CGSize)sizeOfText:(NSString*)text fontSize:(CGFloat)fontSize;
- (void)drawText:(NSString*)text
             atX:(CGFloat)x
               y:(CGFloat)y
        fontSize:(CGFloat)fontSize
               r:(CGFloat)r
               g:(CGFloat)g
               b:(CGFloat)b
               a:(CGFloat)a;

@end
