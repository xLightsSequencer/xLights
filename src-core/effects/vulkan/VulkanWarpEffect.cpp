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

static inline double warpInterpolate(double x, double loIn, double loOut, double hiIn, double hiOut) {
    return (loIn != hiIn)
        ? (loOut + (hiOut - loOut) * ((x - loIn) / (hiIn - loIn)))
        : ((loOut + hiOut) / 2);
}

// Mirrors ADDENDUM 2 of the porting conventions: gather effects snapshot the
// current pixels into pixelBufferCopy (the source, binding 1) before
// dispatching, so every warp style reads a frozen buffer instead of racing
// with `result` (binding 0). binding 2 is always the dissolve pattern -- only
// Wavy/Dissolve/Drop read it, but binding it unconditionally keeps this
// wrapper (and the unified WarpEffect.comp switch) simple.
static bool renderWarpGPU(VulkanWarpData& d, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VulkanBuffer& copy = rbcd->getPixelBufferCopy();
    if (!copy) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Warp");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    VulkanComputeUtilities::computeBarrier(cb);
    VkBufferCopy region = { 0, 0, (VkDeviceSize)(d.width * d.height * 4) };
    vkCmdCopyBuffer(cb, px.buffer, copy.buffer, 1, &region);
    VulkanComputeUtilities::computeBarrier(cb);
    return rbcd->encodeEffectDispatch(cb, u.warpEffectFunction, "WarpEffect", &d, sizeof(d),
                                      { px.buffer, copy.buffer, u.dissolveBuffer.buffer }, d.width * d.height, 0);
}

VulkanWarpEffect::VulkanWarpEffect(int i) : WarpEffect(i) {
}
VulkanWarpEffect::~VulkanWarpEffect() {
}

void VulkanWarpEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    std::string warpType = SettingsMap.Get("CHOICE_Warp_Type", sTypeDefault);
    WarpEffect::WarpType Style = mapWarpType(warpType);

    // Mirrors MetalWarpEffectData::canRenderStyle(): only styles 0..FLIP have
    // a GPU kernel; SPEED (temporal slow-motion) needs CPU-side cross-frame
    // frame capture and has no Vulkan kernel either.
    bool canRenderStyle = Style >= 0 && Style < WarpEffect::WarpType::SPEED;

    //currently just Styles 1-5 are GPU enabled, if smaller buffer, overhead of prep for GPU will be higher than benefit
    if (rbcd == nullptr || !canRenderStyle
        || ((buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold)) {
        WarpEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    VulkanWarpData wdata = {};
    wdata.width = buffer.BufferWi;
    wdata.height = buffer.BufferHt;

    float progress = buffer.GetEffectTimeIntervalPosition(1.f);
    std::string warpTreatment = SettingsMap.Get("CHOICE_Warp_Treatment_APPLYLAST", sTreatmentDefault);
    float xPercentage = GetValueCurveInt("Warp_X", sXDefault, SettingsMap, progress, sXMin, sXMax,
                                       buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float yPercentage = GetValueCurveInt("Warp_Y", sYDefault, SettingsMap, progress, sYMin, sYMax,
                                       buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float x = 0.01 * xPercentage;
    float y = 0.01 * yPercentage;

    wdata.xPos = std::round((float)buffer.BufferWi - 1) * x;
    wdata.yPos = std::round((float)buffer.BufferHt - 1) * y;
    wdata.progress = progress;
    wdata.speed = SettingsMap.GetFloat("TEXTCTRL_Warp_Speed", (float)sSpeedDefault);
    wdata.frequency = SettingsMap.GetFloat("TEXTCTRL_Warp_Frequency", (float)sFrequencyDefault);
    wdata.direction = warpTreatment == "out";
    wdata.warpType = (int32_t)Style;

    //adjust params based on type (if needed)
    switch (Style) {
        case WarpEffect::WarpType::WATER_DROPS:
        case WarpEffect::WarpType::SAMPLE_ON:
        case WarpEffect::WarpType::MIRROR:
        case WarpEffect::WarpType::COPY:
            break;
        case WarpEffect::WarpType::WAVY:
            wdata.speed = warpInterpolate(wdata.speed, 0.0, 0.5, 40.0, 5.0);
            break;
        case WarpEffect::WarpType::SINGLE_WATER_DROP:
            {
                float cycleCount = SettingsMap.GetFloat("TEXTCTRL_Warp_Cycle_Count", (float)sCycleCountDefault);
                float intervalLen = 1.f / cycleCount;
                float scaledProgress = progress / intervalLen;
                float intervalProgress, intervalIndex;
                intervalProgress = std::modf(scaledProgress, &intervalIndex);
                float interpolatedProgress = warpInterpolate(intervalProgress, 0.0, 0.20, 1.0, 0.45);
                wdata.progress = interpolatedProgress;
            }
            break;
        default:
            if (warpTreatment == "constant") {
                float cycleCount = SettingsMap.GetFloat("TEXTCTRL_Warp_Cycle_Count", (float)sCycleCountDefault);
                float intervalLen = 1.f / (2 * cycleCount);
                float scaledProgress = progress / intervalLen;
                float intervalProgress, intervalIndex;
                intervalProgress = std::modf(scaledProgress, &intervalIndex);
                if (int(intervalIndex) % 2) {
                    intervalProgress = 1.f - intervalProgress;
                }
                wdata.progress = intervalProgress;
                if (Style == WarpEffect::WarpType::CIRCULAR_SWIRL || Style == WarpEffect::WarpType::DROP) {
                    wdata.progress = 1. - wdata.progress;
                }
            } else {
                if (Style == WarpEffect::WarpType::DROP && warpTreatment == "in") {
                    wdata.progress = 1. - wdata.progress;
                }
            }
            if (Style == WarpEffect::WarpType::CIRCULAR_SWIRL) {
                wdata.speed = warpInterpolate(wdata.speed, 0.0, 1.0, 40.0, 9.0);
                if (warpTreatment == "in") {
                    wdata.progress = 1. - wdata.progress;
                }
            }
            break;
    }

    if (renderWarpGPU(wdata, buffer)) {
        return;
    }
    WarpEffect::Render(effect, SettingsMap, buffer);
}

#endif
