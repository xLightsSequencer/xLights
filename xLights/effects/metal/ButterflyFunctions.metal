
#include <metal_stdlib>
using namespace metal;


#include "ButterflyTypes.h"

char4 hsv2rgb(float3 c) {
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    c = c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    return char4(c.r * 255, c.g * 255, c.b * 255, -1);
}

uint8_t channelBlend(uint8_t c1, uint8_t c2, float ratio) {
    return c1 + floor(ratio * (c2 - c1) + 0.5);
}
char4 getMultiColorBlend(constant ButterflyData &data, float n, bool circular) {
    if (data.numColors <= 1) {
        return data.colors[0];
    }

    if (n >= 1.0) n = 0.99999f;
    if (n < 0.0) n = 0.0f;
    float realidx = circular ? n * data.numColors : n * (data.numColors - 1);
    int coloridx1 = floor(realidx);
    int coloridx2 = (coloridx1 + 1) % data.numColors;
    float ratio = realidx - float(coloridx1);
    char4 ret;
    ret.r = channelBlend(data.colors[coloridx1].r, data.colors[coloridx2].r,  ratio);
    ret.g = channelBlend(data.colors[coloridx1].g, data.colors[coloridx2].g,  ratio);
    ret.b = channelBlend(data.colors[coloridx1].b, data.colors[coloridx2].b,  ratio);
    ret.a = -1;
    return ret;
}
constant float pi2 = 3.14159*2.0;

kernel void ButterflyEffect(constant ButterflyData &data,
                            device char4* result,
                            uint2 index [[thread_position_in_grid]])
{
    int x = index.x;
    int y = index.y;

    if (x > data.width) return;
    if (y > data.height) return;

    int bindex = y * data.width + x;

    float n = abs((x*x - y*y) * sin(data.offset + ((x+y)*pi2 / float(data.height + data.width))));
    float d = x*x + y*y;

    //  This section is to fix the colors on pixels at {0,1} and {1,0}
    float x0 = x + 1;
    float y0 = y + 1;
    if((x==0 && y==1)) {
        n = abs((x*x - y0*y0) * sin(data.offset + ((x+y0)*pi2 / float(data.height+data.width))));
        d = x*x + y0*y0;
    }
    if((x==1 && y==0)) {
        n = abs((x0*x0 - y*y) * sin(data.offset + ((x0+y)*pi2 / float(data.height+data.width))));
        d = x0*x0 + y*y;
    }
    // end of fix

    float h=d>0.001 ? n/d : 0.0;

    float3 hsv = float3(h, 1.0, 1.0);
    if (data.chunks <= 1 || int(h*data.chunks) % data.skip != 0) {
        if (data.colorScheme == 0) {
            result[bindex] = hsv2rgb(hsv);
        } else {
            result[bindex] = getMultiColorBlend(data, h, false);
        }
    }
}
