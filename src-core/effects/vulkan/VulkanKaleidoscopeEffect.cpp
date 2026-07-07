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
#include <cmath>

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"

static inline double toRadiansLocal(double deg) {
    return deg * M_PI / 180.0;
}

static bool renderKaleidoscopeGPU(VulkanKaleidoscopeData& d, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    // Buffers before command buffer — order matters (see blur()/doTransition()).
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VulkanBuffer& copy = rbcd->getPixelBufferCopy();
    if (!copy) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Kaleidoscope");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }

    VulkanComputeUtilities::computeBarrier(cb);
    VkBufferCopy region = { 0, 0, (VkDeviceSize)(d.width * d.height * 4) };
    vkCmdCopyBuffer(cb, px.buffer, copy.buffer, 1, &region);
    VulkanComputeUtilities::computeBarrier(cb);

    uint32_t pixelCount = d.width * d.height;
    return rbcd->encodeEffectDispatch(cb, u.kaleidoscopeEffectFunction, "KaleidoscopeEffect",
                                      &d, sizeof(d), { px.buffer, copy.buffer }, pixelCount, 0);
}

VulkanKaleidoscopeEffect::VulkanKaleidoscopeEffect(int i) : KaleidoscopeEffect(i) {
}
VulkanKaleidoscopeEffect::~VulkanKaleidoscopeEffect() {
}

void VulkanKaleidoscopeEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    std::string type = SettingsMap.Get("CHOICE_Kaleidoscope_Type", sTypeDefault);

    // Only GPU-accelerate the "new" types — Square 2, 6-Fold, 8-Fold, 12-Fold, Radial
    // (mirrors MetalKaleidoscopeEffect::Render's type gate exactly).
    if (type != "Square 2" && type != "6-Fold" && type != "8-Fold" && type != "12-Fold" && type != "Radial") {
        KaleidoscopeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        KaleidoscopeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float progress = buffer.GetEffectTimeIntervalPosition(1.f);

    int xCentreP = GetValueCurveInt("Kaleidoscope_X", sXDefault, SettingsMap, progress, sXMin, sXMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yCentreP = GetValueCurveInt("Kaleidoscope_Y", sYDefault, SettingsMap, progress, sYMin, sYMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int xCentre  = xCentreP * buffer.BufferWi / 100;
    int yCentre  = yCentreP * buffer.BufferHt / 100;
    int size     = GetValueCurveInt("Kaleidoscope_Size", sSizeDefault, SettingsMap, progress, sSizeMin, sSizeMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int rotation = GetValueCurveInt("Kaleidoscope_Rotation", sRotationDefault, SettingsMap, progress, sRotationMin, sRotationMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    double rotRad = toRadiansLocal((double)rotation);

    VulkanKaleidoscopeData kdata = {};
    kdata.width  = buffer.BufferWi;
    kdata.height = buffer.BufferHt;
    kdata.cx     = (float)xCentre;
    kdata.cy     = (float)yCentre;
    kdata.rotRad = (float)rotRad;

    if (type == "Square 2") {
        kdata.kType   = VULKAN_KALEIDOSCOPE_KTYPE_SQUARE2;
        kdata.size    = (float)size / 2.0f;   // halfSize
        kdata.maxIter = 0;
    } else if (type == "Radial") {
        kdata.kType   = VULKAN_KALEIDOSCOPE_KTYPE_RADIAL;
        kdata.size    = (float)std::max(2, size);  // number of segments
        kdata.maxIter = 0;
    } else {
        // Triangle types (6-Fold, 8-Fold, 12-Fold) all dispatch the same
        // kernel — they only differ in the CPU-computed vertices/maxIter.
        kdata.kType = VULKAN_KALEIDOSCOPE_KTYPE_TRIANGLE;

        KaleidoscopeTriangle tri = KaleidoscopeEffect::ComputeTriangle(type, (double)xCentre, (double)yCentre, (double)size, rotRad);
        kdata.v[0].x = (float)tri.v[0].x;
        kdata.v[0].y = (float)tri.v[0].y;
        kdata.v[1].x = (float)tri.v[1].x;
        kdata.v[1].y = (float)tri.v[1].y;
        kdata.v[2].x = (float)tri.v[2].x;
        kdata.v[2].y = (float)tri.v[2].y;

        int maxDim  = std::max(buffer.BufferWi, buffer.BufferHt);
        int maxIter = std::max(50, (maxDim * 3) / std::max(size, 1));
        if (maxIter > 500) maxIter = 500;
        kdata.maxIter = maxIter;
        kdata.size    = (float)size;
    }

    if (renderKaleidoscopeGPU(kdata, buffer)) {
        return;
    }

    // GPU render failed — fall back to CPU
    KaleidoscopeEffect::Render(effect, SettingsMap, buffer);
}

#endif
