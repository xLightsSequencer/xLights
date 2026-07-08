// Shared code for the layer-blending kernels — port of the helpers in
// LayerBlendingFunctions.metal.  The push-constant block must match
// LayerBlendingData in VulkanEffectDataTypes.h.

#include "vulkan_common.glsl"

layout(push_constant) uniform PC {
    int nodeCount;
    uint bufferWi;
    uint bufferHi;
    uint useMask;

    float hueAdjust;
    float valueAdjust;
    float saturationAdjust;

    int brightness;
    int contrast;
    float fadeFactor;
    float effectMixThreshold;
    uint effectMixVaries;
    uint brightnessLevel;
    int mixTypeData;

    int outputSparkleCount;
    uint sparkleColor; // packed uchar4

    uint isChromaKey;
    int chromaSensitivity;
    uint chromaColor; // packed uchar4
} data;

const float XL_PI = 3.1415926535897932384626433832;

vec3 toHSV(uvec4 c) {
    float r = float(c.r) / 255.0;
    float g = float(c.g) / 255.0;
    float b = float(c.b) / 255.0;

    float K = 0.0;
    if (g < b) {
        float tmp = g;
        g = b;
        b = tmp;
        K = -1.0;
    }
    float min_gb = b;
    if (r < g) {
        float tmp = r;
        r = g;
        g = tmp;
        K = -2.0 / 6.0 - K;
        min_gb = min(g, b);
    }
    float chroma = r - min_gb;
    vec3 v;
    v.x = abs(K + (g - b) / (6.0 * chroma + 1e-20));
    v.y = chroma / (r + 1e-20);
    v.z = r;
    return v;
}

uvec4 fromHSV(vec3 hsv) {
    float red, green, blue;

    if (0.0 == hsv.y) {
        // Grey
        red = hsv.z;
        green = hsv.z;
        blue = hsv.z;
    } else { // not grey
        float hue = hsv.x * 6.0; // sector 0 to 5
        int i = int(floor(hue));
        float f = hue - float(i); // fractional part of h
        float p = hsv.z * (1.0 - hsv.y);

        switch (i) {
            case 0:
                red = hsv.z;
                green = hsv.z * (1.0 - hsv.y * (1.0 - f));
                blue = p;
                break;
            case 1:
                red = hsv.z * (1.0 - hsv.y * f);
                green = hsv.z;
                blue = p;
                break;
            case 2:
                red = p;
                green = hsv.z;
                blue = hsv.z * (1.0 - hsv.y * (1.0 - f));
                break;
            case 3:
                red = p;
                green = hsv.z * (1.0 - hsv.y * f);
                blue = hsv.z;
                break;
            case 4:
                red = hsv.z * (1.0 - hsv.y * (1.0 - f));
                green = p;
                blue = hsv.z;
                break;
            default: // case 5:
                red = hsv.z;
                green = p;
                blue = hsv.z * (1.0 - hsv.y * f);
                break;
        }
    }
    return uvec4(uint(red * 255.0), uint(green * 255.0), uint(blue * 255.0), 255u);
}

bool equalsRGB(uvec4 a, uvec3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

uvec4 ApplyBrightnessC(uvec4 c, float b) {
    return uvec4(uint(float(c.x) * b), uint(float(c.y) * b), uint(float(c.z) * b), c.a);
}

float ColourDistance(uvec4 e1, uvec4 e2) {
    int rmean = int(e1.r + e2.r) / 2;
    int r = int(e1.r) - int(e2.r);
    int g = int(e1.g) - int(e2.g);
    int b = int(e1.b) - int(e2.b);
    int f1 = (((512 + rmean) * r * r) >> 8);
    int f2 = (((767 - rmean) * b * b) >> 8);
    float f3 = float(f1 + 4 * g * g + f2);
    return sqrt(f3);
}

// Mirrors Metal applyChroma: premultiplies alpha into c (even when returning
// false) and returns true when the color matches the chroma key.
bool applyChroma(inout uvec4 c) {
    if (c.a < 255u) {
        c.r = (c.r * c.a) / 255u;
        c.g = (c.g * c.a) / 255u;
        c.b = (c.b * c.a) / 255u;
        c.a = 255u;
    }
    if (ColourDistance(c, unpackPx(data.chromaColor)) < float(data.chromaSensitivity * 402 / 255)) {
        return true;
    }
    return false;
}

uvec4 AlphaBlendForgroundOnto(uvec4 bg, uvec4 fc) {
    if (fc.a == 0u) return bg;
    if (fc.a == 255u) {
        return fc;
    }
    float a = float(fc.a);
    a /= 255.0; // 0 (transparent) - 1.0 (opague)
    float dr = float(fc.r) * a + float(bg.r) * (1.0 - a);
    float dg = float(fc.g) * a + float(bg.g) * (1.0 - a);
    float db = float(fc.b) * a + float(bg.b) * (1.0 - a);
    return uvec4(uint(dr), uint(dg), uint(db), bg.a);
}
