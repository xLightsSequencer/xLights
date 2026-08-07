// Shared code for the transition kernels — port of the helpers in
// TransitionFunctions.metal.  The push-constant block must match
// TransitionData in VulkanEffectDataTypes.h ("out" renamed: GLSL keyword).

#include "vulkan_common.glsl"

layout(push_constant) uniform PC {
    uint width;
    uint height;

    uint pWidth;
    uint pHeight;

    float adjust;
    float progress;

    uint hasPrev;
    uint reverse;
    uint isOut;
} data;

const float XL_PI = 3.1415926535897932384626433832;
const float XL_PI_2 = XL_PI / 2.0;

float xl_lerpf(float a, float b, float progress) {
    return a + progress * (b - a);
}
uint xl_lerpc(uint a, uint b, float progress) {
    return uint(xl_round(float(a) + progress * (float(b) - float(a))));
}
uvec4 xl_lerp4(uvec4 a, uvec4 b, float progress) {
    return uvec4(xl_lerpc(a.r, b.r, progress), xl_lerpc(a.g, b.g, progress), xl_lerpc(a.b, b.b, progress), 255u);
}
vec2 xl_lerp2(vec2 a, vec2 b, float progress) {
    return vec2(a.x + progress * (b.x - a.x), a.y + progress * (b.y - a.y));
}
// Metal uchar4 addition wraps mod 256 per channel
uvec4 addWrap(uvec4 a, uvec4 b) {
    return (a + b) & uvec4(0xFFu);
}

float interpolate(float x, float loIn, float loOut, float hiIn, float hiOut) {
    return (loIn != hiIn)
               ? (loOut + (hiOut - loOut) * ((x - loIn) / (hiIn - loIn)))
               : ((loOut + hiOut) / 2.0);
}

vec2 Rotate(vec2 pt, float fAngle) {
    float cs = cos(fAngle);
    float sn = sin(fAngle);
    return vec2(pt.x * cs + pt.y * sn, -pt.x * sn + pt.y * cs);
}
vec2 RotateAbout(vec2 pt1, float angle, vec2 pt) {
    vec2 p = pt1 - pt;
    p = Rotate(p, angle);
    return p + pt;
}
bool isLeft(vec2 a, vec2 b, vec2 test) {
    return ((b.x - a.x) * (test.y - a.y) - (b.y - a.y) * (test.x - a.x)) > 0.0;
}
float rand3(int x, int y, int z) {
    int seed = x + y * 57 + z * 241;
    seed = (seed << 13) ^ seed;
    return ((1.0 - float((seed * (seed * seed * 15731 + 789221) + 1376312589) & 2147483647) / 1073741824.0) + 1.0) / 2.0;
}
