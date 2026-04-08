/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernels for the Warp effect — one export per warp mode.
// Modes that only relocate whole pixels use uint32 gather/scatter.
// Modes that blend channels use uint8<4>.

#ifndef PI
#define PI 3.14159265358979323846f
#endif

struct WarpData {
    unsigned int width;
    unsigned int height;
    int          warpMode;     // unused by individual kernels, kept for struct compat
    float        progress;
    float        cx;
    float        cy;
    float        speed;
    float        frequency;
    int          dissolveW;
    int          dissolveH;
};

// ---------------------------------------------------------------------------
// uint32 helpers — for modes that just relocate whole pixels
// ---------------------------------------------------------------------------

static inline uint32 sampleClamped32(
        const uniform uint32 * uniform src,
        uniform int w, uniform int h,
        float s, float t)
{
    s = clamp(s, 0.0f, 1.0f);
    t = clamp(t, 0.0f, 1.0f);
    int ix = clamp((int)(s * (float)(w - 1) + 0.5f), 0, w - 1);
    int iy = clamp((int)(t * (float)(h - 1) + 0.5f), 0, h - 1);
    return src[iy * w + ix];
}

static inline uint32 sampleBorder32(
        const uniform uint32 * uniform src,
        uniform int w, uniform int h,
        float s, float t, uniform uint32 border)
{
    if (s < 0.0f || s > 1.0f || t < 0.0f || t > 1.0f) return border;
    int ix = clamp((int)(s * (float)(w - 1) + 0.5f), 0, w - 1);
    int iy = clamp((int)(t * (float)(h - 1) + 0.5f), 0, h - 1);
    return src[iy * w + ix];
}

// ---------------------------------------------------------------------------
// uint8<4> helpers — for modes that need per-channel math
// ---------------------------------------------------------------------------

static inline uint8<4> sampleClamped4(
        const uniform uint8<4> * uniform src,
        uniform int w, uniform int h,
        float s, float t)
{
    s = clamp(s, 0.0f, 1.0f);
    t = clamp(t, 0.0f, 1.0f);
    int ix = clamp((int)(s * (float)(w - 1) + 0.5f), 0, w - 1);
    int iy = clamp((int)(t * (float)(h - 1) + 0.5f), 0, h - 1);
    return src[iy * w + ix];
}

static inline uint8<4> colorLerp(uint8<4> a, uint8<4> b, float t)
{
    uint8<4> out;
    out[0] = (uint8)((float)a[0] + t * ((float)b[0] - (float)a[0]));
    out[1] = (uint8)((float)a[1] + t * ((float)b[1] - (float)a[1]));
    out[2] = (uint8)((float)a[2] + t * ((float)b[2] - (float)a[2]));
    out[3] = (uint8)((float)a[3] + t * ((float)b[3] - (float)a[3]));
    return out;
}

static inline uint8<4> makeBlack()
{
    uint8<4> c; c[0] = 0; c[1] = 0; c[2] = 0; c[3] = 255;
    return c;
}

static inline float dissolveSample(
        const uniform uint8 * uniform dissolve,
        uniform int dw, uniform int dh,
        float s, float t)
{
    s = clamp(s, 0.0f, 1.0f);
    t = clamp(t, 0.0f, 1.0f);
    int ix = clamp((int)(s * (float)(dw - 1) + 0.5f), 0, dw - 1);
    int iy = clamp((int)(t * (float)(dh - 1) + 0.5f), 0, dh - 1);
    return (float)dissolve[iy * dw + ix] / 255.0f;
}

// ---------------------------------------------------------------------------
// genWave — shared by water drops and single water drop
// ---------------------------------------------------------------------------

static inline float genWave(float len, float speed, float time)
{
    float wave = sin(speed * PI * len + time);
    wave = (wave + 1.0f) * 0.5f;
    wave -= 0.3f;
    wave *= wave * wave;
    return wave;
}

// ===== uint32 kernels (whole-pixel relocation) =============================

export void WarpWaterDrops(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint32 * uniform src,
        uniform uint32 * uniform dst)
{
    uniform int   w   = (int)data->width;
    uniform int   h   = (int)data->height;
    uniform float spd = data->speed;
    uniform float prog = data->progress;
    uniform float cx  = data->cx;
    uniform float cy  = data->cy;
    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;

    foreach (index = startIdx ... endIdx) {
        float s = (float)(index % w) * invW;
        float t = (float)(index / w) * invH;

        float time  = -prog * 35.0f;
        float pos2x = s - cx;
        float pos2y = t - cy;
        float len   = sqrt(pos2x * pos2x + pos2y * pos2y);
        float nnx = 0.0f, nny = 0.0f;
        if (len > 0.0001f) { nnx = pos2x / len; nny = pos2y / len; }
        float wave  = genWave(len, spd, time);
        float scale = wave / (1.0f + 5.0f * len);

        dst[index] = sampleClamped32(src, w, h, s - nnx * scale, t - nny * scale);
    }
}

export void WarpSingleWaterDrop(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint32 * uniform src,
        uniform uint32 * uniform dst)
{
    uniform int   w   = (int)data->width;
    uniform int   h   = (int)data->height;
    uniform float prog = data->progress;
    uniform float cx  = data->cx;
    uniform float cy  = data->cy;
    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;

    foreach (index = startIdx ... endIdx) {
        float s = (float)(index % w) * invW;
        float t = (float)(index / w) * invH;

        float pos2x = s - cx, pos2y = t - cy;
        float len = sqrt(pos2x * pos2x + pos2y * pos2y);
        float nnx = 0.0f, nny = 0.0f;
        if (len > 0.0001f) { nnx = pos2x / len; nny = pos2y / len; }

        // droplet height
        float dropTime = prog / 0.5f;
        dropTime = dropTime - floor(dropTime);
        float ringRadius = 1.5f * dropTime - 0.6f;
        float ddx = (s - 0.5f) - (cx - 0.5f);
        float ddy = (t - 0.5f) - (cy - 0.5f);
        float distToDrop = sqrt(ddx * ddx + ddy * ddy);
        float inside = distToDrop > ringRadius ? 0.0f : 1.0f;
        float dh = distToDrop * inside;
        dh = cos(PI + (dh - ringRadius) * 60.0f) * 0.5f + 0.5f;
        dh *= 1.0f - dropTime;
        float safeR = ringRadius > 0.0001f ? ringRadius : 0.0001f;
        dh *= inside * distToDrop / safeR;
        dh = (1.0f - (cos(dh * PI) + 1.0f) * 0.5f) * 0.3f;

        float scale = dh / (1.0f + 3.0f * len);
        dst[index] = sampleClamped32(src, w, h, s - nnx * scale, t - nny * scale);
    }
}

export void WarpMirror(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint32 * uniform src,
        uniform uint32 * uniform dst)
{
    uniform int w  = (int)data->width;
    uniform int h  = (int)data->height;
    uniform int cx_px = (int)(data->cx * (float)(w - 1) + 0.5f);
    uniform int cy_px = (int)(data->cy * (float)(h - 1) + 0.5f);
    uniform uint32 black = 0xFF000000;

    foreach (index = startIdx ... endIdx) {
        int px = index % w;
        int py = index / w;
        int sx = (px > cx_px) ? (2 * cx_px - px) : px;
        int sy = (py > cy_px) ? (2 * cy_px - py) : py;
        if (sx < 0 || sx >= w || sy < 0 || sy >= h)
            dst[index] = black;
        else
            dst[index] = src[sy * w + sx];
    }
}

export void WarpCopy(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint32 * uniform src,
        uniform uint32 * uniform dst)
{
    uniform int w  = (int)data->width;
    uniform int h  = (int)data->height;
    uniform int cx_px = (int)(data->cx * (float)(w - 1) + 0.5f);
    uniform int cy_px = (int)(data->cy * (float)(h - 1) + 0.5f);
    uniform uint32 black = 0xFF000000;

    foreach (index = startIdx ... endIdx) {
        int px = index % w;
        int py = index / w;
        int sx = (px > cx_px) ? (px - cx_px) : px;
        int sy = (py > cy_px) ? (py - cy_px) : py;
        if (sx < 0 || sx >= w || sy < 0 || sy >= h)
            dst[index] = black;
        else
            dst[index] = src[sy * w + sx];
    }
}

export void WarpFlip(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint32 * uniform src,
        uniform uint32 * uniform dst)
{
    uniform int w  = (int)data->width;
    uniform int h  = (int)data->height;
    uniform int cx_px = (int)(data->cx * (float)(w - 1) + 0.5f);
    uniform int cy_px = (int)(data->cy * (float)(h - 1) + 0.5f);

    foreach (index = startIdx ... endIdx) {
        int px = index % w;
        int py = index / w;
        int sx = (px <= cx_px && cx_px != 0) ? (w - 1 - px) : px;
        int sy = (py <= cy_px && cy_px != 0) ? (h - 1 - py) : py;
        sx = clamp(sx, 0, w - 1);
        sy = clamp(sy, 0, h - 1);
        dst[index] = src[sy * w + sx];
    }
}

export void WarpDrop(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint32 * uniform src,
        uniform uint32 * uniform dst,
        const uniform uint8 * uniform dissolve)
{
    uniform int   w  = (int)data->width;
    uniform int   h  = (int)data->height;
    uniform float prog = data->progress;
    uniform int   dw = data->dissolveW;
    uniform int   dh = data->dissolveH;
    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;

    foreach (index = startIdx ... endIdx) {
        float s = (float)(index % w) * invW;
        float t = (float)(index / w) * invH;
        float noiseVal = dissolveSample(dissolve, dw, dh, s, 0.16f);
        dst[index] = sampleClamped32(src, w, h, s, t + noiseVal * prog);
    }
}

// ===== uint8<4> kernels (per-channel math) =================================

export void WarpRipple(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint8<4> * uniform src,
        uniform uint8<4> * uniform dst,
        uniform bool inward)
{
    uniform int   w    = (int)data->width;
    uniform int   h    = (int)data->height;
    uniform float prog = data->progress;
    uniform float cx   = data->cx;
    uniform float cy   = data->cy;
    uniform float spd  = data->speed;
    uniform float freq = data->frequency;
    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;

    foreach (index = startIdx ... endIdx) {
        float s = (float)(index % w) * invW;
        float t = (float)(index / w) * invH;
        float toux = s - cx, touy = t - cy;
        float dist = sqrt(toux * toux + touy * touy);
        float nnx = 0.0f, nny = 0.0f;
        if (dist > 0.0001f) { nnx = toux / dist; nny = touy / dist; }
        float wave   = cos(freq * dist - spd * prog);
        float offset = prog * wave * 0.15f;
        float newUVx = cx + nnx * (dist + offset);
        float newUVy = cy + nny * (dist + offset);
        uint8<4> c1 = sampleClamped4(src, w, h, s, t);
        uint8<4> c2 = sampleClamped4(src, w, h, newUVx, newUVy);
        dst[index] = inward ? colorLerp(c2, c1, prog) : colorLerp(c1, c2, prog);
    }
}

export void WarpDissolve(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint8<4> * uniform src,
        uniform uint8<4> * uniform dst,
        const uniform uint8 * uniform dissolve,
        uniform bool inward)
{
    uniform int   w    = (int)data->width;
    uniform int   h    = (int)data->height;
    uniform float prog = data->progress;
    uniform int   dw   = data->dissolveW;
    uniform int   dh   = data->dissolveH;
    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;

    foreach (index = startIdx ... endIdx) {
        float s = (float)(index % w) * invW;
        float t = (float)(index / w) * invH;
        int dissolveVal  = (int)(dissolveSample(dissolve, dw, dh, s, t) * 255.0f + 0.5f);
        int byteProgress = (int)(prog * 255.0f);
        bool show = inward ? (dissolveVal <= byteProgress) : (dissolveVal > byteProgress);
        dst[index] = show ? sampleClamped4(src, w, h, s, t) : makeBlack();
    }
}

export void WarpCircleReveal(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint8<4> * uniform src,
        uniform uint8<4> * uniform dst,
        uniform bool inward)
{
    uniform int   w    = (int)data->width;
    uniform int   h    = (int)data->height;
    uniform float prog = data->progress;
    uniform float cx   = data->cx;
    uniform float cy   = data->cy;
    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;

    foreach (index = startIdx ... endIdx) {
        float s = (float)(index % w) * invW;
        float t = (float)(index / w) * invH;
        float p = inward ? prog : (1.0f - prog);
        float radius = -0.04f + p * (0.60f + 0.08f);
        float dx = s - cx, dy = t - cy;
        float fromCenter = sqrt(dx * dx + dy * dy);
        float distFromCircle = fromCenter - radius;
        uint8<4> c = sampleClamped4(src, w, h, s, t);
        float blend = clamp((distFromCircle + 0.04f) / 0.08f, 0.0f, 1.0f);
        dst[index] = colorLerp(c, makeBlack(), blend);
    }
}

export void WarpBandedSwirl(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint8<4> * uniform src,
        uniform uint8<4> * uniform dst,
        uniform bool inward)
{
    uniform int   w    = (int)data->width;
    uniform int   h    = (int)data->height;
    uniform float prog = data->progress;
    uniform float cx   = data->cx;
    uniform float cy   = data->cy;
    uniform float freq = data->frequency;
    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;

    foreach (index = startIdx ... endIdx) {
        float s = (float)(index % w) * invW;
        float t = (float)(index / w) * invH;
        float toux = s - cx, touy = t - cy;
        float dist = sqrt(toux * toux + touy * touy);
        float nnx = 1.0f, nny = 0.0f;
        if (dist > 0.0001f) { nnx = toux / dist; nny = touy / dist; }
        float angle = atan2(nny, nnx);
        float twist = inward ? 1.6f * (1.0f - prog) : 1.6f * prog;
        angle += sin(dist * freq) * twist;
        float newUVx = cos(angle) * dist + cx;
        float newUVy = sin(angle) * dist + cy;
        uint8<4> c1 = sampleClamped4(src, w, h, s, t);
        uint8<4> c2 = sampleClamped4(src, w, h, newUVx, newUVy);
        dst[index] = inward ? colorLerp(c1, c2, prog) : colorLerp(c2, c1, prog);
    }
}

export void WarpCircularSwirl(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint8<4> * uniform src,
        uniform uint8<4> * uniform dst)
{
    uniform int   w    = (int)data->width;
    uniform int   h    = (int)data->height;
    uniform float prog = data->progress;
    uniform float cx   = data->cx;
    uniform float cy   = data->cy;
    uniform float spd  = data->speed;
    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;

    foreach (index = startIdx ... endIdx) {
        float s = (float)(index % w) * invW;
        float t = (float)(index / w) * invH;
        float dirx = s - cx, diry = t - cy;
        float len = sqrt(dirx * dirx + diry * diry);
        float radius = (1.0f - prog) * 0.70710678f;

        if (len >= radius) {
            dst[index] = makeBlack();
        } else {
            float angle = -spd * len * prog * PI;
            float cs = cos(angle), sn = sin(angle);
            float rotx = dirx * cs + diry * sn;
            float roty = -dirx * sn + diry * cs;
            float lerpT = 1.0f - prog;
            float scaledx = rotx * lerpT + cx;
            float scaledy = roty * lerpT + cy;
            float newUVx = cx + lerpT * (scaledx - cx);
            float newUVy = cy + lerpT * (scaledy - cy);
            dst[index] = sampleClamped4(src, w, h, newUVx, newUVy);
        }
    }
}

// Wavy needs noise from the dissolve pattern
static inline float warpGradDot(
        const uniform uint8 * uniform dissolve,
        uniform int dw, uniform int dh,
        float gx, float gy, float ox, float oy)
{
    return dissolveSample(dissolve, dw, dh, gx, gy) * (ox + oy);
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

export void WarpWavy(
        const uniform WarpData * uniform data,
        uniform int startIdx, uniform int endIdx,
        const uniform uint32 * uniform src,
        uniform uint32 * uniform dst,
        const uniform uint8 * uniform dissolve)
{
    uniform int   w   = (int)data->width;
    uniform int   h   = (int)data->height;
    uniform float prog = data->progress;
    uniform float spd = data->speed;
    uniform int   dw  = data->dissolveW;
    uniform int   dh  = data->dissolveH;
    uniform float invW = w > 1 ? 1.0f / (float)(w - 1) : 0.0f;
    uniform float invH = h > 1 ? 1.0f / (float)(h - 1) : 0.0f;
    uniform uint32 black = 0xFF000000;

    foreach (index = startIdx ... endIdx) {
        float s = (float)(index % w) * invW;
        float t = (float)(index / w) * invH;
        float time = spd * prog;
        float uvx = s + 0.4f * warpNoise(dissolve, dw, dh, time + 0.3f * s, time + 0.3f * t);
        float uvy = t + 0.5f * warpNoise(dissolve, dw, dh, time + 0.5f * s, time + 0.5f * t);
        dst[index] = sampleBorder32(src, w, h, uvx, uvy, black);
    }
}
