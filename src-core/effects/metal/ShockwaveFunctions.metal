//
//  ShockwaveFunctions.metal
//  EffectComputeFunctions


#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"

// Byte-exact replica of the ISPC shockwaveHsv2rgba (classic sector formula).
// The shared Metal hsv2rgb() helper (Pinwheel's) uses a different, compact
// GLSL K-vector formula that rounds differently, so calling it broke parity
// with the sub-threshold ISPC path in the blend + non-alpha branch. Only
// multiplies/subtracts/clamp/truncation here - none are fast-math-approximated
// - so this matches ISPC exactly.
static inline uchar4 shockwaveHsv2rgb(float h, float s, float v) {
    float r, g, b;
    if (s == 0.0f) {
        r = g = b = v;
    } else {
        float hh = (h - floor(h)) * 6.0f;
        int   i  = (int)hh;
        float f  = hh - (float)i;
        float p  = v * (1.0f - s);
        float q  = v * (1.0f - s * f);
        float t2 = v * (1.0f - s * (1.0f - f));
        if      (i == 0) { r = v;  g = t2; b = p;  }
        else if (i == 1) { r = q;  g = v;  b = p;  }
        else if (i == 2) { r = p;  g = v;  b = t2; }
        else if (i == 3) { r = p;  g = q;  b = v;  }
        else if (i == 4) { r = t2; g = p;  b = v;  }
        else             { r = v;  g = p;  b = q;  }
    }
    uchar4 out;
    out.r = (uchar)(clamp(r, 0.0f, 1.0f) * 255.0f);
    out.g = (uchar)(clamp(g, 0.0f, 1.0f) * 255.0f);
    out.b = (uchar)(clamp(b, 0.0f, 1.0f) * 255.0f);
    out.a = 255;
    return out;
}

kernel void ShockwaveEffectStyle0(constant MetalShockwaveData &data,
                                  device uchar4* result,
                                  uint index [[thread_position_in_grid]])
{
    if (index > (data.width * data.height)) return;
    int x = index % data.width;
    int y = index / data.width;
    float x1 = x - data.xc_adj;
    float y1 = y - data.yc_adj;
    // precise::sqrt, NOT length() or default sqrt(): Metal compiles with
    // fast-math, under which both length() and the default sqrt() lower to an
    // approximate reciprocal-sqrt sequence that diverges from the ISPC/CPU
    // correctly-rounded sqrt by a ULP at the ring boundary, flipping edge
    // pixels' color_pct (byte-level parity break vs the sub-threshold ISPC
    // path). precise::sqrt forces the IEEE correctly-rounded result.
    float r = precise::sqrt(x1 * x1 + y1 * y1);
    if (r >= data.radius1 && r <= data.radius2) {
        if (data.blend > 0) {
            uchar4 ncolor = data.color;
            // precise::divide: fast-math turns a/b into a*approx_recip(b), which
            // diverges from the ISPC/CPU true divide by a ULP and shifts the
            // blended alpha/value byte. Forcing a real divide keeps parity.
            float color_pct = 1.0 - precise::divide(abs(r - data.radius_center), data.half_width);
            if (data.allowAlpha) {
                ncolor.a = 255.0 * color_pct;
            } else {
                simd::float3 hsv = data.colorHSV;
                ncolor = shockwaveHsv2rgb(hsv.x, hsv.y, hsv.z * color_pct);
            }
            result[index] = ncolor;
        } else {
            result[index] = data.color;
        }
    }
}
