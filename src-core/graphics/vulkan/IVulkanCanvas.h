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

// IVulkanCanvas — wx-free abstract interface for a Vulkan rendering canvas,
// the analogue of IMetalCanvas.  Implemented by xlVulkanCanvas (desktop wx);
// consumed by xlVulkanGraphicsContext so the per-frame context has no
// dependency on wx types.

#ifdef HAVE_VULKAN

#include <string>

#include <volk.h>

#include "../../utils/Color.h"

class VulkanPipelineCache;

class IVulkanCanvas {
public:
    virtual ~IVulkanCanvas() = default;

    virtual std::string getName() const = 0;
    virtual xlColor ClearBackgroundColor() const = 0;
    virtual bool RequiresDepthBuffer() const = 0;
    virtual bool drawingUsingLogicalSize() const = 0;
    virtual double translateToBacking(double x) const = 0;

    // The command buffer for the frame currently being recorded (between
    // PrepareContextForDrawing and FinishDrawing), inside an active render
    // pass targeting the acquired swapchain image.
    virtual VkCommandBuffer getFrameCommandBuffer() = 0;
    virtual VkExtent2D getFrameExtent() = 0;
    virtual VulkanPipelineCache* getPipelineCache() = 0;
};

#endif
