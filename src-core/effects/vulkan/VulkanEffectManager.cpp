/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Vulkan compute backend (Linux/Windows analogue of the Metal backend).
// The whole file compiles to nothing without HAVE_VULKAN so that build
// systems that discover src-core/ wholesale (the Xcode synchronized
// groups) can include it harmlessly on Apple platforms.
#ifdef HAVE_VULKAN

#include <cstdlib>
#include <map>
#include <string>

#include "../EffectManager.h"
#include "../../render/GPURenderUtils.h"
#include "../../render/PixelBuffer.h"
#include "../../render/RenderBuffer.h"
#include "VulkanComputeUtilities.h"

class VulkanRenderUtils : public GPURenderUtils {
public:
    VulkanRenderUtils() : isEnabled(true) {}
    virtual ~VulkanRenderUtils() {}

    virtual bool enabled() override {
        return isEnabled && VulkanComputeUtilities::INSTANCE.computeEnabled();
    }
    virtual void enable(bool b) override {
        isEnabled = b;
    }

    virtual void doCleanUp(PixelBufferClass *c) override {
        if (c && c->gpuRenderData) {
            VulkanPixelBufferComputeData *d = static_cast<VulkanPixelBufferComputeData*>(c->gpuRenderData);
            delete d;
            c->gpuRenderData = nullptr;
        }
    }
    virtual void doCleanUp(RenderBuffer *c) override {
        if (c && c->gpuRenderData) {
            VulkanRenderBufferComputeData *d = static_cast<VulkanRenderBufferComputeData*>(c->gpuRenderData);
            delete d;
            c->gpuRenderData = nullptr;
        }
    }
    virtual void doSetupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer, int layer) override {
        // Unlike Metal (where messaging a nil device degrades quietly), the
        // Vulkan data classes need a live device, so gate on full enablement.
        if (enabled()) {
            VulkanPixelBufferComputeData *pbc = nullptr;
            if (!parent->gpuRenderData) {
                pbc = new VulkanPixelBufferComputeData();
                parent->gpuRenderData = pbc;
            }
            if (!buffer->gpuRenderData) {
                buffer->gpuRenderData = new VulkanRenderBufferComputeData(buffer, pbc, layer);
            }
            VulkanRenderBufferComputeData *vrbcd = static_cast<VulkanRenderBufferComputeData*>(buffer->gpuRenderData);
            vrbcd->bufferResized();
        }
    }
    virtual void doWaitForRenderCompletion(RenderBuffer *c) override {
        if (c && c->gpuRenderData) {
            VulkanRenderBufferComputeData *d = static_cast<VulkanRenderBufferComputeData*>(c->gpuRenderData);
            if (d) {
                d->waitForCompletion();
            }
        }
    }
    virtual void doCommitRenderBuffer(RenderBuffer *c) override {
        if (c && c->gpuRenderData) {
            VulkanRenderBufferComputeData *d = static_cast<VulkanRenderBufferComputeData*>(c->gpuRenderData);
            if (d) {
                d->commit();
            }
        }
    }
    virtual bool doBlur(RenderBuffer *c, int radius) override {
        static const bool noGpuBlur = (getenv("XL_NO_GPU_BLUR") != nullptr);
        if (!noGpuBlur && c && c->gpuRenderData) {
            VulkanRenderBufferComputeData *d = static_cast<VulkanRenderBufferComputeData*>(c->gpuRenderData);
            return d ? d->blur(radius) : false;
        }
        return false;
    }
    virtual bool doRotoZoom(RenderBuffer *c, RotoZoomSettings &settings) override {
        static const bool noGpuRoto = (getenv("XL_NO_GPU_ROTO") != nullptr);
        if (!noGpuRoto && c->gpuRenderData) {
            VulkanRenderBufferComputeData *d = static_cast<VulkanRenderBufferComputeData*>(c->gpuRenderData);
            return d ? d->rotoZoom(settings) : false;
        }
        return false;
    }
    virtual void setPrioritizeGraphics(bool p) override {
        VulkanComputeUtilities::INSTANCE.prioritizeGraphics(p);
    }
    // Deliberately ignores the user-toggleable isEnabled flag: pool sizing
    // happens once at startup, before preferences are applied.
    virtual int gpuEffectConcurrency() override {
        return VulkanComputeUtilities::INSTANCE.gpuCoreCount();
    }
    virtual bool doTransitions(PixelBufferClass *pixelBuffer, int layer, RenderBuffer *prevRB) override {
        static const bool noGpuTrans = (getenv("XL_NO_GPU_TRANS") != nullptr);
        if (!noGpuTrans && enabled() && pixelBuffer) {
            VulkanPixelBufferComputeData *d = static_cast<VulkanPixelBufferComputeData*>(pixelBuffer->gpuRenderData);
            return d ? d->doTransitions(pixelBuffer, layer, prevRB) : false;
        }
        return false;
    }
    virtual bool doBlendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels) override {
        if (enabled() && pixelBuffer) {
            VulkanPixelBufferComputeData *d = static_cast<VulkanPixelBufferComputeData*>(pixelBuffer->gpuRenderData);
            return d ? d->doBlendLayers(pixelBuffer, effectPeriod, validLayers, saveLayer, saveToPixels) : false;
        }
        return false;
    }

    bool isEnabled = true;
};

static VulkanRenderUtils VULKAN_RENDER_UTILS;

RenderableEffect* CreateVulkanEffect(EffectManager::RGB_EFFECTS_e eff) {
    return nullptr;
}

#endif
