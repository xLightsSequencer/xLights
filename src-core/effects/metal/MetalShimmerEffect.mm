#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../ShimmerEffect.h"

class MetalShimmerEffectData {
public:
    MetalShimmerEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("ShimmerEffect");
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalShimmerData &sdata, const xlColorVector &lut, RenderBuffer &buffer) {
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
            [computeEncoder setLabel:@"ShimmerEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(sdata);
            [computeEncoder setBytes:&sdata length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
            [computeEncoder setBytes:lut.data() length:(lut.size() * sizeof(xlColor)) atIndex:2];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            // Bound by the actual pixel allocation, not the logical dimensions —
            // a variable sub-buffer can leave GetPixelCount() < width * height and
            // the kernel writes result[index] with no bounds check.
            NSInteger pixelCount = std::min((NSInteger)(sdata.width * sdata.height), (NSInteger)buffer.GetPixelCount());
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


MetalShimmerEffect::MetalShimmerEffect(int i) : ShimmerEffect(i) {
    data = new MetalShimmerEffectData();
}
MetalShimmerEffect::~MetalShimmerEffect() {
    if (data) { delete data; }
}

void MetalShimmerEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || buffer.IsDmxBuffer()
        || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        ShimmerEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int colorIdx = 0;
    bool useAllColors = false;
    if (!CalcFrameState(SettingsMap, buffer, colorIdx, useAllColors)) {
        // duty-cycle "off" frame — buffer stays untouched, same as the CPU path
        return;
    }

    xlColorVector lut;
    MetalShimmerData sdata;
    sdata.width = buffer.BufferWi;
    sdata.height = buffer.BufferHt;
    sdata.lutMode = BuildShimmerLut(buffer, colorIdx, useAllColors, lut);
    sdata.colorCount = buffer.GetColorCount();
    sdata.frameSeed = buffer.hashRandomFrameSeed();

    if (lut.size() > SHIMMER_MAX_LUT) {
        // spatial gradient LUT too large for setBytes — CPU/ISPC path handles it
        ShimmerEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    if (data->Render(sdata, lut, buffer)) {
        return;
    }
    ShimmerEffect::Render(effect, SettingsMap, buffer);
}
