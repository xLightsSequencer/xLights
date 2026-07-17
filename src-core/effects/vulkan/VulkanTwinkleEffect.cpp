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

#include <cstring>

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/Effect.h"
#include "../../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../TwinkleEffect.h"

// The kernel advances the per-light StrobeClass state array in place, so the
// active entries are uploaded, the dispatch is committed and waited on
// immediately (synchronous, unlike the other Vulkan effects which defer
// commit to the blend pass), and the result is copied back into the CPU
// cache before Render() returns -- the cache stays the single source of
// truth and the CPU (ISPC) and Vulkan paths remain interchangeable
// mid-sequence. Mirrors MetalTwinkleEffectData::Render in MetalTwinkleEffect.mm.
//
// StrobeClass (TwinkleEffect.h) is six packed int32 fields (24 bytes) with
// natural 4-byte alignment throughout, so -- unlike CandleState's uint8
// fields -- the state buffer uploads/downloads as a straight memcpy, no
// per-field widening loop needed. The LUT is an xlColorVector (RGBA bytes in
// r,g,b,a memory order), which already matches packPx's r=low-byte packed-
// uint layout byte-for-byte, so it too uploads with a straight memcpy.
static bool renderTwinkleGPU(VulkanTwinkleData& td, std::vector<StrobeClass>& states, const xlColorVector& lut, RenderBuffer& buffer) {
    if (td.curNumStrobe == 0) {
        return true; // nothing lit this frame; buffer stays cleared
    }
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

    size_t statesSize = (size_t)td.curNumStrobe * sizeof(StrobeClass);
    size_t lutBytes = lut.size() * sizeof(xlColor);

    VulkanBuffer stateBuf;
    if (!u.createSharedBuffer(stateBuf, statesSize, "TwinkleState")) {
        return false;
    }
    VulkanBuffer lutBuf;
    if (!u.createSharedBuffer(lutBuf, lutBytes, "TwinkleLut")) {
        u.destroyBuffer(stateBuf);
        return false;
    }
    memcpy(stateBuf.mapped, states.data(), statesSize);
    memcpy(lutBuf.mapped, lut.data(), lutBytes);

    VkCommandBuffer cb = rbcd->getCommandBuffer("-Twinkle");
    if (cb == VK_NULL_HANDLE) {
        u.destroyBuffer(stateBuf);
        u.destroyBuffer(lutBuf);
        return false;
    }

    // Dispatch is per-strobe, not per-pixel: curNumStrobe is the grid bound.
    // Buffer list order mirrors TwinkleFunctions.metal's argument order
    // (state, lut, result) -- encodeEffectDispatch assigns Vulkan bindings
    // positionally (0, 1, 2 here), which does not numerically match Metal's
    // buffer(1)/buffer(2)/buffer(3) indices, but the relative order is the
    // same and that's what the shader's binding declarations follow.
    bool ok = rbcd->encodeEffectDispatch(cb, u.twinkleEffectFunction, "TwinkleEffect",
                                         &td, sizeof(td), { stateBuf.buffer, lutBuf.buffer, px.buffer },
                                         td.curNumStrobe, 0);
    if (ok) {
        rbcd->commit();
        rbcd->waitForCompletion();
        memcpy(states.data(), stateBuf.mapped, statesSize);
    }
    u.destroyBuffer(stateBuf);
    u.destroyBuffer(lutBuf);
    return ok;
}

VulkanTwinkleEffect::VulkanTwinkleEffect(int i) : TwinkleEffect(i) {
}
VulkanTwinkleEffect::~VulkanTwinkleEffect() {
}

void VulkanTwinkleEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    if (buffer.captureSnapshot != nullptr) {
        // Frame-parallel serial capture pass is advance-only (the CPU path runs
        // the kernel with npix=0): no pixels are drawn, so never spend a GPU
        // submit + fence wait on it. Mirrors MetalTwinkleEffect::Render.
        TwinkleEffect::Render(effect, SettingsMap, buffer);
        return;
    }
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || buffer.IsDmxBuffer()
        || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        // Small buffers, DMX (SetPixel channel translation) and the no-GPU
        // case take the CPU path (ISPC / by-node scalar).
        TwinkleEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    // Setup (renewal/compaction) runs exactly once; a by-node buffer
    // discovered here goes to the shared serial scalar tail rather than back
    // through TwinkleEffect::Render (which would re-run the mutating setup).
    TwinkleFrame f = prepareTwinkleFrame(SettingsMap, buffer);
    if (f.isByNode) {
        renderTwinkleByNode(buffer, f);
        return;
    }

    xlColorVector lut;
    buildTwinkleLut(buffer, f, lut);

    VulkanTwinkleData td = {};
    td.width = (uint32_t)f.width;
    td.npix = (uint32_t)f.npix;
    td.curNumStrobe = (uint32_t)f.curNumStrobe;
    td.max_modulo = f.max_modulo;
    td.max_modulo2 = f.max_modulo2;
    td.colorcnt = f.colorcnt;
    td.lutStride = f.max_modulo + 1;
    td.lutSize = (int32_t)lut.size();
    td.new_algorithm = f.new_algorithm ? 1 : 0;
    td.reRandomize = f.reRandomize ? 1 : 0;
    uint64_t frameSeed = f.frameSeed;
    td.frameSeedLo = (uint32_t)(frameSeed & 0xFFFFFFFFu);
    td.frameSeedHi = (uint32_t)(frameSeed >> 32);

    if (renderTwinkleGPU(td, *f.states, lut, buffer)) {
        applyTwinkleFinishCount(f);
        return;
    }
    // Vulkan dispatch failed after setup -- evolve the same prepared state on
    // the ISPC path so the frame still renders and stays byte-identical.
    dispatchTwinkleISPC(buffer, f, lut);
    applyTwinkleFinishCount(f);
}

#endif
