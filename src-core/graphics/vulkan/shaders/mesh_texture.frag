#version 450
// Analogue of Metal meshTextureFragmentShader.

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTex;
layout(location = 2) in float vBrightness;
layout(location = 3) in float vCosTheta;

layout(set = 0, binding = 0) uniform texture2D tex;
layout(set = 0, binding = 1) uniform sampler samplerLinear;

layout(location = 0) out vec4 outColor;

const float ambientLightFraction = 0.25;

void main() {
    vec4 color = texture(sampler2D(tex, samplerLinear), vTex);
    color = vec4(color.rgb * vBrightness, color.a);
    if (vCosTheta != 1.0) {
        vec3 c3 = vCosTheta * color.rgb * (1.0 - ambientLightFraction) + color.rgb * ambientLightFraction;
        color = vec4(c3, color.a);
    }
    outColor = color;
}
