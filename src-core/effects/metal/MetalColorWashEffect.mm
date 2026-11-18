#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../ColorWashEffect.h"

class MetalColorWashEffectData {
public:
    MetalColorWashEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("ColorWashEffect");
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

            bool encoded = rbcd->encodeEffect(commandBuffer, fn, @"ColorWashEffect", [&](id<MTLComputeCommandEncoder> computeEncoder) {
                NSInteger dataSize = sizeof(cdata);
                [computeEncoder setBytes:&cdata length:dataSize atIndex:0];
                [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

                NSInteger pixelCount = cdata.width * cdata.height;
                MetalRenderBufferComputeData::dispatch1D(computeEncoder, fn, pixelCount);
            });
            if (!encoded) {
                return false;
            }
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
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < MetalComputeUtilities::INSTANCE.metalBufferSizeThreshold) {
        ColorWashEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    float cycles = GetValueCurveDouble("ColorWash_Cycles", sCyclesDefault, SettingsMap, oset, sCyclesMin, sCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);
    bool horizFade       = SettingsMap.GetBool("CHECKBOX_ColorWash_HFade", sHFadeDefault);
    bool vertFade        = SettingsMap.GetBool("CHECKBOX_ColorWash_VFade", sVFadeDefault);
    bool reverseFades    = SettingsMap.GetBool("CHECKBOX_ColorWash_ReverseFades", sReverseFadesDefault);
    bool shimmer         = SettingsMap.GetBool("CHECKBOX_ColorWash_Shimmer", sShimmerDefault);
    bool circularPalette = SettingsMap.GetBool("CHECKBOX_ColorWash_CircularPalette", sCircularPaletteDefault);

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
