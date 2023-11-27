
#include "CudaEffectDataTypes.h"
#include "CudaRenderBufferComputeData.h"

#include "../../RenderBuffer.h"

#include "blur.h"

//#include <cuda_runtime.h>
//#include <device_launch_parameters.h>


CudaPixelBufferComputeData::CudaPixelBufferComputeData() {
}

CudaPixelBufferComputeData::~CudaPixelBufferComputeData() {
}

CudaRenderBufferComputeData::CudaRenderBufferComputeData(RenderBuffer *rb, CudaPixelBufferComputeData *pbd) :
    renderBuffer(rb), 
    pixelBufferData(pbd) {
    blurrer = Blurrer::factory();
}

CudaRenderBufferComputeData::~CudaRenderBufferComputeData() {
    pixelBufferData = nullptr;
}

void CudaRenderBufferComputeData::commit() {

}

void CudaRenderBufferComputeData::waitForCompletion() {

}

void CudaRenderBufferComputeData::bufferResized() {

}

bool CudaRenderBufferComputeData::blur(int radius) {
    if ((renderBuffer->BufferHt < (radius * 2)) || (renderBuffer->BufferWi < (radius * 2)) || ((renderBuffer->BufferWi * renderBuffer->BufferHt) < 1024)) {
        // Smallish buffer, overhead of sending to GPU will be more than the gain
        return false;
    }
    blurrer->BlurPixels(renderBuffer->GetPixels(), renderBuffer->BufferHt, renderBuffer->BufferWi, radius);
    return true;
}

bool CudaRenderBufferComputeData::rotoZoom(GPURenderUtils::RotoZoomSettings &settings) {
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

    for (auto &c : settings.rotationorder) {
        switch (c) {
            case 'X':
                if (data.xrotation != 0 && data.xrotation != 360) {
                   // callRotoZoomFunction(CudaComputeUtilities::INSTANCE.xrotateFunction, data);
                }
                break;
            case 'Y':
                if (data.yrotation != 0 && data.yrotation != 360) {
                    //callRotoZoomFunction(CudaComputeUtilities::INSTANCE.yrotateFunction, data);
                }
                break;
            case 'Z':
                if (data.zrotation != 0.0 || data.zoom != 1.0) {
                    //callRotoZoomFunction(CudaComputeUtilities::INSTANCE.zrotateFunction, data);
                }
                break;
        }
    }
    return true;
}

CudaRenderBufferComputeData *CudaRenderBufferComputeData::getCudaRenderBufferComputeData(RenderBuffer *b) {
    return static_cast<CudaRenderBufferComputeData*>(b->gpuRenderData);
}