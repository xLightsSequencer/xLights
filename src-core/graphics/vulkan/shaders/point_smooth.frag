#version 450
// Analogue of Metal pointSmoothFragmentShader: radial alpha falloff.
#include "frame_data.glsl"

layout(location = 0) in vec4 vColor;

layout(location = 0) out vec4 outColor;

void main() {
    float dist = length(gl_PointCoord - vec2(0.5));
    vec4 c = vColor;
    c.a *= 1.0 - smoothstep(frame.pointSmoothMin, frame.pointSmoothMax, dist);
    if (c.a == 0.0) {
        discard;
    }
    outColor = c;
}
