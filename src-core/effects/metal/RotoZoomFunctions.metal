//
//  RotoZoomFunctions.metal
//  EffectComputeFunctions
//
//  Created by Daniel Kulp on 10/27/22.
//  Copyright © 2022 Daniel Kulp. All rights reserved.
//
#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"


constant float M_PI = 3.1415926535897932384626433832;
constant float M_PI2 = M_PI*2;


kernel void RotoZoomBlank(constant RotoZoomData &data,
                          device uchar4* result,
                          uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;
    result[sidx] = {0, 0, 0, 0};
}

// Separable tent blur (replaces MPSImageTent, whose output varied run to
// run).  Plain per-pixel gathers with a fixed summation order, so the result
// is bit-exact on every run.  Edge handling clamps like MPSImageEdgeModeClamp.
kernel void TentBlurH(constant TentBlurData &data,
                      device uchar4* dst,
                      const device uchar4* src,
                      uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    float4 acc = 0.0;
    float sumw = 0.0;
    for (int i = -data.halfK; i <= data.halfK; i++) {
        int x = clamp(int(index.x) + i, 0, int(data.width) - 1);
        float w = float(data.halfK + 1 - abs(i));
        acc += float4(src[index.y * data.width + x]) * w;
        sumw += w;
    }
    dst[index.y * data.width + index.x] = uchar4(round(acc / sumw));
}

kernel void TentBlurV(constant TentBlurData &data,
                      device uchar4* dst,
                      const device uchar4* src,
                      uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    float4 acc = 0.0;
    float sumw = 0.0;
    for (int i = -data.halfK; i <= data.halfK; i++) {
        int y = clamp(int(index.y) + i, 0, int(data.height) - 1);
        float w = float(data.halfK + 1 - abs(i));
        acc += float4(src[y * data.width + index.x]) * w;
        sumw += w;
    }
    dst[index.y * data.width + index.x] = uchar4(round(acc / sumw));
}

// Small box blur. Integer math, matching PixelBufferClass::Blur's CPU "else"
// branch bit-for-bit: sum each channel (as int) over the in-bounds window
// [x-d, x+u] x [y-d, y+u], count valid pixels, integer-divide. src must be a
// stable snapshot separate from dst.
kernel void BoxBlur(constant BoxBlurData &data,
                    device uchar4* dst,
                    const device uchar4* src,
                    uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    int x = int(index.x);
    int y = int(index.y);
    int r = 0, g = 0, b = 0, a = 0, sm = 0;
    for (int i = x - data.d; i <= x + data.u; i++) {
        if (i < 0 || i >= int(data.width)) continue;
        for (int j = y - data.d; j <= y + data.u; j++) {
            if (j < 0 || j >= int(data.height)) continue;
            uchar4 c = src[j * data.width + i];
            r += int(c.x);
            g += int(c.y);
            b += int(c.z);
            a += int(c.w);
            sm++;
        }
    }
    if (sm == 0) sm = 1;
    dst[y * data.width + x] = uchar4(uchar(r / sm), uchar(g / sm), uchar(b / sm), uchar(a / sm));
}
// The rotate kernels scatter: several source pixels can round() to the same
// destination, and concurrent GPU threads racing for it made the output
// non-deterministic run to run.  Each rotation therefore runs in two phases:
// a Claim pass records the highest source index per destination via
// atomic_max, then the write pass lets only that winner store its color.

kernel void RotoZoomRotateXClaim(constant RotoZoomData &data,
                                 device atomic_int* owner,
                                 uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float x = index.x;
    float sine = sin((data.xrotation + 90.0) * M_PI / 180.0);
    float pivot = data.xpivot * data.width / 100.0;
    float tox;
    if (x >= pivot) {
        tox = sine * (x - pivot) + pivot;
    } else {
        tox = -1 * sine * (pivot - x) + pivot;
    }
    // Bounds-check the ROUNDED index, not the float: round() can push a value
    // in [width-0.5, width) up to width, which would scatter past the end of
    // the buffer into the next layer's Metal allocation (a run-to-run
    // determinism corruption).  Matches the CPU SetPixel bounds drop.
    int toxi = (int)round(tox);
    if (toxi >= 0 && toxi < (int)data.width) {
        uint didx = index.y * data.width + toxi;
        atomic_fetch_max_explicit(&owner[didx], (int)sidx, memory_order_relaxed);
    }
}

kernel void RotoZoomRotateX(constant RotoZoomData &data,
                            device uchar4* result,
                            const device uchar4* src,
                            const device int32_t* owner,
                            uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float x = index.x;
    float sine = sin((data.xrotation + 90.0) * M_PI / 180.0);
    float pivot = data.xpivot * data.width / 100.0;
    float tox;
    if (x >= pivot) {
        tox = sine * (x - pivot) + pivot;
    } else {
        tox = -1 * sine * (pivot - x) + pivot;
    }
    int toxi = (int)round(tox);
    if (toxi >= 0 && toxi < (int)data.width) {
        uint didx = index.y * data.width + toxi;
        if (owner[didx] == (int)sidx) {
            result[didx] = src[sidx];
        }
    }
}

kernel void RotoZoomRotateYClaim(constant RotoZoomData &data,
                                 device atomic_int* owner,
                                 uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float y = index.y;
    float sine = sin((data.yrotation + 90.0) * M_PI / 180.0);
    float pivot = (data.ypivot * data.height) / 100.0;

    float toy;
    if (y >= pivot) {
        toy = sine * (y - pivot) + pivot;
    } else {
        toy = -1 * sine * (pivot - y) + pivot;
    }
    // Bounds-check the ROUNDED index (see RotoZoomRotateXClaim): on a height-1
    // buffer, toy in [0.5, 1.0) passes the float guard but rounds to 1, so
    // didx = 1*width + x scatters a full row past the buffer end into the
    // adjacent layer's Metal buffer.
    int toyi = (int)round(toy);
    if (toyi >= 0 && toyi < (int)data.height) {
        uint didx = toyi * data.width + index.x;
        atomic_fetch_max_explicit(&owner[didx], (int)sidx, memory_order_relaxed);
    }
}

kernel void RotoZoomRotateY(constant RotoZoomData &data,
                            device uchar4* result,
                            const device uchar4* src,
                            const device int32_t* owner,
                            uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float y = index.y;
    float sine = sin((data.yrotation + 90.0) * M_PI / 180.0);
    float pivot = (data.ypivot * data.height) / 100.0;

    float toy;
    if (y >= pivot) {
        toy = sine * (y - pivot) + pivot;
    } else {
        toy = -1 * sine * (pivot - y) + pivot;
    }
    int toyi = (int)round(toy);
    if (toyi >= 0 && toyi < (int)data.height) {
        uint didx = toyi * data.width + index.x;
        if (owner[didx] == (int)sidx) {
            result[didx] = src[sidx];
        }
    }
}

kernel void RotoZoomRotateZClaim(constant RotoZoomData &data,
                                 device atomic_int* owner,
                                 uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float zoom = data.zoom;
    float q = data.zoomquality;
    int cx = data.pivotpointx;
    int cy = data.pivotpointy;
    float rotation = data.zrotation;

    float inc = 1.0 / (float)q;

    float angle = M_PI2 * -rotation;
    float xoff = (cx * data.width) / 100.0;
    float yoff = (cy * data.height) / 100.0;
    float anglecos = cos(-angle);
    float anglesin = sin(-angle);

    float x = index.x;
    float y = index.y;

    for (int i = 0; i < q; i++) {
        for (int j = 0; j < q; j++) {
            float xx = x + ((float)i * inc) - xoff;
            float yy = y + ((float)j * inc) - yoff;
            int u = round(xoff + anglecos * xx * zoom + anglesin * yy * zoom);
            if (u >= 0 && u < (int)data.width) {
                int v = round(yoff + -anglesin * xx * zoom + anglecos * yy * zoom);
                if (v >= 0 && v < (int)data.height) {
                    uint didx = v * data.width + u;
                    atomic_fetch_max_explicit(&owner[didx], (int)sidx, memory_order_relaxed);
                }
            }
        }
    }
}

kernel void RotoZoomRotateZ(constant RotoZoomData &data,
                            device uchar4* result,
                            const device uchar4* src,
                            const device int32_t* owner,
                            uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float zoom = data.zoom;
    float q = data.zoomquality;
    int cx = data.pivotpointx;
    int cy = data.pivotpointy;
    float rotation = data.zrotation;

    float inc = 1.0 / (float)q;

    float angle = M_PI2 * -rotation;
    float xoff = (cx * data.width) / 100.0;
    float yoff = (cy * data.height) / 100.0;
    float anglecos = cos(-angle);
    float anglesin = sin(-angle);

    float x = index.x;
    float y = index.y;

    uchar4 c = src[sidx];

    for (int i = 0; i < q; i++) {
        for (int j = 0; j < q; j++) {
            float xx = x + ((float)i * inc) - xoff;
            float yy = y + ((float)j * inc) - yoff;
            int u = round(xoff + anglecos * xx * zoom + anglesin * yy * zoom);
            if (u >= 0 && u < (int)data.width) {
                int v = round(yoff + -anglesin * xx * zoom + anglecos * yy * zoom);
                if (v >= 0 && v < (int)data.height) {
                    uint didx = v * data.width + u;
                    if (owner[didx] == (int)sidx) {
                        result[didx] = c;
                    }
                }
            }
        }
    }
}
