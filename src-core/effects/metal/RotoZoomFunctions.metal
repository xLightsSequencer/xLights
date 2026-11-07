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
// The rotate kernels scatter, so several source pixels can land on one
// destination. Each rotation runs in two phases: a Claim pass records, per
// destination, the source the CPU loop in PixelBufferClass::RotateX/Y/
// RotateZAndZoom writes LAST (atomic_max of its write-order key), then the
// write pass lets only that source store its color. Destinations truncate
// like SetPixel, and the trig, pivots and offsets come precomputed from the
// host (RotoZoomSettings), so the output is deterministic and equal to the
// CPU render.

// X: x from the pivot up, then from pivot - 1 down; key = that order.
kernel void RotoZoomRotateXClaim(constant RotoZoomData &data,
                                 device atomic_int* owner,
                                 uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    int x = index.x;
    int pivot = (int)data.xpivotPx;
    float tox;
    int key;
    if (x >= pivot) {
        tox = data.xsine * (float)(x - pivot) + data.xpivotPx;
        key = x - pivot;
    } else {
        tox = -1 * data.xsine * (float)(pivot - x) + data.xpivotPx;
        key = (int)data.width + (pivot - 1 - x);
    }
    int toxi = (int)tox;
    if (toxi >= 0 && toxi < (int)data.width) {
        uint didx = index.y * data.width + toxi;
        atomic_fetch_max_explicit(&owner[didx], key, memory_order_relaxed);
    }
}

kernel void RotoZoomRotateX(constant RotoZoomData &data,
                            device uchar4* result,
                            const device uchar4* src,
                            const device int32_t* owner,
                            uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    int x = index.x;
    int pivot = (int)data.xpivotPx;
    float tox;
    int key;
    if (x >= pivot) {
        tox = data.xsine * (float)(x - pivot) + data.xpivotPx;
        key = x - pivot;
    } else {
        tox = -1 * data.xsine * (float)(pivot - x) + data.xpivotPx;
        key = (int)data.width + (pivot - 1 - x);
    }
    int toxi = (int)tox;
    if (toxi >= 0 && toxi < (int)data.width) {
        uint didx = index.y * data.width + toxi;
        if (owner[didx] == key) {
            result[didx] = src[index.y * data.width + index.x];
        }
    }
}

// Y: y from the pivot up, then from pivot - 1 down; key = that order.
kernel void RotoZoomRotateYClaim(constant RotoZoomData &data,
                                 device atomic_int* owner,
                                 uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    int y = index.y;
    int pivot = (int)data.ypivotPx;
    float toy;
    int key;
    if (y >= pivot) {
        toy = data.ysine * (float)(y - pivot) + data.ypivotPx;
        key = y - pivot;
    } else {
        toy = -1 * data.ysine * (float)(pivot - y) + data.ypivotPx;
        key = (int)data.height + (pivot - 1 - y);
    }
    int toyi = (int)toy;
    if (toyi >= 0 && toyi < (int)data.height) {
        uint didx = toyi * data.width + index.x;
        atomic_fetch_max_explicit(&owner[didx], key, memory_order_relaxed);
    }
}

kernel void RotoZoomRotateY(constant RotoZoomData &data,
                            device uchar4* result,
                            const device uchar4* src,
                            const device int32_t* owner,
                            uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    int y = index.y;
    int pivot = (int)data.ypivotPx;
    float toy;
    int key;
    if (y >= pivot) {
        toy = data.ysine * (float)(y - pivot) + data.ypivotPx;
        key = y - pivot;
    } else {
        toy = -1 * data.ysine * (float)(pivot - y) + data.ypivotPx;
        key = (int)data.height + (pivot - 1 - y);
    }
    int toyi = (int)toy;
    if (toyi >= 0 && toyi < (int)data.height) {
        uint didx = toyi * data.width + index.x;
        if (owner[didx] == key) {
            result[didx] = src[index.y * data.width + index.x];
        }
    }
}

// Z/zoom: loops x, i, y, j with a float bounds check before SetPixel; key =
// that loop order (the host keeps it within an int).
kernel void RotoZoomRotateZClaim(constant RotoZoomData &data,
                                 device atomic_int* owner,
                                 uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    int q = (int)data.zoomquality;
    int x = index.x;
    int y = index.y;
    for (int i = 0; i < q; i++) {
        for (int j = 0; j < q; j++) {
            float xx = (float)x + ((float)i * data.zinc) - data.zxoff;
            float yy = (float)y + ((float)j * data.zinc) - data.zyoff;
            float u = data.zxoff + data.zcos * xx * data.zoom + data.zsin * yy * data.zoom;
            if (u >= 0 && u < (float)data.width) {
                float v = data.zyoff + -data.zsin * xx * data.zoom + data.zcos * yy * data.zoom;
                if (v >= 0 && v < (float)data.height) {
                    uint didx = (uint)v * data.width + (uint)u;
                    int key = ((x * q + i) * (int)data.height + y) * q + j;
                    atomic_fetch_max_explicit(&owner[didx], key, memory_order_relaxed);
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
    int q = (int)data.zoomquality;
    int x = index.x;
    int y = index.y;
    uchar4 c = src[index.y * data.width + index.x];
    for (int i = 0; i < q; i++) {
        for (int j = 0; j < q; j++) {
            float xx = (float)x + ((float)i * data.zinc) - data.zxoff;
            float yy = (float)y + ((float)j * data.zinc) - data.zyoff;
            float u = data.zxoff + data.zcos * xx * data.zoom + data.zsin * yy * data.zoom;
            if (u >= 0 && u < (float)data.width) {
                float v = data.zyoff + -data.zsin * xx * data.zoom + data.zcos * yy * data.zoom;
                if (v >= 0 && v < (float)data.height) {
                    uint didx = (uint)v * data.width + (uint)u;
                    int key = ((x * q + i) * (int)data.height + y) * q + j;
                    if (owner[didx] == key) {
                        result[didx] = c;
                    }
                }
            }
        }
    }
}
