
#include "MetalComputeUtilities.hpp"
#include "../../PixelBuffer.h"
#include "../../RenderBuffer.h"

MetalComputeUtilities MetalComputeUtilities::INSTANCE;


MetalPixelBufferComputeData::MetalPixelBufferComputeData() {
}
MetalPixelBufferComputeData::~MetalPixelBufferComputeData() {
}


MetalRenderBufferComputeData::MetalRenderBufferComputeData(RenderBuffer *rb, MetalPixelBufferComputeData *pbd) : renderBuffer(rb), pixelBufferData(pbd) {
    commandBuffer = nil;
    pixelBuffer = nil;
    pixelBufferSize = 0;
}
MetalRenderBufferComputeData::~MetalRenderBufferComputeData() {
    pixelBufferData = nullptr;
    @autoreleasepool {
        commandBuffer = nil;
        pixelBuffer = nil;
    }
}
id<MTLCommandBuffer> MetalRenderBufferComputeData::getCommandBuffer() {
    if (commandBuffer == nil) {
        commandBuffer = [MetalComputeUtilities::INSTANCE.commandQueue commandBuffer];
    }
    return commandBuffer;
}
id<MTLBuffer> MetalRenderBufferComputeData::getPixelBuffer() {
    if (pixelBufferSize < renderBuffer->pixelVector.size()) {
        int bufferSize = renderBuffer->pixelVector.size() * 4;
        id<MTLBuffer> newBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithLength:bufferSize options:MTLResourceStorageModeShared];
        memcpy(newBuffer.contents, renderBuffer->pixels, pixelBufferSize * 4);
        pixelBufferSize = renderBuffer->pixelVector.size();
        pixelBuffer = newBuffer;
        renderBuffer->pixels = static_cast<xlColor*>(pixelBuffer.contents);
    }
    return pixelBuffer;
}
void MetalRenderBufferComputeData::waitForCompletion() {
    if (commandBuffer != nil) {
        @autoreleasepool {
            [commandBuffer waitUntilCompleted];
            commandBuffer = nil;
        }
    }
}

MetalRenderBufferComputeData *MetalRenderBufferComputeData::getMetalRenderBufferComputeData(RenderBuffer *b) {
    return static_cast<MetalRenderBufferComputeData*>(b->gpuRenderData);
}



MetalComputeUtilities::MetalComputeUtilities() {
    device = MTLCreateSystemDefaultDevice();

    NSError *libraryError = NULL;
    NSString *libraryFile = [[NSBundle mainBundle] pathForResource:@"EffectComputeFunctions" ofType:@"metallib"];
    if (!libraryFile) {
        NSLog(@"Library file error");
        enabled = false;
    }
    library = [device newLibraryWithFile:libraryFile error:&libraryError];
    if (!library) {
        NSLog(@"Library error: %@", libraryError);
        enabled = false;
    }

    commandQueue = [device newCommandQueue];
    if (!commandQueue) {
        enabled = false;
    }
}
MetalComputeUtilities::~MetalComputeUtilities() {
    @autoreleasepool {
        commandQueue = nil;
        library = nil;
        device = nil;
    }
}

id<MTLComputePipelineState> MetalComputeUtilities::FindComputeFunction(const char *name) {
    NSString *fname = @(name);
    id<MTLFunction> function = [library newFunctionWithName:fname];
    NSError *error = NULL;
    id<MTLComputePipelineState> ps = [device newComputePipelineStateWithFunction:function error:&error];
    if (!ps) {
        NSLog(@"Library error: %@", error);
    }
    return ps;
}




