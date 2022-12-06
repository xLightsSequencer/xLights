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
kernel void RotoZoomRotateX(constant RotoZoomData &data,
                            device uchar4* result,
                            device uchar4* src,
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
        uint didx = index.y * data.width + tox;
        result[didx] = src[sidx];
    }
}

kernel void RotoZoomRotateY(constant RotoZoomData &data,
                            device uchar4* result,
                            device uchar4* src,
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
        result[didx] = src[sidx];
    }
}

kernel void RotoZoomRotateZ(constant RotoZoomData &data,
                            device uchar4* result,
                            device uchar4* src,
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
                    result[didx] = c;
                }
            }
        }
    }
}
