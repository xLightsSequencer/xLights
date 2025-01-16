
#include <metal_stdlib>
using namespace metal;


#include "MetalEffectDataTypes.h"

constant float pi2 = 3.14159*2.0;

uchar4 hsv2rgb(half3 c) {
    float3 rgb;
    half h = c.r;
    half s = c.g;
    half v = c.b;
    if (0.0f == s) {
        rgb.r = v;
        rgb.g = v;
        rgb.b = v;
    } else { // not grey
        half hue = h * 6.0h;      // sector 0 to 5
        int i = (int)floor(hue);
        half f = hue - (half)i;   // fractional part of h
        half p = v * (1.0h - s);

        switch (i) {
         case 0:
             rgb.r = v;
             rgb.g = v * (1.0h - s * (1.0h - f));
             rgb.b = p;
             break;
         case 1:
             rgb.r = v * (1.0h - s * f);
             rgb.g = v;
             rgb.b = p;
             break;
         case 2:
             rgb.r = p;
             rgb.g = v;
             rgb.b = v * (1.0h - s * (1.0h - f));
             break;
         case 3:
             rgb.r = p;
             rgb.g = v * (1.0h - s * f);
             rgb.b = v;
             break;
         case 4:
             rgb.r = v * (1.0h - s * (1.0h - f));
             rgb.g = p;
             rgb.b = v;
             break;
         default:    // case 5:
             rgb.r = v;
             rgb.g = p;
             rgb.b = v * (1.0h - s * f);
             break;
        }
    }
    rgb *= 255.0;
    return uchar4(rgb.x, rgb.y, rgb.z, 255);
}

uint8_t channelBlend(uint8_t c1, uint8_t c2, half ratio) {
    return c1 + floor(ratio * (c2 - c1) + 0.5h);
}
uchar4 getMultiColorBlend(constant ButterflyData &data, half n, bool circular) {
    if (data.numColors <= 1) {
        return data.colors[0];
    }

    if (n >= 1.0) n = 0.99999f;
    if (n < 0.0) n = 0.0f;
    half realidx = circular ? n * data.numColors : n * (data.numColors - 1);
    int coloridx1 = floor(realidx);
    int coloridx2 = (coloridx1 + 1) % data.numColors;
    half ratio = realidx - half(coloridx1);
    uchar4 ret;
    ret.r = channelBlend(data.colors[coloridx1].r, data.colors[coloridx2].r,  ratio);
    ret.g = channelBlend(data.colors[coloridx1].g, data.colors[coloridx2].g,  ratio);
    ret.b = channelBlend(data.colors[coloridx1].b, data.colors[coloridx2].b,  ratio);
    ret.a = 255;
    return ret;
}

kernel void ButterflyEffectStyle1(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;

    float x2 = x*x;
    float y2 = y*y;
    float n = abs((x2 - y2) * sin(data.offset + ((x+y)*pi2 / float(data.height + data.width))));
    float d = x2 + y2;
    float h = d>0.001 ? n/d : 0.0;

    if (data.chunks <= 1 || int(h*data.chunks) % data.skip != 0) {
        if (data.colorScheme == 0) {
            result[index] = hsv2rgb(half3(h, 1.0, 1.0));
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}
kernel void ButterflyEffectStyle2(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;

    int maxframe = data.height * 2;
    int frame = (data.height * data.curState / 200) % maxframe;

    float f= (frame < maxframe/2) ? frame + 1 : maxframe - frame;
    float x1= (float(x) - data.width/2.0)/f;
    float y1= (float(y) - data.height/2.0)/f;
    float h = sqrt(x1*x1 + y1*y1);

    if (data.chunks <= 1 || int(h*data.chunks) % data.skip != 0) {
        if (data.colorScheme == 0) {
            result[index] = hsv2rgb(half3(h, 1.0, 1.0));
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}
kernel void ButterflyEffectStyle3(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;

    int maxframe = data.height * 2;
    int frame = (data.height * data.curState / 200) % maxframe;

    float f = (frame < maxframe/2) ? frame + 1 : maxframe - frame;
    f = f * 0.1 + float(data.height)/60.0;
    float x1 = (x-data.width/2.0)/f;
    float y1 = (y-data.height/2.0)/f;
    float h = sin(x1) * cos(y1);

    if (data.chunks <= 1 || int(h*data.chunks) % data.skip != 0) {
        if (data.colorScheme == 0) {
            result[index] = hsv2rgb(half3(h, 1.0, 1.0));
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}
kernel void ButterflyEffectStyle4(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;

    float n = ((x*x - y*y) * sin(data.offset + ((x+y)*pi2 / float(data.height+data.width))));
    float d = x*x + y*y;

    float h = d>0.001 ? n/d : 0.0;
    float intpart;
    float fractpart = modf(h , intpart);
    h = fractpart;
    if (h < 0) h = 1.0 + h;

    if (data.chunks <= 1 || int(h*data.chunks) % data.skip != 0) {
        if (data.colorScheme == 0) {
            result[index] = hsv2rgb(half3(h, 1.0, 1.0));
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}
kernel void ButterflyEffectStyle5(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;

    //  This section is to fix the colors on pixels at {0,1} and {1,0}
    if (x == 0 && y == 1) y++;
    if (x == 1 && y == 0) x++;

    float n = abs((x*x - y*y) * sin(data.offset + ((x+y)*pi2 / float(data.height*data.width))));
    float d = x*x + y*y;
    float h=d>0.001 ? n/d : 0.0;

    if (data.chunks <= 1 || int(h*data.chunks) % data.skip != 0) {
        if (data.colorScheme == 0) {
            result[index] = hsv2rgb(half3(h, 1.0, 1.0));
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}
