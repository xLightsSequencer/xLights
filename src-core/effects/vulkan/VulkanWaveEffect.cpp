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

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/Effect.h"
#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../WaveEffect.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

// cols[] (2 int32 per column: [2x]=y1, [2x+1]=y2) is freshly computed every
// frame by BuildWaveColumns (the phase accumulator advances exactly once per
// call), so -- like GarlandsEffect's colors[]/yb[] -- it can't be a
// persistent per-RenderBuffer resource; it's uploaded as its own buffer and
// must be freed once the GPU has actually consumed it. Standard effects (Bars,
// Fill, ...) defer their command buffer's commit to the blend pass, which
// runs long after this function returns, so nothing else pins this buffer's
// lifetime until then. Mirroring VulkanGarlandsEffect.cpp's renderGarlandsGPU
// (and MetalCandleEffectData's synchronous-readback tradeoff), the dispatch
// is committed and waited on immediately here rather than deferred, purely so
// there is a safe point at which to destroy the upload buffer.
static bool renderWaveGPU(VulkanWaveData& wd, const std::vector<int32_t>& cols, RenderBuffer& buffer) {
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

    size_t colsSize = cols.size() * sizeof(int32_t);
    VulkanBuffer colsBuf;
    if (!u.createSharedBuffer(colsBuf, colsSize, "WaveCols")) {
        return false;
    }
    memcpy(colsBuf.mapped, cols.data(), colsSize);

    VkCommandBuffer cb = rbcd->getCommandBuffer("-Wave");
    if (cb == VK_NULL_HANDLE) {
        u.destroyBuffer(colsBuf);
        return false;
    }

    // Bound by the actual pixel allocation -- a variable sub-buffer can leave
    // GetPixelCount() < width * height and the kernel writes result[index]
    // with no bounds check beyond width*height (mirrors
    // MetalWaveEffectData::Render's pixelCount clamp).
    uint32_t pixelCount = (uint32_t)std::min<size_t>((size_t)wd.width * wd.height, (size_t)buffer.GetPixelCount());

    bool ok = rbcd->encodeEffectDispatch(cb, u.waveEffectFunction, "WaveEffect",
                                         &wd, sizeof(wd), { px.buffer, colsBuf.buffer },
                                         pixelCount, 0);
    if (ok) {
        rbcd->commit();
        rbcd->waitForCompletion();
    }
    u.destroyBuffer(colsBuf);
    return ok;
}

VulkanWaveEffect::VulkanWaveEffect(int i) : WaveEffect(i) {
}
VulkanWaveEffect::~VulkanWaveEffect() {
}

void VulkanWaveEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        WaveEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Advances the phase accumulator (and Ivy branch buffer) exactly once.
    // Both the GPU dispatch and every fallback below reuse the bands from
    // here, so the accumulator never double-advances (mirrors
    // MetalWaveEffect::Render).
    WaveKernelConfig cfg;
    std::vector<int32_t> cols;
    BuildWaveColumns(SettingsMap, buffer, cfg, cols);

    // Vulkan implements the None (constant-color) fill only. Rainbow/Palette
    // need a double-precision hue division to stay byte-identical (no double
    // on the GPU); DMX buffers need SetPixel channel translation; a variable
    // sub-buffer can be smaller than BufferWi*BufferHt. Those run on the ISPC
    // CPU path. Mirrors MetalWaveEffect::Render's fallback conditions exactly.
    size_t npix = (size_t)buffer.BufferWi * buffer.BufferHt;
    if (cfg.fillColor != WAVE_FILL_NONE || buffer.IsDmxBuffer() || npix > buffer.GetPixelCount()) {
        RenderWaveISPC(cfg, cols, buffer);
        return;
    }

    VulkanWaveData wd{};
    wd.width = buffer.BufferWi;
    wd.height = buffer.BufferHt;
    wd.yoffset = cfg.yoffset;
    wd.mirror = cfg.mirror;
    wd.noneColor = toUchar4(cfg.noneColor);

    if (renderWaveGPU(wd, cols, buffer)) {
        return;
    }
    RenderWaveISPC(cfg, cols, buffer);
}

#endif
