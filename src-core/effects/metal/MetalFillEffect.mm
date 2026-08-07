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
#include "UtilClasses.h"
#include "../FillEffect.h"

class MetalFillEffectData {
public:
    MetalFillEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("FillEffect");
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalFillData &fdata, const xlColorVector &lut, const std::vector<uint8_t> &painted, RenderBuffer &buffer) {
        @autoreleasepool {
            MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
            if (!rbcd) return false;

            id<MTLCommandBuffer> commandBuffer = rbcd->getCommandBuffer();
            if (commandBuffer == nil) return false;

            id<MTLBuffer> bufferResult = rbcd->getPixelBuffer();
            if (bufferResult == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"FillEffect"];
            [computeEncoder setComputePipelineState:fn];

            [computeEncoder setBytes:&fdata length:sizeof(fdata) atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
            [computeEncoder setBytes:lut.data() length:(lut.size() * sizeof(xlColor)) atIndex:2];
            [computeEncoder setBytes:painted.data() length:(painted.size() * sizeof(uint8_t)) atIndex:3];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            // Bound by the actual pixel allocation, not the logical dimensions — a
            // variable sub-buffer can leave GetPixelCount() < width * height and the
            // kernel writes result[index] with no bounds check.
            NSInteger pixelCount = std::min((NSInteger)(fdata.width * fdata.height), (NSInteger)buffer.GetPixelCount());
            NSInteger threads = std::min(pixelCount, maxThreads);
            MTLSize gridSize = MTLSizeMake(pixelCount, 1, 1);
            MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);

            [computeEncoder dispatchThreads:gridSize
                      threadsPerThreadgroup:threadsPerThreadgroup];
            [computeEncoder endEncoding];
        }
        return true;
    }

private:
    id<MTLComputePipelineState> fn = nil;
};


MetalFillEffect::MetalFillEffect(int i) : FillEffect(i) {
    data = new MetalFillEffectData();
}
MetalFillEffect::~MetalFillEffect() {
    if (data) { delete data; }
}

void MetalFillEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    // DMX models need SetPixel's channel translation, which the linear kernel write
    // can't reproduce; those (and undersized/unavailable buffers) take the CPU path.
    if (rbcd == nullptr || !data->canRender() || buffer.IsDmxBuffer()
        || buffer.BufferWi <= 0 || buffer.BufferHt <= 0
        || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        FillEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    xlColorVector lut;
    std::vector<uint8_t> painted;
    int vertical = 0;
    int dim = 0;
    BuildFillLut(effect, SettingsMap, buffer, lut, painted, vertical, dim);

    if (dim > MAX_FILL_LUT) {
        // per-line LUT too large for setBytes — CPU/ISPC path (no size cap) handles it
        FillEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    MetalFillData fdata;
    fdata.width = buffer.BufferWi;
    fdata.height = buffer.BufferHt;
    fdata.vertical = vertical;

    if (data->Render(fdata, lut, painted, buffer)) {
        return;
    }
    FillEffect::Render(effect, SettingsMap, buffer);
}
