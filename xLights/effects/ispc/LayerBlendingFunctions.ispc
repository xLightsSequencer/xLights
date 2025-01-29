
const float M_PI_F = 3.14159265358979;
const float M_PI_2_F = M_PI_F / 2.0f;
const float M_PI_4_F = M_PI_F / 4.0f;

//struct HSVColor {
//    float x;
//    float y;
//    float z;
//};
typedef float<3> HSVColor;

// faster divide by 255 for 0..65535 (uint16)
inline uint8 div_255_fast(uint16 x) {
    return (((x) + (((x) + 257) >> 8)) >> 8);
}
// faster divide by 3 for 0..1024
inline uint8 div_3_fast(uint16 x) {
    return (x * 683) >> 11;
}

inline uint8 red(const uint32 c) {
    return c & 0xFF;
}
inline uint8 green(const uint32 c) {
    return (c >> 8) & 0xFF;
}
inline uint8 blue(const uint32 c) {
    return (c >> 16) & 0xFF;
}
inline uint8 alpha(const uint32 c) {
    return (c >> 24) & 0xFF;
}
inline uint32 fromComponents(const uint8 r, const uint8 g, const uint8 b, const uint8 a) {
    return (((int32)a) << 24) | (((int32)b) << 16) |  (((int32)g) << 8) |  (int32)r;
}

inline HSVColor toHSV(const uint32 &c)
{
    float r = red(c) / 255.0;
    float g = green(c) / 255.0;
    float b = blue(c) / 255.0;

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
    HSVColor v;
    v.x = abs(K + (g - b) / (6.0 * chroma + 1e-20));
    v.y = chroma / (r + 1e-20);
    v.z = r;
    return v;
}


inline uint32 fromHSV(const HSVColor &hsv, uint8 alph = 255) {
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
    return fromComponents((uint8)(red * 255.0), (uint8)(green * 255.0), (uint8)(blue * 255.0), alph);
}

inline uint32 ApplyBrightness(const uint32 c, const uniform float b) {
    return fromComponents((uint8)(red(c) * b), (uint8)(green(c) * b), (uint8)(blue(c) * b), alpha(c));
}


export struct LayerBlendingData {
    uint32 startNode;
    uint32 endNode;
    
    uint32 nodeCount;
    uint32 bufferWi;
    uint32 bufferHi;
    
    bool useMask;
    
    float hueAdjust;
    float valueAdjust;
    float saturationAdjust;
    
    int brightness;
    int contrast;
    float fadeFactor;
    float effectMixThreshold;
    bool effectMixVaries;
    bool brightnessLevel;
    int mixTypeData;
    
    int outputSparkleCount;
    uint32 sparkleColor;
    
    bool isChromaKey;
    int chromaSensitivity;
    uint32 chromaColor;
};

const uniform uint32 ZERO_COLOR = 0;
const uniform uint32 BLACK_COLOR = 0xFF000000;

export void GetColorsISPCKernelSimple(uniform const LayerBlendingData &data,
                                      uniform uint32 result[],
                                      uniform const uint32 src[],
                                      uniform const uint8 mask[],
                                      uniform const uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 idx = indexes[index];
        if (idx == 0xFFFFFFFF) {
            result[index] = ZERO_COLOR;
        } else {
            result[index] = src[idx];
        }
    }
}
export void GetColorsISPCKernel(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                uniform const uint32 src[],
                                uniform const uint8 mask[],
                                uniform const uint32 indexes[]) {
                                
    uniform uint32 max = data.bufferHi * data.bufferWi;
    foreach (index = 0...data.nodeCount) {
        uint32 idx = indexes[index];
        
        if (idx == 0xFFFFFFFF) {
            result[index] = ZERO_COLOR;
        } else if (idx & 0x80000000) {
            result[index] = ZERO_COLOR;
            uint32 idx2 = idx & 0x7FFFFFFF;
            uint32 cnt = indexes[idx2];
            for (int nc = 0; nc < cnt; nc++) {
                uint32 nidx = indexes[idx2 + nc + 1];
                uint32 c = src[nidx];
                if (c & 0xFF000000) {
                    if (!data.useMask || !mask) {
                        result[index] = c;
                        nc = cnt;
                    } else {
                        int32 x = nidx / data.bufferHi;
                        int32 y = nidx - (x * data.bufferHi);
                        int32 midx = x * data.bufferHi + y;
                        if (mask[midx] == 0) {
                            result[index] = c;
                            nc = cnt;
                        }
                    }
                }
            }
        } else if (data.useMask && mask) {
            int32 x = idx / data.bufferHi;
            int32 y = idx - (x * data.bufferHi);
            int32 midx = x * data.bufferHi + y;

            if (mask[midx] > 0) {
                result[index] = ZERO_COLOR;
            } else {
                result[index] = src[idx];
            }
        } else {
            result[index] = src[idx];
        }
    }
}
export void PutColorsForNodesSimple(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                uniform const uint32 src[],
                                uniform const uint8 mask[],
                                uniform const uint32 indexes[])
{
    foreach (index = data.startNode...data.endNode) {
        uint32 idx = indexes[index];
        if (!(idx & 0x80000000)) {
            result[idx] = src[index];
        }
    }
}
export void PutColorsForNodes(uniform const LayerBlendingData &data,
                            uniform uint32 result[],
                            uniform const uint32 src[],
                            uniform const uint8 mask[],
                            uniform const uint32 indexes[])
{
    foreach (index = data.startNode...data.endNode) {
        uint32 idx = indexes[index];
        if (idx == 0xFFFFFFFF) {
            // not in the output
        } else if (idx & 0x80000000) {
            idx &= 0x7FFFFFFF;
            int cnt = indexes[idx++];
            for (int n = 0; n < cnt; n++) {
                int nidx = indexes[idx + n];
                if (nidx >= 0) {
                    if (data.useMask && mask) {
                        int32 x = nidx / data.bufferHi;
                        int32 y = nidx - (x * data.bufferHi);
                        int32 midx = x * data.bufferHi + y;
                        if (mask[midx] > 0) {
                            result[nidx] = ZERO_COLOR;
                        } else {
                            result[nidx] = src[index];
                        }
                    } else {
                        result[nidx] = src[index];
                    }
                }
            }
        } else if (data.useMask && mask) {
            int32 x = idx / data.bufferHi;
            int32 y = idx - (x * data.bufferHi);
            int32 midx = x * data.bufferHi + y;
            if (mask[midx] > 0) {
                result[idx] = ZERO_COLOR;
            } else {
                result[idx] = src[idx];
            }
        } else {
            result[idx] = src[index];
        }
    }
}

export void AdjustHSV(uniform const LayerBlendingData &data,
                      uniform uint32 result[])
{
    foreach (index = data.startNode...data.endNode) {
        uint32 color = result[index];
        HSVColor hsv = toHSV(color);

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
        result[index] = fromHSV(hsv, alpha(color));
    }
}

export void ApplySparkles(uniform const LayerBlendingData &data,
                          uniform uint32 result[],
                          uniform uint16 sparkles[])
{
    foreach (index = data.startNode...data.endNode) {

        uint32 c = result[index];
        if (c & 0xFFFFFF) {
            int sc = data.outputSparkleCount;
            uint16 sparkle = sparkles[index];
            
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
}

export void AdjustBrightnessContrast(uniform const LayerBlendingData &data,
                                    uniform uint32 result[]) {
    float b = data.brightness;
    float c = data.contrast;
    if (data.contrast != 0) {
    foreach (index = data.startNode...data.endNode) {
            uint32 color = result[index];
            // contrast is not 0, can handle brightness change at same time
            HSVColor hsv = toHSV(color);
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
            result[index] = fromHSV(hsv, alpha(color));
        }
    } else if (b != 100) {
    foreach (index = data.startNode...data.endNode) {
            uint32 color = result[index];
            // just brightness
            b /= 100.0f;
            float f = red(color) * b;
            uint8 red = (uint8)min(f, 255.0);
            f = green(color) * b;
            uint8 green = (uint8)min(f, 255.0);
            f = blue(color) * b;
            uint8 blue = (uint8)min(f, 255.0);
            result[index] = fromComponents(red, green, blue, alpha(color));
        }
    }
}
export void AdjustBrightnessLevel(uniform const LayerBlendingData &data,
                                    uniform uint32 result[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 color = result[index];
        uint8 c = 0;
        uint16 r = red(color);
        uint16 g = green(color);
        uint16 b = blue(color);
        if (r > 25) ++c;
        if (g > 25) ++c;
        if (b > 25) ++c;
        // c will either be 0, 1, 2, or 3.  0 and 1 result in no adjustment
        if (c == 2) {
            result[index] = fromComponents(r >> 1, g >> 1, b >> 1, alpha(color));
        } else if (c == 3) {
            result[index] = fromComponents(div_3_fast(r), div_3_fast(g), div_3_fast(b), alpha(color));
        }
    }
}

export void FirstLayerFade(uniform const LayerBlendingData &data,
                           uniform uint32 result[],
                           const uniform uint32 src[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 color = src[index];
        uint8 alph = alpha(color);
        if (data.fadeFactor != 1.0) {
            HSVColor hsv = toHSV(color);
            hsv.z *= data.fadeFactor;
            if (alph != 255) {
                hsv.z *= alph;
                hsv.z /= 255.0f;
            }
            color = fromHSV(hsv);
        }
        if (alph == 0) {
            result[index] = ZERO_COLOR;
        } else if (alph == 255) {
            result[index] = color;
        } else {
            float a = alph;
            a /= 255; // 0 (transparent) - 1.0 (opague)
            float dr = red(color) * a;
            float dg = green(color) * a;
            float db = blue(color) * a;
            result[index] = fromComponents(dr, dg, db, 255);
        }
    }
}

export void NonAlphaFade(uniform const LayerBlendingData &data,
                         uniform uint32 result[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 color = result[index];
        HSVColor hsv = toHSV(color);
        hsv.z *= data.fadeFactor;
        result[index] = fromHSV(hsv);
    }
}

float ColorDistance(const uint32 e1, const uniform uint32 e2) {
    int rmean = (red(e1) + red(e2)) >> 1;
    int r = red(e1) - red(e2);
    int g = green(e1) - green(e2);
    int b = blue(e1) - blue(e2);
    int f1 = (((512 + rmean) * r * r) >> 8);
    int f2 = (((767 - rmean) * b * b) >> 8);
    float f3 = f1 + 4 * g * g + f2;
    return sqrt(f3);
}

bool applyChroma(uniform const LayerBlendingData &data,
                 uint32 &c) {
    if (alpha(c) < 255) {
        float a = alpha(c);
        c = fromComponents(div_255_fast(red(c) * a), div_255_fast(green(c) * a), div_255_fast(blue(c) * a), 255);
    }
    if (ColorDistance(c, data.chromaColor) < (data.chromaSensitivity * 402 / 255)) {
        return true;
    }
    return false;
}
uint32 AlphaBlendForgroundOnto(const uint32 bg, const uint32 fc) {
    if (alpha(fc) == 0) {
        return bg;
    } else if (alpha(fc) == 255) {
        return fc;
    } else {
        float a = alpha(fc);
        a /= 255; // 0 (transparent) - 1.0 (opague)
        float dr = red(fc) * a + red(bg) * (1.0f - a);
        float dg = green(fc) * a + green(bg) * (1.0f - a);
        float db = blue(fc) * a + blue(bg) * (1.0f - a);
        return fromComponents(dr, dg, db, alpha(bg));
    }
}

export void NormalBlendFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            float a = alpha(fg) * data.fadeFactor * (1.0 - data.effectMixThreshold);
            fg = fromComponents(red(fg), green(fg), blue(fg), a);
            uint32 bg = result[index];
            result[index] = AlphaBlendForgroundOnto(bg, fg);
        }
    }
}

export void Effect1_2_Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    const int n = 0; // increase to change the curve of the crossfade
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
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
                fg = fromComponents((uint8)(red(fg) * (emtNot)), (uint8)(green(fg) * (emtNot)), (uint8)(blue(fg) * (emtNot)), 255);
                bg = fromComponents((uint8)(red(bg) * (emt)), (uint8)(green(bg) * (emt)), (uint8)(blue(bg) * (emt)), 255);
            } else {
                fg = fromComponents((uint8)(red(fg) * (emt)), (uint8)(green(fg) * (emt)), (uint8)(blue(fg) * (emt)), 255);
                bg = fromComponents((uint8)(red(bg) * (emtNot)), (uint8)(green(bg) * (emtNot)), (uint8)(blue(bg) * (emtNot)), 255);
            }
            result[index] = fromComponents((uint8)(red(fg) + red(bg)), (uint8)(green(fg) + green(bg)), (uint8)(blue(fg) + blue(bg)), 255);
        }
    }
}

export void Mask1Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            HSVColor hsv0 = toHSV(fg);
            if (hsv0.z > data.effectMixThreshold) {
                result[index] = BLACK_COLOR;
            }
        }
    }
}
export void Mask2Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];

            // second masks first
            HSVColor hsv1 = toHSV(bg);
            if (hsv1.z <= data.effectMixThreshold) {
                result[index] = fg;
            } else {
                result[index] = BLACK_COLOR;
            }
        }
    }
}
export void Unmask1Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            HSVColor hsv0 = toHSV(fg);
            if (hsv0.z > data.effectMixThreshold) {
                HSVColor hsv1 = toHSV(bg);
                hsv1.z = hsv0.z;
                result[index] = fromHSV(hsv1);
            } else {
                result[index] = BLACK_COLOR;
            }
        }
    }
}
export void Unmask2Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            HSVColor hsv1 = toHSV(bg);
            if (hsv1.z > data.effectMixThreshold) {
                HSVColor hsv0 = toHSV(fg);
                // if effect 2 is non black
                hsv0.z = hsv1.z;
                result[index] = fromHSV(hsv0);
            } else {
                result[index] = BLACK_COLOR;
            }
        }
    }
}
export void TrueUnmask1Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            HSVColor hsv0 = toHSV(fg);
            if (hsv0.z <= data.effectMixThreshold) {
                result[index] = BLACK_COLOR;
            }
        }
    }
}
export void TrueUnmask2Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            HSVColor hsv1 = toHSV(bg);
            if (hsv1.z > data.effectMixThreshold) {
                // if effect 2 is non black
                result[index] = fg;
            } else {
                result[index] = BLACK_COLOR;
            }
        }
    }
}
export void Shadow_1on2Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            
            // Effect 1 shadows onto effect 2
            HSVColor hsv0 = toHSV(fg);
            HSVColor hsv1 = toHSV(bg);
            //   if (hsv0.value > effectMixThreshold[layer]) {
            // if effect 1 is non black
            //  to shadow we will shift the hue on the primary layer using the hue and brightness from the
            //  other layer
            if (hsv0.z > 0.0) {
                hsv1.x = hsv1.x + (hsv0.z * (hsv1.x - hsv0.x)) / 5.0;
            }
            // hsv1.value = hsv0.value;
            // hsv1.saturation = hsv0.saturation;
            fromHSV(hsv1, bg);
            result[index] = bg;
        }
    }
}
export void Shadow_2on1Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            
            HSVColor hsv0 = toHSV(fg);
            HSVColor hsv1 = toHSV(bg);
            // if effect 1 is non black
            if (hsv1.z > 0.0) {
                hsv0.x = hsv0.x + (hsv1.z * (hsv0.x - hsv1.x)) / 2.0;
            }
            fromHSV(hsv0, bg);
            result[index] = bg;
        }
    }
}
export void LayeredFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            HSVColor hsv1 = toHSV(bg);
            if (hsv1.z <= data.effectMixThreshold) {
                result[index] = fg;
            }
        }
    }
}

export void Reveal12Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            HSVColor hsv0 = toHSV(fg);
            result[index] = hsv0.z > data.effectMixThreshold ? fg : bg; // if effect 1 is non black
        }
    }
}
export void Reveal21Function(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            HSVColor hsv1 = toHSV(bg);
            result[index] = hsv1.z > data.effectMixThreshold ? bg : fg; // if effect 2 is non black
        }
    }
}
export void AdditiveFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            
            int r = red(fg) + red(bg);
            int g = green(fg) + green(bg);
            int b = blue(fg) + blue(bg);
            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
            result[index] = fromComponents((uint8)r, (uint8)g, (uint8)b, 255);
        }
    }
}

export void SubtractiveFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            int r = red(bg) - red(fg);
            int g = green(bg) - green(fg);
            int b = blue(bg) - blue(fg);
            if (r < 0)
                r = 0;
            if (g < 0)
                g = 0;
            if (b < 0)
                b = 0;
            result[index] = fromComponents((uint8)r, (uint8)g, (uint8)b, 255);
        }
    }
}
export void MaxFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            float alpha = (float)alpha(fg) / 255.0;
            int r = max(red(fg), red(bg)) * alpha;
            int g = max(green(fg), green(bg)) * alpha;
            int b = max(blue(fg), blue(bg)) * alpha;
            result[index] = fromComponents((uint8)r, (uint8)g, (uint8)b, 255);
        }
    }
}

export void MinFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            
            float alpha = (float)alpha(fg) / 255.0;
            int r = min(red(fg), red(bg)) * alpha;
            int g = min(green(fg), green(bg)) * alpha;
            int b = min(blue(fg), blue(bg)) * alpha;
            result[index] = fromComponents((uint8)r, (uint8)g, (uint8)b, 255);
        }
    }
}
export void AsBrightnessFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            float alpha = (float)alpha(fg) / 255.0;
            int r = div_255_fast(red(fg) * red(bg) * alpha);
            int g = div_255_fast(green(fg) * green(bg) * alpha);
            int b = div_255_fast(blue(fg) * blue(bg) * alpha);
            result[index] = fromComponents((uint8)r, (uint8)g, (uint8)b, 255);
        }
    }
}
export void HighlightFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            bool effect1HasColor = (red(fg) > 0 || green(fg) > 0 || blue(fg) > 0);
            bool effect2HasColor = (red(bg) > 0 || green(bg) > 0 || blue(bg) > 0);
            HSVColor hsv1 = toHSV(bg);

            if (effect1HasColor && (effect2HasColor || hsv1.z > data.effectMixThreshold)) {
                result[index] = fg;
            }
        }
    }
}
export void HighlightVibrantFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            HSVColor hsv1 = toHSV(bg);
            if (hsv1.z > data.effectMixThreshold) {
                int r = red(fg) + red(bg);
                int g = green(fg) + green(bg);
                int b = blue(fg) + blue(bg);

                if (r > 255)
                    r = 255;
                if (g > 255)
                    g = 255;
                if (b > 255)
                    b = 255;
                result[index] = fromComponents((uint8)r, (uint8)g, (uint8)b, 255);
            }
        }
    }
}
export void BottomTopFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 idx = indexes[index];
            if (idx == 0xFFFFFFFF) {
                result[index] = ZERO_COLOR;
            } else {
                if (idx & 0x80000000) {
                    idx &= 0x7FFFFFFF;
                    idx = indexes[idx += 2];
                }
                //uint x = idx % data.bufferWi;
                uint y = idx / data.bufferWi;
                if (y < (data.bufferHi >> 1)) {
                    result[index] = fg;
                }
            }
        }
    }
}
export void LeftRightFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 idx = indexes[index];
            if (idx == 0xFFFFFFFF) {
                result[index] = ZERO_COLOR;
            } else {
                if (idx & 0x80000000) {
                    idx &= 0x7FFFFFFF;
                    idx = indexes[idx += 2];
                }
                uint x = idx % data.bufferWi;
                //uint y = idx / data.bufferWi;
                if (x < (data.bufferWi >> 1)) {
                    result[index] = fg;
                }
            }
        }
    }
}

bool isBlack(const uint32 &c) {
    return (c & 0xFFFFFF) == 0;
}
export void AveragedFunction(uniform const LayerBlendingData &data,
                                uniform uint32 result[],
                                const uniform uint32 src[],
                                const uniform uint32 indexes[]) {
    foreach (index = data.startNode...data.endNode) {
        uint32 fg = src[index];
        if (!data.isChromaKey || !applyChroma(data, fg)) {
            uint32 bg = result[index];
            if (isBlack(bg)) {
                result[index] = fg;
            } else if (!isBlack(fg)) {
                result[index] = fromComponents((uint8)((red(fg) + red(bg)) >> 1), (uint8)((green(fg) + green(bg)) >> 1),
                        (uint8)((blue(fg) + blue(bg)) >> 1), (uint8)((alpha(fg) + alpha(bg)) >> 1));
            }
        }
    }
}

