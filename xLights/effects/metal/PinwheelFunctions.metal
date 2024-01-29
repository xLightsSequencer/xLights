
#include <metal_stdlib>
using namespace metal;


#include "MetalEffectDataTypes.h"

constant simd::float4 K = simd::float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);

uchar4 hsv2rgb(simd::float3 c) {
    simd::float3 p = abs(fract(c.xxx + K.xyz) * 6.0h - K.www);
    c = c.z * mix(K.xxx, clamp(p - K.xxx, 0.0h, 1.0h), c.y);
    return uchar4(c.r * 255.0h, c.g * 255.0h, c.b * 255.0h, 255);
}

kernel void PinwheelEffectStyle0(constant MetalPinwheelData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;

    float y1 = y - data.yc_adj - ((float)data.height / 2.0f);
    float x1 = x - data.xc_adj - ((float)data.width / 2.0f);
    float r = sqrt((x1*x1) + (y1*y1));
    
    simd::float3 hsv;
    if (r <= data.max_radius) {
        float degrees_twist = (r / data.max_radius) * data.pinwheel_twist;
        float theta = (atan2(x1, y1) * 180 / 3.14159) + degrees_twist;
        if (data.pinwheel_rotation == 1) { // do we have CW rotation
            theta = data.pos + theta + (data.tmax / 2) + data.poffset;
        } else {
            theta = data.pos - theta + (data.tmax / 2) + data.poffset;
        }
        theta = theta + 540.0;
        int t2 = (int)theta % data.degrees_per_arm;
        if (t2 <= data.tmax) {
            float round = (float)t2 / (float)data.tmax;
            t2 = abs(t2 - (data.tmax / 2)) * 2;
            int ColorIdx2 = ((int)((theta / data.degrees_per_arm))) % data.pinwheel_arms;
            simd::uchar4 color = data.colorsAsColor[ColorIdx2];
            if (!data.allowAlpha) {
                hsv = data.colorsAsHSV[ColorIdx2];
            }
            switch (data.pw3dType) {
                case 1:
                    if (data.allowAlpha) {
                        color.a = 255.0 * ((data.tmax - t2) / data.tmax);
                    } else {
                        hsv.z = hsv.z * ((data.tmax - t2) / data.tmax);
                        color = hsv2rgb(hsv);
                    }
                    break;
                case 2:
                    if (data.allowAlpha) {
                        color.a = 255.0 * ((t2) / data.tmax);
                    } else {
                        hsv.z = hsv.z * ((t2) / data.tmax);
                        color = hsv2rgb(hsv);
                    }
                    break;
                case 3:
                    if (data.allowAlpha) {
                        color.a = 255.0 * (1.0 - round);
                    } else {
                        hsv.z = hsv.z * (1.0 - round);
                        color = hsv2rgb(hsv);
                    }
                    break;
                default:
                    break;
            }
            result[index] = color;
        }
    }
}
