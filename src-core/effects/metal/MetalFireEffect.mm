/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "../../render/Effect.h"
#include "UtilClasses.h"

// The flame grid has to survive between frames, and it is per model/layer rather
// than per effect instance, so it hangs off the RenderBuffer's infoCache under a
// key well clear of the effect ids that share that map.  EffectRenderCache has a
// virtual destructor, so ARC releases the buffers with the RenderBuffer.
static constexpr int FIRE_GPU_CACHE_KEY = 0x51F13E;

class MetalFireGpuCache : public EffectRenderCache {
public:
    MetalFireGpuCache() = default;
    virtual ~MetalFireGpuCache() = default;

    // Ping-ponged: the advance reads prev and writes next, then they swap.
    id<MTLBuffer> prev = nil;
    id<MTLBuffer> next = nil;
    id<MTLBuffer> lut = nil;
    int cells = 0;
    bool resident = false; // prev holds a grid the GPU itself advanced
};

// All Metal work lives here rather than in MetalFireEffect's member functions:
// RenderableEffect has an `int id` data member, which shadows the Objective-C
// `id` type inside anything scoped to the effect class.
class MetalFireEffectData {
public:
    MetalFireEffectData() {
        advanceFn = MetalComputeUtilities::INSTANCE.FindComputeFunction("FireAdvanceEffect");
        drawFn = MetalComputeUtilities::INSTANCE.FindComputeFunction("FireDrawEffect");
    }

    bool canRender() { return advanceFn != nil && drawFn != nil; }

    bool Render(RenderBuffer &buffer, FireEffect::FireFrameParams &params, int paletteSize);

    id<MTLComputePipelineState> advanceFn = nil;
    id<MTLComputePipelineState> drawFn = nil;
};

MetalFireEffect::MetalFireEffect(int i) : FireEffect(i) {
    data = new MetalFireEffectData();
}
MetalFireEffect::~MetalFireEffect() {
    if (data) {
        delete data;
    }
}

static MetalFireGpuCache *getGpuCache(RenderBuffer &buffer) {
    MetalFireGpuCache *c = (MetalFireGpuCache *)buffer.infoCache[FIRE_GPU_CACHE_KEY];
    if (c == nullptr) {
        c = new MetalFireGpuCache();
        buffer.infoCache[FIRE_GPU_CACHE_KEY] = c;
    }
    return c;
}

bool MetalFireEffectData::Render(RenderBuffer &buffer, FireEffect::FireFrameParams &params, int paletteSize) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !canRender() || buffer.IsDmxBuffer()
        || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        // DMX needs the SetPixel channel translation; the CPU path handles it.
        return false;
    }
    const int cells = params.maxMWi * params.maxMHt;
    const int npix = std::min((int)buffer.GetPixelCount(), buffer.BufferWi * buffer.BufferHt);
    if (cells < 1 || npix < 1) {
        return false;
    }

    @autoreleasepool {
        MetalFireGpuCache *gpu = getGpuCache(buffer);
        id<MTLDevice> dev = MetalComputeUtilities::INSTANCE.device;

        if (gpu->cells != cells || gpu->prev == nil || gpu->next == nil) {
            gpu->prev = [dev newBufferWithLength:cells * sizeof(int32_t) options:MTLResourceStorageModeShared];
            gpu->next = [dev newBufferWithLength:cells * sizeof(int32_t) options:MTLResourceStorageModeShared];
            gpu->cells = cells;
            gpu->resident = false;
            if (gpu->prev == nil || gpu->next == nil) {
                return false;
            }
        }
        // Seed (or re-seed) the resident grid from the CPU's copy. After that the
        // GPU owns it and nothing is uploaded per frame.
        if (!gpu->resident || params.gridChangedOnCpu) {
            memcpy([gpu->prev contents], params.grid->data(), cells * sizeof(int32_t));
            gpu->resident = true;
        }

        // Once the GPU has advanced the resident grid, the CPU's copy is stale.
        // Any bail-out from here on has to hand the real state back, or the CPU
        // path would advance a frame that is several frames behind.
        auto bailToCpu = [&]() {
            if (gpu->resident) {
                memcpy(params.grid->data(), [gpu->prev contents], cells * sizeof(int32_t));
                gpu->resident = false;
            }
            return false;
        };

        const size_t lutBytes = (size_t)paletteSize * sizeof(uint32_t);
        if (gpu->lut == nil) {
            gpu->lut = [dev newBufferWithLength:lutBytes options:MTLResourceStorageModeShared];
            if (gpu->lut == nil) {
                return bailToCpu();
            }
        }
        // Cheap enough to just refresh; the hue shift can be value-curved.
        memcpy([gpu->lut contents], params.lut, lutBytes);

        id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
        if (commandBuffer == nil) {
            return bailToCpu();
        }
        id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
        if (bufferResult == nil) {
            rbcd->abortCommandBuffer();
            return bailToCpu();
        }

        MetalFireData fd;
        fd.width = params.maxMWi;
        fd.height = params.maxMHt;
        fd.curWi = params.curWi;
        fd.curHt = params.curHt;
        fd.bufferWi = buffer.BufferWi;
        fd.bufferHt = buffer.BufferHt;
        fd.npix = npix;
        fd.loc = params.loc;
        fd.step = params.step;
        fd.pad = 0;
        fd.frameSeed = params.frameSeed;

        id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];
        if (encoder == nil) {
            rbcd->abortCommandBuffer();
            return bailToCpu();
        }
        [encoder setLabel:@"FireEffect"];

        // Advance: one thread per grid cell, reading last frame's grid.
        [encoder setComputePipelineState:advanceFn];
        [encoder setBytes:&fd length:sizeof(fd) atIndex:0];
        [encoder setBuffer:gpu->prev offset:0 atIndex:1];
        [encoder setBuffer:gpu->next offset:0 atIndex:2];
        NSInteger aw = std::min<NSInteger>(advanceFn.threadExecutionWidth, params.maxMWi);
        NSInteger ah = std::max<NSInteger>(1, std::min<NSInteger>(advanceFn.maxTotalThreadsPerThreadgroup / std::max<NSInteger>(aw, 1), params.maxMHt));
        [encoder dispatchThreads:MTLSizeMake(params.maxMWi, params.maxMHt, 1)
           threadsPerThreadgroup:MTLSizeMake(std::max<NSInteger>(aw, 1), ah, 1)];

        // Draw: one thread per displayed pixel, from the grid just written.
        [encoder setComputePipelineState:drawFn];
        [encoder setBytes:&fd length:sizeof(fd) atIndex:0];
        [encoder setBuffer:gpu->next offset:0 atIndex:1];
        [encoder setBuffer:gpu->lut offset:0 atIndex:2];
        [encoder setBuffer:bufferResult offset:0 atIndex:3];
        NSInteger dw = std::min<NSInteger>(drawFn.threadExecutionWidth, params.curWi);
        NSInteger dh = std::max<NSInteger>(1, std::min<NSInteger>(drawFn.maxTotalThreadsPerThreadgroup / std::max<NSInteger>(dw, 1), params.curHt));
        [encoder dispatchThreads:MTLSizeMake(params.curWi, params.curHt, 1)
           threadsPerThreadgroup:MTLSizeMake(std::max<NSInteger>(dw, 1), dh, 1)];
        [encoder endEncoding];

        // Both dispatches share one encoder, so the draw already sees the
        // advance's writes; swap so next frame reads what was just produced.
        std::swap(gpu->prev, gpu->next);

        // The whole grid was advanced, so the flame may have grown; the CPU
        // fallback's row bound must not clip it if it ever takes over again.
        params.liveRow = params.maxMHt - 1;
    }
    return true;
}

bool MetalFireEffect::RenderFireGPU(RenderBuffer &buffer, FireFrameParams &params) {
    return data->Render(buffer, params, FirePaletteSize());
}
