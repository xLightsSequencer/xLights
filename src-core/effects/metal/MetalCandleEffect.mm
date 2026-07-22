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
#include "../CandleEffect.h"

class MetalCandleEffectData {
public:
    MetalCandleEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("CandleEffect");
    }

    bool canRender() { return fn != nil; }

    // The kernel updates the per-node CandleState array in place, so the state
    // buffer is committed and waited on immediately and copied back into the
    // CPU-side cache: the cache stays the single source of truth and the CPU
    // and GPU paths remain interchangeable mid-sequence.
    bool Render(MetalCandleData &cd, std::vector<CandleState> &states, RenderBuffer &buffer) {
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

            NSUInteger statesSize = states.size() * sizeof(CandleState);
            id<MTLBuffer> stateBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithBytes:states.data()
                                                                                            length:statesSize
                                                                                           options:MTLResourceStorageModeShared];
            if (stateBuffer == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"CandleEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(cd);
            [computeEncoder setBytes:&cd length:dataSize atIndex:0];
            [computeEncoder setBuffer:stateBuffer offset:0 atIndex:1];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:2];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger pixelCount = cd.width * cd.height;
            NSInteger threads = std::min(pixelCount, maxThreads);
            MTLSize gridSize = MTLSizeMake(pixelCount, 1, 1);
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


MetalCandleEffect::MetalCandleEffect(int i) : CandleEffect(i) {
    data = new MetalCandleEffectData();
}
MetalCandleEffect::~MetalCandleEffect() {
    if (data) { delete data; }
}

void MetalCandleEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    if (buffer.pendingSnapshot == nullptr) {
        // The GPU draw is only ever reached through the engine's AdvanceState +
        // Render(pendingSnapshot) sequence, which the engine drives in BOTH serial
        // and frame-parallel rendering - so the GPU now runs inside parallel
        // windows too (it used to early-out to the CPU there).  Any direct call
        // without a snapshot (defensive preview path) takes the CPU base Render,
        // which advances then draws.  The capture pre-pass calls AdvanceState, not
        // Render, so it never reaches here.
        CandleEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        CandleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Metal implements the perNode flame simulation only. The single-flame mode,
    // DMX buffers, and out-of-range params fall back to the CPU path (which itself
    // does ISPC/scalar).
    bool perNode = SettingsMap.GetBool("CHECKBOX_PerNode", sPerNodeDefault);
    if (!perNode || buffer.IsDmxBuffer()) {
        CandleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    int flameAgility = GetValueCurveInt("Candle_FlameAgility", sFlameAgilityDefault, SettingsMap, oset, sFlameAgilityMin, sFlameAgilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windCalmness = GetValueCurveInt("Candle_WindCalmness", sWindCalmnessDefault, SettingsMap, oset, sWindCalmnessMin, sWindCalmnessMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windVariability = GetValueCurveInt("Candle_WindVariability", sWindVariabilityDefault, SettingsMap, oset, sWindVariabilityMin, sWindVariabilityMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int windBaseline = GetValueCurveInt("Candle_WindBaseline", sWindBaselineDefault, SettingsMap, oset, sWindBaselineMin, sWindBaselineMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool usePalette = SettingsMap.GetBool("CHECKBOX_UsePalette", sUsePaletteDefault);

    size_t npix = (size_t)buffer.BufferWi * buffer.BufferHt;
    if (npix > buffer.GetPixelCount()
        || windVariability < 0 || windVariability > 255
        || windBaseline < 0 || windBaseline > 255
        || windCalmness < 0 || windCalmness > 31
        || flameAgility < 0 || flameAgility > 254) {
        CandleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    const auto& pal = effect->GetPalette();
    xlColor c1, c2;
    if (usePalette) {
        if (pal.empty()) {
            c1 = xlWHITE;
            c2 = xlBLACK;
        } else {
            c1 = pal[0];
            c2 = (pal.size() > 1 ? pal[1] : xlBLACK);
        }
    }

    // Draw pass: restore the captured pre-frame (post-init) state into the cache and
    // feed THAT to the GPU, which advances the simulation in place and draws.  We
    // must NOT call getPerNodeStates here - on a parallel clone that would reseed
    // per-node state via the stream RNG and discard the captured simulation.
    int maxW = 0;
    std::vector<CandleState>& states = *RestoreSnapshotStates(buffer, *buffer.pendingSnapshot, maxW);
    if (states.empty()) {
        CandleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    MetalCandleData cd;
    cd.width = buffer.BufferWi;
    cd.height = buffer.BufferHt;
    cd.maxWid = maxW;
    cd.numStates = (uint32_t)states.size();
    cd.frameSeed = buffer.hashRandomFrameSeed();
    cd.windVariability = windVariability;
    cd.flameAgility = flameAgility;
    cd.windCalmness = windCalmness;
    cd.windBaseline = windBaseline;
    cd.usePalette = usePalette ? 1 : 0;
    cd.c1 = { c1.red, c1.green, c1.blue, 255 };
    cd.c2 = { c2.red, c2.green, c2.blue, 255 };

    if (data->Render(cd, states, buffer)) {
        return;
    }
    CandleEffect::Render(effect, SettingsMap, buffer);
}
