#pragma once

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
    static void copyGPUData(RenderBuffer *to, RenderBuffer *from) {
        if (INSTANCE) {
            INSTANCE->doCopyGPUData(to, from);
        }
    }
    static void setupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer) {
        if (INSTANCE) {
            INSTANCE->doSetupRenderBuffer(parent, buffer);
        }
    }

    static void waitForRenderCompletion(RenderBuffer *buffer) {
        if (INSTANCE) {
            INSTANCE->doWaitForRenderCompletion(buffer);
        }
    };

protected:
    GPURenderUtils() { INSTANCE = this; }
    virtual ~GPURenderUtils() { INSTANCE = nullptr; };
    virtual bool enabled() = 0;
    virtual void enable(bool b) = 0;
    virtual void doCleanUp(PixelBufferClass *c) = 0;
    virtual void doCleanUp(RenderBuffer *c) = 0;
    virtual void doCopyGPUData(RenderBuffer *to, RenderBuffer *from) = 0;
    virtual void doSetupRenderBuffer(PixelBufferClass *parent, RenderBuffer *buffer) = 0;
    virtual void doWaitForRenderCompletion(RenderBuffer *buffer) = 0;


    static GPURenderUtils *INSTANCE;
};
