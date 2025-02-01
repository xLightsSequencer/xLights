#pragma once

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>


#include "GPURenderUtils.h"
#include "MetalEffectDataTypes.h"
#include "PixelBuffer.h"

class PixelBufferClass;
class RenderBuffer;

class MetalPixelBufferComputeData {
public:
    MetalPixelBufferComputeData();
    ~MetalPixelBufferComputeData();

    bool doTransitions(PixelBufferClass *pixelBuffer, int layer, RenderBuffer *prevRB);
    bool doBlendLayers(PixelBufferClass *pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels);

    bool doTransition(id<MTLComputePipelineState> &f, TransitionData &data, RenderBuffer *buffer, RenderBuffer *prevRB);
    bool doMap(id<MTLComputePipelineState> &f, TransitionData &data, RenderBuffer *buffer);
    bool doTransition(id<MTLComputePipelineState> &f, TransitionData &data, RenderBuffer *buffer, id<MTLBuffer> &prev);
    
    id<MTLBuffer> sparkleBuffer;
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


    void commit();
    bool isCommitted() { return committed; }
    void waitForCompletion();
    
    void setDataLocation(CurrentDataLocation dl) { currentDataLocation = dl; }
    bool blur(int radius);
    bool rotoZoom(GPURenderUtils::RotoZoomSettings &settings);

    id<MTLBuffer> maskBuffer;
private:
    bool callRotoZoomFunction(id<MTLComputePipelineState> &f, RotoZoomData &data);
    
    RenderBuffer *renderBuffer;
    int layer;
    id<MTLCommandBuffer> commandBuffer;
    id<MTLBuffer> blendBuffer;
    id<MTLBuffer> pixelBuffer;
    id<MTLBuffer> pixelBufferCopy;
    id<MTLTexture> pixelTexture;
    int pixelBufferSize;
    id<MTLBuffer> indexBuffer;
    int32_t *indexes;
    int indexesSize;
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
        ~BlendFunctionInfo();
        
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
        ~TransitionInfo();
        id<MTLComputePipelineState> function;
        int type;
        bool reversed;
    };
    std::map<std::string, TransitionInfo*> transitions;
    id<MTLBuffer> dissolveBuffer;
    
    static MetalComputeUtilities INSTANCE;
};
