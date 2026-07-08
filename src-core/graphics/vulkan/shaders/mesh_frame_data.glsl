// Shared push-constant block for the Vulkan mesh shaders.
// Field order/types must match VulkanMeshFrameData.h (std430 layout).
layout(push_constant) uniform MeshFrameData {
    mat4 MVP;
    mat4 model;
    mat4 view;
    vec4 fragmentColor;
    float brightness;
    int useViewMatrix;
} frame;
