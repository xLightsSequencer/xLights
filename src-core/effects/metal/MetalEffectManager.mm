#include "../EffectManager.h"
#include "../../render/GPURenderUtils.h"
#include "../../render/PixelBuffer.h"
#include "../../render/RenderBuffer.h"
#include "MetalComputeUtilities.hpp"

#include "MetalEffects.hpp"


class MetalRenderUtils : public GPURenderUtils {
public:
    MetalRenderUtils() : isEnabled(true) {}
    virtual ~MetalRenderUtils() {}

    virtual bool enabled() override {
        return isEnabled && MetalComputeUtilities::INSTANCE.computeEnabled();
    }
    virtual void enable(bool b) override {
        if (getenv("XL_NATIVE_SHADER_DEBUG")) {
            fprintf(stderr, "NATIVE enable(%d) this=%p (was %d)\n", (int)b, (void*)this, (int)isEnabled);
        }
        isEnabled = b;
    }


    virtual void doCleanUp(PixelBufferClass *c) override {
        if (c && c->gpuRenderData) {
            MetalPixelBufferComputeData *d = static_cast<MetalPixelBufferComputeData*>(c->gpuRenderData);
            delete d;
            c->gpuRenderData = nullptr;
        }
    }
    virtual void doCleanUp(RenderBuffer *c) override {
        if (c && c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            delete d;
            c->gpuRenderData = nullptr;
        }
    }
    virtual void doSetupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer, int layer) override {
        static const bool dbg = getenv("XL_NATIVE_SHADER_DEBUG") != nullptr;
        if (dbg) {
            fprintf(stderr, "NATIVE setup rb=%p model=%s layer=%d enabled=%d this=%p\n",
                    (void*)buffer, buffer->GetModelName().c_str(), layer, (int)isEnabled, (void*)this);
        }
        if (isEnabled) {
            if (!parent->gpuRenderData) {
                parent->gpuRenderData = new MetalPixelBufferComputeData();
            }
            // every RenderBuffer of this PixelBuffer needs the parent's data —
            // its command queue is what keeps their command buffers in order
            MetalPixelBufferComputeData *pbc = static_cast<MetalPixelBufferComputeData*>(parent->gpuRenderData);
            if (!buffer->gpuRenderData) {
                buffer->gpuRenderData = new MetalRenderBufferComputeData(buffer, pbc, layer);
            }
            MetalRenderBufferComputeData *mrbcd = static_cast<MetalRenderBufferComputeData*>(buffer->gpuRenderData);
            mrbcd->bufferResized();
        }
    }
    virtual void doWaitForRenderCompletion(RenderBuffer *c) override {
        if (c && c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            if (d) {
                d->waitForCompletion();
            }
        }
    }
    virtual void doCommitRenderBuffer(RenderBuffer *c) override {
        if (c && c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            if (d) {
                d->commit();
            }
        }
    }
    virtual bool doBlur(RenderBuffer *c, int radius) override {
        static const bool noGpuBlur = (getenv("XL_NO_GPU_BLUR") != nullptr);
        if (!noGpuBlur && c && c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            return d ? d->blur(radius) : false;
        }
        return false;
    }
    virtual bool doRotoZoom(RenderBuffer *c, RotoZoomSettings &settings) override {
        static const bool noGpuRoto = (getenv("XL_NO_GPU_ROTO") != nullptr);
        if (!noGpuRoto && c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            return d ? d->rotoZoom(settings) : false;
        }
        return false;
    }
    virtual void setPrioritizeGraphics(bool p) override {
        MetalComputeUtilities::INSTANCE.prioritizeGraphics(p);
    }
    // Deliberately ignores the user-toggleable isEnabled flag: pool sizing
    // happens once at startup, before preferences are applied.
    virtual int gpuEffectConcurrency() override {
        return MetalComputeUtilities::INSTANCE.gpuCoreCount();
    }
    virtual bool doTransitions(PixelBufferClass *pixelBuffer, int layer, RenderBuffer *prevRB) override {
        static const bool noGpuTrans = (getenv("XL_NO_GPU_TRANS") != nullptr);
        if (!noGpuTrans && enabled() && pixelBuffer) {
            MetalPixelBufferComputeData *d = static_cast<MetalPixelBufferComputeData*>(pixelBuffer->gpuRenderData);
            return d ? d->doTransitions(pixelBuffer, layer, prevRB) : false;
        }
        return false;
    }
    virtual bool doBlendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels) override {
        if (enabled() && pixelBuffer) {
            MetalPixelBufferComputeData *d = static_cast<MetalPixelBufferComputeData*>(pixelBuffer->gpuRenderData);
            return d ? d->doBlendLayers(pixelBuffer, effectPeriod, validLayers, saveLayer, saveToPixels) : false;
        }
        return false;
    }


    bool isEnabled = true;
};


static MetalRenderUtils METAL_RENDER_UTILS;

// XL_NO_METAL_FX determinism diagnostic: "ALL" or a comma-separated list of
// effect names ("Shockwave,Kaleidoscope") forces those effects to their CPU
// implementation while the rest of the GPU pipeline stays active.
static bool metalEffectDisabled(EffectManager::RGB_EFFECTS_e eff) {
    static const char* env = getenv("XL_NO_METAL_FX");
    if (env == nullptr) {
        return false;
    }
    std::string list = env;
    if (list == "ALL") {
        return true;
    }
    static const std::map<EffectManager::RGB_EFFECTS_e, std::string> names = {
        { EffectManager::eff_BUTTERFLY, "Butterfly" },
        { EffectManager::eff_PLASMA, "Plasma" },
        { EffectManager::eff_WARP, "Warp" },
        { EffectManager::eff_PINWHEEL, "Pinwheel" },
        { EffectManager::eff_SHOCKWAVE, "Shockwave" },
        { EffectManager::eff_KALEIDOSCOPE, "Kaleidoscope" },
        { EffectManager::eff_FAN, "Fan" },
        { EffectManager::eff_GALAXY, "Galaxy" },
        { EffectManager::eff_TREE, "Tree" },
        { EffectManager::eff_TWINKLE, "Twinkle" },
        { EffectManager::eff_SHIMMER, "Shimmer" },
        { EffectManager::eff_CANDLE, "Candle" },
        { EffectManager::eff_LIFE, "Life" },
        { EffectManager::eff_WAVE, "Wave" },
        { EffectManager::eff_GARLANDS, "Garlands" },
        { EffectManager::eff_SPIRALS, "Spirals" },
        { EffectManager::eff_COLORWASH, "ColorWash" },
        { EffectManager::eff_FILL, "Fill" },
        { EffectManager::eff_BARS, "Bars" },
        { EffectManager::eff_CIRCLES, "Circles" },
        { EffectManager::eff_METEORS, "Meteors" },
        { EffectManager::eff_SHADER, "Shader" },
    };
    auto it = names.find(eff);
    return it != names.end() && list.find(it->second) != std::string::npos;
}

RenderableEffect* CreateMetalEffect(EffectManager::RGB_EFFECTS_e eff) {
    if (MetalComputeUtilities::INSTANCE.computeEnabled() && !metalEffectDisabled(eff)) {
        switch (eff) {
        case EffectManager::eff_BUTTERFLY:
            return new MetalButterflyEffect(eff);
        case EffectManager::eff_PLASMA:
            return new MetalPlasmaEffect(eff);
        case EffectManager::eff_WARP:
            return new MetalWarpEffect(eff);
        case EffectManager::eff_PINWHEEL:
            return new MetalPinwheelEffect(eff);
        case EffectManager::eff_SHOCKWAVE:
            return new MetalShockwaveEffect(eff);
        case EffectManager::eff_KALEIDOSCOPE:
            return new MetalKaleidoscopeEffect(eff);
        case EffectManager::eff_FAN:
            return new MetalFanEffect(eff);
        case EffectManager::eff_GALAXY:
            return new MetalGalaxyEffect(eff);
        case EffectManager::eff_TREE:
            return new MetalTreeEffect(eff);
        case EffectManager::eff_TWINKLE:
            return new MetalTwinkleEffect(eff);
        case EffectManager::eff_SHIMMER:
            return new MetalShimmerEffect(eff);
        case EffectManager::eff_CANDLE:
            return new MetalCandleEffect(eff);
        case EffectManager::eff_LIFE:
            return new MetalLifeEffect(eff);
        case EffectManager::eff_WAVE:
            return new MetalWaveEffect(eff);
        case EffectManager::eff_GARLANDS:
            return new MetalGarlandsEffect(eff);
        case EffectManager::eff_SPIRALS:
            return new MetalSpiralsEffect(eff);
        case EffectManager::eff_COLORWASH:
            return new MetalColorWashEffect(eff);
        case EffectManager::eff_FILL:
            return new MetalFillEffect(eff);
        case EffectManager::eff_BARS:
            return new MetalBarsEffect(eff);
        case EffectManager::eff_CIRCLES:
            return new MetalCirclesEffect(eff);
        case EffectManager::eff_METEORS:
            return new MetalMeteorsEffect(eff);
        case EffectManager::eff_SHADER:
            return new MetalShaderEffect(eff);
        default:
            return nullptr;
        }
    }
    return nullptr;
}
