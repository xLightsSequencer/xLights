#version 450
// Analogue of Metal meshVertexShader: diffuse lighting when the material is
// opaque, flat color otherwise.
#include "mesh_frame_data.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTex;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vTex;
layout(location = 2) out float vBrightness;
layout(location = 3) out float vCosTheta;

const vec3 lightDirection = vec3(0.1, 0.1, 1.0);
const float ambientLightFraction = 0.25;

void main() {
    gl_Position = frame.MVP * vec4(inPos, 1.0);
    if (frame.fragmentColor.a == 1.0) {
        mat4 normalMatrix = frame.useViewMatrix != 0 ? (frame.view * frame.model) : frame.model;
        vec3 n = normalize((normalMatrix * vec4(inNormal, 0.0)).xyz);
        vec3 l = normalize(lightDirection);
        float cosTheta = abs(clamp(dot(n, l), -1.0, 1.0));
        vCosTheta = cosTheta;
        vec4 lit = frame.fragmentColor * vec4(cosTheta, cosTheta, cosTheta, 1.0);
        vColor = lit * (1.0 - ambientLightFraction) + frame.fragmentColor * ambientLightFraction;
    } else {
        vColor = frame.fragmentColor;
        vCosTheta = 1.0;
    }
    vTex = inTex;
    vBrightness = frame.brightness;
}
