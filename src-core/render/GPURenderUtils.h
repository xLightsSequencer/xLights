#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "RenderProfile.h"

class PixelBufferClass;
class RenderBuffer;

class GPURenderUtils {
public:


    static bool IsEnabled() { return INSTANCE != nullptr && INSTANCE->enabled(); }
    static void SetEnabled(bool b) {
        if (INSTANCE) {
            INSTANCE->enable(b);
        }
    }

    static void cleanUp(PixelBufferClass *c) {
        if (INSTANCE) {
            INSTANCE->doCleanUp(c);
        }
    }
    static void cleanUp(RenderBuffer *c) {
        if (INSTANCE) {
            INSTANCE->doCleanUp(c);
        }
    }
    static void setupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer, int layer) {
        if (INSTANCE) {
            INSTANCE->doSetupRenderBuffer(parent, buffer, layer);
        }
    }

    static void commitRenderBuffer(RenderBuffer *buffer) {
        if (INSTANCE) {
            INSTANCE->doCommitRenderBuffer(buffer);
        }
    };

    static void waitForRenderCompletion(RenderBuffer *buffer) {
        if (INSTANCE) {
            RenderJobProfile* prof = tlsRenderProfile;
            if (prof != nullptr) {
                auto t0 = std::chrono::steady_clock::now();
                INSTANCE->doWaitForRenderCompletion(buffer);
                prof->gpuWaitNs += xlProfNs(t0, std::chrono::steady_clock::now());
            } else {
                INSTANCE->doWaitForRenderCompletion(buffer);
            }
        }
    };
    static bool Blur(RenderBuffer *buffer, int radius) {
        if (INSTANCE) {
            return INSTANCE->doBlur(buffer, radius);
        }
        return false;
    };
    static void prioritizeGraphics(bool p) {
        if (INSTANCE) {
            return INSTANCE->setPrioritizeGraphics(p);
        }
    }

    // Estimated number of effect renders the GPU can process concurrently
    // (~GPU core count); 0 when there is no GPU render backend.  Used to size
    // the render pool: a thread parked in waitForRenderCompletion leaves its
    // CPU core free for another job, so the pool wants cpu + gpu headroom.
    static int GetGPUEffectConcurrency() {
        if (INSTANCE) {
            return INSTANCE->gpuEffectConcurrency();
        }
        return 0;
    }

    
    class RotoZoomSettings {
    public:
        RotoZoomSettings() {}
        
        std::string rotationorder;

        float offset = 0.0;        
        float xrotation;
        int xpivot;
        float yrotation;
        int ypivot;
        
        float zrotation;
        float zoom;
        float zoomquality;
        int pivotpointx;
        int pivotpointy;
    };
    
    static bool RotoZoom(RenderBuffer *buffer, RotoZoomSettings &settings) {
        if (INSTANCE) {
            return INSTANCE->doRotoZoom(buffer, settings);
        }
        return false;
    };
    
    static bool Transitions(PixelBufferClass *pixelBuffer, int layer, RenderBuffer *prevRB) {
        if (INSTANCE) {
            return INSTANCE->doTransitions(pixelBuffer, layer, prevRB);
        }
        return false;
    }
    
    static bool BlendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels) {
        if (INSTANCE) {
            return INSTANCE->doBlendLayers(pixelBuffer, effectPeriod, validLayers, saveLayer, saveToPixels);
        }
        return false;
    }
protected:
    GPURenderUtils() { INSTANCE = this; }
    virtual ~GPURenderUtils() { INSTANCE = nullptr; };
    virtual bool enabled() = 0;
    virtual void enable(bool b) = 0;
    virtual void doCleanUp(PixelBufferClass *c) = 0;
    virtual void doCleanUp(RenderBuffer *c) = 0;
    virtual void doSetupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer, int layer) = 0;
    virtual void doCommitRenderBuffer(RenderBuffer *buffer) = 0;
    virtual void doWaitForRenderCompletion(RenderBuffer *buffer) = 0;

    virtual bool doBlur(RenderBuffer *buffer, int radius) = 0;
    virtual bool doRotoZoom(RenderBuffer *buffer, RotoZoomSettings &settings) = 0;
    virtual bool doTransitions(PixelBufferClass *pixelBuffer, int layer, RenderBuffer *prevRB) = 0;
    virtual bool doBlendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels) = 0;

    virtual void setPrioritizeGraphics(bool p) = 0;

    virtual int gpuEffectConcurrency() { return 0; }

    static GPURenderUtils *INSTANCE;
};
