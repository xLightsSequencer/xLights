
#include <metal_stdlib>
using namespace metal;


#include "MetalEffectDataTypes.h"

// Float throughout, with the ISPC kernels' formulas (ButterflyFunctions.ispc),
// so the GPU render matches the CPU one.
constant float pi = 3.14159;
constant float pi2 = 3.14159 * 2.0;

// HSV to RGB with s and v at 1.0.
uchar4 h2rgb(float h) {
    h = clamp(h, 0.0f, 1.0f);
    float hue = h * 6.0f;
    int i = (int)floor(hue);
    float f = hue - (float)i;
    float3 rgb;
    switch (i) {
        case 6:
        case 0: rgb = float3(1.0f, f, 0.0f); break;
        case 1: rgb = float3(1.0f - f, 1.0f, 0.0f); break;
        case 2: rgb = float3(0.0f, 1.0f, f); break;
        case 3: rgb = float3(0.0f, 1.0f - f, 1.0f); break;
        case 4: rgb = float3(f, 0.0f, 1.0f); break;
        default: rgb = float3(1.0f, 0.0f, 1.0f - f); break;
    }
    rgb *= 255.0f;
    return uchar4(uchar(rgb.r), uchar(rgb.g), uchar(rgb.b), 255);
}

uint8_t channelBlend(float c1, float c2, float ratio) {
    return c1 + floor(ratio * (c2 - c1) + 0.5f);
}
uchar4 getMultiColorBlend(constant ButterflyData &data, float n, bool circular) {
    if (data.numColors <= 1) {
        return data.colors[0];
    }

    if (n >= 1.0f) n = 0.99999f;
    if (n < 0.0f) n = 0.0f;
    float nc = data.numColors;
    float realidx = circular ? n * nc : n * (nc - 1.0f);
    int coloridx1 = floor(realidx);
    int coloridx2 = (coloridx1 + 1) % data.numColors;
    float ratio = realidx - coloridx1;
    uchar4 c1 = data.colors[coloridx1];
    uchar4 c2 = data.colors[coloridx2];
    uchar4 ret;
    ret.r = channelBlend(c1.r, c2.r, ratio);
    ret.g = channelBlend(c1.g, c2.g, ratio);
    ret.b = channelBlend(c1.b, c2.b, ratio);
    ret.a = 255;
    return ret;
}

kernel void ButterflyEffectStyle1(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index >= uint(data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;

    float x2 = x*x;
    float y2 = y*y;
    float rsz = 1.0f / float(data.height + data.width) * pi2;
    float n = abs((x2 - y2) * sin(data.offset + ((x+y)*rsz)));
    float d = x2 + y2;
    float h = d > 0.001 ? clamp(n/d, 0.0f, 1.0f) : 0.0;

    if (data.chunks <= 1 || int(h*data.chunks) % data.skip != 0) {
        if (data.colorScheme == 0) {
            result[index] = h2rgb(h);
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}
kernel void ButterflyEffectStyle2(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index >= uint(data.width * data.height)) return;
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
            result[index] = h2rgb(h);
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}
kernel void ButterflyEffectStyle3(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index >= uint(data.width * data.height)) return;
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
            result[index] = h2rgb(h);
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}
kernel void ButterflyEffectStyle4(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index >= uint(data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;

    float rsz = 1.0f / float(data.height + data.width) * pi2;
    float n = ((x*x - y*y) * sin(data.offset + ((x+y)*rsz)));
    float d = x*x + y*y;

    float h = d>0.001 ? n/d : 0.0;
    h = h - floor(h);
    if (h < 0) h = 1.0 + h;

    if (data.chunks <= 1 || int(h*data.chunks) % data.skip != 0) {
        if (data.colorScheme == 0) {
            result[index] = h2rgb(h);
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}
kernel void ButterflyEffectStyle5(constant ButterflyData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index >= uint(data.width * data.height)) return;
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
            result[index] = h2rgb(h);
        } else {
            result[index] = getMultiColorBlend(data, h, false);
        }
    }
}



// (s + 1) * 128 reaches 256 at s == 1, past the top of a uchar.
float butterflyLevel(float s) {
    return min((s + 1.0f) * 128.0f, 255.0f);
}

kernel void ButterflyEffectPlasmaStyles(constant ButterflyData &data,
                                        device uchar4* result,
                                        uint index [[thread_position_in_grid]]) {
    const float invh = 1.0f / ((float)(data.height));
    const float invw = 1.0f / ((float)(data.width));
    const float time = data.plasmaTime;
    const float onehalf = 1.0 / 2.0;
    const float onethird = 1.0 / 3.0;
    const float halfTime = time / 2.0;
    const float thirdTime = time / 3.0;
    const float fifthTime = time / 5.0;
    const float chunks = data.chunks;

    if (index >= uint(data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;
    
    float v = 0;

    float rx = ((float)x) * invw - 0.5f;
    float ry = ((float)y) * invh - 0.5f;

    // 1st equation
    v = sin(rx * 10.0f + time);
            
    //  second equation
    v += sin(10.0f * (rx * sin(halfTime) + ry * cos(thirdTime)) + time);
            
    //  third equation
    float cx = rx + 0.5f * sin(fifthTime);
    float cy = ry + 0.5f * cos(thirdTime);
    v += sin(sqrt(100*((cx*cx)+(cy*cy))+1+time));
                            
    v += sin(rx + time);
    v += sin((ry + time) * onehalf);
    v += sin((rx + ry + time) * onehalf);
    
    v += sin(sqrt(rx * rx + ry * ry + 1.0) + time);
    v = v * onehalf;
            
    uchar4 color;
    color.a = 255;
    switch (data.plasmaStyle) {
        case 6:
            color.r = butterflyLevel(sin(v * chunks * pi));
            color.g= butterflyLevel(cos(v * chunks * pi));
            color.b = 0;
        break;
        case 7:
            color.r = 1;
            color.g = butterflyLevel(cos(v * chunks * pi));
            color.b = butterflyLevel(sin(v * chunks * pi));
            break;
        case 8:
            color.r = butterflyLevel(sin(v * chunks * pi));
            color.g= butterflyLevel(sin(v * chunks * pi + 2.0f * pi * onethird));
            color.b =butterflyLevel(sin(v * chunks * pi + 4.0f * pi * onethird));
            break;
        case 9:
            color.r = color.g = color.b = butterflyLevel(sin(v * chunks * pi));
            break;
        case 10:
            if (data.numColors >= 2) {
                float h = sin(v * chunks * pi + 2.0f * pi * onethird) + 0.5f;
                color = getMultiColorBlend(data, h, false);
            } else {
                color.r = 0; color.g = 0; color.b = 0;
            }
            break;
    }
    result[index] = color;
}
