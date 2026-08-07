#version 450
// Analogue of Metal multiColorVertexShader.  Colors arrive as a
// VK_FORMAT_R8G8B8A8_UNORM vertex attribute (normalized in hardware).
#include "frame_data.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 vColor;

void main() {
    gl_Position = frame.MVP * vec4(inPos, 1.0);
    gl_PointSize = frame.pointSize;
    vColor = inColor;
}
