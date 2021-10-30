#pragma once

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

class PixelBufferClass;
class RenderBuffer;

class MetalPixelBufferComputeData {
public:
    MetalPixelBufferComputeData();
    ~MetalPixelBufferComputeData();

};

class MetalRenderBufferComputeData {
public:
    MetalRenderBufferComputeData(RenderBuffer *rb, MetalPixelBufferComputeData *pixelBufferData);
    ~MetalRenderBufferComputeData();

    MetalPixelBufferComputeData *pixelBufferData;


    static MetalRenderBufferComputeData *getMetalRenderBufferComputeData(RenderBuffer *);

    id<MTLCommandBuffer> getCommandBuffer();
    id<MTLBuffer> getPixelBuffer();
    void waitForCompletion();
private:
    RenderBuffer *renderBuffer;
    id<MTLCommandBuffer> commandBuffer;
    id<MTLBuffer> pixelBuffer;
    int pixelBufferSize;
};


class MetalComputeUtilities {
public:
    MetalComputeUtilities();
    ~MetalComputeUtilities();
    bool computeEnabled() {
        return enabled;
    }

    id<MTLComputePipelineState> FindComputeFunction(const char *name);


    bool enabled = true;
    id<MTLDevice> device;
    id<MTLLibrary> library;
    id<MTLCommandQueue> commandQueue;


    static MetalComputeUtilities INSTANCE;
};
