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

static bool renderShimmerGPU(VulkanShimmerData& sdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Shimmer");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = sdata.width * sdata.height;
    return rbcd->encodeEffectDispatch(cb, u.shimmerEffectFunction, "ShimmerEffect",
                                      &sdata, sizeof(sdata), { px.buffer }, pixelCount, 0);
}

VulkanShimmerEffect::VulkanShimmerEffect(int i) : ShimmerEffect(i) {
}
VulkanShimmerEffect::~VulkanShimmerEffect() {
}

void VulkanShimmerEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        ShimmerEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // DMX buffers need the SetPixel channel translation - take the CPU path
    // (which itself does ISPC/scalar).
    if (buffer.IsDmxBuffer()) {
        ShimmerEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int colorIdx = 0;
    bool useAllColors = false;
    if (!CalcFrameState(SettingsMap, buffer, colorIdx, useAllColors)) {
        // duty-cycle "off" frame -- buffer stays untouched, same as the CPU path
        return;
    }

    xlColorVector lut;
    VulkanShimmerData sdata = {};
    sdata.width = buffer.BufferWi;
    sdata.height = buffer.BufferHt;
    sdata.lutMode = BuildShimmerLut(buffer, colorIdx, useAllColors, lut);
    sdata.colorCount = buffer.GetColorCount();
    uint64_t frameSeed = buffer.hashRandomFrameSeed();
    sdata.frameSeedLo = (uint32_t)(frameSeed & 0xFFFFFFFFu);
    sdata.frameSeedHi = (uint32_t)(frameSeed >> 32);

    if (lut.size() > VULKAN_SHIMMER_MAX_LUT) {
        // spatial gradient LUT too large for the params SSBO array -- CPU/ISPC path handles it
        ShimmerEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    for (size_t i = 0; i < lut.size(); i++) {
        sdata.lut[i] = toUchar4(lut[i]);
    }

    if (renderShimmerGPU(sdata, buffer)) {
        return;
    }
    ShimmerEffect::Render(effect, SettingsMap, buffer);
}

#endif
