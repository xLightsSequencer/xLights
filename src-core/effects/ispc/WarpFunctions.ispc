/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Warp effect.
// Implements the same per-pixel logic as the scalar WarpEffect.cpp transforms
// so that all platforms get a SIMD-accelerated render path.

#define WARP_WATER_DROPS        0
#define WARP_RIPPLE_IN          1
#define WARP_RIPPLE_OUT         2
#define WARP_DISSOLVE_IN        3
#define WARP_DISSOLVE_OUT       4
#define WARP_CIRCLE_REVEAL_IN   5
#define WARP_CIRCLE_REVEAL_OUT  6
#define WARP_BANDED_SWIRL_IN    7
#define WARP_BANDED_SWIRL_OUT   8
#define WARP_SINGLE_WATER_DROP  9
#define WARP_CIRCULAR_SWIRL     10
#define WARP_DROP               11
#define WARP_WAVY               12
#define WARP_MIRROR             13
#define WARP_COPY               14
#define WARP_FLIP               15

#ifndef PI
#define PI 3.14159265358979323846f
#endif

struct WarpData {
    unsigned int width;
    unsigned int height;
    int          warpMode;
    float        progress;
    float        cx;          // center x in [0,1]
    float        cy;          // center y in [0,1]
    float        speed;
    float        frequency;
    int          dissolveW;
    int          dissolveH;
};

// ---- texture helpers --------------------------------------------------------

static inline uint8<4> warpSampleClamped(
        const uniform uint8<4> * uniform src,
        uniform int w, uniform int h,
        float s, float t)
{
    s = clamp(s, 0.0f, 1.0f);
    t = clamp(t, 0.0f, 1.0f);
    int ix = (int)(s * (float)(w - 1));
    int iy = (int)(t * (float)(h - 1));
    return src[iy * w + ix];
}

static inline uint8<4> warpSampleBorder(
        const uniform uint8<4> * uniform src,
        uniform int w, uniform int h,
        float s, float t)
{
    uint8<4> black; black[0] = 0; black[1] = 0; black[2] = 0; black[3] = 255;
    if (s < 0.0f || s > 1.0f || t < 0.0f || t > 1.0f) return black;
    int ix = (int)(s * (float)(w - 1));
    int iy = (int)(t * (float)(h - 1));
    return src[iy * w + ix];
}

// Returns a normalised [0,1] scalar from the dissolve pattern at (s,t) in [0,1].
static inline float dissolveSample(
        const uniform uint8 * uniform dissolve,
        uniform int dw, uniform int dh,
        float s, float t)
{
    s = clamp(s, 0.0f, 1.0f);
    t = clamp(t, 0.0f, 1.0f);
    int ix = (int)(s * (float)(dw - 1));
    int iy = (int)(t * (float)(dh - 1));
    return (float)dissolve[iy * dw + ix] / 255.0f;
}

// ---- colour lerp ------------------------------------------------------------

static inline uint8<4> colorLerp(uint8<4> a, uint8<4> b, float t)
{
    uint8<4> out;
    out[0] = (uint8)((float)a[0] + t * ((float)b[0] - (float)a[0]));
    out[1] = (uint8)((float)a[1] + t * ((float)b[1] - (float)a[1]));
    out[2] = (uint8)((float)a[2] + t * ((float)b[2] - (float)a[2]));
    out[3] = (uint8)((float)a[3] + t * ((float)b[3] - (float)a[3]));
    return out;
}

// ---- warp-specific helpers --------------------------------------------------

// Cubic wave used by waterDrops and singleWaterDrop. Matches genWave() in C++.
static inline float warpGenWave(float len, float speed, float time)
{
    float wave = sin(speed * PI * len + time);
    wave = (wave + 1.0f) * 0.5f;
    wave -= 0.3f;
    wave *= wave * wave;
    return wave;
}

// Gradient noise used by wavy. Matches noise() / noiseVec() in C++.
// noiseVec(p) = Vec2D(c, c) where c = dissolveTex(p).red/255
// dot(noiseVec(gp), offset) = c * (offset.x + offset.y)
static inline float warpGradDot(
        const uniform uint8 * uniform dissolve,
        uniform int dw, uniform int dh,
        float gx, float gy,
        float ox, float oy)
{
    float g = dissolveSample(dissolve, dw, dh, gx, gy);
    return g * (ox + oy);
}

static inline float warpNoise(
        const uniform uint8 * uniform dissolve,
        uniform int dw, uniform int dh,
        float px, float py)
{
    float ix = floor(px);
    float iy = floor(py);
    float fx = px - ix;
    float fy = py - iy;

    // Hermite smoothstep
    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);

    float ab = warpGradDot(dissolve, dw, dh, ix,        iy,        fx,        fy);
    float cd = warpGradDot(dissolve, dw, dh, ix + 1.0f, iy,        fx - 1.0f, fy);
    float ef = warpGradDot(dissolve, dw, dh, ix,        iy + 1.0f, fx,        fy - 1.0f);
    float gh = warpGradDot(dissolve, dw, dh, ix + 1.0f, iy + 1.0f, fx - 1.0f, fy - 1.0f);

    float lo = ab + ux * (cd - ab);
    float hi = ef + ux * (gh - ef);
    return lo + uy * (hi - lo);
}

// Droplet height for singleWaterDrop. Matches getDropletHeight() in C++.
static inline float warpDropletHeight(
        float uvx, float uvy,
        float dropX, float dropY,
        float time)
{
    const float dropletExpandSpeed  = 1.5f;
    const float dropletHeightFactor = 0.3f;
    const float dropletRipple       = 60.0f;
    const float dropletStrengthBias = 0.6f;

    float dropFraction = time / 0.5f;
    dropFraction = dropFraction - floor(dropFraction);  // modf equivalent

    float ringRadius       = dropletExpandSpeed * dropFraction - dropletStrengthBias;
    float ddx              = uvx - dropX;
    float ddy              = uvy - dropY;
    float distToDroplet    = sqrt(ddx * ddx + ddy * ddy);

    float inside           = distToDroplet > ringRadius ? 0.0f : 1.0f;
    float dropletHeight    = distToDroplet * inside;
    dropletHeight = cos(PI + (dropletHeight - ringRadius) * dropletRipple) * 0.5f + 0.5f;
    dropletHeight *= 1.0f - dropFraction;
    float safeRadius = ringRadius > 0.0001f ? ringRadius : 0.0001f;
    dropletHeight *= inside * distToDroplet / safeRadius;

    return (1.0f - (cos(dropletHeight * PI) + 1.0f) * 0.5f) * dropletHeightFactor;
}

// ---- per-mode transforms ----------------------------------------------------

static inline uint8<4> applyWaterDrops(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        float s, float t,
        float progress, float cx, float cy, float speed)
{
    float time  = -progress * 35.0f;
    float pos2x = s - cx;
    float pos2y = t - cy;
    float len   = sqrt(pos2x * pos2x + pos2y * pos2y);
    float nnx = 0.0f, nny = 0.0f;
    if (len > 0.0001f) { nnx = pos2x / len; nny = pos2y / len; }
    float wave  = warpGenWave(len, speed, time);
    float scale = wave / (1.0f + 5.0f * len);
    return warpSampleClamped(src, w, h, s - nnx * scale, t - nny * scale);
}

static inline uint8<4> applyRipple(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        float s, float t,
        float progress, float cx, float cy, float speed, float frequency,
        bool inward)
{
    const float amplitude = 0.15f;
    float toux = s - cx, touy = t - cy;
    float dist = sqrt(toux * toux + touy * touy);
    float nnx = 0.0f, nny = 0.0f;
    if (dist > 0.0001f) { nnx = toux / dist; nny = touy / dist; }
    float wave   = cos(frequency * dist - speed * progress);
    float offset = progress * wave * amplitude;
    float newUVx = cx + nnx * (dist + offset);
    float newUVy = cy + nny * (dist + offset);
    uint8<4> c1 = warpSampleClamped(src, w, h, s, t);
    uint8<4> c2 = warpSampleClamped(src, w, h, newUVx, newUVy);
    return inward ? colorLerp(c2, c1, progress) : colorLerp(c1, c2, progress);
}

static inline uint8<4> applyDissolve(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        const uniform uint8 * uniform dissolve, uniform int dw, uniform int dh,
        float s, float t, float progress, bool inward)
{
    // Raw byte comparison matches the C++ unsigned char comparison exactly.
    int dissolveVal  = (int)(dissolveSample(dissolve, dw, dh, s, t) * 255.0f + 0.5f);
    int byteProgress = (int)(progress * 255.0f);
    uint8<4> black; black[0] = 0; black[1] = 0; black[2] = 0; black[3] = 255;
    bool show = inward ? (dissolveVal <= byteProgress) : (dissolveVal > byteProgress);
    return show ? warpSampleClamped(src, w, h, s, t) : black;
}

static inline uint8<4> applyCircleReveal(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        float s, float t,
        float progress, float cx, float cy, bool inward)
{
    const float FuzzyAmount = 0.04f;
    const float CircleSize  = 0.60f;
    float prog   = inward ? progress : (1.0f - progress);
    float radius = -FuzzyAmount + prog * (CircleSize + 2.0f * FuzzyAmount);
    float dx = s - cx, dy = t - cy;
    float fromCenter     = sqrt(dx * dx + dy * dy);
    float distFromCircle = fromCenter - radius;
    uint8<4> c     = warpSampleClamped(src, w, h, s, t);
    uint8<4> black; black[0] = 0; black[1] = 0; black[2] = 0; black[3] = 255;
    float p = clamp((distFromCircle + FuzzyAmount) / (2.0f * FuzzyAmount), 0.0f, 1.0f);
    return colorLerp(c, black, p);
}

static inline uint8<4> applyBandedSwirl(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        float s, float t,
        float progress, float cx, float cy, float frequency, bool inward)
{
    const float TwistAmount = 1.6f;
    float toux = s - cx, touy = t - cy;
    float dist = sqrt(toux * toux + touy * touy);
    float nnx = 1.0f, nny = 0.0f;
    if (dist > 0.0001f) { nnx = toux / dist; nny = touy / dist; }
    float angle = atan2(nny, nnx);
    float twist = inward ? TwistAmount * (1.0f - progress) : TwistAmount * progress;
    angle += sin(dist * frequency) * twist;
    float newUVx = cos(angle) * dist + cx;
    float newUVy = sin(angle) * dist + cy;
    uint8<4> c1 = warpSampleClamped(src, w, h, s, t);
    uint8<4> c2 = warpSampleClamped(src, w, h, newUVx, newUVy);
    return inward ? colorLerp(c1, c2, progress) : colorLerp(c2, c1, progress);
}

static inline uint8<4> applySingleWaterDrop(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        float s, float t,
        float progress, float cx, float cy)
{
    float pos2x = s - cx, pos2y = t - cy;
    float len   = sqrt(pos2x * pos2x + pos2y * pos2y);
    float nnx = 0.0f, nny = 0.0f;
    if (len > 0.0001f) { nnx = pos2x / len; nny = pos2y / len; }
    float dh    = warpDropletHeight(s - 0.5f, t - 0.5f, cx - 0.5f, cy - 0.5f, progress);
    float scale = dh / (1.0f + 3.0f * len);
    return warpSampleClamped(src, w, h, s - nnx * scale, t - nny * scale);
}

static inline uint8<4> applyCircularSwirl(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        float s, float t,
        float progress, float cx, float cy, float speed)
{
    float dirx = s - cx, diry = t - cy;
    float len    = sqrt(dirx * dirx + diry * diry);
    float radius = (1.0f - progress) * 0.70710678f;
    uint8<4> black; black[0] = 0; black[1] = 0; black[2] = 0; black[3] = 255;
    if (len >= radius) return black;

    float angle  = -speed * len * progress * PI;
    float cs = cos(angle), sn = sin(angle);
    float rotx   = dirx * cs + diry * sn;
    float roty   = -dirx * sn + diry * cs;
    float lerpT  = 1.0f - progress;
    float scaledx = rotx * lerpT + cx;
    float scaledy = roty * lerpT + cy;
    float newUVx  = cx + lerpT * (scaledx - cx);
    float newUVy  = cy + lerpT * (scaledy - cy);
    return warpSampleClamped(src, w, h, newUVx, newUVy);
}

static inline uint8<4> applyDrop(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        const uniform uint8 * uniform dissolve, uniform int dw, uniform int dh,
        float s, float t, float progress)
{
    const float notSoRandomY = 0.16f;
    float noiseVal = dissolveSample(dissolve, dw, dh, s, notSoRandomY);
    return warpSampleClamped(src, w, h, s, t + noiseVal * progress);
}

static inline uint8<4> applyWavy(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        const uniform uint8 * uniform dissolve, uniform int dw, uniform int dh,
        float s, float t, float progress, float speed)
{
    float time = speed * progress;
    float uvx  = s + 0.4f * warpNoise(dissolve, dw, dh, time + 0.3f * s, time + 0.3f * t);
    float uvy  = t + 0.5f * warpNoise(dissolve, dw, dh, time + 0.5f * s, time + 0.5f * t);
    return warpSampleBorder(src, w, h, uvx, uvy);
}

static inline uint8<4> applyMirror(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        float s, float t, float cx, float cy)
{
    float px = (s > cx) ? (2.0f * cx - s) : s;
    float py = (t > cy) ? (2.0f * cy - t) : t;
    return warpSampleBorder(src, w, h, px, py);
}

static inline uint8<4> applyCopy(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        float s, float t, float cx, float cy)
{
    float px = (s > cx) ? (s - cx) : s;
    float py = (t > cy) ? (t - cy) : t;
    return warpSampleBorder(src, w, h, px, py);
}

static inline uint8<4> applyFlip(
        const uniform uint8<4> * uniform src, uniform int w, uniform int h,
        float s, float t, float cx, float cy)
{
    int ix = (int)(s * (float)(w - 1));
    if (s <= cx && cx != 0.0f)
        ix = (int)((1.0f - s) * (float)(w - 1));

    int iy = (int)(t * (float)(h - 1));
    if (t <= cy && cy != 0.0f)
        iy = (int)((1.0f - t) * (float)(h - 1));

    ix = clamp(ix, 0, w - 1);
    iy = clamp(iy, 0, h - 1);
    return src[iy * w + ix];
}

// ---- main export ------------------------------------------------------------

export void WarpEffectISPC(
        const uniform WarpData * uniform data,
        uniform int startIdx,
        uniform int endIdx,
        const uniform uint8<4> * uniform src,   // snapshot of original pixels
        uniform uint8<4> * uniform dst,          // destination pixel buffer
        const uniform uint8 * uniform dissolve)  // DissolveTransitonPattern data
{
    uniform int   w    = (int)data->width;
    uniform int   h    = (int)data->height;
    uniform int   mode = data->warpMode;
    uniform float prog = data->progress;
    uniform float cx   = data->cx;
    uniform float cy   = data->cy;
    uniform float spd  = data->speed;
    uniform float freq = data->frequency;
    uniform int   dw   = data->dissolveW;
    uniform int   dh   = data->dissolveH;

    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;

    foreach (index = startIdx ... endIdx) {
        int   px = index % w;
        int   py = index / w;
        float s  = (float)px * invW;
        float t  = (float)py * invH;

        uint8<4> result;

        if (mode == WARP_WATER_DROPS) {
            result = applyWaterDrops(src, w, h, s, t, prog, cx, cy, spd);
        } else if (mode == WARP_RIPPLE_IN) {
            result = applyRipple(src, w, h, s, t, prog, cx, cy, spd, freq, true);
        } else if (mode == WARP_RIPPLE_OUT) {
            result = applyRipple(src, w, h, s, t, prog, cx, cy, spd, freq, false);
        } else if (mode == WARP_DISSOLVE_IN) {
            result = applyDissolve(src, w, h, dissolve, dw, dh, s, t, prog, true);
        } else if (mode == WARP_DISSOLVE_OUT) {
            result = applyDissolve(src, w, h, dissolve, dw, dh, s, t, prog, false);
        } else if (mode == WARP_CIRCLE_REVEAL_IN) {
            result = applyCircleReveal(src, w, h, s, t, prog, cx, cy, true);
        } else if (mode == WARP_CIRCLE_REVEAL_OUT) {
            result = applyCircleReveal(src, w, h, s, t, prog, cx, cy, false);
        } else if (mode == WARP_BANDED_SWIRL_IN) {
            result = applyBandedSwirl(src, w, h, s, t, prog, cx, cy, freq, true);
        } else if (mode == WARP_BANDED_SWIRL_OUT) {
            result = applyBandedSwirl(src, w, h, s, t, prog, cx, cy, freq, false);
        } else if (mode == WARP_SINGLE_WATER_DROP) {
            result = applySingleWaterDrop(src, w, h, s, t, prog, cx, cy);
        } else if (mode == WARP_CIRCULAR_SWIRL) {
            result = applyCircularSwirl(src, w, h, s, t, prog, cx, cy, spd);
        } else if (mode == WARP_DROP) {
            result = applyDrop(src, w, h, dissolve, dw, dh, s, t, prog);
        } else if (mode == WARP_WAVY) {
            result = applyWavy(src, w, h, dissolve, dw, dh, s, t, prog, spd);
        } else if (mode == WARP_MIRROR) {
            result = applyMirror(src, w, h, s, t, cx, cy);
        } else if (mode == WARP_COPY) {
            result = applyCopy(src, w, h, s, t, cx, cy);
        } else {  // WARP_FLIP
            result = applyFlip(src, w, h, s, t, cx, cy);
        }

        dst[index] = result;
    }
}
