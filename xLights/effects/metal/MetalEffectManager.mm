#include "../EffectManager.h"
#include "../../GPURenderUtils.h"
#include "../../PixelBuffer.h"
#include "../../RenderBuffer.h"
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
        isEnabled = b;
    }


    virtual void doCleanUp(PixelBufferClass *c) override {
        if (c->gpuRenderData) {
            MetalPixelBufferComputeData *d = static_cast<MetalPixelBufferComputeData*>(c->gpuRenderData);
            delete d;
            c->gpuRenderData = nullptr;
        }
    }
    virtual void doCleanUp(RenderBuffer *c) override {
        if (c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            delete d;
            c->gpuRenderData = nullptr;
        }
    }
    virtual void doSetupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer) override {
        if (isEnabled) {
            MetalPixelBufferComputeData *pbc = nullptr;
            if (!parent->gpuRenderData) {
                pbc = new MetalPixelBufferComputeData();
                parent->gpuRenderData = pbc;
            }
            if (!buffer->gpuRenderData) {
                buffer->gpuRenderData = new MetalRenderBufferComputeData(buffer, pbc);
            }
            MetalRenderBufferComputeData *mrbcd = static_cast<MetalRenderBufferComputeData*>(buffer->gpuRenderData);
            mrbcd->bufferResized();
        }
    }
    virtual void doWaitForRenderCompletion(RenderBuffer *c) override {
        if (c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            d->waitForCompletion();
        }
    }
    virtual void doCommitRenderBuffer(RenderBuffer *c) override {
        if (c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            d->commit();
        }
    }
    virtual bool doBlur(RenderBuffer *c, int radius) override {
        if (c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            return d->blur(radius);
        }
        return false;
    }
    virtual bool doRotoZoom(RenderBuffer *c, RotoZoomSettings &settings) override {
        if (c->gpuRenderData) {
            MetalRenderBufferComputeData *d = static_cast<MetalRenderBufferComputeData*>(c->gpuRenderData);
            return d->rotoZoom(settings);
        }
        return false;
    }
    virtual void setPrioritizeGraphics(bool p) override {
        MetalComputeUtilities::INSTANCE.prioritizeGraphics(p);
    }

    bool isEnabled = true;
};


static MetalRenderUtils METAL_RENDER_UTILS;

RenderableEffect* CreateMetalEffect(EffectManager::RGB_EFFECTS_e eff) {
    if (MetalComputeUtilities::INSTANCE.computeEnabled()) {
        switch (eff) {
        case EffectManager::eff_BUTTERFLY:
            return new MetalButterflyEffect(eff);
        case EffectManager::eff_PLASMA:
            return new MetalPlasmaEffect(eff);
        case EffectManager::eff_WARP:
            return new MetalWarpEffect(eff);
        default:
            return nullptr;
        }
    }
    return nullptr;
}
