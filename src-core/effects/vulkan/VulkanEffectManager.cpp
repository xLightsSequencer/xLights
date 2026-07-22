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
#include "VulkanEffects.h"
#include "VulkanShaderEffect.h"

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
            VulkanComputeUtilities::INSTANCE.statSetup++;
            if (!parent->gpuRenderData) {
                parent->gpuRenderData = new VulkanPixelBufferComputeData();
            }
            // every RenderBuffer of this PixelBuffer needs the parent's data —
            // its compute queue is picked there
            VulkanPixelBufferComputeData *pbc = static_cast<VulkanPixelBufferComputeData*>(parent->gpuRenderData);
            if (!buffer->gpuRenderData) {
                buffer->gpuRenderData = new VulkanRenderBufferComputeData(buffer, pbc, layer);
            }
            VulkanRenderBufferComputeData *vrbcd = static_cast<VulkanRenderBufferComputeData*>(buffer->gpuRenderData);
            vrbcd->bufferResized();
        } else if (buffer != nullptr && buffer->gpuRenderData != nullptr) {
            // The backend went away after this buffer's pixels were pointed into
            // GPU memory -- either a device loss latched computeEnabled() false or
            // the user turned GPU rendering off. Nothing else re-points them, and
            // RenderBuffer::InitBuffer deliberately leaves a non-pixelVector pixels
            // pointer alone, so it would keep growing pixelVector around a pointer
            // into a mapping that is about to be freed. Hand ownership back now.
            buffer->ReleasePixelsToCpu();
            doCleanUp(buffer);
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
        VulkanComputeUtilities::INSTANCE.statBlurCall++;
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

// XL_NO_VULKAN_FX determinism diagnostic: "ALL" or a comma-separated list of
// effect names ("Bars,Circles") forces those effects to their CPU
// implementation while the rest of the GPU pipeline stays active.
static bool vulkanEffectDisabled(EffectManager::RGB_EFFECTS_e eff) {
    static const char* env = getenv("XL_NO_VULKAN_FX");
    if (env == nullptr) {
        return false;
    }
    std::string list = env;
    if (list == "ALL") {
        return true;
    }
    static const std::map<EffectManager::RGB_EFFECTS_e, std::string> names = {
        { EffectManager::eff_BARS, "Bars" },
        { EffectManager::eff_COLORWASH, "ColorWash" },
        { EffectManager::eff_SHOCKWAVE, "Shockwave" },
        { EffectManager::eff_FAN, "Fan" },
        { EffectManager::eff_SPIRALS, "Spirals" },
        { EffectManager::eff_GALAXY, "Galaxy" },
        { EffectManager::eff_CIRCLES, "Circles" },
        { EffectManager::eff_PLASMA, "Plasma" },
        { EffectManager::eff_BUTTERFLY, "Butterfly" },
        { EffectManager::eff_PINWHEEL, "Pinwheel" },
        { EffectManager::eff_KALEIDOSCOPE, "Kaleidoscope" },
        { EffectManager::eff_WARP, "Warp" },
        { EffectManager::eff_TREE, "Tree" },
        { EffectManager::eff_SHIMMER, "Shimmer" },
        { EffectManager::eff_CANDLE, "Candle" },
        { EffectManager::eff_WAVE, "Wave" },
        { EffectManager::eff_GARLANDS, "Garlands" },
        { EffectManager::eff_FILL, "Fill" },
        { EffectManager::eff_METEORS, "Meteors" },
        { EffectManager::eff_TWINKLE, "Twinkle" },
        { EffectManager::eff_LIFE, "Life" },
#ifdef HAVE_VULKAN_SHADER
        { EffectManager::eff_SHADER, "Shader" },
#endif
    };
    auto it = names.find(eff);
    return it != names.end() && list.find(it->second) != std::string::npos;
}

RenderableEffect* CreateVulkanEffect(EffectManager::RGB_EFFECTS_e eff) {
    if (VulkanComputeUtilities::INSTANCE.computeEnabled() && !vulkanEffectDisabled(eff)) {
        switch (eff) {
        case EffectManager::eff_BARS:
            return new VulkanBarsEffect(eff);
        case EffectManager::eff_COLORWASH:
            return new VulkanColorWashEffect(eff);
        case EffectManager::eff_SHOCKWAVE:
            return new VulkanShockwaveEffect(eff);
        case EffectManager::eff_FAN:
            return new VulkanFanEffect(eff);
        case EffectManager::eff_SPIRALS:
            return new VulkanSpiralsEffect(eff);
        case EffectManager::eff_GALAXY:
            return new VulkanGalaxyEffect(eff);
        case EffectManager::eff_CIRCLES:
            return new VulkanCirclesEffect(eff);
        case EffectManager::eff_PLASMA:
            return new VulkanPlasmaEffect(eff);
        case EffectManager::eff_BUTTERFLY:
            return new VulkanButterflyEffect(eff);
        case EffectManager::eff_PINWHEEL:
            return new VulkanPinwheelEffect(eff);
        case EffectManager::eff_KALEIDOSCOPE:
            return new VulkanKaleidoscopeEffect(eff);
        case EffectManager::eff_WARP:
            return new VulkanWarpEffect(eff);
        case EffectManager::eff_TREE:
            return new VulkanTreeEffect(eff);
        case EffectManager::eff_SHIMMER:
            return new VulkanShimmerEffect(eff);
        case EffectManager::eff_CANDLE:
            return new VulkanCandleEffect(eff);
        case EffectManager::eff_WAVE:
            return new VulkanWaveEffect(eff);
        case EffectManager::eff_GARLANDS:
            return new VulkanGarlandsEffect(eff);
        case EffectManager::eff_FILL:
            return new VulkanFillEffect(eff);
        case EffectManager::eff_METEORS:
            return new VulkanMeteorsEffect(eff);
        case EffectManager::eff_TWINKLE:
            return new VulkanTwinkleEffect(eff);
        case EffectManager::eff_LIFE:
            return new VulkanLifeEffect(eff);
#ifdef HAVE_VULKAN_SHADER
        case EffectManager::eff_SHADER:
            return new VulkanShaderEffect(eff);
#endif
        default:
            return nullptr;
        }
    }
    return nullptr;
}

#endif
