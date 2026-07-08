#version 450
// Analogue of Metal singleColorVertexShader.
#include "frame_data.glsl"

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec4 vColor;

void main() {
    gl_Position = frame.MVP * vec4(inPos, 1.0);
    gl_PointSize = frame.pointSize;
    vColor = frame.fragmentColor;
}
