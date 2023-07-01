#pragma once

#include <string>

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
    static void setupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer) {
        if (INSTANCE) {
            INSTANCE->doSetupRenderBuffer(parent, buffer);
        }
    }

    static void commitRenderBuffer(RenderBuffer *buffer) {
        if (INSTANCE) {
            INSTANCE->doCommitRenderBuffer(buffer);
        }
    };

    static void waitForRenderCompletion(RenderBuffer *buffer) {
        if (INSTANCE) {
            INSTANCE->doWaitForRenderCompletion(buffer);
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
protected:
    GPURenderUtils() { INSTANCE = this; }
    virtual ~GPURenderUtils() { INSTANCE = nullptr; };
    virtual bool enabled() = 0;
    virtual void enable(bool b) = 0;
    virtual void doCleanUp(PixelBufferClass *c) = 0;
    virtual void doCleanUp(RenderBuffer *c) = 0;
    virtual void doSetupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer) = 0;
    virtual void doCommitRenderBuffer(RenderBuffer *buffer) = 0;
    virtual void doWaitForRenderCompletion(RenderBuffer *buffer) = 0;

    virtual bool doBlur(RenderBuffer *buffer, int radius) = 0;
    virtual bool doRotoZoom(RenderBuffer *buffer, RotoZoomSettings &settings) = 0;
    virtual void setPrioritizeGraphics(bool p) = 0;


    static GPURenderUtils *INSTANCE;
};
