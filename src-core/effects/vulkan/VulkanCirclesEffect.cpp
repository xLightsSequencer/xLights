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

#include <algorithm>

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../CirclesEffect.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

static bool renderCirclesGPU(VulkanCirclesData& cdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Circles");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = cdata.width * cdata.height;
    return rbcd->encodeEffectDispatch(cb, u.circlesEffectFunction, "CirclesEffect",
                                      &cdata, sizeof(cdata), { px.buffer }, pixelCount, 0);
}

VulkanCirclesEffect::VulkanCirclesEffect(int i) : CirclesEffect(i) {
}
VulkanCirclesEffect::~VulkanCirclesEffect() {
}

void VulkanCirclesEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        CirclesEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    int number      = GetValueCurveInt("Circles_Count", sCountDefault, SettingsMap, oset, sCountMin, sCountMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int circleSpeed = GetValueCurveInt("Circles_Speed", sSpeedDefault, SettingsMap, oset, sSpeedMin, sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int radius      = GetValueCurveInt("Circles_Size",  sSizeDefault, SettingsMap, oset, sSizeMin,  sSizeMax,  buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    bool plasma   = SettingsMap.GetBool("CHECKBOX_Circles_Plasma",    sPlasmaDefault);
    bool radial   = SettingsMap.GetBool("CHECKBOX_Circles_Radial",    sRadialDefault);
    bool radial3D = SettingsMap.GetBool("CHECKBOX_Circles_Radial_3D", sRadial3DDefault);
    bool fade     = SettingsMap.GetBool("CHECKBOX_Circles_Linear_Fade", sLinearFadeDefault);
    bool bubbles  = SettingsMap.GetBool("CHECKBOX_Circles_Bubbles",   sBubblesDefault);
    bool bounce   = SettingsMap.GetBool("CHECKBOX_Circles_Bounce",    sBounceDefault);

    // Bubbles uses DrawCircle(filled=false) — outline only, not easily GPU-invertible
    if (bubbles) {
        CirclesEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    size_t colorCnt = buffer.GetColorCount();
    if (colorCnt == 0) colorCnt = 1;
    if ((int)colorCnt > MAX_VULKAN_CIRCLES_COLORS) {
        CirclesEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    for (size_t i = 0; i < colorCnt; i++) {
        if (buffer.palette.IsSpatial(i)) {
            CirclesEffect::Render(effect, SettingsMap, buffer);
            return;
        }
    }
    if (number > MAX_VULKAN_CIRCLES_BALLS) number = MAX_VULKAN_CIRCLES_BALLS;

    VulkanCirclesData cdata = {};
    cdata.width      = buffer.BufferWi;
    cdata.height     = buffer.BufferHt;
    cdata.colorCount = (int)colorCnt;
    cdata.allowAlpha = buffer.allowAlpha ? 1 : 0;
    cdata.numBalls   = number;
    cdata.wrap       = !bounce ? 1 : 0;

    // Populate palette colors
    for (int i = 0; i < (int)colorCnt; i++) {
        xlColor c;
        buffer.palette.GetColor(i, c);
        cdata.colorsAsRGBA[i] = toUchar4(c);
        HSVValue hsv = c.asHSV();
        cdata.colorsAsHSV[i] = { (float)hsv.hue, (float)hsv.saturation, (float)hsv.value };
    }

    if (radial || radial3D) {
        // Pure math — no cache needed
        int start_x = buffer.BufferWi / 2;
        int start_y = buffer.BufferHt / 2;
        start_x += (int)(GetValueCurveInt("Circles_XC", sXCDefault, SettingsMap, oset, sPosMin, sPosMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / float(sPosMax) * start_x);
        start_y += (int)(GetValueCurveInt("Circles_YC", sYCDefault, SettingsMap, oset, sPosMin, sPosMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / float(sPosMax) * start_y);

        int effectState = (buffer.curPeriod - buffer.curEffStartPer) * circleSpeed * buffer.frameTimeInMs / 50;
        int barht   = buffer.BufferHt / (radius + 1);
        if (barht < 1) barht = 1;
        int maxRadius = effectState > buffer.BufferHt ? buffer.BufferHt : effectState / 2 + radius;
        int blockHt = (int)colorCnt * barht;
        int f_offset = effectState / 4 % (blockHt + 1);

        cdata.mode        = radial3D ? CIRCLES_MODE_RADIAL_3D : CIRCLES_MODE_RADIAL;
        cdata.cx          = start_x;
        cdata.cy          = start_y;
        cdata.barSize     = barht;
        cdata.blockSize   = blockHt > 0 ? blockHt : 1;
        cdata.f_offset    = f_offset;
        cdata.maxRadius   = maxRadius;
        cdata.number      = number;
        cdata.effectState = effectState;
    } else {
        // Update ball positions via the base class — no pixel drawing
        CirclesRenderCache* cache = UpdateCacheState(effect, SettingsMap, buffer);

        int actualBalls = std::min(cache->numBalls, MAX_VULKAN_CIRCLES_BALLS);
        cdata.numBalls = actualBalls;

        RgbBalls* src = plasma ? (RgbBalls*)cache->metaballs : cache->balls;
        for (int i = 0; i < actualBalls; i++) {
            cdata.balls[i].x        = src[i]._x;
            cdata.balls[i].y        = src[i]._y;
            cdata.balls[i].radius   = src[i]._radius;
            cdata.balls[i].colorIdx = src[i]._colorindex;
        }

        cdata.mode = plasma ? CIRCLES_MODE_METABALLS :
                     (fade  ? CIRCLES_MODE_FADING    : CIRCLES_MODE_REGULAR);
    }

    if (renderCirclesGPU(cdata, buffer)) {
        return;
    }
    CirclesEffect::Render(effect, SettingsMap, buffer);
}

#endif
