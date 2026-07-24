#pragma once

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

#include <array>
#include <atomic>

#include "GPURenderUtils.h"
#include "MetalEffectDataTypes.h"
#include "../../render/PixelBuffer.h"

class PixelBufferClass;
class RenderBuffer;

class MetalPixelBufferComputeData {
public:
    MetalPixelBufferComputeData();
    ~MetalPixelBufferComputeData() = default;

    bool doTransitions(PixelBufferClass *pixelBuffer, int layer, RenderBuffer *prevRB);
    bool doBlendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels);

    bool doTransition(id<MTLComputePipelineState> f, TransitionData &data, RenderBuffer *buffer, RenderBuffer *prevRB);
    bool doMap(id<MTLComputePipelineState> f, TransitionData &data, RenderBuffer *buffer);
    bool doTransition(id<MTLComputePipelineState> f, TransitionData &data, RenderBuffer *buffer, id<MTLBuffer> prev);

    id<MTLBuffer> sparkleBuffer;
    size_t sparkleBufferCount = 0;
    id<MTLBuffer> tmpBufferBlend;
};

class MetalRenderBufferComputeData {
public:
    enum CurrentDataLocation {
        BUFFER,
        TEXTURE
    };
    
    MetalRenderBufferComputeData(RenderBuffer *rb, MetalPixelBufferComputeData *pixelBufferData, int l);
    ~MetalRenderBufferComputeData();
    
    void bufferResized();

    MetalPixelBufferComputeData *pixelBufferData;


    static MetalRenderBufferComputeData *getMetalRenderBufferComputeData(RenderBuffer *);

    id<MTLCommandBuffer> getCommandBuffer(const std::string &postfix = "");
    void abortCommandBuffer();
    
    id<MTLBuffer> getPixelBuffer(bool sendToGPU = true);
    id<MTLTexture> getPixelTexture();
    id<MTLBuffer> getPixelBufferCopy();
    id<MTLBuffer> getIndexBuffer();
    id<MTLBuffer> getBlendBuffer();
    id<MTLBuffer> getOwnerBuffer();
    id<MTLBuffer> getCPUMaskBuffer(int sz);


    void commit();
    bool isCommitted() { return committed; }
    // True when this frame's effect left GPU work queued (an open, uncommitted
    // command buffer).  The render cache uses this to avoid draining that queue
    // (a pipeline-breaking download) just to store a cheap-to-recompute effect.
    bool hasOpenCommandBuffer() { return commandBuffer != nil && !committed; }
    void waitForCompletion();

    void setDataLocation(CurrentDataLocation dl) { currentDataLocation = dl; }
    bool blur(int radius);
    // Integer box blur (small-blur case). Only worthwhile when the buffer is
    // already GPU-resident (no upload) -- the caller gates on that.
    bool boxBlur(int d, int u);
    bool rotoZoom(GPURenderUtils::RotoZoomSettings &settings);

    id<MTLBuffer> maskBuffer;
private:
    bool callRotoZoomFunction(id<MTLComputePipelineState> f, id<MTLComputePipelineState> claimF, RotoZoomData &data);
    
    RenderBuffer *renderBuffer;
    int layer;
    id<MTLCommandBuffer> commandBuffer;
    GpuCommandBufferTag cbTag;  // XL_RENDER_PROFILE; inert (null profile) when off
    id<MTLBuffer> blendBuffer;
    id<MTLBuffer> pixelBuffer;
    id<MTLBuffer> pixelBufferCopy;
    id<MTLTexture> pixelTexture;
    int pixelBufferSize;
    id<MTLBuffer> indexBuffer;
    int32_t *indexes;
    int indexesSize;
    // pixel index -> node index that deterministically owns the pixel (the
    // last node in Nodes order covering it).  PutColorsForNodes gates its
    // scatter on this so concurrent GPU threads never race for a shared
    // pixel; rebuilt lazily after any geometry change (see bufferResized).
    id<MTLBuffer> ownerBuffer;
    int ownerSize = 0;
    bool ownerStale = true;
    // per-dispatch scratch for the rotozoom claim pass (pixel -> winning
    // source index); refilled to -1 before every rotation, so no staleness
    // tracking is needed.
    id<MTLBuffer> rotoOwnerBuffer;
    int rotoOwnerSize = 0;
    // cached grow-only staging buffer for CPU-built transition masks (the GPU
    // transition path uses maskBuffer instead); avoids a per-blend allocation.
    id<MTLBuffer> cpuMaskBuffer = nil;
    int cpuMaskBufferSize = 0;
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

    // ~Number of GPU cores, for sizing the render pool (0 if Metal unusable).
    int gpuCoreCount();

    id<MTLComputePipelineState> FindComputeFunction(const char *name);

    bool enabled = true;
    std::atomic<bool> pg = false;

    id<MTLDevice> device;
    id<MTLLibrary> library;
    // One queue for all effect compute.  A pool of queues sharded per
    // PixelBuffer was tried and measured no faster on Metal, and no GPU we can
    // test exposes more than one compute queue on Vulkan, so it was dropped
    // rather than carry untestable command-ordering risk.
    id<MTLCommandQueue> commandQueue;
    NSUInteger maxTextureSize = 16384;
    NSUInteger metalBufferSizeThreshold = 2048;

    id<MTLComputePipelineState> xrotateFunction;
    id<MTLComputePipelineState> yrotateFunction;
    id<MTLComputePipelineState> zrotateFunction;
    id<MTLComputePipelineState> xrotateClaimFunction;
    id<MTLComputePipelineState> yrotateClaimFunction;
    id<MTLComputePipelineState> zrotateClaimFunction;
    id<MTLComputePipelineState> rotateBlankFunction;
    id<MTLComputePipelineState> tentBlurHFunction;
    id<MTLComputePipelineState> tentBlurVFunction;
    id<MTLComputePipelineState> boxBlurFunction;
    
    id<MTLComputePipelineState> getColorsFunction;
    id<MTLComputePipelineState> putColorsFunction;
    id<MTLComputePipelineState> adjustHSVFunction;
    id<MTLComputePipelineState> applySparklesFunction;
    id<MTLComputePipelineState> brightnessContrastFunction;
    id<MTLComputePipelineState> brightnessLevelFunction;
    id<MTLComputePipelineState> firstLayerFadeFunction;
    id<MTLComputePipelineState> nonAlphaFadeFunction;

    class BlendFunctionInfo {
    public:
        BlendFunctionInfo(const char *fn, int mtd = 0, bool needIndexes = false);
        ~BlendFunctionInfo() = default;
        
        id<MTLComputePipelineState> function;
        std::string name;
        int mixTypeData;
        bool needIndexes;
    };
    std::map<MixTypes, BlendFunctionInfo*> blendFunctions;

    
    
    class TransitionInfo {
    public:
        TransitionInfo(int t);
        TransitionInfo(const char *fn, int t, bool r = false);
        ~TransitionInfo() = default;
        id<MTLComputePipelineState> function;
        int type;
        bool reversed;
    };
    std::map<std::string, TransitionInfo*> transitions;
    id<MTLBuffer> dissolveBuffer;
    
    static MetalComputeUtilities INSTANCE;
};
