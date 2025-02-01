//
//  LayerBlendingFunctions.metal
//  EffectComputeFunctions

#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"

float3 toHSV(uchar4 c)
{
    float r = c.r / 255.0;
    float g = c.g / 255.0;
    float b = c.b / 255.0;

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
    float3 v;
    v.x = abs(K + (g - b) / (6.0 * chroma + 1e-20));
    v.y = chroma / (r + 1e-20);
    v.z = r;
    return v;
}
uchar4 fromHSV(float3 hsv) {
    float red, green, blue;

    if (0.0f == hsv.y) {
        // Grey
        red = hsv.z;
        green = hsv.z;
        blue = hsv.z;
    } else { // not grey
        float hue = hsv.x * 6.0;      // sector 0 to 5
        int i = (int)floor(hue);
        float f = hue - i;          // fractional part of h
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

            default:    // case 5:
                red = hsv.z;
                green = p;
                blue = hsv.z * (1.0 - hsv.y * f);
                break;
        }
    }
    return {(uint8_t)(red * 255.0),
        (uint8_t)(green * 255.0),
        (uint8_t)(blue * 255.0),
        255};
}
bool equals(uchar4 a, uchar4 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.a == b.a;
}
bool equals(uchar4 a, uchar3 b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}
uchar4 ApplyBrightness(uchar4 c, float b) {
    return {(uchar)(c.x * b), (uchar)(c.y * b), (uchar)(c.z * b), c.a};
}


kernel void GetColorsForNodes(constant LayerBlendingData &data,
                              device uchar4* result,
                              const device uchar4* src,
                              const device uchar*  mask,
                              const device int32_t *indexes,
                              uint index [[thread_position_in_grid]])
{
    if (index > (uint)data.nodeCount) return;
    int32_t idx = indexes[index];
    
    int32_t x = idx % data.bufferWi;
    int32_t y = idx / data.bufferWi;
    int32_t midx = x * data.bufferHi + y;
    
    if (idx == -1) {
        result[index] = {0, 0, 0, 0};
    } else if (idx & 0x80000000) {
        result[index] = {0, 0, 0, 0};
        idx &= 0x7FFFFFFF;
        int cnt = indexes[idx++];
        for (int nc = 0; nc < cnt; nc++) {
            int nidx = indexes[idx + nc];
            uchar4 c = src[nidx];
            
            x = nidx / data.bufferHi;
            y = nidx % data.bufferHi;
            midx = x * data.bufferHi + y;
            
            if (c.a > 0 && (!data.useMask || mask[midx] == 0)) {
                result[index] = c;
                nc = cnt;
            }
        }
    } else if (data.useMask && mask[midx] > 0) {
        result[index] = {0, 0, 0, 0};
    } else {
        result[index] = src[idx];
    }
}

kernel void PutColorsForNodes(constant LayerBlendingData &data,
                              device uchar4* result,
                              const device uchar4* src,
                              const constant uchar*  mask,
                              const device int32_t *indexes,
                              uint index [[thread_position_in_grid]])
{
    if (index > (uint)data.nodeCount) return;
    int32_t idx = indexes[index];
    if (idx == -1) {
        //result[idx] = {0, 0, 0, 0};
    } else if (idx & 0x80000000) {
        idx &= 0x7FFFFFFF;
        int cnt = indexes[idx++];
        for (int n = 0; n < cnt; n++) {
            int nidx = indexes[idx + n];
            int32_t x = nidx / data.bufferHi;
            int32_t y = nidx - (x * data.bufferHi);
            int32_t midx = x * data.bufferHi + y;
            if (data.useMask && mask[midx] > 0) {
                result[nidx] = {0, 0, 0, 0};
            } else {
                result[nidx] = src[index];
            }
        }
    } else if (data.useMask) {
        int32_t x = idx / data.bufferHi;
        int32_t y = idx - (x * data.bufferHi);
        int32_t midx = x * data.bufferHi + y;
        if (mask[midx] > 0) {
            result[idx] = {0, 0, 0, 0};
        } else {
            result[idx] = src[index];
        }
    } else {
        result[idx] = src[index];
    }
}

kernel void AdjustHSV(constant LayerBlendingData &data,
                      device uchar4* result,
                      uint index [[thread_position_in_grid]])
{
    if (index > (uint)data.nodeCount) return;
    uchar4 color = result[index];
    
    float3 hsv = toHSV(color);

    if (data.hueAdjust != 0) {
        hsv.x += data.hueAdjust;
        if (hsv.x < 0) {
            hsv.x += 1.0;
        } else if (hsv.x > 1) {
            hsv.x -= 1.0;
        }
    }
    if (data.saturationAdjust != 0) {
        hsv.y += data.saturationAdjust;
        if (hsv.y < 0) {
            hsv.y = 0.0;
        } else if (hsv.y > 1) {
            hsv.y = 1.0;
        }
    }

    if (data.valueAdjust != 0) {
        hsv.z += data.valueAdjust;
        if (hsv.z < 0) {
            hsv.z = 0.0;
        } else if (hsv.z > 1) {
            hsv.z = 1.0;
        }
    }

    unsigned char alpha = color.a;
    color = fromHSV(hsv);
    color.a = alpha;
    result[index] = color;
}

kernel void ApplySparkles(constant LayerBlendingData &data,
                          device uchar4* result,
                          device uint16_t* sparkles,
                          uint index [[thread_position_in_grid]])
{
    if (index > (uint)data.nodeCount) return;
    
    uchar4 c = result[index];
    if (c.r != 0 || c.g != 0 || c.b != 0) {
        int sc = data.outputSparkleCount;
        uint16_t sparkle = sparkles[index];
        
        switch (sparkle % (208 - sc)) {
            case 1:
            case 7:
                // too dim
                // color.Set("#444444");
                break;
            case 2:
            case 6:
                result[index] = ApplyBrightness(data.sparkleColor, 0.53f);
                break;
            case 3:
            case 5:
                result[index] = ApplyBrightness(data.sparkleColor, 0.75f);
                break;
            case 4:
                result[index] = data.sparkleColor;
                break;
            default:
                break;
        }
        sparkles[index] = sparkle + 1;
    }
}

kernel void AdjustBrightnessContrast(constant LayerBlendingData &data,
                                     device uchar4* result,
                                     uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 color = result[index];

    float b = data.brightness;
    float c = data.contrast;
    if (data.contrast != 0) {
        // contrast is not 0, can handle brightness change at same time
        float3 hsv = toHSV(color);
        hsv.z = hsv.z * (b / 100.0);

        // Apply Contrast
        if (hsv.z < 0.5) {
            // reduce brightness when below 0.5 in the V value or increase if > 0.5
            hsv.z = hsv.z - (hsv.z * (c / 100.0));
        } else {
            hsv.z = hsv.z + (hsv.z * (c / 100.0));
        }

        if (hsv.z < 0.0)
            hsv.z = 0.0;
        if (hsv.z > 1.0)
            hsv.z = 1.0;
        unsigned char alpha = color.a;
        color = fromHSV(hsv);
        color.a = alpha;
    } else if (b != 100) {
        // just brightness
        b /= 100.0f;
        float f = color.r * b;
        color.r = min((int)f, 255);
        f = color.g * b;
        color.g = min((int)f, 255);
        f = color.b * b;
        color.b = min((int)f, 255);
    }
    result[index] = color;
}
kernel void AdjustBrightnessLevel(constant LayerBlendingData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 color = result[index];
    uint8_t c = 0;
    if (color.r > 25) ++c;
    if (color.g > 25) ++c;
    if (color.b > 25) ++c;
    if (c == 0) {
        return;
    }
    color /= {c, c, c, 1};
    result[index] = color;
}


kernel void FirstLayerFade(constant LayerBlendingData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 color = src[index];
    if (data.fadeFactor != 1.0) {
        float3 hsv = toHSV(color);
        hsv.z *= data.fadeFactor;
        if (color.a != 255) {
            hsv.z *= color.a;
            hsv.z /= 255.0f;
        }
        color = fromHSV(hsv);
    }
    if (color.a == 0) {
        result[index] = {0, 0, 0, 0};
    } else if (color.a == 255) {
        result[index] = color;
    } else {
        float a = color.a;
        a /= 255; // 0 (transparent) - 1.0 (opague)
        float dr = color.r * a;
        float dg = color.g * a;
        float db = color.b * a;
        result[index] = {(uchar)dr, (uchar)dg, (uchar)db, 255};
    }
}

kernel void NonAlphaFade(constant LayerBlendingData &data,
                         device uchar4* result,
                         uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 color = result[index];
    float3 hsv = toHSV(color);
    hsv.z *= data.fadeFactor;
    result[index] = fromHSV(hsv);
}

float ColourDistance(const uchar4 e1, const uchar4 e2) {
    int rmean = (e1.r + e2.r) / 2;
    int r = e1.r - e2.r;
    int g = e1.g - e2.g;
    int b = e1.b - e2.b;
    int f1 = (((512 + rmean) * r * r) >> 8);
    int f2 = (((767 - rmean) * b * b) >> 8);
    float f3 = f1 + 4 * g * g + f2;
    return sqrt(f3);
}

bool applyChroma(constant LayerBlendingData &data,
                 uchar4 c) {
    if (c.a < 255) {
        c.r = (int)(c.r * c.a) / 255;
        c.g = (int)(c.g * c.a) / 255;
        c.b = (int)(c.b * c.a) / 255;
        c.a = 255;
    }
    if (ColourDistance(c, data.chromaColor) < (data.chromaSensitivity * 402 / 255)) {
        return true;
    }
    return false;
}
uchar4 AlphaBlendForgroundOnto(const uchar4 bg, const uchar4 fc) {
    if (fc.a == 0) return bg;
    if (fc.a == 255) {
        return fc;
    }
    float a = fc.a;
    a /= 255; // 0 (transparent) - 1.0 (opague)
    float dr = fc.r * a + bg.r * (1.0f - a);
    float dg = fc.g * a + bg.g * (1.0f - a);
    float db = fc.b * a + bg.b * (1.0f - a);
    return {(uint8_t)dr, (uint8_t)dg, (uint8_t)db, bg.a};
}

kernel void NormalBlendFunction(constant LayerBlendingData &data,
                                device uchar4* result,
                                const device uchar4* src,
                                uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        fg.a = fg.a * data.fadeFactor * (1.0 - data.effectMixThreshold);
        uchar4 bg = result[index];
        result[index] = AlphaBlendForgroundOnto(bg, fg);
    }
}
kernel void Effect1_2_Function(constant LayerBlendingData &data,
                               device uchar4* result,
                               const device uchar4* src,
                               uint index [[thread_position_in_grid]]) {
    const int n = 0; // increase to change the curve of the crossfade
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float emt, emtNot;
        
        if (!data.effectMixVaries) {
            emt = data.effectMixThreshold;
            if ((emt > 0.000001) && (emt < 0.99999)) {
                emtNot = 1.0 - data.effectMixThreshold;
                // make cross-fade linear
                emt = cos((M_PI_F / 4.0) * (pow(2.0 * emt - 1.0, 2.0 * n + 1.0) + 1.0));
                emtNot = cos((M_PI_F / 4.0) * (pow(2.0 * emtNot - 1.0, 2.0 * n + 1.0) + 1.0));
            } else {
                emtNot = data.effectMixThreshold;
                emt = 1.0 - data.effectMixThreshold;
            }
        } else {
            emt = data.effectMixThreshold;
            emtNot = 1.0 - data.effectMixThreshold;
        }

        if (data.mixTypeData == 1) {
            fg = {(uchar)(fg.r * (emtNot)), (uchar)(fg.g * (emtNot)), (uchar)(fg.b * (emtNot))};
            bg = {(uchar)(bg.r * (emt)), (uchar)(bg.g * (emt)), (uchar)(bg.b * (emt))};
        } else {
            fg = {(uchar)(fg.r * (emt)), (uchar)(fg.g * (emt)), (uchar)(fg.b * (emt))};
            bg = {(uchar)(bg.r * (emtNot)), (uchar)(bg.g * (emtNot)), (uchar)(bg.b * (emtNot))};
        }
        result[index] = {(uchar)(fg.r + bg.r), (uchar)(fg.g + bg.g), (uchar)(fg.b + bg.b), 255};
    }
}
kernel void Mask1Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        float3 hsv0 = toHSV(fg);
        if (hsv0.z > data.effectMixThreshold) {
            result[index] = {0, 0, 0, 255};
        }
    }
}
kernel void Mask2Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];

        // second masks first
        float3 hsv1 = toHSV(bg);
        if (hsv1.z <= data.effectMixThreshold) {
            result[index] = fg;
        } else {
            result[index] = {0, 0, 0, 255};
        }        
    }
}

kernel void Unmask1Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float3 hsv0 = toHSV(fg);
        if (hsv0.z > data.effectMixThreshold) {
            float3 hsv1 = toHSV(bg);
            hsv1.z = hsv0.z;
            result[index] = fromHSV(hsv1);
        } else {
            result[index] = {0, 0, 0, 255};
        }
    }
}
kernel void Unmask2Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float3 hsv1 = toHSV(bg);
        if (hsv1.z > data.effectMixThreshold) {
            float3 hsv0 = toHSV(fg);
            // if effect 2 is non black
            hsv0.z = hsv1.z;
            result[index] = fromHSV(hsv0);
        } else {
            result[index] = {0, 0, 0, 255};
        }
    }
}
kernel void TrueUnmask1Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        float3 hsv0 = toHSV(fg);
        if (hsv0.z <= data.effectMixThreshold) {
            result[index] = {0, 0, 0, 255};
        }
    }
}
kernel void TrueUnmask2Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float3 hsv1 = toHSV(bg);
        if (hsv1.z > data.effectMixThreshold) {
            // if effect 2 is non black
            result[index] = fg;
        } else {
            result[index] = {0, 0, 0, 255};
        }
    }
}

kernel void Shadow_1on2Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        
        // Effect 1 shadows onto effect 2
        float3 hsv0 = toHSV(fg);
        float3 hsv1 = toHSV(bg);
        //   if (hsv0.value > effectMixThreshold[layer]) {
        // if effect 1 is non black
        //  to shadow we will shift the hue on the primary layer using the hue and brightness from the
        //  other layer
        if (hsv0.z > 0.0) {
            hsv1.x = hsv1.x + (hsv0.z * (hsv1.x - hsv0.x)) / 5.0;
        }
        // hsv1.value = hsv0.value;
        // hsv1.saturation = hsv0.saturation;
        result[index] = fromHSV(hsv1);
    }
}
kernel void Shadow_2on1Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        
        float3 hsv0 = toHSV(fg);
        float3 hsv1 = toHSV(bg);
        // if effect 1 is non black
        if (hsv1.z > 0.0) {
            hsv0.x = hsv0.x + (hsv1.z * (hsv0.x - hsv1.x)) / 2.0;
        }
        result[index] = fromHSV(hsv0);
    }
}
kernel void LayeredFunction(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float3 hsv1 = toHSV(bg);
        if (hsv1.z <= data.effectMixThreshold) {
            result[index] = fg;
        }
    }
}
kernel void AveragedFunction(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    const uchar3 BLACK = {0, 0, 0};
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        if (equals(bg, BLACK)) {
            result[index] = fg;
        } else if (!equals(fg, BLACK)) {
            result[index] = {uchar((fg.r + bg.r) / 2), uchar((fg.g + bg.g) / 2),
                    uchar((fg.b + bg.b) / 2), uchar((fg.a + bg.a) / 2)};
        }
    }
}

kernel void Reveal12Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float3 hsv0 = toHSV(fg);
        result[index] = hsv0.z > data.effectMixThreshold ? fg : bg; // if effect 1 is non black
    }
}
kernel void Reveal21Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float3 hsv1 = toHSV(bg);
        result[index] = hsv1.z > data.effectMixThreshold ? bg : fg; // if effect 2 is non black
    }
}

kernel void AdditiveFunction(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        
        int r = fg.r + bg.r;
        int g = fg.g + bg.g;
        int b = fg.b + bg.b;
        if (r > 255)
            r = 255;
        if (g > 255)
            g = 255;
        if (b > 255)
            b = 255;
        result[index] = {(uchar)r, (uchar)g, (uchar)b, 255};
    }
}
kernel void SubtractiveFunction(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        int r = bg.r - fg.r;
        int g = bg.g - fg.g;
        int b = bg.b - fg.b;
        if (r < 0)
            r = 0;
        if (g < 0)
            g = 0;
        if (b < 0)
            b = 0;
        result[index] = {(uchar)r, (uchar)g, (uchar)b, 255};
    }
}
kernel void MaxFunction(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float alpha = (float)fg.a / 255.0;
        int r = max(fg.r, bg.r) * alpha;
        int g = max(fg.g, bg.g) * alpha;
        int b = max(fg.b, bg.b) * alpha;
        result[index] = {(uchar)r, (uchar)g, (uchar)b, 255};
    }
}
kernel void MinFunction(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        
        float alpha = (float)fg.a / 255.0;
        int r = min(fg.r, bg.r) * alpha;
        int g = min(fg.g, bg.g) * alpha;
        int b = min(fg.b, bg.b) * alpha;
        result[index] = {(uchar)r, (uchar)g, (uchar)b, 255};
    }
}
kernel void AsBrightnessFunction(constant LayerBlendingData &data,
                                 device uchar4* result,
                                 const device uchar4* src,
                                 uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float alpha = (float)fg.a / 255.0;
        int r = fg.r * bg.r / 255 * alpha;
        int g = fg.g * bg.g / 255 * alpha;
        int b = fg.b * bg.b / 255 * alpha;
        result[index] = {(uchar)r, (uchar)g, (uchar)b, 255};
    }
}
kernel void HighlightFunction(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        bool effect1HasColor = (fg.r > 0 || fg.g > 0 || fg.b > 0);
        bool effect2HasColor = (bg.r > 0 || bg.g > 0 || bg.b > 0);
        float3 hsv1 = toHSV(bg);

        if (effect1HasColor && (effect2HasColor || hsv1.z > data.effectMixThreshold)) {
            result[index] = fg;
        }
    }
}
kernel void HighlightVibrantFunction(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
        float3 hsv1 = toHSV(bg);
        if (hsv1.z > data.effectMixThreshold) {
            int r = fg.r + bg.r;
            int g = fg.g + bg.g;
            int b = fg.b + bg.b;

            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
            result[index] = {(uchar)r, (uchar)g, (uchar)b, 255};
        }
    }
}

kernel void BottomTopFunction(constant LayerBlendingData &data,
                              device uchar4* result,
                              const device uchar4* src,
                              const device int32_t* indexes,
                              uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        int32_t idx = indexes[index];
        if (idx == -1) {
            result[index] = {0, 0, 0, 0};
        } else {
            if (idx & 0x80000000) {
                idx &= 0x7FFFFFFF;
                idx = indexes[idx += 2];
            }
            //uint x = idx % data.bufferWi;
            uint y = idx / data.bufferWi;
            if (y < (data.bufferHi / 2)) {
                result[index] = fg;
            }
        }
    }
}
kernel void LeftRightFunction(constant LayerBlendingData &data,
                              device uchar4* result,
                              const device uchar4* src,
                              const device int32_t* indexes,
                              uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        int32_t idx = indexes[index];
        if (idx == -1) {
            result[index] = {0, 0, 0, 0};
        } else {
            if (idx & 0x80000000) {
                idx &= 0x7FFFFFFF;
                idx = indexes[idx += 2];
            }
            uint x = idx % data.bufferWi;
            //uint y = idx / data.bufferWi;
            if (x < data.bufferWi / 2) {
                result[index] = fg;
            }
        }
    }
}
/*
kernel void Function(constant LayerBlendingData &data,
                          device uchar4* result,
                          const device uchar4* src,
                          uint index [[thread_position_in_grid]]) {
    if (index > (uint)data.nodeCount) return;
    uchar4 fg = src[index];
    if (!data.isChromaKey || !applyChroma(data, fg)) {
        uchar4 bg = result[index];
    }
}
*/
