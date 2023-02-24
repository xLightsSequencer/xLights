
#include <metal_stdlib>
using namespace metal;


#include "MetalEffectDataTypes.h"

constant float pi = 3.1415926535897932384626433832;
constant float pi3 = 3.1415926535897932384626433832 / 3.0;

uint8_t plasma_channelBlend(uint8_t c1, uint8_t c2, half ratio) {
    return c1 + floor(ratio * (c2 - c1) + 0.5h);
}
uchar4 plasma_getMultiColorBlend(constant PlasmaData &data, half n, bool circular) {
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
    ret.r = plasma_channelBlend(data.colors[coloridx1].r, data.colors[coloridx2].r,  ratio);
    ret.g = plasma_channelBlend(data.colors[coloridx1].g, data.colors[coloridx2].g,  ratio);
    ret.b = plasma_channelBlend(data.colors[coloridx1].b, data.colors[coloridx2].b,  ratio);
    ret.a = 255;
    return ret;
}

// reference: http://www.bidouille.org/prog/plasma

float plasmaCalc_vldpi(constant PlasmaData &data, uint32_t x, uint32_t y) {
    float time = data.time;
    
    float rx = data.width > 0 ? ((float)x / (float)(data.width - 1)) : 0.0; // rx is now in the range 0.0 to 1.0
    float rx2 = rx * rx;
    float cx = rx + 0.5f * data.sin_time_5;
    float cx2 = cx*cx;
    float sin_rx_time = sin(rx + time);

    // 1st equation
    float v = sin(rx * 10.0f + time);
    float ry = data.height > 0 ? ((float)y/(float)(data.height - 1)) : 0;

    //  second equation
    v += sin(10.0f*(rx*data.sin_time_2+ry*data.cos_time_3)+time);

    //  third equation
    float cy=ry + 0.5*data.cos_time_3;
    v += sin(sqrt((data.Style*50.0f)*((cx2)+(cy*cy))+time));

    //    vec2 c = v_coords * u_k - u_k/2.0;
    v += sin_rx_time;
    v += sin((ry+time)/2.0f);
    v += sin((rx+ry+time)/2.0f);
    //   c += u_k/2.0 * vec2(buffer.sin (u_time/3.0), buffer.cos (u_time/2.0));
    v += sin(sqrt(rx2+ry*ry)+time);
    v = v/2.0f;
    // vec3 col = vec3(1, buffer.sin (PI*v), buffer.cos (PI*v));
    //   gl_FragColor = vec4(col*.5 + .5, 1);

    return v * data.Line_Density * pi;
}

kernel void PlasmaEffectStyle0(constant PlasmaData &data,
                               device uchar4* result,
                               uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    uint32_t x = index % data.width;
    uint32_t y = index / data.width;

    float vldpi = plasmaCalc_vldpi(data, x, y);
    float h = (sin (vldpi + 2.0 * pi3) + 1.0) * 0.5;
    result[index] = plasma_getMultiColorBlend(data, h, false);
}


kernel void PlasmaEffectStyle1(constant PlasmaData &data,
                               device uchar4* result,
                               uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    uint32_t x = index % data.width;
    uint32_t y = index / data.width;

    float vldpi = plasmaCalc_vldpi(data, x, y);
    uchar4 ret;
    ret.r = (sin(vldpi) + 1.0) * 255.0 / 2.0;
    ret.g = (cos(vldpi) + 1.0) * 255.0 / 2.0;
    ret.b = 0;
    ret.a = 255;
    result[index] = ret;
}
 
kernel void PlasmaEffectStyle2(constant PlasmaData &data,
                               device uchar4* result,
                               uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    uint32_t x = index % data.width;
    uint32_t y = index / data.width;
       
    float vldpi = plasmaCalc_vldpi(data, x, y);
    uchar4 ret;
    ret.b = (sin(vldpi) + 1.0) * 255.0 / 2.0;
    ret.g = (cos(vldpi) + 1.0) * 255.0 / 2.0;
    ret.r = 1;
    ret.a = 255;
    result[index] = ret;
}

kernel void PlasmaEffectStyle3(constant PlasmaData &data,
                               device uchar4* result,
                               uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;
    
    float vldpi = plasmaCalc_vldpi(data, x, y);
    
    uchar4 ret;
    ret.r = (sin(vldpi) + 1.0) * 255.0 / 2.0;
    ret.g = (sin(vldpi + 2.0 * pi3) + 1.0) * 255.0 / 2.0;
    ret.b = (sin(vldpi + 4.0 * pi3) + 1.0) * 255.0 / 2.0;
    ret.a = 255;
    result[index] = ret;
}


kernel void PlasmaEffectStyle4(constant PlasmaData &data,
                               device uchar4* result,
                               uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;
    float vldpi = plasmaCalc_vldpi(data, x, y);

    uchar4 ret;
    ret.r = (sin(vldpi) + 1.0) * 255.0 / 2.0;
    ret.g = ret.r;
    ret.b = ret.r;
    ret.a = 255;
    result[index] = ret;
}
