#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"
#include "../ColorWashEffect.h"

class MetalColorWashEffectData {
public:
    MetalColorWashEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("ColorWashEffect");
    }
    ~MetalColorWashEffectData() {
        if (fn) { [fn release]; }
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalColorWashData &cdata, RenderBuffer &buffer) {
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
            [computeEncoder setLabel:@"ColorWashEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(cdata);
            [computeEncoder setBytes:&cdata length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger pixelCount = cdata.width * cdata.height;
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


MetalColorWashEffect::MetalColorWashEffect(int i) : ColorWashEffect(i) {
    data = new MetalColorWashEffectData();
}
MetalColorWashEffect::~MetalColorWashEffect() {
    if (data) { delete data; }
}

void MetalColorWashEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < 2048) {
        ColorWashEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    float cycles = GetValueCurveDouble("ColorWash_Cycles", 1.0, SettingsMap, oset, COLOURWASH_CYCLES_MIN, COLOURWASH_CYCLES_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool horizFade       = SettingsMap.GetBool("CHECKBOX_ColorWash_HFade");
    bool vertFade        = SettingsMap.GetBool("CHECKBOX_ColorWash_VFade");
    bool reverseFades    = SettingsMap.GetBool("CHECKBOX_ColorWash_ReverseFades");
    bool shimmer         = SettingsMap.GetBool("CHECKBOX_ColorWash_Shimmer");
    bool circularPalette = SettingsMap.GetBool("CHECKBOX_ColorWash_CircularPalette");

    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    xlColor color;
    buffer.GetMultiColorBlend(position, circularPalette, color);

    int tot = buffer.curPeriod - buffer.curEffStartPer;
    bool shimmerBlack = shimmer && (tot % 2) != 0;

    MetalColorWashData cdata;
    cdata.width        = buffer.BufferWi;
    cdata.height       = buffer.BufferHt;
    cdata.color        = color.asChar4();
    HSVValue hsv       = color.asHSV();
    cdata.colorHSV     = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    cdata.horizFade    = horizFade    ? 1 : 0;
    cdata.vertFade     = vertFade     ? 1 : 0;
    cdata.reverseFades = reverseFades ? 1 : 0;
    cdata.shimmerBlack = shimmerBlack ? 1 : 0;
    cdata.allowAlpha   = buffer.allowAlpha ? 1 : 0;

    if (data->Render(cdata, buffer)) {
        return;
    }
    ColorWashEffect::Render(effect, SettingsMap, buffer);
}
