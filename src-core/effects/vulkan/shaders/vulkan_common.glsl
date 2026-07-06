// Shared helpers for the xLights Vulkan compute kernels.
// Pixels are xlColor RGBA bytes viewed as one uint per pixel; on the
// little-endian targets we support, r is the low byte.

uvec4 unpackPx(uint p) {
    return uvec4(p & 0xFFu, (p >> 8) & 0xFFu, (p >> 16) & 0xFFu, p >> 24);
}

uint packPx(uvec4 c) {
    return (c.x & 0xFFu) | ((c.y & 0xFFu) << 8) | ((c.z & 0xFFu) << 16) | ((c.w & 0xFFu) << 24);
}

vec4 pxToFloat4(uint p) {
    return vec4(unpackPx(p));
}

// Metal/C round(): half away from zero.  GLSL round() leaves the .5 case
// implementation-defined, which would make output driver-dependent.
float xl_round(float x) {
    return trunc(x + (x >= 0.0 ? 0.5 : -0.5));
}

// Metal/C fmod(): truncation-based remainder.  GLSL mod() is floor-based
// and differs for negative operands.
float xl_fmod(float x, float y) {
    return x - y * trunc(x / y);
}
