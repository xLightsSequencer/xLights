//
//  TransitionFunctions.metal
//  EffectComputeFunctions


#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"

extern float lerp(float a, float b, float progress);
extern uchar4 lerp(uchar4 a, uchar4 b, float progress);
float2 lerp(float2 a, float2 b, float progress) {
    float x = a.x + progress * (b.x - a.x);
    float y = a.y + progress * (b.y - a.y);
    return float2(x, y);
}

uchar4 tex2D(const device uchar4* src, float s, float t, uint32_t w, uint32_t h) {
    s = clamp(0.0, s, 1.0);
    t = clamp(0.0, t, 1.0);
    int x = int(round((s * (w - 1))));
    int y = int(round(t * (h - 1)));
    int idx = y * w + x;
    return src[idx];
}
uchar4 tex2D(const device uchar4* src, float2 st, uint32_t w, uint32_t h) {
    return tex2D(src, st.x, st.y, w, h);
}
float rand(int x, int y, int z) {
    int seed = x + y * 57 + z * 241;
    seed = (seed << 13) ^ seed;
    return (( 1.0 - ( (seed * (seed * seed * 15731 + 789221) + 1376312589) & 2147483647) / 1073741824.0f) + 1.0f) / 2.0f;
}

inline float interpolate(float x, float loIn, float loOut, float hiIn, float hiOut) {
   return ( loIn != hiIn )
      ? ( loOut + (hiOut - loOut) * ( (x-loIn)/(hiIn-loIn) ) )
      : ( (loOut + hiOut) / 2 );
}

float2 Rotate(float2 pt, float fAngle) {
    float cs = cos(fAngle);
    float sn = sin(fAngle);
    return float2(pt.x * cs + pt.y * sn, -pt.x * sn + pt.y * cs);
}
float2 RotateAbout(float2 pt1, float angle, float2 pt) {
    float2 p(pt1 - pt);
    p = Rotate(p, angle);
    return p + pt;
}
static bool isLeft(float2 a, float2 b, float2 test) {
    return ((b.x - a.x) * (test.y - a.y) - (b.y - a.y) * (test.x - a.x)) > 0;
}
kernel void wipeTransition(constant TransitionData &data,
                           device uchar* result,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.x * data.height + index.y;  // masks are different order than pixels
    
    float adjust = data.adjust;
    float factor = data.progress;
    bool reverse = data.reverse;

    if (reverse) {
        adjust += 50;
        if (adjust >= 100) {
            adjust -= 100;
        }
    }

    float angle = 2.0 * M_PI_F * adjust / 100.0;
    float slope = tan(angle);
    
    uint8_t m1 = 255;
    uint8_t m2 = 0;

    float curx = round(factor * ((float)data.width - 1.0));
    float cury = round(factor * ((float)data.height - 1.0));

    if (angle >= 0 && angle < M_PI_2_F) {
        curx = data.width - curx - 1;
        float tmp = m1;
        m1 = m2;
        m2 = tmp;
    } else if (angle >= M_PI_2_F && angle < M_PI_F) {
        curx = data.width - curx - 1;
        cury = data.height - cury - 1;
    } else if (angle >= M_PI_F && angle < (M_PI_F + M_PI_2_F)) {
        cury = data.height - cury - 1;
    } else {
        float tmp = m1;
        m1 = m2;
        m2 = tmp;
    }
    float endx = curx == -1 ? -5 : -1;
    float endy = slope * (endx - curx) + cury;
    if (slope > 999) {
        // nearly vertical
        endx = curx;
        endy = cury - 10;
    } else if (slope < -999) {
        // nearly vertical
        endx = curx;
        endy = cury + 10;
    }
    float2 start(curx, cury);
    float2 end(endx, endy);

    result[sidx] = isLeft(start, end, float2(index.x, index.y)) ? m1 : m2;;
}

kernel void clockTransition(constant TransitionData &data,
                            device uchar* result,
                            uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.x * data.height + index.y;  // masks are different order than pixels
    
    float adjust = data.adjust;
    float factor = data.progress;
    bool reverse = data.reverse;
    
    uint8_t m1 = 255;
    uint8_t m2 = 0;

    float startradians = 2.0 * M_PI_F * adjust / 100.0;
    float currentradians = 2.0 * M_PI_F * factor;
    if (reverse) {
        float tmp = startradians;
        startradians = startradians - currentradians;
        currentradians = tmp;
        if (startradians < 0) {
            startradians += 2.0f * M_PI_F;
            currentradians += 2.0f * M_PI_F;
        }
    } else {
        currentradians = startradians + currentradians;
    }

    float radianspixel;
    if (index.x - data.width / 2 == 0 && index.y - data.height / 2 == 0) {
        radianspixel = 0.0;
    } else {
        radianspixel = atan2(index.x - data.width / 2.0f,
                             index.y - data.height / 2.0f);
    }
    if (radianspixel < 0) {
        radianspixel += 2.0f * M_PI_F;
    }
    if (currentradians > 2.0f * M_PI_F && radianspixel < startradians) {
        radianspixel += 2.0f * M_PI_F;
    }
    bool s_lt_p = radianspixel > startradians;
    bool c_gt_p = radianspixel < currentradians;
    result[sidx] = (s_lt_p && c_gt_p) ? m2 : m1;
}

kernel void fromMiddleTransition(constant TransitionData &data,
                            device uchar* result,
                            uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.x * data.height + index.y;  // masks are different order than pixels
    
    float adjust = data.adjust;
    float factor = data.progress;
    bool reverse = data.reverse;
    
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    
    
    if (reverse) {
        factor = 1.0 - factor;
        m1 = 0;
        m2 = 255;
    }

    float w_2 = 0.5 * (float)data.width;
    float h_2 = 0.5 * (float)data.height;
    float2 p1(w_2, 0.0f);
    float2 p2(w_2, 500.0f); //  something sufficiently large so the rotations will work better

    float angle = interpolate(0.01 * adjust, 0.0, -M_PI_2_F, 1.0, M_PI_2_F);
    p1 = RotateAbout(p1, angle, float2(w_2, h_2));
    p2 = RotateAbout(p2, angle, float2(w_2, h_2));

    float p1_p2_len = distance(p2, p1);
    float y2_less_y1 = p2.y - p1.y;
    float x2_less_x1 = p2.x - p1.x;
    float offset = p2.x * p1.y - p2.y * p1.x;

    // find the distance from the line to the four corners and use the max for timing
    float distBR = abs(y2_less_y1 * (data.width - 1) - x2_less_x1 * 0 + offset) / p1_p2_len;
    float distUR = abs(y2_less_y1 * (data.width - 1) - x2_less_x1 * (data.height - 1) + offset) / p1_p2_len;
    float distBL = abs(y2_less_y1 * 0 - x2_less_x1 * 0 + offset) / p1_p2_len;
    float distUL = abs(y2_less_y1 * 0 - x2_less_x1 * (data.height - 1) + offset) / p1_p2_len;
    float len = max(max(distBR, distBL), max(distUR, distUL));
    float step = len * factor;

    float dist = abs(y2_less_y1 * index.x - x2_less_x1 * index.y + offset) / p1_p2_len;
    result[sidx] = (dist > step) ? m1 : m2;
}


kernel void squareExplodeTransition(constant TransitionData &data,
                                    device uchar* result,
                                    uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.x * data.height + index.y;  // masks are different order than pixels

    //float adjust = data.adjust;
    float factor = data.progress;
    bool reverse = data.reverse;

    if (data.out) {
        reverse = !reverse;
    }

    uint8_t m1 = 255;
    uint8_t m2 = 0;

    if (reverse) {
        factor = 1.0 - factor;
        m1 = 0;
        m2 = 255;
    }

    float xstep = ((float)data.width / 2.0) * (float)factor;
    float ystep = ((float)data.height / 2.0) * (float)factor;

    uint x1 = data.width / 2 - xstep;
    uint x2 = data.width / 2 + xstep;
    uint y1 = data.height / 2 - ystep;
    uint y2 = data.height / 2 + ystep;
    uint8_t c;
    if (index.x < x1 || index.x > x2 || index.y < y1 || index.y > y2) {
        c = m1;
    } else {
        c = m2;
    }
    result[sidx] = c;
}

kernel void circleExplodeTransition(constant TransitionData &data,
                                    device uchar* result,
                                    uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.x * data.height + index.y;  // masks are different order than pixels

    //float adjust = data.adjust;
    float factor = data.progress;
    bool reverse = data.reverse;

    if (data.out) {
        reverse = !reverse;
    }

    uint8_t m1 = 255;
    uint8_t m2 = 0;

    if (reverse) {
        factor = 1.0 - factor;
        m1 = 0;
        m2 = 255;
    }
    float maxradius = sqrt(((data.width / 2.0) * (data.width / 2.0)) + ((data.height / 2.0) * (data.height / 2.0)));
    float rad = maxradius * factor;
    float radius = sqrt((index.x - (data.width / 2.0)) * (index.x - (data.width / 2.0)) + (index.y - (data.height / 2.0)) * (index.y - (data.height / 2.0)));
    result[sidx] = radius < rad ? m2 : m1;
}

kernel void blindsTransition(constant TransitionData &data,
                                    device uchar* result,
                                    uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.x * data.height + index.y;  // masks are different order than pixels

    float factor = data.progress;
    bool reverse = data.reverse;

    uint8_t m1 = 255;
    uint8_t m2 = 0;

    int adjust = (data.width / 2.0) * data.adjust / 100.0;
    if (adjust < 1) {
        adjust = 1;
    }

    int per = data.width / adjust;
    if (per < 1) {
        per = 1;
    }

    float xp = index.x % per;
    if (reverse) {
        xp = (data.width - index.x - 1) % per;
    }
    xp = xp / (float)per;
    result[sidx] = xp < factor ? m2 : m1;
}
kernel void blendTransition(constant TransitionData &data,
                                    device uchar* result,
                                    uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.x * data.height + index.y;  // masks are different order than pixels

    float adjust = data.adjust;
    float factor = data.progress;

    uint8_t m1 = 255;
    uint8_t m2 = 0;
    
    adjust = 10 * adjust / 100;
    if (adjust < 1) {
        adjust = 1;
    }

    int x = index.x / adjust;
    int y = index.y / adjust;
    float rng = rand(x, y, 22);
    result[sidx] = (rng < factor) ? m2 : m1;
}


kernel void slideChecksTransition(constant TransitionData &data,
                                    device uchar* result,
                                    uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;

    float adjust = data.adjust;
    float factor = data.progress;
    bool reverse = data.reverse;
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    
    if (adjust < 2) {
        adjust = 2;
    }
    adjust = (max(data.width / 2.0, data.height / 2.0)) * adjust / 100.0;
    if (adjust < 2) {
        adjust = 2;
    }
    int xper = data.width * 2 / adjust;
    if (xper < 1) {
        xper = 1;
    }
    int yper = data.height / adjust;
    if (yper < 1) {
        yper = 1;
    }
    float step = (((float)xper * 2.0) * factor);

    int xb = index.x / xper;
    int xp = (index.x - xb * xper) % xper;
    int xpos = index.x;
    if (reverse) {
        xpos = data.width - index.x - 1;
    }

    int yb = index.y / yper;
    if (yb % 2) {
        if (xp >= (xper / 2)) {
            int xp2 = xp - xper / 2;
            result[xpos * data.height + index.y] = xp2 < step ? m2 : m1;
        } else {
            int step2 = step - (xper / 2);
            result[xpos * data.height + index.y] = xp < step2 ? m2 : m1;
        }
    } else {
        result[xpos * data.height + index.y] = xp < step ? m2 : m1;
    }
}

kernel void slideBarsTransition(constant TransitionData &data,
                                    device uchar* result,
                                uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    //uint sidx = index.x * data.height + index.y;  // masks are different order than pixels
    
    float adjust = data.adjust;
    float factor = data.progress;
    //bool reverse = data.reverse;
    uint8_t m1 = 255;
    uint8_t m2 = 0;

    if (adjust == 0) {
        adjust = 1;
    }
    adjust = (data.width / 2.0) * adjust / 100.0;
    if (adjust == 0) {
        adjust = 1;
    }
    int per = data.height / adjust;
    if (per < 1) {
        per = 1;
    }
    uint blinds = data.height / per;
    while (blinds * per < data.height) {
        blinds++;
    }

    float step = (float)data.width * factor;
    int blind = index.y / per;
    int xpos = index.x;
    if ((blind % 2 == 1) == data.out) {
        xpos = data.width - index.x - 1;
    }
    result[xpos * data.height + index.y] = index.x <= step ? m2 : m1;
}










// procedural white noise
float2 ShatterTransitionCode_hash2(float2 p) {
    float a = dot(p, float2(127.1, 311.7));
    float b = dot(p, float2(269.5, 183.3));
    float2 c(sin(a), sin(b));
    float2 d(4958.5453 * c);
    float dummy1, dummy2;
    return float2(modf(d.x, dummy1), modf(d.y, dummy2));
}

float2 ShatterTransitionCode_voronoi(float2 x) {
    float2 n;
    float nx, ny;
    float2 f(modf(x.x, nx), modf(x.y, ny));
    n.x = nx;
    n.y = ny;
    float2 mc;
    float md = 8.0;
    for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
            float2 g(i, j);
            float2 o(ShatterTransitionCode_hash2(n + g));
            float2 r(g + o - f);
            float d = dot(r, r);
            if (d < md) {
                md = d;
                mc = x + r;
            }
        }
    }
    return mc;
}
kernel void shatterTransition(constant TransitionData &data,
                              device uchar4* result,
                              const device uchar4* src,
                              const device uchar4* prevSrc,
                              uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float t = index.y / (data.height - 1.0);
    float s = index.x / (data.width - 1.0);
    
    float num = 8.;

    float2 texCentered(float2(s, t) - float2(0.5, 0.5));
    float ang = atan2(texCentered.y, texCentered.x);
    float a = data.progress * 8.0f;
    float originalLength = (-1. + sqrt(1. + 4. * a * length(texCentered))) / (2.0 * a);
    if (a == 0.) {
        originalLength = length(texCentered);
    }

    float2 originalLocation(float2(0.5, 0.5) + originalLength * float2(cos(ang), sin(ang)));
    // float2 ol( texCentered / (data.progress + 1. ) + float2( 0.5, 0.5 ) );
    float2 originalShard(ShatterTransitionCode_voronoi(num * originalLocation));
    float2 originalCenter(originalShard.x / num, originalShard.y / num);
    float ca = atan2(originalCenter.y - 0.5, originalCenter.x - 0.5);
    float originalCenterLength = length(originalCenter - (float2(0.5, 0.5)));
    float currentCenterLength = originalCenterLength + originalCenterLength * originalCenterLength * a;
    float2 currentCenter(float2(0.5, 0.5) + currentCenterLength * float2(cos(ca), sin(ca)));
    float2 c4((float2(s, t) - currentCenter) / (1.0 + 0.6 * data.progress) + originalCenter);
    float2 currentShard(ShatterTransitionCode_voronoi(num * c4));

    float transition = 1.;
    if (distance(originalShard, currentShard) < 0.0001) {
        transition = smoothstep(0.70, 1.0, data.progress);
    }

    uchar4 toColor = tex2D(src, c4.x, c4.y, data.width, data.height);
    uchar4 fromColor = {0, 0, 0, 255};
    if (data.hasPrev) {
        fromColor = tex2D(prevSrc, s, t, data.pWidth, data.pHeight);
    }
    result[sidx] = lerp(toColor, fromColor, transition);
}

kernel void starTransition(constant TransitionData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar4* prevSrc,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float progress = data.progress;
    if (data.reverse) {
        progress = 1.0 - progress;
    }
    // want to default to a 6-point star at 50%
    int numSegments = (data.adjust == 0) ? 1 : (1 + data.adjust / 10);

    // for this transition, we fudge the progress a bit b/c not much happens at the end
    progress = interpolate(progress, 0.0, 0.0, 1.0, 0.85);

    float t = float(index.y) / (data.height - 1);
    float s = float(index.x) / (data.width - 1);

    float2 xy(s, t);

    float angle = atan2(xy.y - 0.5, xy.x - 0.5) - 0.5 * M_PI_F;
    float radius = (cos(numSegments * angle) + 4.0) / 4.0;
    float difference = length(float2(xy - float2(0.5, 0.5)));

    bool useSrc = difference <= radius * progress;
    if (data.reverse) {
        useSrc = !useSrc;
    }
    if (useSrc) {
        result[sidx] = tex2D(src, xy, data.width, data.height);
    } else if (data.hasPrev) {
        result[sidx] = tex2D(prevSrc, xy, data.pWidth, data.pHeight);
    } else {
        result[sidx] = {0, 0, 0, 0};
    }
}

kernel void pinwheelTransition(constant TransitionData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar4* prevSrc,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;
    
    float adjust = floor(interpolate(data.adjust, 0., 3.0, 100., 10.0));
    float t = float(index.y) / (data.height - 1);
    float s = float(index.x) / (data.width - 1);
    
    float x = s - 0.5;
    float y = t - 0.5;
    if (t < 0.5) {
        y = -y;
        x = -x;
    }
    float arcTangent = atan2(y, x);
    float dummy;
    float toProgress = modf(arcTangent / M_PI_F * adjust, dummy);
    if (toProgress > data.progress) {
        result[sidx] = tex2D(src, s, t, data.width, data.height);
    } else if (data.hasPrev) {
        result[sidx] = tex2D(prevSrc, s, t, data.pWidth, data.pHeight);
    } else {
        result[sidx] = {0, 0, 0, 0};
    }
}



// code for bowTie transition
float check(float2 p1, float2 p2, float2 p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}
bool pointInTriangle(float2 pt, float2 p1, float2 p2, float2 p3) {
    bool b1 = check(pt, p1, p2) < 0.0;
    bool b2 = check(pt, p2, p3) < 0.0;
    bool b3 = check(pt, p3, p1) < 0.0;
    return b1 == b2 && b2 == b3;
}
constant float bowTieHeight = 0.5;
uchar4 bowTie_firstHalf(const device uchar4*  cb, const device uchar4* rb1, float2 uv, float progress, float adjust, constant TransitionData &data) {
    if (uv.y < 0.5) {
        float2 botLeft(0., progress - bowTieHeight);
        float2 botRight(1., progress - bowTieHeight);
        float2 tip(adjust, progress);
        if (pointInTriangle(uv, botLeft, botRight, tip)) {
            return tex2D(cb, uv, data.width, data.height);
        }
    } else {
        float2 topLeft(0., 1. - progress + bowTieHeight);
        float2 topRight(1., 1. - progress + bowTieHeight);
        float2 tip(adjust, 1. - progress);
        if (pointInTriangle(uv, topLeft, topRight, tip)) {
            return tex2D(cb, uv, data.width, data.height);
        }
    }
    if (data.hasPrev) {
        return tex2D(rb1, uv.x, uv.y, data.pWidth, data.pHeight);
    }
    return {0, 0, 0, 255};
}
uchar4 bowTie_secondHalf(const device uchar4*  cb, const device uchar4* rb1, float2 uv, float progress, float adjust, constant TransitionData &data) {
    if (uv.x > adjust) {
        float2 top(progress + bowTieHeight, 1.);
        float2 bot(progress + bowTieHeight, 0.);
        float2 tip(lerp(adjust, 1.0, 2.0 * (progress - 0.5)), 0.5);
        if (pointInTriangle(uv, top, bot, tip)) {
            if (data.hasPrev) {
                return tex2D(rb1, uv.x, uv.y, data.pWidth, data.pHeight);
            } else {
                return {0, 0, 0, 255};
            }
        }
    } else {
        float2 top(1.0 - progress - bowTieHeight, 1.);
        float2 bot(1.0 - progress - bowTieHeight, 0.);
        float2 tip(lerp(adjust, 0.0, 2.0 * (progress - 0.5)), 0.5);
        if (pointInTriangle(uv, top, bot, tip)) {
            if (data.hasPrev) {
                return tex2D(rb1, uv.x, uv.y, data.pWidth, data.pHeight);
            } else {
                return {0, 0, 0, 255};
            }
        }
    }
    return tex2D(cb, uv, data.width, data.height);
}

kernel void bowTieTransition(constant TransitionData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar4* prevSrc,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float bowTieAdjust = 0.01 * data.adjust;
    float t = float(index.y) / (data.height - 1);
    float s = float(index.x) / (data.width - 1);
    float2 xy(s, t);
    
    float progress = data.progress;
    if (data.reverse) {
        progress = 1.0 - progress;
    }
    result[sidx] = (progress < 0.5) ? bowTie_firstHalf(src, prevSrc, xy, progress, bowTieAdjust, data) : bowTie_secondHalf(src, prevSrc, xy, progress, bowTieAdjust, data);
}


// code for blobs transition
constant float blobsSmoothness = 0.01f;
constant float blobsSeed = 12.9898f;

float blobsRandom(float2 co) {
    float a = blobsSeed;
    float b = 78.233f;
    float c = 43758.5453f;
    float dt = dot(co, float2(a, b));
    float sn = dt - 3.14f * floor(dt / 3.14f);

    float intpart;
    return modf(sin(sn) * c, intpart);
}

float blobsNoise(float2 st) {
    float2 i, f;
    float ix, iy;
    f.x = modf(st.x, ix);
    f.y = modf(st.y, iy);
    i = {ix, iy};

    // Four corners in 2D of a tile
    float a = blobsRandom(i);
    float b = blobsRandom(i + float2(1.0, 0.0));
    float c = blobsRandom(i + float2(0.0, 1.0));
    float d = blobsRandom(i + float2(1.0, 1.0));

    // Cubic Hermine Curve.  Same as SmoothStep()
    float2 u(f * f * (3.0 - 2.0 * f));

    // Mix 4 coorners porcentages
    return lerp(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

kernel void blobsTransition(constant TransitionData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar4* prevSrc,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float t = index.y / (data.height - 1.0);
    float s = index.x / (data.width - 1.0);
    float scale = interpolate(data.adjust, 0., 4., 100., 14.);
    
    uchar4 fromColor = {0, 0, 0, 255};
    if (data.hasPrev){
        fromColor = tex2D(prevSrc, s, t, data.pWidth, data.pHeight);
    }
    uchar4 toColor(tex2D(src, s, t, data.width, data.height));
    float n = blobsNoise(scale * float2(s, t));

    float p = lerp(-blobsSmoothness, 1. - +blobsSmoothness, data.progress);
    float lo = p - blobsSmoothness;
    float hi = p + blobsSmoothness;

    float q = smoothstep(lo, hi, n);
    result[sidx] = lerp(fromColor, toColor, 1.f - q);
}

constant float CAMERA_DIST = 2.0;
kernel void foldTransition(constant TransitionData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar4* prevSrc,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float t = index.y / (data.height - 1.0);
    float s = index.x / (data.width - 1.0);
    float3 ray(s * 2 - 1, t * 2 - 1, CAMERA_DIST);
    float phi = data.progress * M_PI_F; // rotation angle
    
    // rotated basis vectors
    float3 rx(cos(phi), 0., data.reverse ? sin(phi) : -sin(phi));
    float3 ry(0., 1., 0.);
    float3 rz(-rx.z, 0., rx.x);

    // corner and direction vectors of "from" polygon
    float3 p0(-rx + float3(0., -1., CAMERA_DIST));
    float3 u0(rx);
    float3 v0(ry);
    float3 n0(rz);

    // ray-plane intersection
    float3 a0((dot(p0, n0) / dot(ray, n0)) * ray);
    float2 uv0(dot(a0 - p0, u0) / 2., dot(a0 - p0, v0) / 2.);

    if (data.out) {
        if (uv0.x >= 0. && uv0.x < 1. && uv0.y >= 0. && uv0.y < 1.) {
            if (!data.hasPrev) {
                result[sidx] = tex2D(src, uv0.x, uv0.y, data.width, data.height);
            } else if (data.progress < 0.5) {
                result[sidx] = tex2D(prevSrc, uv0.x, uv0.y, data.pWidth, data.pHeight);
            } else {
                uchar4 c = tex2D(src, 1 - uv0.x, uv0.y, data.width, data.height);
                if (c.a == 0) {
                    result[sidx] = tex2D(src, uv0.x, uv0.y, data.width, data.height);
                } else {
                    result[sidx] = c;
                }
            }
        } else {
            result[sidx] = {0 ,0, 0, 255};
        }
    } else {
        if (uv0.x >= 0. && uv0.x < 1. && uv0.y >= 0. && uv0.y < 1.) {
            if (!data.hasPrev) {
                result[sidx] = tex2D(src, 1 - uv0.x, uv0.y, data.width, data.height);
            } else if (data.progress < 0.5) {
                uchar4 c = tex2D(prevSrc, uv0.x, uv0.y, data.pWidth, data.pHeight);
                if (c.a == 0) {
                    result[sidx] = tex2D(src, 1 - uv0.x, uv0.y, data.width, data.height);
                } else {
                    result[sidx] = c;
                }
            } else {
                result[sidx] = tex2D(src, 1 - uv0.x, uv0.y, data.width, data.height);
            }
        } else {
            result[sidx] = {0, 0, 0, 255};
        }
    }
}

kernel void zoomTransition(constant TransitionData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar4* prevSrc,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float t = index.y / (data.height - 1.0);
    float s = index.x / (data.width - 1.0);
    float2 xy(s, t);
    float2 fv = ((xy - 0.5) * data.progress) + 0.5;
    result[sidx] = tex2D(src, fv, data.width, data.height);
}

kernel void circularSwirlTransition(constant TransitionData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar4* prevSrc,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;
    
    float2 xy(0.5, 0.5);
    float speed = interpolate(0.2, 0.0, 1.0, 40.0, 9.0);

    float t = index.y / (data.height - 1.0);
    float s = index.x / (data.width - 1.0);
    
    float2 uv(s, t);
    float2 dir(uv - xy);
    float len = length(dir);

    float radius = (1. - data.progress) * 0.70710678;
    if (len < radius) {
        float2 rotated(Rotate(dir, -speed * len * data.progress * M_PI_F));
        float2 scaled(rotated * (1. - data.progress) + xy);
        float2 newUV(lerp(xy, scaled, 1. - data.progress));
        result[sidx] = tex2D(src, newUV.x, newUV.y, data.width, data.height);
    } else {
        result[sidx] = {0, 0, 0, 255};
    }
}

constant float dw_reflection = 0.4f;
constant float dw_perspective = 0.4f;
constant float dw_depth = 3.f;
constant float2 dw_boundMin(0.0, 0.0);
constant float2 dw_boundMax(1.0, 1.0);
bool dwInBounds(float2 p) {
    return (dw_boundMin.x < p.x && dw_boundMin.y < p.y) && (p.x < dw_boundMax.x && p.y < dw_boundMax.y);
}
float2 project(float2 p) {
    return p * float2(1.0, -1.2) + float2(0.0, -0.02);
}
uchar4 bgColor(float2 p, float2 pto, const device uchar4* src, constant TransitionData &data) {
    uchar4 c = {0, 0, 0, 255};
    pto = project(pto);
    if (dwInBounds(pto)) {
        uchar4 toColor = tex2D(src, pto, data.width, data.height);
        c += lerp({0, 0, 0, 255}, toColor, dw_reflection * lerp(1.0, 0.0, pto.y));
    }
    return c;
}

kernel void doorwayTransition(constant TransitionData &data,
                              device uchar4* result,
                              const device uchar4* src,
                              const device uchar4* prevSrc,
                              uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;
    
    float t = index.y / (data.height - 1.0);
    float s = index.x / (data.width - 1.0);
    float progress = data.progress;
    
    float2 pfr(-1.);
    float2 pto(-1.);
    float2 p(s, t);
    float middleSlit = 2.0 * fabs(p.x - 0.5) - progress;
    if (middleSlit > 0.0) {
        pfr = p + (p.x > 0.5 ? -1.0 : 1.0) * float2(0.5 * progress, 0.0);
        float d = 1.0 / (1.0 + dw_perspective * progress * (1.0 - middleSlit));
        pfr.y -= d / 2.;
        pfr.y *= d;
        pfr.y += d / 2.;
    }
    float size = lerp(1.0, dw_depth, 1. - progress);
    pto = (p + float2(-0.5, -0.5)) * float2(size, size) + float2(0.5, 0.5);
    
    if (dwInBounds(pfr)) { // sliding left/right
        result[sidx] = (!data.hasPrev) ? uchar4{0, 0, 0, 255} : tex2D(prevSrc, pfr.x, pfr.y, data.pWidth, data.pHeight);
    } else if (dwInBounds(pto)) { // zooming in
        result[sidx] = tex2D(src, pto, data.width, data.height);
    } else {
        // reflection part
        result[sidx] = bgColor(p, pto, src, data);
    }
}

namespace SwapTransitionCode {
    constant float reflection = 0.4f;

    constant float2 boundMin(0.0, 0.0);
    constant float2 boundMax(1.0, 1.0);
    bool lessThan(float2 lhs, float2 rhs) {
        return lhs.x < rhs.x && lhs.y < rhs.y;
    }
    bool inBounds_for_swap(float2 p) {
        return lessThan(boundMin, p) && lessThan(p, boundMax);
    }
    float2 project_for_swap(float2 p) {
        return p * float2(1.0, -1.2) + float2(0.0, -0.02);
    }
    uchar4 bgColor(float2 p, float2 pfr, float2 pto, 
                   const device uchar4* src,
                   const device uchar4* prevSrc, constant TransitionData &data) {
        uchar4 c = {0, 0, 0, 255};

        float2 projectedPFR(project_for_swap(pfr));
        if (inBounds_for_swap(projectedPFR)) {
            c += lerp(uchar4(0, 0, 0, 255), !data.hasPrev ? uchar4(0, 0, 0, 255) : tex2D(prevSrc, projectedPFR.x, projectedPFR.y, data.pWidth, data.pHeight), reflection * lerp(1.0, 0.0, projectedPFR.y));
        }

        float2 projectedPTO(project_for_swap(pto));
        if (inBounds_for_swap(projectedPTO)) {
            c += lerp(uchar4(0, 0, 0, 255), tex2D(src, projectedPTO, data.width, data.height), reflection * lerp(1.0, 0.0, projectedPTO.y));
        }
        return c;
    }
    
    uchar4 swapTransition(constant TransitionData &data,
                          const device uchar4* src,
                          const device uchar4* prevSrc,
                          float t, float s) {
        const float depth = 3.0f;
        const float perspective = 0.4f;
        float progress = data.progress;
        float size = lerp(1.0, depth, progress);
        float persp = perspective * progress;

        float2 pto(-1.0, -1.0);
        float2 pfr((float2(s, t) + float2(-0.0, -0.5)) * float2(size / (1.0 - perspective * progress), size / (1.0 - size * persp * s)) + float2(0.0, 0.5));

        size = lerp(1.0, depth, 1. - progress);
        persp = perspective * (1. - progress);
        pto = (float2(s, t) + float2(-1.0, -0.5)) * float2(size / (1.0 - perspective * (1.0 - progress)), size / (1.0 - size * persp * (0.5 - s))) + float2(1.0, 0.5);

        if (progress < 0.5) {
            if (SwapTransitionCode::inBounds_for_swap(pfr))
                return (!data.hasPrev) ? uchar4(0, 0, 0, 255) : tex2D(prevSrc, pfr.x, pfr.y, data.pWidth, data.pHeight);
            if (SwapTransitionCode::inBounds_for_swap(pto))
                return tex2D(src, pto, data.width, data.height);
        }
        if (SwapTransitionCode::inBounds_for_swap(pto))
            return tex2D(src, pto, data.width, data.height);
        if (SwapTransitionCode::inBounds_for_swap(pfr))
            return !data.hasPrev ? uchar4(0, 0, 0, 255) : tex2D(prevSrc, pfr.x, pfr.y, data.pWidth, data.pHeight);
        return SwapTransitionCode::bgColor(float2(s, t), pfr, pto, src, prevSrc, data);
    }
}

kernel void swapTransition(constant TransitionData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar4* prevSrc,
                           uint2 index [[thread_position_in_grid]]) {

    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float t = index.y / (data.height - 1.0);
    float s = index.x / (data.width - 1.0);
    result[sidx] = SwapTransitionCode::swapTransition(data, src, prevSrc, t, s);
}


kernel void circlesTransition(constant TransitionData &data,
                              device uchar4* result,
                              const device uchar4* src,
                              const device uchar4* prevSrc,
                              uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float t = index.y / (data.height - 1.0);
    float s = index.x / (data.width - 1.0);
    
    float adjust = floor(interpolate(data.adjust, 0., 2., 100., 8.));

    const int NumCircles = 3;

    float dummy1, dummy2;
    float2 cell(modf(s * adjust, dummy1), modf(t * adjust, dummy2));

    float m = 0.0;
    float alphaPerCircle = 1.0 / float(NumCircles) + 0.01;
    for (int i = 0; i < NumCircles; ++i) {
        float delay = i * /*0.3*/ 0.5 / (NumCircles - 1);
        float p = max(0., data.progress - delay);
        m += alphaPerCircle * (1.0 - smoothstep(p * 1.40, p * 1.45, length(cell - float2(0.5, 0.5))));
    }
    m = min(1.0, m);
    uchar4 s1 = tex2D(src, s, t, data.width, data.height);
    uchar4 s2 = {0, 0, 0, 255};
    if (data.hasPrev) {
        s2 = tex2D(prevSrc, s, t, data.pWidth, data.pHeight);
    }
    result[sidx] = lerp(s1, s2, m);
}


constant uint DissolvePatternWidth = 512;
constant uint DissolvePatternHeight = 512;
uchar4 dissolveTexTransition(float s, float t, const device uchar* data) {
    //const unsigned char* data = DissolveTransitonPattern;
    s = clamp(0., s, 1.);
    t = clamp(0., t, 1.);

    int x = int(s * (DissolvePatternWidth - 1));
    int y = int(t * (DissolvePatternHeight - 1));

    const unsigned char val = data[y * DissolvePatternWidth + x];
    return uchar4(val, val, val, 255);
}

kernel void dissolveTransition(constant TransitionData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar* disolvePattern,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint sidx = index.y * data.width + index.x;

    float t = index.y / (data.height - 1.0);
    float s = index.x / (data.width - 1.0);
    
    uchar4 dissolveColor = dissolveTexTransition(s, t, disolvePattern);
    unsigned char byteProgress = (unsigned char)(255 * data.progress);
    result[sidx] = (dissolveColor.r > byteProgress) ? tex2D(src, s, t, data.width, data.height) : uchar4(0, 0, 0, 255);
}
