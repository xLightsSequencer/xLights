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
    if (tox >= 0 && tox < data.width) {
        int toxi = (int)round(tox);
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
    if (tox >= 0 && tox < data.width) {
        int toxi = (int)round(tox);
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
    if (toy >= 0 && toy < data.height) {
        int toyi = round(toy);
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
    if (toy >= 0 && toy < data.height) {
        int toyi = round(toy);
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
