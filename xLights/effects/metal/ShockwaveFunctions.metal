//
//  ShockwaveFunctions.metal
//  EffectComputeFunctions


#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"

extern uchar4 hsv2rgb(simd::float3 c);

kernel void ShockwaveEffectStyle0(constant MetalShockwaveData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;
    float x1 = x - data.xc_adj;
    float y1 = y - data.yc_adj;
    float r = length({x1, y1});
    if (r >= data.radius1 && r <= data.radius2) {
        if (data.blend > 0) {
            uchar4 ncolor = data.color;
            float color_pct = 1.0 - abs(r - data.radius_center) / data.half_width;
            if (data.allowAlpha) {
                ncolor.a = 255.0 * color_pct;
            } else {
                simd::float3 hsv = data.colorHSV;
                hsv.z = hsv.z * color_pct;
                ncolor = hsv2rgb(hsv);
            }
            result[index] = ncolor;
        } else {
            result[index] = data.color;
        }
    }
}
