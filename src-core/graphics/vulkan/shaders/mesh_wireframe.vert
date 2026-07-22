#version 450
// Analogue of Metal meshWireframeVertexShader: flat color, no lighting.
#include "mesh_frame_data.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vTex;
layout(location = 2) out float vBrightness;
layout(location = 3) out float vCosTheta;

void main() {
    gl_Position = frame.MVP * vec4(inPos, 1.0);
    vColor = frame.fragmentColor;
    vTex = inTex;
    vBrightness = frame.brightness;
    vCosTheta = 1.0;
}
