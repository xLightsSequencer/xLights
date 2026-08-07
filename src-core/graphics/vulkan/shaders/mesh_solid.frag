#version 450
// Analogue of Metal meshSolidFragmentShader.

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTex;
layout(location = 2) in float vBrightness;
layout(location = 3) in float vCosTheta;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(vColor.rgb * vBrightness, vColor.a);
}
