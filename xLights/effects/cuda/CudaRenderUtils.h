#pragma once

#include "../../GPURenderUtils.h"
#include "../../PixelBuffer.h"
#include "../../RenderBuffer.h"

#include "CudaRenderBufferComputeData.h"

class CudaRenderUtils : public GPURenderUtils {
public:
    CudaRenderUtils() : isEnabled(true) 
    {

    }
    virtual ~CudaRenderUtils() {}

    virtual bool enabled() override {
        return isEnabled;
    }
    virtual void enable(bool b) override {
        isEnabled = b;
    }


    virtual void doCleanUp(PixelBufferClass *c) override {
        if (c->gpuRenderData) {
            CudaPixelBufferComputeData *d = static_cast<CudaPixelBufferComputeData*>(c->gpuRenderData);
            delete d;
            c->gpuRenderData = nullptr;
        }
    }
    virtual void doCleanUp(RenderBuffer *c) override {
        if (c->gpuRenderData) {
            CudaRenderBufferComputeData *d = static_cast<CudaRenderBufferComputeData*>(c->gpuRenderData);
            delete d;
            c->gpuRenderData = nullptr;
        }
    }
    virtual void doSetupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer) override {
        if (isEnabled) {
            CudaPixelBufferComputeData *pbc = nullptr;
            if (!parent->gpuRenderData) {
                pbc = new CudaPixelBufferComputeData();
                parent->gpuRenderData = pbc;
            }
            if (!buffer->gpuRenderData) {
                buffer->gpuRenderData = new CudaRenderBufferComputeData(buffer, pbc);
            }
            CudaRenderBufferComputeData *mrbcd = static_cast<CudaRenderBufferComputeData*>(buffer->gpuRenderData);
            mrbcd->bufferResized();
        }
    }
    virtual void doWaitForRenderCompletion(RenderBuffer *c) override {
        if (c->gpuRenderData) {
            CudaRenderBufferComputeData *d = static_cast<CudaRenderBufferComputeData*>(c->gpuRenderData);
            d->waitForCompletion();
        }
    }
    virtual void doCommitRenderBuffer(RenderBuffer *c) override {
        if (c->gpuRenderData) {
            CudaRenderBufferComputeData *d = static_cast<CudaRenderBufferComputeData*>(c->gpuRenderData);
            d->commit();
        }
    }
    virtual bool doBlur(RenderBuffer *c, int radius) override {
        if (c->gpuRenderData) {
            CudaRenderBufferComputeData *d = static_cast<CudaRenderBufferComputeData*>(c->gpuRenderData);
            return d->blur(radius);
        }
        return false;
    }
    virtual bool doRotoZoom(RenderBuffer *c, RotoZoomSettings &settings) override {
        if (c->gpuRenderData) {
            CudaRenderBufferComputeData *d = static_cast<CudaRenderBufferComputeData*>(c->gpuRenderData);
            return d->rotoZoom(settings);
        }
        return false;
    }
    virtual void setPrioritizeGraphics(bool p) override {}

    bool isEnabled = true;
};


static CudaRenderUtils CUDA_RENDER_UTILS;