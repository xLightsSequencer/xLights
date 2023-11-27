#pragma once


#include "CudaEffectDataTypes.h"
#include "../../GPURenderUtils.h"

class PixelBufferClass;
class RenderBuffer;
class Blurrer;

class CudaPixelBufferComputeData {
public:
    CudaPixelBufferComputeData();
    ~CudaPixelBufferComputeData();

};

class CudaRenderBufferComputeData {
public:

    CudaRenderBufferComputeData(RenderBuffer *rb, CudaPixelBufferComputeData *pixelBufferData);
    ~CudaRenderBufferComputeData();
    
    void bufferResized();

    CudaPixelBufferComputeData *pixelBufferData;

    static CudaRenderBufferComputeData *getCudaRenderBufferComputeData(RenderBuffer *);

    void commit();
    void waitForCompletion();

    bool blur(int radius);
    bool rotoZoom(GPURenderUtils::RotoZoomSettings &settings);

private:

    RenderBuffer *renderBuffer;
    Blurrer * blurrer;

    int pixelBufferSize;
    bool committed {false};
};