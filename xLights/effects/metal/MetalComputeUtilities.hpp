#pragma once

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

#include "GPURenderUtils.h"
#include "MetalEffectDataTypes.h"

class PixelBufferClass;
class RenderBuffer;

class MetalPixelBufferComputeData {
public:
    MetalPixelBufferComputeData();
    ~MetalPixelBufferComputeData();

};

class MetalRenderBufferComputeData {
public:
    enum CurrentDataLocation {
        BUFFER,
        TEXTURE
    };
    
    MetalRenderBufferComputeData(RenderBuffer *rb, MetalPixelBufferComputeData *pixelBufferData);
    ~MetalRenderBufferComputeData();
    
    void bufferResized();

    MetalPixelBufferComputeData *pixelBufferData;


    static MetalRenderBufferComputeData *getMetalRenderBufferComputeData(RenderBuffer *);

    id<MTLCommandBuffer> getCommandBuffer();
    void abortCommandBuffer();
    
    id<MTLBuffer> getPixelBuffer(bool sendToGPU = true);
    id<MTLTexture> getPixelTexture();
    id<MTLBuffer> getPixelBufferCopy();

    void commit();
    void waitForCompletion();
    
    void setDataLocation(CurrentDataLocation dl) { currentDataLocation = dl; }
    bool blur(int radius);
    bool rotoZoom(GPURenderUtils::RotoZoomSettings &settings);

private:
    bool callRotoZoomFunction(id<MTLComputePipelineState> &f, RotoZoomData &data);
    
    RenderBuffer *renderBuffer;
    id<MTLCommandBuffer> commandBuffer;
    id<MTLBuffer> pixelBuffer;
    id<MTLBuffer> pixelBufferCopy;
    id<MTLTexture> pixelTexture;
    int pixelBufferSize;
    std::pair<uint32_t, uint32_t> pixelTextureSize;
    bool committed = false;
    CurrentDataLocation currentDataLocation = BUFFER;
    
    static std::atomic<uint32_t> commandBufferCount;
};


class MetalComputeUtilities {
public:
    MetalComputeUtilities();
    ~MetalComputeUtilities();
    bool computeEnabled() {
        return enabled;
    }
    bool prioritizeGraphics() {
        return pg;
    }
    void prioritizeGraphics(bool p) {
        pg = p;
    }

    id<MTLComputePipelineState> FindComputeFunction(const char *name);


    bool enabled = true;
    std::atomic<bool> pg = false;    
    
    id<MTLDevice> device;
    id<MTLLibrary> library;
    id<MTLCommandQueue> commandQueue;


    id<MTLComputePipelineState> xrotateFunction;
    id<MTLComputePipelineState> yrotateFunction;
    id<MTLComputePipelineState> zrotateFunction;
    id<MTLComputePipelineState> rotateBlankFunction;
    
    static MetalComputeUtilities INSTANCE;
};
