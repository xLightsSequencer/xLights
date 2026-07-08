#version 450
// Analogue of Metal textureColorFragmentShader: texture alpha masks the
// force color (used for font-atlas text rendering).

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTex;

layout(set = 0, binding = 0) uniform texture2D tex;
layout(set = 0, binding = 1) uniform sampler samplerLinear;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 s = texture(sampler2D(tex, samplerLinear), vTex);
    outColor = vec4(vColor.rgb, s.a * vColor.a);
}
