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
#include <cstring>
#include <vector>

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/Effect.h"
#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"

// packPx's convention (r = low byte) -- see vulkan_common.glsl / unpackPx.
static uint32_t toU32(const xlColor& c) {
    return (uint32_t)c.red | ((uint32_t)c.green << 8) | ((uint32_t)c.blue << 16) | ((uint32_t)c.alpha << 24);
}

// The colors[]/yb[] tables are sized to buffMax (== BufferWi or BufferHt,
// unbounded), so -- unlike the fixed-size color LUTs baked into other
// effects' Params structs -- they are uploaded as real buffers, freshly
// created every frame (mirrors MetalGarlandsEffectData::Render's
// newBufferWithBytes calls). Because they are recreated per frame rather
// than being a persistent per-RenderBuffer resource, they cannot be freed
// until the GPU has actually consumed them, so this dispatch is committed
// and waited on immediately (synchronous, same tradeoff CandleEffect makes
// for its per-frame CandleState upload) rather than deferred like the
// other Vulkan effects -- there is no result read-back, only a safe point
// at which to destroy the two upload buffers.
static bool renderGarlandsGPU(VulkanGarlandsData& gdata, const std::vector<uint32_t>& colors,
                               const std::vector<int32_t>& yb, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    // Buffers before command buffer (a grow can reset the command pool).
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }

    VulkanBuffer colorsBuf;
    VulkanBuffer ybBuf;
    if (!u.createSharedBuffer(colorsBuf, colors.size() * sizeof(uint32_t), "GarlandsColors")) {
        return false;
    }
    if (!u.createSharedBuffer(ybBuf, yb.size() * sizeof(int32_t), "GarlandsYb")) {
        u.destroyBuffer(colorsBuf);
        return false;
    }
    memcpy(colorsBuf.mapped, colors.data(), colors.size() * sizeof(uint32_t));
    memcpy(ybBuf.mapped, yb.data(), yb.size() * sizeof(int32_t));

    VkCommandBuffer cb = rbcd->getCommandBuffer("-Garlands");
    if (cb == VK_NULL_HANDLE) {
        u.destroyBuffer(colorsBuf);
        u.destroyBuffer(ybBuf);
        return false;
    }

    // Bound by the actual pixel allocation, not the logical dimensions -- a
    // variable sub-buffer can leave GetPixelCount() < width * height and the
    // kernel writes result[index] with no bounds check (mirrors the Metal
    // wrapper's pixelCount clamp).
    uint32_t pixelCount = std::min(gdata.width * gdata.height, buffer.GetPixelCount());

    bool ok = rbcd->encodeEffectDispatch(cb, u.garlandsEffectFunction, "GarlandsEffect",
                                         &gdata, sizeof(gdata), { px.buffer, colorsBuf.buffer, ybBuf.buffer },
                                         pixelCount, 0);
    if (ok) {
        rbcd->commit();
        rbcd->waitForCompletion();
    }
    u.destroyBuffer(colorsBuf);
    u.destroyBuffer(ybBuf);
    return ok;
}

VulkanGarlandsEffect::VulkanGarlandsEffect(int i) : GarlandsEffect(i) {
}
VulkanGarlandsEffect::~VulkanGarlandsEffect() {
}

void VulkanGarlandsEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || buffer.IsDmxBuffer()
        || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        // DMX needs the SetPixel channel translation; the CPU path (ISPC) handles it.
        GarlandsEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    float oset = buffer.GetEffectTimeIntervalPosition();
    int GarlandType = SettingsMap.GetInt("SLIDER_Garlands_Type", sTypeDefault);
    int Spacing = GetValueCurveInt("Garlands_Spacing", sSpacingDefault, SettingsMap, oset, sSpacingMin, sSpacingMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    float cycles = GetValueCurveDouble("Garlands_Cycles", sCyclesDefault, SettingsMap, oset, sCyclesMin, sCyclesMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sCyclesDivisor);

    if (Spacing < 1) {
        Spacing = 1;
    }
    int dir = GarlandsEffect::GetDirection(SettingsMap.Get("CHOICE_Garlands_Direction", sDirectionDefault));
    double position = buffer.GetEffectTimeIntervalPosition(cycles);
    if (dir > 3) {
        dir -= 4;
        if (position > 0.5) {
            position = (1.0 - position) * 2.0;
        } else {
            position *= 2.0;
        }
    }
    int buffMax = buffer.BufferHt;
    int garlandWid = buffer.BufferWi;
    if (dir > 1) {
        buffMax = buffer.BufferWi;
        garlandWid = buffer.BufferHt;
    }
    if (buffMax < 1 || garlandWid < 1) {
        GarlandsEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    double PixelSpacing = Spacing * buffMax / 100.0;
    if (PixelSpacing < 2.0) PixelSpacing = 2.0;
    double total = buffMax * PixelSpacing - buffMax + 1;
    double positionOffset = total * position;

    std::vector<uint32_t> colors(buffMax);
    std::vector<int32_t> yb(buffMax);
    xlColor color;
    for (int ring = 0; ring < buffMax; ring++) {
        double ratio = double(buffMax - ring - 1) / double(buffMax);
        buffer.GetMultiColorBlend(ratio, false, color);
        colors[ring] = toU32(color);
        yb[ring] = (int)(1.0 + ring * PixelSpacing - positionOffset);
    }

    VulkanGarlandsData gdata = {};
    gdata.width = buffer.BufferWi;
    gdata.height = buffer.BufferHt;
    gdata.buffMax = buffMax;
    gdata.garlandType = GarlandType;
    gdata.dir = dir;
    gdata.invPS = (float)(1.0 / PixelSpacing);
    gdata.posOffOverPS = (float)(positionOffset / PixelSpacing);

    if (renderGarlandsGPU(gdata, colors, yb, buffer)) {
        return;
    }
    GarlandsEffect::Render(effect, SettingsMap, buffer);
}

#endif
