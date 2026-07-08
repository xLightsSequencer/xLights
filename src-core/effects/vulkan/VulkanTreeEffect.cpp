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

static bool renderTreeGPU(VulkanTreeData& tdata, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }
    VkCommandBuffer cb = rbcd->getCommandBuffer("-Tree");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t pixelCount = tdata.width * tdata.height;
    return rbcd->encodeEffectDispatch(cb, u.treeEffectFunction, "TreeEffect",
                                      &tdata, sizeof(tdata), { px.buffer }, pixelCount, 0);
}

VulkanTreeEffect::VulkanTreeEffect(int i) : TreeEffect(i) {
}
VulkanTreeEffect::~VulkanTreeEffect() {
}

void VulkanTreeEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        TreeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int Branches = SettingsMap.GetInt("SLIDER_Tree_Branches", sBranchesDefault);
    int tspeed = SettingsMap.GetInt("SLIDER_Tree_Speed", sSpeedDefault);
    bool showlights = SettingsMap.GetBool("CHECKBOX_Tree_ShowLights", sShowLightsDefault);

    int effectState = (buffer.curPeriod - buffer.curEffStartPer) * tspeed * buffer.frameTimeInMs / 50;

    if (Branches < 1) Branches = 1;
    int pixels_per_branch = (int)(0.5 + buffer.BufferHt / Branches);
    if (pixels_per_branch < 1) pixels_per_branch = 1;

    // DMX buffers need the SetPixel channel translation, oversized branches don't fit the
    // params array, and a variable sub-buffer can be smaller than BufferWi*BufferHt
    // (the kernel writes unguarded) — those all take the CPU path (which itself does ISPC/scalar).
    if (buffer.IsDmxBuffer() || pixels_per_branch > MAX_VULKAN_TREE_PPB
        || (size_t)(buffer.BufferWi * buffer.BufferHt) > buffer.GetPixelCount()) {
        TreeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int maxFrame = (Branches + 1) * buffer.BufferWi;
    int frame;
    if (effectState > 0 && maxFrame > 0) frame = (effectState / 4) % maxFrame;
    else frame = 1;

    VulkanTreeData tdata = {};
    tdata.width = buffer.BufferWi;
    tdata.height = buffer.BufferHt;
    tdata.ppb = pixels_per_branch;
    tdata.frame = frame;
    tdata.branch_row = (effectState / buffer.BufferWi) % Branches;
    tdata.f_mod = (effectState / 4) % buffer.BufferWi;
    tdata.showlights = showlights ? 1 : 0;

    xlColor color;
    for (int mod = 1; mod <= pixels_per_branch; mod++) {
        float V = 1 - (1.0 * mod / pixels_per_branch) * 0.70;
        buffer.palette.GetColor(0, color);
        if (buffer.allowAlpha) {
            color.alpha = 255.0 * V;
        } else {
            HSVValue hsv = color.asHSV();
            hsv.value = V;
            color = hsv;
        }
        tdata.bgColors[mod - 1] = toUchar4(color);
    }
    for (int r = 0; r < 5; r++) {
        float H = r / 4.0;
        HSVValue hsv;
        hsv.hue = H;
        hsv.saturation = 1.0;
        hsv.value = 1.0;
        color = hsv;
        tdata.lightColors[r] = toUchar4(color);
    }

    if (renderTreeGPU(tdata, buffer)) {
        return;
    }
    TreeEffect::Render(effect, SettingsMap, buffer);
}

#endif
