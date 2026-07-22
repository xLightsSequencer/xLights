#version 450

// Built-in validation fragment shader for the graphics-pipeline foundation.
// Emits a deterministic UV gradient (R=x, G=y) so a headless render can be
// pixel-checked: bottom-left ~= (0,0,0), top-right ~= (255,255,0).  Not an ISF
// shader — just proves render pass -> pipeline -> draw -> copy-to-pixels.
layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(clamp(vUV, 0.0, 1.0), 0.0, 1.0);
}
