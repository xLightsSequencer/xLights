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

class MetalGarlandsEffectData {
public:
    MetalGarlandsEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("GarlandsEffect");
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalGarlandsData &gdata,
                const std::vector<simd::uchar4> &colors,
                const std::vector<int32_t> &yb,
                RenderBuffer &buffer) {
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

            // Per-ring tables sized to the ring count (no fixed cap); the command
            // buffer retains these until it completes, so the local handles are safe.
            id<MTLBuffer> colorBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithBytes:colors.data()
                                                                                            length:colors.size() * sizeof(simd::uchar4)
                                                                                           options:MTLResourceStorageModeShared];
            id<MTLBuffer> ybBuffer = [MetalComputeUtilities::INSTANCE.device newBufferWithBytes:yb.data()
                                                                                         length:yb.size() * sizeof(int32_t)
                                                                                        options:MTLResourceStorageModeShared];
            if (colorBuffer == nil || ybBuffer == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }

            id<MTLComputeCommandEncoder> computeEncoder = [commandBuffer computeCommandEncoder];
            if (computeEncoder == nil) {
                rbcd->abortCommandBuffer();
                return false;
            }
            [computeEncoder setLabel:@"GarlandsEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(gdata);
            [computeEncoder setBytes:&gdata length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];
            [computeEncoder setBuffer:colorBuffer offset:0 atIndex:2];
            [computeEncoder setBuffer:ybBuffer offset:0 atIndex:3];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            // Bound by the actual pixel allocation, not the logical dimensions — a
            // variable sub-buffer can leave GetPixelCount() < width * height and the
            // kernel writes result[index] with no bounds check.
            NSInteger pixelCount = std::min((NSInteger)(gdata.width * gdata.height), (NSInteger)buffer.GetPixelCount());
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


MetalGarlandsEffect::MetalGarlandsEffect(int i) : GarlandsEffect(i) {
    data = new MetalGarlandsEffectData();
}
MetalGarlandsEffect::~MetalGarlandsEffect() {
    if (data) { delete data; }
}

void MetalGarlandsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || buffer.IsDmxBuffer()
        || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        // DMX needs the SetPixel channel translation; the CPU path (ISPC) handles it.
        GarlandsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    int GarlandType = SettingsMap.GetInt("SLIDER_Garlands_Type", sTypeDefault);
    int Spacing = GetValueCurveInt("Garlands_Spacing", sSpacingDefault, SettingsMap, oset, sSpacingMin, sSpacingMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float cycles = GetValueCurveDouble("Garlands_Cycles", sCyclesDefault, SettingsMap, oset, sCyclesMin, sCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);

    if (Spacing < 1) {
        Spacing = 1;
    }
    int dir = GarlandsEffect::GetDirection(SettingsMap.Get("CHOICE_Garlands_Direction", sDirectionDefault));
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    if (dir > 3) {
        dir -= 4;
        if (position > 0.5) {
            position = (1.0 - position) * 2.0;
        } else {
            position *= 2.0;
        }
    }
    int buffMax = buffer.BufferHt;
    int garlandWid = buffer.BufferWi;
    if (dir > 1) {
        buffMax = buffer.BufferWi;
        garlandWid = buffer.BufferHt;
    }
    if (buffMax < 1 || garlandWid < 1) {
        GarlandsEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    double PixelSpacing = Spacing * buffMax / 100.0;
    if (PixelSpacing < 2.0) PixelSpacing = 2.0;
    double total = buffMax * PixelSpacing - buffMax + 1;
    double positionOffset = total * position;

    std::vector<simd::uchar4> colors(buffMax);
    std::vector<int32_t> yb(buffMax);
    xlColor color;
    for (int ring = 0; ring < buffMax; ring++) {
        double ratio = double(buffMax - ring - 1) / double(buffMax);
        buffer.GetMultiColorBlend(ratio, false, color);
        colors[ring] = color.asChar4();
        yb[ring] = (int)(1.0 + ring * PixelSpacing - positionOffset);
    }

    MetalGarlandsData gdata;
    gdata.width = buffer.BufferWi;
    gdata.height = buffer.BufferHt;
    gdata.buffMax = buffMax;
    gdata.garlandType = GarlandType;
    gdata.dir = dir;
    gdata.invPS = (float)(1.0 / PixelSpacing);
    gdata.posOffOverPS = (float)(positionOffset / PixelSpacing);

    if (data->Render(gdata, colors, yb, buffer)) {
        return;
    }
    GarlandsEffect::Render(effect, SettingsMap, buffer);
}
