/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#ifdef HAVE_VULKAN

#include <cmath>

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

static bool renderSpiralsGPU(VulkanSpiralsData& sdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Spirals");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = sdata.width * sdata.height;
    return rbcd->encodeEffectDispatch(cb, u.spiralsEffectFunction, "SpiralsEffect",
                                      &sdata, sizeof(sdata), { px.buffer }, pixelCount, 0);
}

VulkanSpiralsEffect::VulkanSpiralsEffect(int i) : SpiralsEffect(i) {
}
VulkanSpiralsEffect::~VulkanSpiralsEffect() {
}

void VulkanSpiralsEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        SpiralsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float offset = buffer.GetEffectTimeIntervalPosition();
    int PaletteRepeat = GetValueCurveInt("Spirals_Count", sCountDefault, SettingsMap, offset, sCountMin, sCountMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float Movement = GetValueCurveDouble("Spirals_Movement", sMovementDefault, SettingsMap, offset, sMovementMin, sMovementMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sMovementDivisor);
    float Rotation = GetValueCurveDouble("Spirals_Rotation", sRotationDefault, SettingsMap, offset, sRotationMin, sRotationMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sRotationDivisor);
    if (SettingsMap.Contains("VALUECURVE_Spirals_Rotation") && std::string(SettingsMap["VALUECURVE_Spirals_Rotation"]).find("Active=TRUE") != std::string::npos) {
        Rotation *= 10;
    }
    int Thickness = GetValueCurveInt("Spirals_Thickness", sThicknessDefault, SettingsMap, offset, sThicknessMin, sThicknessMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    bool Blend  = SettingsMap.GetBool("CHECKBOX_Spirals_Blend", sBlendDefault);
    bool Show3D = SettingsMap.GetBool("CHECKBOX_Spirals_3D", s3DDefault);
    bool grow   = SettingsMap.GetBool("CHECKBOX_Spirals_Grow", sGrowDefault);
    bool shrink = SettingsMap.GetBool("CHECKBOX_Spirals_Shrink", sShrinkDefault);

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
    if ((int)colorcnt > MAX_VULKAN_SPIRALS_COLORS) {
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

    VulkanSpiralsData sdata = {};
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
        sdata.colorsAsRGBA[i] = toUchar4(c);
        HSVValue hsv = c.asHSV();
        sdata.colorsAsHSV[i] = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    }

    if (renderSpiralsGPU(sdata, buffer)) {
        return;
    }
    SpiralsEffect::Render(effect, SettingsMap, buffer);
}

#endif
