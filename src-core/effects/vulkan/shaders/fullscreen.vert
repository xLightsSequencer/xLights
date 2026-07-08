#version 450

// Attribute-less fullscreen triangle: three vertices covering the whole
// viewport, generated from gl_VertexIndex (no vertex buffer).  vUV is 0..1
// across the visible area (the triangle extends to 2.0 off-screen).  This is
// the fixed vertex stage the native shader render pipeline pairs with a
// translated ISF fragment shader.
layout(location = 0) out vec2 vUV;

void main() {
    vec2 p = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vUV = p;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
