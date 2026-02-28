#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"

constant float FAN_PI = 3.14159265358979323846f;

// Convert HSV (h,s,v all in [0,1]) to uchar4 RGBA
inline uchar4 fanHsv2rgb(float3 hsv) {
    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;
    float r, g, b;
    if (s == 0.0f) {
        r = g = b = v;
    } else {
        h = fmod(h, 1.0f) * 6.0f;
        int i = int(h);
        float f = h - float(i);
        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));
        switch (i) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            default: r = v; g = p; b = q; break;
        }
    }
    return uchar4(uchar(r * 255.0f), uchar(g * 255.0f), uchar(b * 255.0f), 255);
}

kernel void FanEffect(constant MetalFanData &data,
                      device uchar4* result,
                      uint index [[thread_position_in_grid]])
{
    if (index >= data.width * data.height) return;

    int x = int(index % data.width);
    int y = int(index / data.width);

    float x1 = float(x) - float(data.xc_adj) - (float(data.width)  / 2.0f);
    float y1 = float(y) - float(data.yc_adj) - (float(data.height) / 2.0f);

    float r = sqrt(x1 * x1 + y1 * y1);

    if (r < data.radius1 || r > data.radius2) return;

    float degrees_twist = (r / data.max_radius) * data.blade_angle;
    // Matches CPU: theta = atan2(x1,y1)*180/PI + degrees_twist + start_angle
    float theta = (atan2(x1, y1) * 180.0f / FAN_PI) + degrees_twist + data.start_angle;

    if (data.reverse_dir) {
        theta = data.angle_offset - theta + 180.0f;
    } else {
        theta = theta + 180.0f + data.angle_offset;
    }

    // CPU: single correction to bring theta >= 0
    if (theta < 0.0f) theta += 360.0f;

    // current_blade_angle = theta mod blade_div_angle (CPU uses integer truncation)
    float current_blade_angle = theta - float(int(theta / data.blade_div_angle)) * data.blade_div_angle;

    if (current_blade_angle > data.blade_width_angle) return;

    float current_element_angle = current_blade_angle - float(int(current_blade_angle / data.element_angle)) * data.element_angle;

    if (current_element_angle > data.element_size) return;

    int color_index = int(current_blade_angle / data.color_angle);
    if (color_index < 0) color_index = 0;
    if (color_index >= data.num_colors) color_index = data.num_colors - 1;

    float color_pct = 1.0f - ((abs(current_element_angle - (data.element_size / 2.0f)) * 2.0f) / data.element_size);

    float3 hsv = data.colorsAsHSV[color_index];

    if (data.blend_edges) {
        if (data.allowAlpha) {
            uchar4 c = data.colorsAsRGBA[color_index];
            c.a = uchar(255.0f * color_pct);
            result[index] = c;
        } else {
            hsv.z = hsv.z * color_pct;
            result[index] = fanHsv2rgb(hsv);
        }
    } else {
        result[index] = fanHsv2rgb(hsv);
    }
}
