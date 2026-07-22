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

// Mesh-draw push constants — the analogue of the mesh subset of MTLFrameData.
// Needs model/view matrices for normal lighting, so it exceeds the 128-byte
// push-constant guarantee (216 bytes) and rides its own pipeline layout;
// every desktop GPU supports >= 256-byte push constants.  Must match the
// push_constant block in shaders/mesh_frame_data.glsl (std430).

struct VulkanMeshFrameData {
    glm::mat4 MVP = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec4 fragmentColor = glm::vec4(1.0f);
    float brightness = 1.0f;
    int32_t useViewMatrix = 0;
};
static_assert(sizeof(VulkanMeshFrameData) == 216, "VulkanMeshFrameData must match the shader push_constant layout");
