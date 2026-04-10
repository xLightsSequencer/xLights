/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC kernel for the Kaleidoscope effect.
// Implements the same per-pixel logic as KaleidoscopeFunctions.metal so that
// Windows and Linux get a SIMD-accelerated path that matches macOS Metal.
// The kernel reads from a source snapshot and writes to the result buffer,
// matching the Metal src/result pattern.

#define KALE_PI       3.14159265358979323846f
#define KALE_STYLE_SQUARE2  0
#define KALE_STYLE_6FOLD    1
#define KALE_STYLE_8FOLD    2
#define KALE_STYLE_12FOLD   3
#define KALE_STYLE_RADIAL   4

struct KaleidoscopeData {
    unsigned int width;
    unsigned int height;
    float cx;
    float cy;
    float size;     // halfSize for Square2; segment count for Radial; size for triangle types
    float rotRad;
    int   style;    // one of KALE_STYLE_* above
    int   maxIter;  // max reflections for triangle types
    // Triangle vertices (for 6-fold, 8-fold, 12-fold)
    float v0x, v0y;
    float v1x, v1y;
    float v2x, v2y;
};

// Triangle-wave reflection: fold v into [-halfSize, +halfSize].
// Mirrors ReflectCoord() from KaleidoscopeFunctions.metal.
static inline float ReflectCoord(float v, float halfSize) {
    float period = 4.0f * halfSize;
    float t = fmod(v + halfSize, period);
    if (t < 0.0f) t += period;
    if (t <= 2.0f * halfSize) {
        return t - halfSize;
    } else {
        return 3.0f * halfSize - t;
    }
}

// Signed distance of (px,py) relative to the directed line from (lx1,ly1) to (lx2,ly2).
static inline float SignedDist(float px, float py, float lx1, float ly1, float lx2, float ly2) {
    return (lx2 - lx1) * (py - ly1) - (ly2 - ly1) * (px - lx1);
}

// Reflect (px, py) across the line from (lx1,ly1) to (lx2,ly2).
static inline void ReflectPointAcrossLine(float& px, float& py,
                                          float lx1, float ly1,
                                          float lx2, float ly2) {
    float dx = lx2 - lx1;
    float dy = ly2 - ly1;
    float denom = dx * dx + dy * dy;
    float a = (dx * dx - dy * dy) / denom;
    float b = 2.0f * dx * dy / denom;
    float rx = px - lx1;
    float ry = py - ly1;
    px = a * rx + b * ry + lx1;
    py = b * rx - a * ry + ly1;
}

// Read from source snapshot and write to result.
// Only writes pixels that map to a different source location (matching CPU/Metal).
// Style dispatch is hoisted outside foreach so ISPC can vectorize each branch
// independently — a varying branch on data->style inside foreach prevents the
// compiler from emitting efficient SIMD for any of the three paths.
export void KaleidoscopeEffectISPC(const uniform KaleidoscopeData * uniform data,
                                   const uniform uint8<4> * uniform src,
                                   uniform uint8<4> * uniform result,
                                   uniform int startIdx,
                                   uniform int endIdx)
{
    uniform int iwidth  = (int)data->width;
    uniform int iheight = (int)data->height;

    if (data->style == KALE_STYLE_SQUARE2) {
        uniform float halfSize = data->size;
        uniform float cosNeg   = cos(-data->rotRad);
        uniform float sinNeg   = sin(-data->rotRad);
        uniform float cosPos   = cos(data->rotRad);
        uniform float sinPos   = sin(data->rotRad);
        foreach (index = startIdx ... endIdx) {
            int x = index % iwidth;
            int y = index / iwidth;
            float dx = (float)x - data->cx;
            float dy = (float)y - data->cy;
            float lx = dx * cosNeg - dy * sinNeg;
            float ly = dx * sinNeg + dy * cosNeg;
            lx = ReflectCoord(lx, halfSize);
            ly = ReflectCoord(ly, halfSize);
            float wx = lx * cosPos - ly * sinPos + data->cx;
            float wy = lx * sinPos + ly * cosPos + data->cy;
            int sx = (int)floor(wx + 0.5f);
            int sy = (int)floor(wy + 0.5f);
            if (sx >= 0 && sx < iwidth && sy >= 0 && sy < iheight)
                if (sx != x || sy != y)
                    result[index] = src[sy * iwidth + sx];
        }
    } else if (data->style == KALE_STYLE_RADIAL) {
        uniform int   segments   = max(2, (int)data->size);
        uniform float wedgeAngle = 2.0f * KALE_PI / (float)segments;
        foreach (index = startIdx ... endIdx) {
            int x = index % iwidth;
            int y = index / iwidth;
            float dx = (float)x - data->cx;
            float dy = (float)y - data->cy;
            float radius = sqrt(dx * dx + dy * dy);
            if (radius < 0.5f) continue;
            float angle = atan2(dy, dx) - data->rotRad;
            angle = fmod(angle, 2.0f * KALE_PI);
            if (angle < 0.0f) angle += 2.0f * KALE_PI;
            float folded = fmod(angle, 2.0f * wedgeAngle);
            if (folded > wedgeAngle) folded = 2.0f * wedgeAngle - folded;
            folded += data->rotRad;
            int sx = (int)floor(data->cx + radius * cos(folded) + 0.5f);
            int sy = (int)floor(data->cy + radius * sin(folded) + 0.5f);
            if (sx >= 0 && sx < iwidth && sy >= 0 && sy < iheight)
                if (sx != x || sy != y)
                    result[index] = src[sy * iwidth + sx];
        }
    } else {
        // Triangle types: iterative reflection into the reference triangle
        // (styles: KALE_STYLE_6FOLD, KALE_STYLE_8FOLD, KALE_STYLE_12FOLD)
        uniform float v0x    = data->v0x, v0y = data->v0y;
        uniform float v1x    = data->v1x, v1y = data->v1y;
        uniform float v2x    = data->v2x, v2y = data->v2y;
        uniform int   maxIter = data->maxIter;
        foreach (index = startIdx ... endIdx) {
            int x = index % iwidth;
            int y = index / iwidth;
            float px = (float)x;
            float py = (float)y;
            for (int i = 0; i < maxIter; i++) {
                float d0 = SignedDist(px, py, v0x, v0y, v1x, v1y);
                float d1 = SignedDist(px, py, v1x, v1y, v2x, v2y);
                float d2 = SignedDist(px, py, v2x, v2y, v0x, v0y);
                if (d0 >= -0.5f && d1 >= -0.5f && d2 >= -0.5f) break;
                if (d0 < d1 && d0 < d2)
                    ReflectPointAcrossLine(px, py, v0x, v0y, v1x, v1y);
                else if (d1 < d0 && d1 < d2)
                    ReflectPointAcrossLine(px, py, v1x, v1y, v2x, v2y);
                else
                    ReflectPointAcrossLine(px, py, v2x, v2y, v0x, v0y);
            }
            int sx = (int)floor(px + 0.5f);
            int sy = (int)floor(py + 0.5f);
            if (sx >= 0 && sx < iwidth && sy >= 0 && sy < iheight)
                if (sx != x || sy != y)
                    result[index] = src[sy * iwidth + sx];
        }
    }
}
