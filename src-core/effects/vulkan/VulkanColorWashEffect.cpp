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

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

static bool renderColorWashGPU(VulkanColorWashData& cdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-ColorWash");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = cdata.width * cdata.height;
    return rbcd->encodeEffectDispatch(cb, u.colorWashEffectFunction, "ColorWashEffect",
                                      &cdata, sizeof(cdata), { px.buffer }, pixelCount, 0);
}

VulkanColorWashEffect::VulkanColorWashEffect(int i) : ColorWashEffect(i) {
}
VulkanColorWashEffect::~VulkanColorWashEffect() {
}

void VulkanColorWashEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
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

    VulkanColorWashData cdata = {};
    cdata.width        = buffer.BufferWi;
    cdata.height       = buffer.BufferHt;
    cdata.color        = toUchar4(color);
    HSVValue hsv       = color.asHSV();
    cdata.colorHSV     = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    cdata.horizFade    = horizFade    ? 1 : 0;
    cdata.vertFade     = vertFade     ? 1 : 0;
    cdata.reverseFades = reverseFades ? 1 : 0;
    cdata.shimmerBlack = shimmerBlack ? 1 : 0;
    cdata.allowAlpha   = buffer.allowAlpha ? 1 : 0;

    if (renderColorWashGPU(cdata, buffer)) {
        return;
    }
    ColorWashEffect::Render(effect, SettingsMap, buffer);
}

#endif
