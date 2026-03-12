#include "MetalComputeUtilities.hpp"
#include "MetalEffects.hpp"
#include "MetalEffectDataTypes.h"

#include "../../RenderBuffer.h"
#include "../SpiralsEffect.h"

class MetalSpiralsEffectData {
public:
    MetalSpiralsEffectData() {
        fn = MetalComputeUtilities::INSTANCE.FindComputeFunction("SpiralsEffect");
    }
    ~MetalSpiralsEffectData() {
        if (fn) { [fn release]; }
    }

    bool canRender() { return fn != nil; }

    bool Render(MetalSpiralsData &sdata, RenderBuffer &buffer) {
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
            [computeEncoder setLabel:@"SpiralsEffect"];
            [computeEncoder setComputePipelineState:fn];

            NSInteger dataSize = sizeof(sdata);
            [computeEncoder setBytes:&sdata length:dataSize atIndex:0];
            [computeEncoder setBuffer:bufferResult offset:0 atIndex:1];

            NSInteger maxThreads = fn.maxTotalThreadsPerThreadgroup;
            NSInteger pixelCount = sdata.width * sdata.height;
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


MetalSpiralsEffect::MetalSpiralsEffect(int i) : SpiralsEffect(i) {
    data = new MetalSpiralsEffectData();
}
MetalSpiralsEffect::~MetalSpiralsEffect() {
    if (data) { delete data; }
}

void MetalSpiralsEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    MetalRenderBufferComputeData *rbcd = MetalRenderBufferComputeData::getMetalRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || !data->canRender() || (buffer.BufferWi * buffer.BufferHt) < 2048) {
        SpiralsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float offset = buffer.GetEffectTimeIntervalPosition();
    int PaletteRepeat = GetValueCurveInt("Spirals_Count", 1, SettingsMap, offset, SPIRALS_COUNT_MIN, SPIRALS_COUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float Movement = GetValueCurveDouble("Spirals_Movement", 1.0, SettingsMap, offset, SPIRALS_MOVEMENT_MIN, SPIRALS_MOVEMENT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), SPIRALS_MOVEMENT_DIVISOR);
    float Rotation = GetValueCurveDouble("Spirals_Rotation", 0.0, SettingsMap, offset, SPIRALS_ROTATION_MIN, SPIRALS_ROTATION_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), SPIRALS_ROTATION_DIVISOR);
    if (SettingsMap.Contains("VALUECURVE_Spirals_Rotation") && wxString(SettingsMap["VALUECURVE_Spirals_Rotation"]).Contains("Active=TRUE")) {
        Rotation *= 10;
    }
    int Thickness = GetValueCurveInt("Spirals_Thickness", 0, SettingsMap, offset, SPIRALS_THICKNESS_MIN, SPIRALS_THICKNESS_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool Blend   = SettingsMap.GetBool("CHECKBOX_Spirals_Blend");
    bool Show3D  = SettingsMap.GetBool("CHECKBOX_Spirals_3D");
    bool grow    = SettingsMap.GetBool("CHECKBOX_Spirals_Grow");
    bool shrink  = SettingsMap.GetBool("CHECKBOX_Spirals_Shrink");

    // Fall back to CPU for spatial colors or blend mode
    size_t colorcnt = buffer.GetColorCount();
    for (size_t i = 0; i < colorcnt; i++) {
        if (buffer.palette.IsSpatial(i)) {
            SpiralsEffect::Render(effect, SettingsMap, buffer);
            return;
        }
    }
    if (Blend) {
        SpiralsEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    if ((int)colorcnt > MAX_METAL_SPIRALS_COLORS) {
        SpiralsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    if (PaletteRepeat == 0) PaletteRepeat = 1;

    int SpiralCount = (int)colorcnt * PaletteRepeat;
    double deltaStrands = (double)buffer.BufferWi / (double)SpiralCount;
    double SpiralThickness = (deltaStrands * Thickness / 100.0) + 1.0;
    double spiralGap = deltaStrands - SpiralThickness;

    int Direction = Movement > 0.001f ? 1 : (Movement < -0.001f ? -1 : 0);
    double position = buffer.GetEffectTimeIntervalPosition(std::abs(Movement));
    long ThicknessState = 0;
    if (grow && shrink) {
        ThicknessState = position <= 0.5 ? spiralGap * (position * 2) : spiralGap * ((1.0 - position) * 2);
    } else if (grow) {
        ThicknessState = spiralGap * position;
    } else if (shrink) {
        ThicknessState = spiralGap * (1.0 - position);
    }
    long SpiralState = (long)(position * buffer.BufferWi * 10 * Direction);

    SpiralThickness += ThicknessState;

    MetalSpiralsData sdata;
    sdata.width          = buffer.BufferWi;
    sdata.height         = buffer.BufferHt;
    sdata.spiralCount    = SpiralCount;
    sdata.colorCount     = (int)colorcnt;
    sdata.paletteRepeat  = PaletteRepeat;
    sdata.deltaStrands   = (float)deltaStrands;
    sdata.spiralThickness= (float)SpiralThickness;
    sdata.spiralState    = (float)SpiralState;
    sdata.rotation       = (float)Rotation;
    sdata.show3D         = Show3D ? 1 : 0;
    sdata.allowAlpha     = buffer.allowAlpha ? 1 : 0;
    sdata.rotation_sign  = (Rotation >= 0.0f) ? 1.0f : -1.0f;

    for (int i = 0; i < (int)colorcnt; i++) {
        xlColor c;
        buffer.palette.GetColor(i, c);
        sdata.colorsAsRGBA[i] = c.asChar4();
        HSVValue hsv = c;
        sdata.colorsAsHSV[i] = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    }

    if (data->Render(sdata, buffer)) {
        return;
    }
    SpiralsEffect::Render(effect, SettingsMap, buffer);
}
