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
#include "../TwinkleEffect.h"

class MetalTwinkleEffectData {
public:
    MetalTwinkleEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("TwinkleEffect");
    }

    bool canRender() { return fn != nil; }

    // The kernel advances the per-light StrobeClass state array in place, so the
    // active entries are committed, waited on, and copied back into the CPU cache:
    // the cache stays the single source of truth and the CPU (ISPC) and Metal
    // paths remain interchangeable mid-sequence.
    bool Render(MetalTwinkleData &td, std::vector<StrobeClass> &states, const xlColorVector &lut, RenderBuffer &buffer) {
        if (td.curNumStrobe == 0) {
            return true; // nothing lit this frame; buffer stays cleared
        }
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

            NSUInteger statesSize = (NSUInteger)td.curNumStrobe * sizeof(StrobeClass);
            id<MTLBuffer> stateBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithBytes:states.data()
                                                                                            length:statesSize
                                                                                           options:MTLResourceStorageModeShared];
            id<MTLBuffer> lutBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithBytes:lut.data()
                                                                                          length:(lut.size() * sizeof(xlColor))
                                                                                         options:MTLResourceStorageModeShared];
            if (stateBuffer == nil || lutBuffer == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"TwinkleEffect"];
            [computeEncoder setComputePipelineState:fn];

            [computeEncoder setBytes:&td length:sizeof(td) atIndex:0];
            [computeEncoder setBuffer:stateBuffer offset:0 atIndex:1];
            [computeEncoder setBuffer:lutBuffer offset:0 atIndex:2];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:3];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger strobeCount = td.curNumStrobe;
            NSInteger threads = std::min(strobeCount, maxThreads);
            MTLSize gridSize = MTLSizeMake(strobeCount, 1, 1);
            MTLSize threadsPerThreadgroup = MTLSizeMake(threads, 1, 1);

            [computeEncoder dispatchThreads:gridSize
                      threadsPerThreadgroup:threadsPerThreadgroup];
            [computeEncoder endEncoding];

            rbcd->commit();
            rbcd->waitForCompletion();
            memcpy(states.data(), stateBuffer.contents, statesSize);
        }
        return true;
    }

private:
    id<MTLComputePipelineState> fn = nil;
};


MetalTwinkleEffect::MetalTwinkleEffect(int i) : TwinkleEffect(i) {
    data = new MetalTwinkleEffectData();
}
MetalTwinkleEffect::~MetalTwinkleEffect() {
    if (data) { delete data; }
}

void MetalTwinkleEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    if (buffer.captureSnapshot != nullptr) {
        // Frame-parallel serial capture pass is advance-only (the CPU path runs
        // the kernel with npix=0): no pixels are drawn, so never spend a GPU
        // command buffer + wait on it.
        TwinkleEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || buffer.IsDmxBuffer()
        || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        // Small buffers, DMX (SetPixel channel translation) and the no-GPU case
        // take the CPU path (ISPC / by-node scalar).
        TwinkleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Setup (renewal/compaction) runs exactly once; a by-node buffer discovered
    // here goes to the shared serial scalar tail rather than back through
    // TwinkleEffect::Render (which would re-run the mutating setup).
    TwinkleFrame f = prepareTwinkleFrame(SettingsMap, buffer);
    if (f.isByNode) {
        renderTwinkleByNode(buffer, f);
        return;
    }

    xlColorVector lut;
    buildTwinkleLut(buffer, f, lut);

    MetalTwinkleData td;
    td.width = (uint32_t)f.width;
    td.npix = (uint32_t)f.npix;
    td.curNumStrobe = (uint32_t)f.curNumStrobe;
    td.max_modulo = f.max_modulo;
    td.max_modulo2 = f.max_modulo2;
    td.colorcnt = f.colorcnt;
    td.lutStride = f.max_modulo + 1;
    td.lutSize = (int32_t)lut.size();
    td.new_algorithm = f.new_algorithm ? 1 : 0;
    td.reRandomize = f.reRandomize ? 1 : 0;
    td.frameSeed = f.frameSeed;

    if (data->Render(td, *f.states, lut, buffer)) {
        applyTwinkleFinishCount(f);
        return;
    }
    // Metal dispatch failed after setup — evolve the same prepared state on the
    // ISPC path so the frame still renders and stays byte-identical.
    dispatchTwinkleISPC(buffer, f, lut);
    applyTwinkleFinishCount(f);
}
