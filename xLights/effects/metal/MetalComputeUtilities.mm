
#include "MetalComputeUtilities.hpp"
#include "../../PixelBuffer.h"
#include "../../RenderBuffer.h"

#include <MetalPerformanceShaders/MetalPerformanceShaders.h>

#include "MetalEffectDataTypes.h"

MetalComputeUtilities MetalComputeUtilities::INSTANCE;


MetalPixelBufferComputeData::MetalPixelBufferComputeData() {
}
MetalPixelBufferComputeData::~MetalPixelBufferComputeData() {
}

static std::mutex commandBufferMutex;
std::atomic<uint32_t> MetalRenderBufferComputeData::commandBufferCount(0);
#define MAX_COMMANDBUFFER_COUNT 256

MetalRenderBufferComputeData::MetalRenderBufferComputeData(RenderBuffer *rb, MetalPixelBufferComputeData *pbd) : renderBuffer(rb), pixelBufferData(pbd) {
    commandBuffer = nil;
    pixelBuffer = nil;
    pixelBufferCopy = nil;
    pixelTexture = nil;
    pixelBufferSize = 0;
    pixelTextureSize = {0, 0};
}
MetalRenderBufferComputeData::~MetalRenderBufferComputeData() {
    pixelBufferData = nullptr;
    @autoreleasepool {
        if (commandBuffer != nil) {
            [commandBuffer release];
            --commandBufferCount;
        }
        if (pixelBuffer != nil) {
            [pixelBuffer release];
        }
        if (pixelBufferCopy != nil) {
            [pixelBufferCopy release];
        }
        if (pixelTexture != nil) {
            [pixelTexture release];
        }
    }
}

id<MTLCommandBuffer> MetalRenderBufferComputeData::getCommandBuffer() {
    if (commandBuffer == nil) {
        int max = MAX_COMMANDBUFFER_COUNT - 4;
        if (MetalComputeUtilities::INSTANCE.prioritizeGraphics()) {
            // use a lower command buffer count if the GPU is needed for frontend
            // 64 is the "default" in macOS, we'll try it
            max = 64;
        }
        
        if (commandBufferCount.fetch_add(1) > max) {
            --commandBufferCount;
            return nil;
        }
        std::unique_lock<std::mutex> lock(commandBufferMutex);
        commandBuffer = [[MetalComputeUtilities::INSTANCE.commandQueue commandBuffer] retain];
        NSString* mn = [NSString stringWithUTF8String:renderBuffer->GetModelName().c_str()];
        [commandBuffer setLabel:mn];
    }
    return commandBuffer;
}
void MetalRenderBufferComputeData::abortCommandBuffer() {
    @autoreleasepool {
        [commandBuffer release];
        commandBuffer = nil;
        --commandBufferCount;
    }
}

id<MTLBuffer> MetalRenderBufferComputeData::getPixelBufferCopy() {
    if (pixelBufferCopy == nil) {
        int bufferSize = std::max((int)renderBuffer->GetPixelCount(), (int)pixelBufferSize) * 4;
        id<MTLBuffer> newBuffer = [[MetalComputeUtilities::INSTANCE.device newBufferWithLength:bufferSize options:MTLResourceStorageModePrivate] retain];
        std::string name = renderBuffer->GetModelName() + "PixelBufferCopy";
        NSString* mn = [NSString stringWithUTF8String:name.c_str()];
        [newBuffer setLabel:mn];
        pixelBufferCopy = newBuffer;
    }
    return pixelBufferCopy;
}
void MetalRenderBufferComputeData::bufferResized() {
    if (pixelBuffer && pixelBufferSize < renderBuffer->GetPixelCount()) {
        //buffer needs to get bigger
        getPixelBuffer(false);
    }
}

id<MTLBuffer> MetalRenderBufferComputeData::getPixelBuffer(bool sendToGPU) {
    if (pixelBufferSize < renderBuffer->GetPixelCount()) {
        if (pixelBuffer) {
            [pixelBuffer release];
        }
        if (pixelBufferCopy) {
            [pixelBufferCopy release];
            pixelBufferCopy = nil;
        }
        int bufferSize = renderBuffer->GetPixelCount() * 4;
        id<MTLBuffer> newBuffer = [[MetalComputeUtilities::INSTANCE.device newBufferWithLength:bufferSize options:MTLResourceStorageModeShared] retain];
        std::string name = renderBuffer->GetModelName() + "PixelBuffer";
        NSString* mn = [NSString stringWithUTF8String:name.c_str()];
        [newBuffer setLabel:mn];
        memcpy(newBuffer.contents, renderBuffer->pixels, pixelBufferSize * 4);
        pixelBufferSize = renderBuffer->pixelVector.size();
        pixelBuffer = newBuffer;
        renderBuffer->pixels = static_cast<xlColor*>(pixelBuffer.contents);
        currentDataLocation = BUFFER;
    }
    if (currentDataLocation == TEXTURE) {
        //use GPU to copy over to Buffer
        NSUInteger bytesPerRow = 4 * renderBuffer->BufferWi;
        NSUInteger bytesPerImage = bytesPerRow * renderBuffer->BufferHt;
        MTLSize size = MTLSizeMake(renderBuffer->BufferWi, renderBuffer->BufferHt, 1);
        id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
        [blitCommandEncoder setLabel:@"CopyTextureToBuffer"];
        [blitCommandEncoder copyFromTexture:pixelTexture
                                sourceSlice:0
                                sourceLevel:0
                               sourceOrigin:{0,0,0}
                                 sourceSize:size
                                   toBuffer:pixelBuffer
                          destinationOffset:0
                     destinationBytesPerRow:bytesPerRow
                   destinationBytesPerImage:bytesPerImage];
        [blitCommandEncoder endEncoding];
        currentDataLocation = BUFFER;
    }
    return pixelBuffer;
}
id<MTLTexture> MetalRenderBufferComputeData::getPixelTexture() {
    getPixelBuffer(true);
    
    if (pixelTexture != nil &&
        (renderBuffer->BufferWi != pixelTextureSize.first
        || renderBuffer->BufferHt != pixelTextureSize.second)) {
     
        @autoreleasepool {
            [pixelTexture release];
            pixelTexture = nil;
        }
        pixelTextureSize = { 0, 0 };
    }
    
    if (pixelTexture == nil) {
        @autoreleasepool {
            MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: MTLPixelFormatRGBA8Unorm
                                                                                         width: renderBuffer->BufferWi
                                                                                        height: renderBuffer->BufferHt
                                                                                     mipmapped: NO];
            d.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
            d.storageMode = MTLStorageModePrivate;
            // Create the texture from the device by using the descriptor
            pixelTexture = [[MetalComputeUtilities::INSTANCE.device newTextureWithDescriptor:d] retain];
            
            std::string name = renderBuffer->GetModelName() + "PixelTexture";
            NSString* mn = [NSString stringWithUTF8String:name.c_str()];
            [pixelTexture setLabel:mn];
            pixelTextureSize = { renderBuffer->BufferWi, renderBuffer->BufferHt };
        }
    }
    if (currentDataLocation == BUFFER) {
        NSUInteger bytesPerRow = 4 * renderBuffer->BufferWi;
        NSUInteger bytesPerImage = bytesPerRow * renderBuffer->BufferHt;
        MTLSize size = MTLSizeMake(renderBuffer->BufferWi, renderBuffer->BufferHt, 1);
        id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
        [blitCommandEncoder setLabel:@"CopyBufferToTexture"];
        [blitCommandEncoder copyFromBuffer:pixelBuffer
                              sourceOffset:0
                         sourceBytesPerRow:bytesPerRow
                       sourceBytesPerImage:bytesPerImage
                                sourceSize:size
                                 toTexture:pixelTexture
                          destinationSlice:0
                          destinationLevel:0
                         destinationOrigin:{0,0,0}];
        [blitCommandEncoder endEncoding];
        currentDataLocation = TEXTURE;
    }
    return pixelTexture;
}

void MetalRenderBufferComputeData::commit() {
    if (commandBuffer != nil && !committed) {
        @autoreleasepool {
            if (currentDataLocation == TEXTURE) {
                //use GPU to copy over to Buffer
                NSUInteger bytesPerRow = 4 * renderBuffer->BufferWi;
                NSUInteger bytesPerImage = bytesPerRow * renderBuffer->BufferHt;
                MTLSize size = MTLSizeMake(renderBuffer->BufferWi, renderBuffer->BufferHt, 1);
                id <MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
                [blitCommandEncoder setLabel:@"CopyTextureToBufferForCommit"];

                [blitCommandEncoder copyFromTexture:pixelTexture
                                        sourceSlice:0
                                        sourceLevel:0
                                       sourceOrigin:{0,0,0}
                                         sourceSize:size
                                           toBuffer:pixelBuffer
                                  destinationOffset:0
                             destinationBytesPerRow:bytesPerRow
                           destinationBytesPerImage:bytesPerImage];
                [blitCommandEncoder endEncoding];
                currentDataLocation = BUFFER;
            }
        }
        std::unique_lock<std::mutex> lock(commandBufferMutex);
        [commandBuffer commit];
        committed = true;
    }
}


void MetalRenderBufferComputeData::waitForCompletion() {
    if (commandBuffer != nil) {
        @autoreleasepool {
            commit();
            [commandBuffer waitUntilCompleted];
            [commandBuffer release];
            commandBuffer = nil;
            committed = false;
            --commandBufferCount;
        }
    }
}
bool MetalRenderBufferComputeData::blur(int radius) {
    if ((renderBuffer->BufferHt < (radius * 2)) || (renderBuffer->BufferWi < (radius * 2)) || ((renderBuffer->BufferWi * renderBuffer->BufferHt) < 1024)) {
        // Smallish buffer, overhead of sending to GPU will be more than the gain
        return false;
    }
    @autoreleasepool {
        id<MTLCommandBuffer> commandBuffer = getCommandBuffer();
        if (commandBuffer == nil) {
            return false;
        }
        getPixelTexture();
        
        /*
        float sigma = radius - 1;
        sigma = std::sqrt(sigma);
        MPSImageGaussianBlur* gblur = [[MPSImageGaussianBlur alloc] initWithDevice:MetalComputeUtilities::INSTANCE.device sigma:sigma];
        */
        //tent blur is closest to what is implemented on the C++/CPU side
        float r = (radius - 1) * 2 - 1;
        MPSImageTent *gblur = [[MPSImageTent alloc] initWithDevice:MetalComputeUtilities::INSTANCE.device
                                                     kernelWidth:r
                                                    kernelHeight:r];
        [gblur setEdgeMode:MPSImageEdgeModeClamp];
        [gblur setLabel:@"Blur"];

        MPSCopyAllocator myAllocator = ^id <MTLTexture>( MPSKernel * __nonnull filter,
                                                        __nonnull id <MTLCommandBuffer> cmdBuf,
                                                        __nonnull id <MTLTexture> sourceTexture)
        {
            MTLPixelFormat format = sourceTexture.pixelFormat;
            MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: format
                                                                                         width: sourceTexture.width
                                                                                        height: sourceTexture.height
                                                                                     mipmapped: NO];
            [sourceTexture release];
            d.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
            return [cmdBuf.device newTextureWithDescriptor: d];
        };
        [commandBuffer pushDebugGroup:@"Blur"];
        BOOL ok = [gblur encodeToCommandBuffer:commandBuffer
                                inPlaceTexture:&pixelTexture
                         fallbackCopyAllocator:myAllocator];
        [commandBuffer popDebugGroup];
        [gblur release];
        return ok;
    }
}

bool MetalRenderBufferComputeData::rotoZoom(GPURenderUtils::RotoZoomSettings &settings) {
    if ((renderBuffer->BufferWi * renderBuffer->BufferHt) < 256) {
        // Smallish buffer, overhead of sending to GPU will be more than the gain
        return false;
    }
    
    RotoZoomData data;
    data.width = renderBuffer->BufferWi;
    data.height = renderBuffer->BufferHt;
    
    data.offset = settings.offset;
    data.xrotation = settings.xrotation;
    data.xpivot = settings.xpivot;
    data.yrotation = settings.yrotation;
    data.ypivot = settings.ypivot;
    data.zrotation = settings.zrotation;
    data.zoom = settings.zoom;
    data.zoomquality = settings.zoomquality;
    data.pivotpointx = settings.pivotpointx;
    data.pivotpointy = settings.pivotpointy;
    
    id<MTLBuffer> bufferResult = getPixelBuffer();
    if (bufferResult == nil) {
        return false;
    }
    id<MTLBuffer> bufferCopy = getPixelBufferCopy();
    if (bufferCopy == nil) {
        return false;
    }
    bool dbg = false;
    if (commandBuffer != nil) {
        dbg = true;
        [commandBuffer pushDebugGroup:@"RotoZoom"];
    }
    for (auto &c : settings.rotationorder) {
        switch (c) {
            case 'X':
                if (data.xrotation != 0 && data.xrotation != 360) {
                    callRotoZoomFunction(MetalComputeUtilities::INSTANCE.xrotateFunction, data);
                }
                break;
            case 'Y':
                if (data.yrotation != 0 && data.yrotation != 360) {
                    callRotoZoomFunction(MetalComputeUtilities::INSTANCE.yrotateFunction, data);
                }
                break;
            case 'Z':
                if (data.zrotation != 0.0 || data.zoom != 1.0) {
                    callRotoZoomFunction(MetalComputeUtilities::INSTANCE.zrotateFunction, data);
                }
                break;
        }
    }
    if (dbg) {
        [commandBuffer popDebugGroup];
    }
    return true;
}

bool MetalRenderBufferComputeData::callRotoZoomFunction(id<MTLComputePipelineState> &function, RotoZoomData &data) {
    id<MTLCommandBuffer> commandBuffer = getCommandBuffer();
    if (commandBuffer == nil) {
        return false;
    }
    id<MTLBuffer> bufferResult = getPixelBuffer();
    id<MTLBuffer> bufferCopy = getPixelBufferCopy();
    id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
    [blitCommandEncoder setLabel:@"CopyDataToCopyBuffer"];
    [blitCommandEncoder copyFromBuffer:bufferResult
                          sourceOffset:0
                              toBuffer:bufferCopy
                     destinationOffset:0
                                  size:(data.width*data.height*4)];
    [blitCommandEncoder endEncoding];
    
    id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
    [computeEncoder setLabel:MetalComputeUtilities::INSTANCE.rotateBlankFunction.label];
    [computeEncoder setComputePipelineState:MetalComputeUtilities::INSTANCE.rotateBlankFunction];
    
    NSInteger dataSize = sizeof(data);
    [computeEncoder setBytes:&data length:dataSize atIndex:0];
    [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
    int w = MetalComputeUtilities::INSTANCE.rotateBlankFunction.threadExecutionWidth;
    int h = MetalComputeUtilities::INSTANCE.rotateBlankFunction.maxTotalThreadsPerThreadgroup / w;
    MTLSize threadsPerThreadgroup = MTLSizeMake(w, h, 1);
    MTLSize threadsPerGrid = MTLSizeMake(data.width, data.height, 1);
    [computeEncoder dispatchThreads:threadsPerGrid
              threadsPerThreadgroup:threadsPerThreadgroup];
    [computeEncoder endEncoding];
    
    computeEncoder = [commandBuffer computeCommandEncoder];
    [computeEncoder setLabel:function.label];
    [computeEncoder setComputePipelineState:function];
    
    dataSize = sizeof(data);
    [computeEncoder setBytes:&data length:dataSize atIndex:0];
    [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
    [computeEncoder setBuffer:bufferCopy offset:0 atIndex:2];
    w = function.threadExecutionWidth;
    h = function.maxTotalThreadsPerThreadgroup / w;
    threadsPerThreadgroup = MTLSizeMake(w, h, 1);
    threadsPerGrid = MTLSizeMake(data.width, data.height, 1);
    [computeEncoder dispatchThreads:threadsPerGrid
              threadsPerThreadgroup:threadsPerThreadgroup];

    [computeEncoder endEncoding];
    return true;
}

MetalRenderBufferComputeData *MetalRenderBufferComputeData::getMetalRenderBufferComputeData(RenderBuffer *b) {
    return static_cast<MetalRenderBufferComputeData*>(b->gpuRenderData);
}



MetalComputeUtilities::MetalComputeUtilities() {
    enabled = false;
    device = nil;
    
    NSArray *devices = MTLCopyAllDevices();
    for (id d in devices) {
        if ([d isRemovable]) {
            device = [d retain];
        }
    }
    [devices release];
    if (device == nil) {
        device = MTLCreateSystemDefaultDevice();
    }
    if (device.argumentBuffersSupport == MTLArgumentBuffersTier1) {
        device = nil;
        return;
    }

    NSError *libraryError = NULL;
    NSString *libraryFile = [[NSBundle mainBundle] pathForResource:@"EffectComputeFunctions" ofType:@"metallib"];
    if (!libraryFile) {
        NSLog(@"Library file error");
        return;
    }
    library = [device newLibraryWithFile:libraryFile error:&libraryError];
    if (!library) {
        NSLog(@"Library error: %@", libraryError);
        return;
    }
    [library setLabel:@"EffectComputeFunctionsLibrary"];

    commandQueue = [device newCommandQueueWithMaxCommandBufferCount:MAX_COMMANDBUFFER_COUNT];
    if (!commandQueue) {
        return;
    }
    [commandQueue setLabel:@"MetalEffectCommandQueue"];
    enabled = true;
    
    xrotateFunction = FindComputeFunction("RotoZoomRotateX");
    yrotateFunction = FindComputeFunction("RotoZoomRotateY");
    zrotateFunction = FindComputeFunction("RotoZoomRotateZ");
    rotateBlankFunction = FindComputeFunction("RotoZoomBlank");
}
MetalComputeUtilities::~MetalComputeUtilities() {
    @autoreleasepool {
        commandQueue = nil;
        library = nil;
        device = nil;
        
        xrotateFunction = nil;
        yrotateFunction = nil;
        zrotateFunction = nil;
        rotateBlankFunction = nil;
    }
}

id<MTLComputePipelineState> MetalComputeUtilities::FindComputeFunction(const char *name) {
    NSString *fname = @(name);
    id<MTLFunction> function = [library newFunctionWithName:fname];
    NSError *error = NULL;
    
    MTLComputePipelineDescriptor *desc = [MTLComputePipelineDescriptor new];
    desc.computeFunction = function;
    desc.label = fname;
    
    //id<MTLComputePipelineState> ps = [device newComputePipelineStateWithFunction:function error:&error];
    id<MTLComputePipelineState> ps = [device newComputePipelineStateWithDescriptor:desc
                                                                           options:MTLPipelineOptionNone
                                                                        reflection:nil
                                                                             error:&error];
    [function release];
    [desc release];
    if (!ps) {
        NSLog(@"Library error: %@", error);
    }
    return ps;
}




extern "C" {
bool isMetalComputeSupported() {
    return MetalComputeUtilities::INSTANCE.enabled;
}
}

