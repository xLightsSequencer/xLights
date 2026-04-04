/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Fan effect.
// Implements the same per-pixel logic as FanFunctions.metal so that
// Windows and Linux get a SIMD-accelerated path that matches macOS Metal.

#define MAX_ISPC_FAN_COLORS 8
#define FAN_PI 3.14159265358979323846f

struct FanData {
    unsigned int width;
    unsigned int height;

    int          xc_adj;            // center offset from buffer middle (pixels)
    int          yc_adj;

    float        radius1;           // effective inner radius
    float        radius2;           // effective outer radius
    float        max_radius;        // max(start_radius, end_radius)

    float        blade_div_angle;   // 360 / num_blades
    float        blade_width_angle; // blade_div_angle * blade_width%
    float        color_angle;       // blade_width_angle / num_colors
    float        element_angle;     // color_angle / num_elements
    float        element_size;      // element_angle * element_width%
    float        angle_offset;      // eff_pos_adj * revolutions
    float        start_angle;       // start_angle parameter (degrees)
    float        blade_angle;       // twist angle (degrees)

    int          reverse_dir;
    int          blend_edges;
    int          allowAlpha;
    int          num_colors;

    uint8<4>     colorsAsRGBA[MAX_ISPC_FAN_COLORS];
    float        colorsH[MAX_ISPC_FAN_COLORS];
    float        colorsS[MAX_ISPC_FAN_COLORS];
    float        colorsV[MAX_ISPC_FAN_COLORS];
};

// HSV → RGBA conversion (alpha = 255).
// Mirrors fanHsv2rgb() from FanFunctions.metal.
static inline uint8<4> fanHsv2rgba(float h, float s, float v) {
    float r, g, b;
    if (s == 0.0f) {
        r = g = b = v;
    } else {
        float hh = (h - floor(h)) * 6.0f;
        int   i  = (int)hh;
        float f  = hh - (float)i;
        float p  = v * (1.0f - s);
        float q  = v * (1.0f - s * f);
        float t  = v * (1.0f - s * (1.0f - f));
        if      (i == 0) { r = v; g = t; b = p; }
        else if (i == 1) { r = q; g = v; b = p; }
        else if (i == 2) { r = p; g = v; b = t; }
        else if (i == 3) { r = p; g = q; b = v; }
        else if (i == 4) { r = t; g = p; b = v; }
        else             { r = v; g = p; b = q; }
    }
    uint8<4> out;
    out[0] = (uint8)(clamp(r, 0.0f, 1.0f) * 255.0f);
    out[1] = (uint8)(clamp(g, 0.0f, 1.0f) * 255.0f);
    out[2] = (uint8)(clamp(b, 0.0f, 1.0f) * 255.0f);
    out[3] = 255;
    return out;
}

export void FanEffectISPC(const uniform FanData * uniform data,
                          uniform int startIdx,
                          uniform int endIdx,
                          uniform uint8<4> * uniform result)
{
    float halfW = (float)data->width  * 0.5f;
    float halfH = (float)data->height * 0.5f;

    foreach (index = startIdx ... endIdx) {
        int x = index % data->width;
        int y = index / data->width;

        float x1 = (float)x - (float)data->xc_adj - halfW;
        float y1 = (float)y - (float)data->yc_adj - halfH;

        float r = sqrt(x1 * x1 + y1 * y1);

        if (r >= data->radius1 && r <= data->radius2) {
            float degrees_twist = (r / data->max_radius) * data->blade_angle;
            float theta = (atan2(x1, y1) * (180.0f / FAN_PI)) + degrees_twist + data->start_angle;

            if (data->reverse_dir) {
                theta = data->angle_offset - theta + 180.0f;
            } else {
                theta = theta + 180.0f + data->angle_offset;
            }

            if (theta < 0.0f) theta += 360.0f;

            // Integer truncation mod (matches CPU and Metal)
            float current_blade_angle = theta - (float)((int)(theta / data->blade_div_angle)) * data->blade_div_angle;

            if (current_blade_angle <= data->blade_width_angle) {
                float current_element_angle = current_blade_angle
                    - (float)((int)(current_blade_angle / data->element_angle)) * data->element_angle;

                if (current_element_angle <= data->element_size) {
                    int color_index = (int)(current_blade_angle / data->color_angle);
                    if (color_index < 0) color_index = 0;
                    if (color_index >= data->num_colors) color_index = data->num_colors - 1;

                    float color_pct = 1.0f - ((abs(current_element_angle - data->element_size * 0.5f) * 2.0f) / data->element_size);

                    if (data->blend_edges) {
                        if (data->allowAlpha) {
                            result[index][0] = data->colorsAsRGBA[color_index][0];
                            result[index][1] = data->colorsAsRGBA[color_index][1];
                            result[index][2] = data->colorsAsRGBA[color_index][2];
                            result[index][3] = (uint8)(255.0f * color_pct);
                        } else {
                            float v = clamp(data->colorsV[color_index] * color_pct, 0.0f, 1.0f);
                            result[index] = fanHsv2rgba(data->colorsH[color_index], data->colorsS[color_index], v);
                        }
                    } else {
                        result[index] = fanHsv2rgba(data->colorsH[color_index], data->colorsS[color_index], data->colorsV[color_index]);
                    }
                }
            }
        }
    }
}
