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

class MetalTreeEffectData {
public:
    MetalTreeEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("TreeEffect");
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalTreeData &tdata, RenderBuffer &buffer) {
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
            [computeEncoder setLabel:@"TreeEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(tdata);
            [computeEncoder setBytes:&tdata length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger pixelCount = tdata.width * tdata.height;
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


MetalTreeEffect::MetalTreeEffect(int i) : TreeEffect(i) {
    data = new MetalTreeEffectData();
}
MetalTreeEffect::~MetalTreeEffect() {
    if (data) { delete data; }
}

void MetalTreeEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        TreeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int Branches = SettingsMap.GetInt("SLIDER_Tree_Branches", sBranchesDefault);
    int tspeed = SettingsMap.GetInt("SLIDER_Tree_Speed", sSpeedDefault);
    bool showlights = SettingsMap.GetBool("CHECKBOX_Tree_ShowLights", sShowLightsDefault);

    int effectState = (buffer.curPeriod - buffer.curEffStartPer) * tspeed * buffer.frameTimeInMs / 50;

    if (Branches < 1) Branches = 1;
    int pixels_per_branch = (int)(0.5 + buffer.BufferHt / Branches);
    if (pixels_per_branch < 1) pixels_per_branch = 1;

    // DMX buffers need the SetPixel channel translation, oversized branches don't fit the
    // constant table, and a variable sub-buffer can be smaller than BufferWi*BufferHt
    // (the kernel writes unguarded) — those all take the CPU path (which itself does ISPC/scalar).
    if (buffer.IsDmxBuffer() || pixels_per_branch > MAX_METAL_TREE_PPB
        || (size_t)(buffer.BufferWi * buffer.BufferHt) > buffer.GetPixelCount()) {
        TreeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int maxFrame = (Branches + 1) * buffer.BufferWi;
    int frame;
    if (effectState > 0 && maxFrame > 0) frame = (effectState / 4) % maxFrame;
    else frame = 1;

    MetalTreeData tdata;
    tdata.width = buffer.BufferWi;
    tdata.height = buffer.BufferHt;
    tdata.ppb = pixels_per_branch;
    tdata.frame = frame;
    tdata.branch_row = (effectState / buffer.BufferWi) % Branches;
    tdata.f_mod = (effectState / 4) % buffer.BufferWi;
    tdata.showlights = showlights ? 1 : 0;

    xlColor color;
    for (int mod = 1; mod <= pixels_per_branch; mod++) {
        float V = 1 - (1.0 * mod / pixels_per_branch) * 0.70;
        buffer.palette.GetColor(0, color);
        if (buffer.allowAlpha) {
            color.alpha = 255.0 * V;
        } else {
            HSVValue hsv = color.asHSV();
            hsv.value = V;
            color = hsv;
        }
        tdata.bgColors[mod - 1] = color.asChar4();
    }
    for (int r = 0; r < 5; r++) {
        float H = r / 4.0;
        HSVValue hsv;
        hsv.hue = H;
        hsv.saturation = 1.0;
        hsv.value = 1.0;
        color = hsv;
        tdata.lightColors[r] = color.asChar4();
    }

    if (data->Render(tdata, buffer)) {
        return;
    }
    TreeEffect::Render(effect, SettingsMap, buffer);
}
