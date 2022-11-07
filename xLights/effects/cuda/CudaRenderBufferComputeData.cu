
#include "CudaEffectDataTypes.h"
#include "CudaRenderBufferComputeData.cuh"

#include "../../PixelBuffer.h"
#include "../../RenderBuffer.h"

#include <cuda_runtime.h>
#include <device_launch_parameters.h>

__global__ void apply_box_blur(int height, int width, unsigned char* buffer, unsigned char* out) {
    int i, j;

    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    if (row < 2 || col < 2 || row >= height -3 || col >= width -3 ) return ;

    float v = 1.0 / 9.0;
    float kernel[3][3] = { {v,v,v},
                        {v,v,v},
                        {v,v,v} };

    float sum0 = 0.0;
    float sum1 = 0.0;
    float sum2 = 0.0;
    for (i = -1; i <= 1; i++)
    {
        for (j = -1; j <= 1; j++)
        {
            // matrix multiplication with kernel with every color plane
            sum0 = sum0 + (float)kernel[i + 1][j + 1] * buffer[((row + i) * width + (col + j)) * 3 + 0];
            sum1 = sum1 + (float)kernel[i + 1][j + 1] * buffer[((row + i) * width + (col + j)) * 3 + 1];
            sum2 = sum2 + (float)kernel[i + 1][j + 1] * buffer[((row + i) * width + (col + j)) * 3 + 2];
        }
    }
    out[(row * width + col) * 3 + 0] = (unsigned char)sum0;
    out[(row * width + col) * 3 + 1] = (unsigned char)sum1;
    out[(row * width + col) * 3 + 2] = (unsigned char)sum2;
};

CudaPixelBufferComputeData::CudaPixelBufferComputeData() {
}

CudaPixelBufferComputeData::~CudaPixelBufferComputeData() {
}

CudaRenderBufferComputeData::CudaRenderBufferComputeData(RenderBuffer *rb, CudaPixelBufferComputeData *pbd) :
    renderBuffer(rb), 
    pixelBufferData(pbd) {

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
    unsigned char* d_buffer;
    unsigned char* d_out;
    const size_t numPixels = renderBuffer->BufferWi * renderBuffer->BufferHt;

    // allocate space for device copies
    cudaMalloc((void**)&d_buffer, numPixels * 4 * sizeof(unsigned char));
    cudaMalloc((void**)&d_out, numPixels * 4 * sizeof(unsigned char));
    //
    // Copy inputs to device
    cudaMemcpy(d_buffer, renderBuffer->GetPixels(), numPixels * 4 * sizeof(unsigned char), cudaMemcpyHostToDevice);
    //
    //// perform the Box blur and store the resulting pixels in the output buffer
    dim3 blockSize(16, 16, 1);
    dim3 gridSize((numPixels*4)/blockSize.x, (numPixels*4)/blockSize.y, 1);
    apply_box_blur <<<gridSize, blockSize>>> (radius, radius, d_buffer, d_out);
    //copy output from device
    cudaMemcpy(renderBuffer->GetPixels(), d_out, numPixels * 4 * sizeof(unsigned char), cudaMemcpyDeviceToHost);

    cudaFree(d_buffer);
    cudaFree(d_out);
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