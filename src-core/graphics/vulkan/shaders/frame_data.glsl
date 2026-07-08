// Shared push-constant block for the Vulkan graphics shaders.
// Field order/types must match VulkanFrameData.h (std430 layout).
layout(push_constant) uniform FrameData {
    mat4 MVP;
    vec4 fragmentColor;
    float brightness;
    int renderType;
    float pointSize;
    float pointSmoothMin;
    float pointSmoothMax;
} frame;
