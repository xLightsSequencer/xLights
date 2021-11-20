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
    enum CurrentDataLocation {
        BUFFER,
        TEXTURE
    };
    
    MetalRenderBufferComputeData(RenderBuffer *rb, MetalPixelBufferComputeData *pixelBufferData);
    ~MetalRenderBufferComputeData();

    MetalPixelBufferComputeData *pixelBufferData;


    static MetalRenderBufferComputeData *getMetalRenderBufferComputeData(RenderBuffer *);

    id<MTLCommandBuffer> getCommandBuffer();
    
    id<MTLBuffer> getPixelBuffer(bool sendToGPU = true);
    id<MTLTexture> getPixelTexture();

    void commit();
    void waitForCompletion();
    
    void setDataLocation(CurrentDataLocation dl) { currentDataLocation = dl; }
    bool blur(int radius);
private:
    RenderBuffer *renderBuffer;
    id<MTLCommandBuffer> commandBuffer;
    id<MTLBuffer> pixelBuffer;
    id<MTLTexture> pixelTexture;
    int pixelBufferSize;
    bool committed = false;
    CurrentDataLocation currentDataLocation = BUFFER;
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
