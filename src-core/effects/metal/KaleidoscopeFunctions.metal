#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"

constant float KALE_PI = 3.14159265358979323846f;

// Triangle-wave reflection: fold v into [-halfSize, +halfSize]
inline float ReflectCoord(float v, float halfSize) {
    float period = 4.0f * halfSize;
    v = fmod(v + halfSize, period);
    if (v < 0.0f)
        v += period;
    if (v <= 2.0f * halfSize) {
        return v - halfSize;
    } else {
        return 3.0f * halfSize - v;
    }
}

// Signed distance of point (px,py) relative to directed line from (lx1,ly1) to (lx2,ly2).
// Positive = left side (inside for CCW triangle), negative = outside.
inline float SignedDist(float px, float py, float lx1, float ly1, float lx2, float ly2) {
    return (lx2 - lx1) * (py - ly1) - (ly2 - ly1) * (px - lx1);
}

// Reflect point (px, py) across the line from (lx1,ly1) to (lx2,ly2).
inline void ReflectPointAcrossLine(thread float& px, thread float& py,
                                    float lx1, float ly1, float lx2, float ly2) {
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

// Square 2: coordinate folding (no iteration needed)
kernel void KaleidoscopeEffectSquare2(constant KaleidoscopeData &data,
                                       const device uchar4* src,
                                       device uchar4* result,
                                       uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;

    uint didx = index.y * data.width + index.x;

    float px = float(index.x);
    float py = float(index.y);
    float halfSize = data.size;

    // Transform to local coordinate frame (centered, unrotated)
    float dx = px - data.cx;
    float dy = py - data.cy;
    float cosR = cos(-data.rotRad);
    float sinR = sin(-data.rotRad);
    float lx = dx * cosR - dy * sinR;
    float ly = dx * sinR + dy * cosR;

    // Fold both coordinates into [-halfSize, +halfSize]
    lx = ReflectCoord(lx, halfSize);
    ly = ReflectCoord(ly, halfSize);

    // Transform back to world coordinates
    float cosRF = cos(data.rotRad);
    float sinRF = sin(data.rotRad);
    float wx = lx * cosRF - ly * sinRF + data.cx;
    float wy = lx * sinRF + ly * cosRF + data.cy;

    int sx = int(round(wx));
    int sy = int(round(wy));

    if (sx >= 0 && sx < int(data.width) && sy >= 0 && sy < int(data.height)) {
        if (sx != int(index.x) || sy != int(index.y)) {
            result[didx] = src[sy * data.width + sx];
        }
    }
}

// Radial: polar folding into wedge segments
kernel void KaleidoscopeEffectRadial(constant KaleidoscopeData &data,
                                      const device uchar4* src,
                                      device uchar4* result,
                                      uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;

    uint didx = index.y * data.width + index.x;

    float dx = float(index.x) - data.cx;
    float dy = float(index.y) - data.cy;
    float radius = sqrt(dx * dx + dy * dy);

    if (radius < 0.5f) return;

    int segments = max(2, int(data.size));
    float wedgeAngle = 2.0f * KALE_PI / float(segments);

    float angle = atan2(dy, dx);
    angle -= data.rotRad;

    angle = fmod(angle, 2.0f * KALE_PI);
    if (angle < 0.0f)
        angle += 2.0f * KALE_PI;

    float folded = fmod(angle, 2.0f * wedgeAngle);
    if (folded > wedgeAngle)
        folded = 2.0f * wedgeAngle - folded;

    folded += data.rotRad;

    int sx = int(round(data.cx + radius * cos(folded)));
    int sy = int(round(data.cy + radius * sin(folded)));

    if (sx >= 0 && sx < int(data.width) && sy >= 0 && sy < int(data.height)) {
        if (sx != int(index.x) || sy != int(index.y)) {
            result[didx] = src[sy * data.width + sx];
        }
    }
}

// Triangle types (6-fold, 8-fold, 12-fold): iterative reflection into triangle
kernel void KaleidoscopeEffectTriangle(constant KaleidoscopeData &data,
                                        const device uchar4* src,
                                        device uchar4* result,
                                        uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;

    uint didx = index.y * data.width + index.x;

    float x = float(index.x);
    float y = float(index.y);

    float v0x = data.v[0].x, v0y = data.v[0].y;
    float v1x = data.v[1].x, v1y = data.v[1].y;
    float v2x = data.v[2].x, v2y = data.v[2].y;

    for (int i = 0; i < data.maxIter; i++) {
        float d0 = SignedDist(x, y, v0x, v0y, v1x, v1y);
        float d1 = SignedDist(x, y, v1x, v1y, v2x, v2y);
        float d2 = SignedDist(x, y, v2x, v2y, v0x, v0y);

        // Inside the triangle (small tolerance for rounding)
        if (d0 >= -0.5f && d1 >= -0.5f && d2 >= -0.5f) {
            break;
        }

        // Reflect across the edge we are most outside of
        if (d0 < d1 && d0 < d2) {
            ReflectPointAcrossLine(x, y, v0x, v0y, v1x, v1y);
        } else if (d1 < d2) {
            ReflectPointAcrossLine(x, y, v1x, v1y, v2x, v2y);
        } else {
            ReflectPointAcrossLine(x, y, v2x, v2y, v0x, v0y);
        }
    }

    int sx = int(round(x));
    int sy = int(round(y));

    if (sx >= 0 && sx < int(data.width) && sy >= 0 && sy < int(data.height)) {
        if (sx != int(index.x) || sy != int(index.y)) {
            result[didx] = src[sy * data.width + sx];
        }
    }
}
