#version 450
// Analogue of Metal indexedColorVertexShader: interleaved position +
// color-index vertices, colors looked up in an SSBO of packed RGBA8.
#include "frame_data.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in uint inColorIdx;

layout(std430, set = 1, binding = 0) readonly buffer ColorTable {
    uint colors[];
} colorTable;

layout(location = 0) out vec4 vColor;

void main() {
    gl_Position = frame.MVP * vec4(inPos, 1.0);
    gl_PointSize = frame.pointSize;
    vColor = unpackUnorm4x8(colorTable.colors[inColorIdx]);
}
