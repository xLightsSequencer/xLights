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
#include "../WaveEffect.h"

class MetalWaveEffectData {
public:
    MetalWaveEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("WaveEffect");
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalWaveData &wd, const std::vector<int32_t> &cols, RenderBuffer &buffer) {
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

            // Per-column band [y1,y2] (2 ints per column). Too large for setBytes
            // on wide buffers, so upload as its own shared buffer.
            NSUInteger colsSize = cols.size() * sizeof(int32_t);
            id<MTLBuffer> colBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithBytes:cols.data()
                                                                                          length:colsSize
                                                                                         options:MTLResourceStorageModeShared];
            if (colBuffer == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"WaveEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(wd);
            [computeEncoder setBytes:&wd length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
            [computeEncoder setBuffer:colBuffer offset:0 atIndex:2];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            // Bound by the actual pixel allocation — a variable sub-buffer can
            // leave GetPixelCount() < width * height and the kernel writes
            // result[index] with no bounds check beyond width*height.
            NSInteger pixelCount = std::min((NSInteger)(wd.width * wd.height), (NSInteger)buffer.GetPixelCount());
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


MetalWaveEffect::MetalWaveEffect(int i) : WaveEffect(i) {
    data = new MetalWaveEffectData();
}
MetalWaveEffect::~MetalWaveEffect() {
    if (data) { delete data; }
}

void MetalWaveEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        WaveEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Advances the phase accumulator (and Ivy branch buffer) exactly once. Both
    // the Metal dispatch and every fallback below reuse the bands from here, so
    // the accumulator never double-advances.
    WaveKernelConfig cfg;
    std::vector<int32_t> cols;
    BuildWaveColumns(SettingsMap, buffer, cfg, cols);
    if (buffer.captureSnapshot != nullptr) {
        // Frame-parallel serial capture pass: the phase was advanced + handed
        // off to a later parallel draw; nothing to render now.
        return;
    }

    // Metal implements the None (constant-color) fill only. Rainbow/Palette need
    // a double-precision hue division to stay byte-identical (no double on the
    // GPU); DMX buffers need SetPixel channel translation; a variable sub-buffer
    // can be smaller than BufferWi*BufferHt. Those run on the ISPC CPU path.
    size_t npix = (size_t)buffer.BufferWi * buffer.BufferHt;
    if (cfg.fillColor != WAVE_FILL_NONE || buffer.IsDmxBuffer() || npix > buffer.GetPixelCount()) {
        RenderWaveISPC(cfg, cols, buffer);
        return;
    }

    MetalWaveData wd;
    wd.width = buffer.BufferWi;
    wd.height = buffer.BufferHt;
    wd.yoffset = cfg.yoffset;
    wd.mirror = cfg.mirror;
    wd.noneColor = cfg.noneColor.asChar4();

    if (data->Render(wd, cols, buffer)) {
        return;
    }
    RenderWaveISPC(cfg, cols, buffer);
}
