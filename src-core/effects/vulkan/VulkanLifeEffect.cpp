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

// SOURCE-READING: the kernel reads the previous generation from a separate
// uploaded buffer (prev, a snapshot of RenderBuffer::GetTempBuf()) rather than
// from the shared pixel buffer, and writes the new generation into the pixel
// buffer. Because the CPU copies that new generation back into TempBuf after
// the dispatch (to seed the next generation), the command buffer is committed
// and waited on immediately here (synchronous, unlike the other Vulkan
// effects which defer commit to the blend pass). Mirrors
// MetalLifeEffectData::Render in MetalLifeEffect.mm.
static bool renderLifeGPU(VulkanLifeData& ld, const std::vector<uint32_t>& palette, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (!rbcd) {
        return false;
    }
    // Buffer before command buffer (a grow can reset the command pool).
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }

    int npix = ld.npix;
    if (npix < 1) {
        return false;
    }

    VulkanBuffer prevBuf;
    if (!u.createSharedBuffer(prevBuf, (size_t)npix * sizeof(uint32_t), "LifePrev")) {
        return false;
    }
    memcpy(prevBuf.mapped, buffer.GetTempBuf(), (size_t)npix * sizeof(uint32_t));

    VulkanBuffer paletteBuf;
    if (!u.createSharedBuffer(paletteBuf, palette.size() * sizeof(uint32_t), "LifePalette")) {
        u.destroyBuffer(prevBuf);
        return false;
    }
    memcpy(paletteBuf.mapped, palette.data(), palette.size() * sizeof(uint32_t));

    VkCommandBuffer cb = rbcd->getCommandBuffer("-Life");
    if (cb == VK_NULL_HANDLE) {
        u.destroyBuffer(prevBuf);
        u.destroyBuffer(paletteBuf);
        return false;
    }

    // Buffer order matches the kernel's binding declarations: prev=0,
    // result=1, palette=2 (params always ride binding 5).
    bool ok = rbcd->encodeEffectDispatch(cb, u.lifeEffectFunction, "LifeEffect",
                                         &ld, sizeof(ld), { prevBuf.buffer, px.buffer, paletteBuf.buffer },
                                         (uint32_t)npix, 0);
    if (ok) {
        rbcd->commit();
        rbcd->waitForCompletion();
    }
    u.destroyBuffer(prevBuf);
    u.destroyBuffer(paletteBuf);
    return ok;
}

VulkanLifeEffect::VulkanLifeEffect(int i) : LifeEffect(i) {
}
VulkanLifeEffect::~VulkanLifeEffect() {
}

void VulkanLifeEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || buffer.IsDmxBuffer()
        || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        // DMX needs the SetPixel channel translation; the CPU path (ISPC) handles it.
        LifeEffect::Render(effect, SettingsMap, buffer);
        return;
    }

    int Type = 0;
    if (!PrepareLifeGeneration(buffer, SettingsMap, Type)) {
        return; // frame satisfied by the tempbuf -> pixels copy
    }
    if (buffer.BufferWi < 1 || buffer.BufferHt < 1) {
        return;
    }

    std::vector<uint32_t> palette;
    BuildLifePalette(buffer, palette);

    VulkanLifeData ld = {};
    ld.width = buffer.BufferWi;
    ld.height = buffer.BufferHt;
    ld.npix = std::min((int)buffer.GetPixelCount(), buffer.BufferWi * buffer.BufferHt);
    ld.type = Type;
    ld.numColors = (int)palette.size();
    uint64_t frameSeed = buffer.hashRandomFrameSeed();
    ld.frameSeedLo = (uint32_t)(frameSeed & 0xFFFFFFFFu);
    ld.frameSeedHi = (uint32_t)(frameSeed >> 32);

    if (renderLifeGPU(ld, palette, buffer)) {
        buffer.CopyPixelsToTempBuf();
        return;
    }
    // Vulkan dispatch failed after the generation gate/seed already advanced; finish
    // this generation on the CPU (ISPC) without re-running PrepareLifeGeneration.
    RenderLifeGenerationISPC(buffer, Type);
}

#endif
