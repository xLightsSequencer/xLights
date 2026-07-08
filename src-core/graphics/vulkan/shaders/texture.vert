#version 450
// Analogue of Metal textureVertexShader.  fragmentColor doubles as the
// force/modulate color (brightness and alpha pre-baked by the context).
#include "frame_data.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vTex;

void main() {
    gl_Position = frame.MVP * vec4(inPos, 1.0);
    vColor = frame.fragmentColor;
    vTex = inTex;
}
