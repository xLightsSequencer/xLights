#version 450
// Analogue of Metal textureNearestFragmentShader (nearest mag filter).

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTex;

layout(set = 0, binding = 0) uniform texture2D tex;
layout(set = 0, binding = 2) uniform sampler samplerNearest;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(sampler2D(tex, samplerNearest), vTex) * vColor;
}
