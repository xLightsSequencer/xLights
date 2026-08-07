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

#include "../../render/Effect.h"
#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

static bool renderPlasmaGPU(VulkanPlasmaData& pdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Plasma");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = pdata.width * pdata.height;
    return rbcd->encodeEffectDispatch(cb, u.plasmaEffectFunction, "PlasmaEffect",
                                      &pdata, sizeof(pdata), { px.buffer }, pixelCount, 0);
}

VulkanPlasmaEffect::VulkanPlasmaEffect(int i) : PlasmaEffect(i) {
}
VulkanPlasmaEffect::~VulkanPlasmaEffect() {
}

void VulkanPlasmaEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);

    const int ColorScheme = GetPlasmaColorScheme(SettingsMap["CHOICE_Plasma_Color"]);
    // ColorScheme is always 0-4 (Metal's canRenderStyle(ColorScheme) check is
    // therefore always true and has no Vulkan-side equivalent); the size
    // check below guards our own fixed-size colors[8] array (the palette UI
    // caps at 8 colors, so this is not expected to trip in practice, but it
    // mirrors the same defensive check every other multi-color Vulkan effect
    // wrapper makes against its own MAX_VULKAN_*_COLORS array).
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold
        || buffer.palette.Size() > MAX_VULKAN_PLASMA_COLORS) {
        PlasmaEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    float Style = SettingsMap.GetInt("SLIDER_Plasma_Style", sStyleDefault);
    float Line_Density = SettingsMap.GetInt("SLIDER_Plasma_Line_Density", sLineDensityDefault);
    float PlasmaSpeed = GetValueCurveInt("Plasma_Speed", sSpeedDefault, SettingsMap, oset, sSpeedMin, sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    const float state = (buffer.curPeriod - buffer.curEffStartPer); // frames 0 to N
    const float Speed_plasma = (101.0f - PlasmaSpeed) * 3.0f; // we want a large number to divide by
    const float time = (state + 1.0f) / Speed_plasma;
    const float sin_time_5 = std::sin(time / 5.0f);
    const float cos_time_3 = std::cos(time / 3.0f);
    const float sin_time_2 = std::sin(time / 2.0f);

    VulkanPlasmaData pdata = {};
    pdata.width = buffer.BufferWi;
    pdata.height = buffer.BufferHt;
    pdata.style = Style;
    pdata.lineDensity = Line_Density;
    pdata.time = time;
    pdata.sinTime5 = sin_time_5;
    pdata.cosTime3 = cos_time_3;
    pdata.sinTime2 = sin_time_2;
    pdata.colorScheme = ColorScheme;

    pdata.numColors = buffer.palette.Size();
    for (int x = 0; x < pdata.numColors; x++) {
        pdata.colors[x] = toUchar4(buffer.palette.GetColor(x));
    }

    if (renderPlasmaGPU(pdata, buffer)) {
        return;
    }
    PlasmaEffect::Render(effect, SettingsMap, buffer);
}

#endif
