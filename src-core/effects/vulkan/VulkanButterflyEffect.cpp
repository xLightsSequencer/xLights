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

static bool renderButterflyGPU(VulkanButterflyData& bdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Butterfly");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = bdata.width * bdata.height;
    return rbcd->encodeEffectDispatch(cb, u.butterflyEffectFunction, "ButterflyEffect",
                                      &bdata, sizeof(bdata), { px.buffer }, pixelCount, 0);
}

VulkanButterflyEffect::VulkanButterflyEffect(int i) : ButterflyEffect(i) {
}
VulkanButterflyEffect::~VulkanButterflyEffect() {
}

void VulkanButterflyEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    int Style = SettingsMap.GetInt("SLIDER_Butterfly_Style", ButterflyEffect::sStyleDefault);

    // if smaller buffer, overhead of prep for GPU will be higher than benefit
    if (rbcd == nullptr || Style < 1 || Style > 10
        || ((buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold)) {
        ButterflyEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Defensive bound not present in the Metal wrapper (which indexes a
    // fixed uchar4 colors[8] with no size check) — avoids overrunning our
    // own fixed-size colors[MAX_VULKAN_BUTTERFLY_COLORS] array.
    size_t colorCnt = buffer.palette.Size();
    if (colorCnt > MAX_VULKAN_BUTTERFLY_COLORS) {
        ButterflyEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    const int Chunks = GetValueCurveInt("Butterfly_Chunks", ButterflyEffect::sChunksDefault, SettingsMap, oset, ButterflyEffect::sChunksMin, ButterflyEffect::sChunksMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int Skip = GetValueCurveInt("Butterfly_Skip", ButterflyEffect::sSkipDefault, SettingsMap, oset, ButterflyEffect::sSkipMin, ButterflyEffect::sSkipMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int butterFlySpeed = GetValueCurveInt("Butterfly_Speed", ButterflyEffect::sSpeedDefault, SettingsMap, oset, ButterflyEffect::sSpeedMin, ButterflyEffect::sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    std::string colors = SettingsMap.Get("CHOICE_Butterfly_Colors", ButterflyEffect::sColorsDefault);
    int ColorScheme = (colors == "Palette") ? 1 : 0;

    int ButterflyDirection = SettingsMap.Get("CHOICE_Butterfly_Direction", ButterflyEffect::sDirectionDefault) == "Reverse" ? 1 : 0;

    const int curState = (buffer.curPeriod - buffer.curEffStartPer) * butterFlySpeed * buffer.frameTimeInMs / 50;
    const float offset = (ButterflyDirection == 1 ? -1.0f : 1.0f) * float(curState) / 200.0f;

    VulkanButterflyData bdata = {};
    bdata.width = buffer.BufferWi;
    bdata.height = buffer.BufferHt;
    bdata.curState = curState;
    bdata.numColors = (int)colorCnt;
    bdata.offset = offset;
    bdata.chunks = Chunks;
    bdata.skip = Skip;
    bdata.colorScheme = ColorScheme;
    bdata.style = Style;
    if (Style > 5) {
        // slightly different setup for "plasmas"
        int state = (buffer.curPeriod - buffer.curEffStartPer); // frames 0 to N
        double Speed_plasma = (Style == 10) ? (101 - butterFlySpeed) * 3 : (101 - butterFlySpeed) * 5;
        double time = (state + 1.0) / Speed_plasma;
        bdata.plasmaTime = (float)time;
    } else {
        bdata.plasmaTime = 0.0f;
    }

    for (int x = 0; x < bdata.numColors; x++) {
        bdata.colors[x] = toUchar4(buffer.palette.GetColor(x));
    }

    if (renderButterflyGPU(bdata, buffer)) {
        return;
    }
    ButterflyEffect::Render(effect, SettingsMap, buffer);
}

#endif
