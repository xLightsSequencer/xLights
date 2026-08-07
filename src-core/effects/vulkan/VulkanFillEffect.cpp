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
#include "../FillEffect.h"

// Encode the Fill kernel into the render buffer's command stream. lut/painted
// are per-line arrays (BuildFillLut precomputed, one entry per row for
// Up/Down or per column for Left/Right) uploaded as real SSBOs -- unlike
// Metal's setBytes (capped at MAX_FILL_LUT = 1024 lines), createSharedBuffer
// has no such cap, so the Vulkan path never needs Metal's dim > MAX_FILL_LUT
// CPU fallback. Returns false (-> CPU fallback) if GPU resources aren't
// available.
static bool renderFillGPU(VulkanFillData& fdata, const xlColorVector& lut, const std::vector<uint8_t>& painted, RenderBuffer& buffer) {
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

    size_t dim = painted.size();

    // xlColor is r,g,b,a -- the same low-to-high byte order packPx uses for a
    // packed uint, so the LUT uploads with a straight memcpy (no per-element
    // pack needed).
    static_assert(sizeof(xlColor) == sizeof(uint32_t), "xlColor must pack into one uint32 for the GPU LUT upload");

    VulkanBuffer lutBuf;
    if (!u.createSharedBuffer(lutBuf, dim * sizeof(uint32_t), "FillLut")) {
        return false;
    }
    VulkanBuffer paintedBuf;
    if (!u.createSharedBuffer(paintedBuf, dim * sizeof(uint32_t), "FillPainted")) {
        u.destroyBuffer(lutBuf);
        return false;
    }

    std::memcpy(lutBuf.mapped, lut.data(), dim * sizeof(uint32_t));
    // GLSL has no 8-bit storage type -- widen each painted byte to a uint.
    uint32_t* paintedGpu = (uint32_t*)paintedBuf.mapped;
    for (size_t i = 0; i < dim; i++) {
        paintedGpu[i] = painted[i];
    }

    VkCommandBuffer cb = rbcd->getCommandBuffer("-Fill");
    if (cb == VK_NULL_HANDLE) {
        u.destroyBuffer(lutBuf);
        u.destroyBuffer(paintedBuf);
        return false;
    }

    // Bound the dispatch by the actual pixel allocation, not the logical
    // dimensions -- a variable sub-buffer can leave GetPixelCount() < width *
    // height, and the kernel writes result[index] with no bounds check on the
    // result buffer's real size (mirrors MetalFillEffectData::Render's
    // pixelCount clamp).
    uint32_t pixelCount = std::min<uint32_t>(fdata.width * fdata.height, (uint32_t)buffer.GetPixelCount());

    bool ok = rbcd->encodeEffectDispatch(cb, u.fillEffectFunction, "FillEffect",
                                         &fdata, sizeof(fdata), { px.buffer, lutBuf.buffer, paintedBuf.buffer },
                                         pixelCount, 0);
    if (ok) {
        // The aux buffers are referenced by the command buffer, so they must
        // outlive its GPU execution; destroyBuffer is immediate (no deferred
        // trash list), so commit+wait here before freeing (mirrors Candle).
        rbcd->commit();
        rbcd->waitForCompletion();
    }
    u.destroyBuffer(lutBuf);
    u.destroyBuffer(paintedBuf);
    return ok;
}

VulkanFillEffect::VulkanFillEffect(int i) : FillEffect(i) {
}
VulkanFillEffect::~VulkanFillEffect() {
}

void VulkanFillEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    // DMX models need SetPixel's channel translation, which the linear kernel
    // write can't reproduce; those (and undersized/unavailable buffers) take
    // the CPU path. Mirrors MetalFillEffect::Render exactly.
    if (rbcd == nullptr || buffer.IsDmxBuffer()
        || buffer.BufferWi <= 0 || buffer.BufferHt <= 0
        || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        FillEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    xlColorVector lut;
    std::vector<uint8_t> painted;
    int vertical = 0;
    int dim = 0;
    BuildFillLut(effect, SettingsMap, buffer, lut, painted, vertical, dim);

    // NOTE: Metal falls back to the CPU/ISPC path here when dim > MAX_FILL_LUT
    // (its LUT/painted arrays ride in setBytes, capped at 4KB). Vulkan uploads
    // both via createSharedBuffer -- a real buffer with no such cap -- so that
    // fallback is intentionally omitted; the Vulkan path covers the
    // large-buffer cases Metal punts to the CPU.
    (void)dim;

    VulkanFillData fdata{};
    fdata.width = buffer.BufferWi;
    fdata.height = buffer.BufferHt;
    fdata.vertical = vertical;

    if (renderFillGPU(fdata, lut, painted, buffer)) {
        return;
    }
    FillEffect::Render(effect, SettingsMap, buffer);
}

#endif
