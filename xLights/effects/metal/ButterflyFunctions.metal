
#include <metal_stdlib>
using namespace metal;


#include "MetalEffectDataTypes.h"

constant half pi2 = 3.14159h * 2.0h;
constant half pi = 3.14159h;
constant half4 K = half4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
uchar4 hsv2rgb(half3 c) {
    c = clamp(c, 0.0h, 1.0h);
    half3 p = abs(fract(c.xxx + K.xyz) * 6.0h - K.www);
    c = c.z * mix(K.xxx, clamp(p - K.xxx, 0.0h, 1.0h), c.y);
    return uchar4(c.r * 255.0h, c.g * 255.0h, c.b * 255.0h, 255);
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



kernel void ButterflyEffectPlasmaStyles(constant ButterflyData &data,
                                        device uchar4* result,
                                        uint index [[thread_position_in_grid]]) {
    const half invh = 1.0h / ((half)(data.height));
    const half invw = 1.0h / ((half)(data.width));
    const half time = data.plasmaTime;
    const half onehalf = 1.0 / 2.0;
    const half onethird = 1.0 / 3.0;
    const half halfTime = time / 2.0;
    const half thirdTime = time / 3.0;
    const half fifthTime = time / 5.0;
    const half chunks = data.chunks;

    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;
    
    half v = 0;

    half rx = ((half)x) * invw - 0.5h;
    half ry = ((half)y) * invh - 0.5h;

    // 1st equation
    v = sin(rx * 10.0h + time);
            
    //  second equation
    v += sin(10.0h * (rx * sin(halfTime) + ry * cos(thirdTime)) + time);
            
    //  third equation
    float cx = rx + 0.5h * sin(fifthTime);
    float cy = ry + 0.5h * cos(thirdTime);
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
            color.r = (sin(v * chunks * pi) + 1.0h) * 128.0h;
            color.g= (cos(v * chunks * pi) + 1.0h) * 128.0h;
            color.b = 0;
        break;
        case 7:
            color.r = 1;
            color.g = (cos(v * chunks * pi) + 1.0h) * 128.0h;
            color.b = (sin(v * chunks * pi) + 1.0h) * 128.0h;
            break;
        case 8:
            color.r = (sin(v * chunks * pi) + 1.0h) * 128.0h;
            color.g= (sin(v * chunks * pi + 2.0h * pi * onethird) + 1.0h) * 128.0h;
            color.b =(sin(v * chunks * pi + 4.0h * pi * onethird) + 1.0h) * 128.0h;
            break;
        case 9:
            color.r = color.g = color.b = (sin(v * chunks * pi) + 1.0h) * 128.0h;
            break;
        case 10:
            if (data.numColors >= 2) {
                half h = sin(v * chunks * pi + 2.0h * pi * onethird) + 0.5h;
                color = getMultiColorBlend(data, h, false);
            } else {
                color.r = 0; color.g = 0; color.b = 0;
            }
            break;
    }
    result[index] = color;
}
