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
#include <vector>

#include "VulkanEffects.h"
#include "VulkanComputeUtilities.h"
#include "VulkanEffectDataTypes.h"

#include "../../render/RenderBuffer.h"
#include "../MeteorsEffect.h"
#include "UtilClasses.h"

// GPU port of the axis-aligned Meteors gather (mirrors MetalMeteorsEffect.mm /
// MeteorsFunctions.metal). Meteors' GPU path hooks GatherMeteors only -- the
// CPU-side meteor simulation (add/move/remove, swirl folded into the axis
// coordinate, palette/range HSV precompute) stays entirely in MeteorsEffect
// and is shared with the CPU/ISPC path; only the final per-pixel gather
// dispatches to the kernel. No result read-back is needed, so (unlike
// Candle/Garlands) this could in principle defer the commit -- but the
// per-frame particle buffer is only alive for this call, so it is still
// committed and waited on before being destroyed (see the aux-buffer
// lifetime rule below).
static bool renderMeteorsGPU(VulkanMeteorsData& mdata, const std::vector<MeteorSnapshot>& parts, RenderBuffer& buffer) {
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

    // The kernel needs a valid particle buffer even when there are no meteors
    // (an icicle frame may still draw its dim background), so allocate at
    // least one (mirrors MetalMeteorsEffectData::Render's zero-particle
    // placeholder buffer).
    size_t numParts = std::max((size_t)1, parts.size());
    VulkanBuffer partsBuf;
    if (!u.createSharedBuffer(partsBuf, numParts * sizeof(VulkanMeteorParticle), "MeteorsParts")) {
        return false;
    }
    VulkanMeteorParticle* gpuParts = (VulkanMeteorParticle*)partsBuf.mapped;
    if (parts.empty()) {
        gpuParts[0] = VulkanMeteorParticle{};
    } else {
        for (size_t i = 0; i < parts.size(); i++) {
            gpuParts[i].a = parts[i].a;
            gpuParts[i].base = parts[i].base;
            gpuParts[i].h = parts[i].h;
            gpuParts[i].hue = (float)parts[i].hue;
            gpuParts[i].sat = (float)parts[i].sat;
            gpuParts[i].val = (float)parts[i].val;
        }
    }

    VkCommandBuffer cb = rbcd->getCommandBuffer("-Meteors");
    if (cb == VK_NULL_HANDLE) {
        u.destroyBuffer(partsBuf);
        return false;
    }

    uint32_t pixelCount = mdata.width * mdata.height;
    bool ok = rbcd->encodeEffectDispatch(cb, u.meteorsEffectFunction, "MeteorsEffect",
                                         &mdata, sizeof(mdata), { px.buffer, partsBuf.buffer },
                                         pixelCount, 0);
    if (ok) {
        // partsBuf is referenced by the command buffer, which submits later;
        // destroyBuffer is immediate (no deferred-trash list), so it would be
        // a use-after-free to destroy it before the GPU has actually consumed
        // it. Commit + wait here, exactly like Candle/Garlands' per-frame aux
        // buffers.
        rbcd->commit();
        rbcd->waitForCompletion();
    }
    u.destroyBuffer(partsBuf);
    return ok;
}

VulkanMeteorsEffect::VulkanMeteorsEffect(int i) : MeteorsEffect(i) {
}
VulkanMeteorsEffect::~VulkanMeteorsEffect() {
}

void VulkanMeteorsEffect::GatherMeteors(RenderBuffer& buffer, const MeteorsGatherParams& params, const std::vector<MeteorSnapshot>& parts) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        MeteorsEffect::GatherMeteors(buffer, params, parts);
        return;
    }

    VulkanMeteorsData mdata = {};
    mdata.width = buffer.BufferWi;
    mdata.height = buffer.BufferHt;
    mdata.mode = params.mode;
    mdata.direction = params.direction;
    mdata.tailLength = params.tailLength;
    mdata.colorScheme = params.colorScheme;
    mdata.allowAlpha = params.allowAlpha;
    mdata.numMeteors = (int)parts.size();
    mdata.wantBkg = params.wantBkg;
    mdata.frameSeedLo = (uint32_t)(params.frameSeed & 0xFFFFFFFFu);
    mdata.frameSeedHi = (uint32_t)(params.frameSeed >> 32);

    if (renderMeteorsGPU(mdata, parts, buffer)) {
        return;
    }
    MeteorsEffect::GatherMeteors(buffer, params, parts);
}

#endif
