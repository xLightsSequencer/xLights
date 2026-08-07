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

class MetalLifeEffectData {
public:
    MetalLifeEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("LifeEffect");
    }

    bool canRender() { return fn != nil; }

    // The kernel reads the previous generation from prevBuffer (uploaded from
    // TempBuf) and writes the new generation into the shared pixel buffer. The
    // command buffer is committed and waited on so the caller can copy the new
    // generation back into TempBuf (the CPU-side source of truth) afterwards.
    bool Render(MetalLifeData &ld, const std::vector<uint32_t> &palette, RenderBuffer &buffer) {
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

            int npix = ld.npix;
            if (npix < 1) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLBuffer> prevBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithBytes:buffer.GetTempBuf()
                                                                                           length:npix * sizeof(uint32_t)
                                                                                          options:MTLResourceStorageModeShared];
            id<MTLBuffer> paletteBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithBytes:palette.data()
                                                                                             length:palette.size() * sizeof(uint32_t)
                                                                                            options:MTLResourceStorageModeShared];
            if (prevBuffer == nil || paletteBuffer == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"LifeEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(ld);
            [computeEncoder setBytes:&ld length:dataSize atIndex:0];
            [computeEncoder setBuffer:prevBuffer offset:0 atIndex:1];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:2];
            [computeEncoder setBuffer:paletteBuffer offset:0 atIndex:3];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger pixelCount = npix;
            NSInteger threads = std::min(pixelCount, maxThreads);
            MTLSize gridSize = MTLSizeMake(pixelCount, 1, 1);
            MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);

            [computeEncoder dispatchThreads:gridSize
                      threadsPerThreadgroup:threadsPerThreadgroup];
            [computeEncoder endEncoding];

            rbcd->commit();
            rbcd->waitForCompletion();
        }
        return true;
    }

private:
    id<MTLComputePipelineState> fn = nil;
};


MetalLifeEffect::MetalLifeEffect(int i) : LifeEffect(i) {
    data = new MetalLifeEffectData();
}
MetalLifeEffect::~MetalLifeEffect() {
    if (data) { delete data; }
}

void MetalLifeEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || buffer.IsDmxBuffer()
        || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        // DMX needs the SetPixel channel translation; the CPU path (ISPC) handles it.
        LifeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int Type = 0;
    if (!PrepareLifeGeneration(buffer, SettingsMap, Type)) {
        return; // frame satisfied by the tempbuf -> pixels copy
    }
    if (buffer.BufferWi < 1 || buffer.BufferHt < 1) {
        return;
    }

    std::vector<uint32_t> palette;
    BuildLifePalette(buffer, palette);

    MetalLifeData ld;
    ld.width = buffer.BufferWi;
    ld.height = buffer.BufferHt;
    ld.npix = std::min((int)buffer.GetPixelCount(), buffer.BufferWi * buffer.BufferHt);
    ld.type = Type;
    ld.numColors = (int)palette.size();
    ld.frameSeed = buffer.hashRandomFrameSeed();

    if (data->Render(ld, palette, buffer)) {
        buffer.CopyPixelsToTempBuf();
        return;
    }
    // Metal dispatch failed after the generation gate/seed already advanced; finish
    // this generation on the CPU (ISPC) without re-running PrepareLifeGeneration.
    RenderLifeGenerationISPC(buffer, Type);
}
