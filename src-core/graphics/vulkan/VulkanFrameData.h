#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cstdint>

#include <glm/glm.hpp>

// Per-draw uniform data pushed to the Vulkan graphics shaders — the analogue
// of MTLFrameData (MetalShaderTypes.h), trimmed to fit the 128-byte push
// constant guarantee.  The mesh-only matrices (model/view/perspective,
// useViewMatrix) move to a UBO when the mesh pipelines are added.
//
// Field order and types must match the push_constant block in
// shaders/frame_data.glsl (std430: mat4 @0, vec4 @64, scalars packed from 80).

enum VulkanRenderType : int32_t {
    VkRenderTypeNormal = 0,
    VkRenderTypePoints = 1,
    VkRenderTypePointsSmooth = 2,
    VkRenderTypeTexture = 3
};

struct VulkanFrameData {
    glm::mat4 MVP = glm::mat4(1.0f);
    glm::vec4 fragmentColor = glm::vec4(1.0f);
    float brightness = 1.0f;
    int32_t renderType = VkRenderTypeNormal;
    float pointSize = 1.0f;
    float pointSmoothMin = 0.25f;
    float pointSmoothMax = 0.5f;
};
static_assert(sizeof(VulkanFrameData) == 100, "VulkanFrameData must match the shader push_constant layout");
