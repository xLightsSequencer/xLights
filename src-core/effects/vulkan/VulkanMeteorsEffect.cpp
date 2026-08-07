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
static bool renderMeteorsGPU(VulkanMeteorsData& mdata, const std::vector<MeteorSnapshot>& parts,
                             const std::vector<int>& lineItems, const std::vector<int>& lineStart, RenderBuffer& buffer) {
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

    // The kernel needs valid particle and line-bucket buffers even when there
    // are no meteors (an icicle frame may still draw its dim background), so
    // allocate at least one element of each (mirrors MetalMeteorsEffectData::
    // Render's zero-particle placeholder buffer). lineStart always carries its
    // lineCount+1 prefix offsets.
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

    size_t numItems = std::max((size_t)1, lineItems.size());
    VulkanBuffer itemsBuf;
    if (!u.createSharedBuffer(itemsBuf, numItems * sizeof(int32_t), "MeteorsLineItems")) {
        u.destroyBuffer(partsBuf);
        return false;
    }
    int32_t* gpuItems = (int32_t*)itemsBuf.mapped;
    if (lineItems.empty()) {
        gpuItems[0] = 0;
    } else {
        for (size_t i = 0; i < lineItems.size(); i++) {
            gpuItems[i] = lineItems[i];
        }
    }

    VulkanBuffer startsBuf;
    if (!u.createSharedBuffer(startsBuf, lineStart.size() * sizeof(int32_t), "MeteorsLineStart")) {
        u.destroyBuffer(itemsBuf);
        u.destroyBuffer(partsBuf);
        return false;
    }
    int32_t* gpuStarts = (int32_t*)startsBuf.mapped;
    for (size_t i = 0; i < lineStart.size(); i++) {
        gpuStarts[i] = lineStart[i];
    }

    VkCommandBuffer cb = rbcd->getCommandBuffer("-Meteors");
    if (cb == VK_NULL_HANDLE) {
        u.destroyBuffer(startsBuf);
        u.destroyBuffer(itemsBuf);
        u.destroyBuffer(partsBuf);
        return false;
    }

    uint32_t pixelCount = mdata.width * mdata.height;
    bool ok = rbcd->encodeEffectDispatch(cb, u.meteorsEffectFunction, "MeteorsEffect",
                                         &mdata, sizeof(mdata), { px.buffer, partsBuf.buffer, itemsBuf.buffer, startsBuf.buffer },
                                         pixelCount, 0);
    if (ok) {
        // partsBuf/itemsBuf/startsBuf are referenced by the command buffer,
        // which submits later; destroyBuffer is immediate (no deferred-trash
        // list), so it would be a use-after-free to destroy them before the GPU
        // has actually consumed them. Commit + wait here, exactly like
        // Candle/Garlands' per-frame aux buffers.
        rbcd->commit();
        rbcd->waitForCompletion();
    }
    u.destroyBuffer(startsBuf);
    u.destroyBuffer(itemsBuf);
    u.destroyBuffer(partsBuf);
    return ok;
}

// GPU port of the radial (Implode/Explode) draw, mirroring MeteorsRadialScatter
// / MeteorsRadialResolve in MeteorsFunctions.metal.  Three steps in one command
// buffer: fill the key buffer with 0 ("no trail reached this pixel"), scatter
// one thread per (meteor, phase) atomic-maxing the winning key, then resolve one
// thread per pixel.  Vulkan does not hazard-track between dispatches the way
// Metal does, so each step is separated by a computeBarrier.
static bool renderMeteorsRadialGPU(VulkanMeteorsRadialData& mdata,
                                   const std::vector<VulkanMeteorRadial>& parts, RenderBuffer& buffer) {
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

    VulkanBuffer partsBuf;
    if (!u.createSharedBuffer(partsBuf, std::max((size_t)1, parts.size()) * sizeof(VulkanMeteorRadial), "MeteorsRadialParts")) {
        return false;
    }
    VulkanMeteorRadial* gpuParts = (VulkanMeteorRadial*)partsBuf.mapped;
    if (parts.empty()) {
        gpuParts[0] = VulkanMeteorRadial{};
    } else {
        for (size_t i = 0; i < parts.size(); i++) {
            gpuParts[i] = parts[i];
        }
    }

    // Keys are scratch the GPU alone ever touches, so they never need to be
    // host-visible or read back.
    const uint32_t pixelCount = mdata.width * mdata.height;
    VulkanBuffer keysBuf;
    if (!u.createDeviceBuffer(keysBuf, (size_t)pixelCount * sizeof(uint32_t), "MeteorsRadialKeys")) {
        u.destroyBuffer(partsBuf);
        return false;
    }

    VkCommandBuffer cb = rbcd->getCommandBuffer("-MeteorsRadial");
    if (cb == VK_NULL_HANDLE) {
        u.destroyBuffer(keysBuf);
        u.destroyBuffer(partsBuf);
        return false;
    }

    VulkanComputeUtilities::computeBarrier(cb);
    vkCmdFillBuffer(cb, keysBuf.buffer, 0, (VkDeviceSize)pixelCount * sizeof(uint32_t), 0);

    VulkanComputeUtilities::computeBarrier(cb);
    bool ok = rbcd->encodeEffectDispatch(cb, u.meteorsRadialScatterFunction, "MeteorsRadialScatter",
                                         &mdata, sizeof(mdata), { keysBuf.buffer, partsBuf.buffer },
                                         (uint32_t)(parts.size() * (size_t)mdata.stride), 0);
    if (ok) {
        VulkanComputeUtilities::computeBarrier(cb);
        ok = rbcd->encodeEffectDispatch(cb, u.meteorsRadialResolveFunction, "MeteorsRadialResolve",
                                        &mdata, sizeof(mdata), { px.buffer, partsBuf.buffer, keysBuf.buffer },
                                        pixelCount, 0);
    }
    if (ok) {
        // partsBuf/keysBuf are referenced by the command buffer, which submits
        // later; destroyBuffer is immediate, so commit and wait before freeing
        // them (same aux-buffer lifetime rule as the gather path above).
        rbcd->commit();
        rbcd->waitForCompletion();
    }
    u.destroyBuffer(keysBuf);
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

    const int lineCount = (params.mode == 1) ? buffer.BufferHt : buffer.BufferWi;
    std::vector<int> lineStart;
    std::vector<int> lineItems;
    BucketMeteorsByLine(parts, lineCount, lineStart, lineItems);

    if (renderMeteorsGPU(mdata, parts, lineItems, lineStart, buffer)) {
        return;
    }
    MeteorsEffect::GatherMeteors(buffer, params, parts);
}

void VulkanMeteorsEffect::DrawRadialSnapshot(RenderBuffer& buffer, const MeteorsRadialFrameState& fs) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || fs.meteors.empty() || buffer.dmx_buffer
        || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        MeteorsEffect::DrawRadialSnapshot(buffer, fs);
        return;
    }

    VulkanMeteorsRadialData mdata = {};
    mdata.width = buffer.BufferWi;
    mdata.height = buffer.BufferHt;
    mdata.implode = fs.implode ? 1 : 0;
    mdata.tailLength = fs.tailLength;
    mdata.stride = fs.tailLength + 1;
    mdata.colorScheme = fs.colorScheme;
    mdata.allowAlpha = buffer.allowAlpha ? 1 : 0;
    mdata.fadeWithDistance = fs.fadeWithDistance ? 1 : 0;
    mdata.centerX = fs.centerX;
    mdata.centerY = fs.centerY;
    mdata.maxdiag = fs.maxdiag;
    mdata.numMeteors = (int)fs.meteors.size();
    const uint64_t seed = buffer.hashRandomFrameSeed();
    mdata.frameSeedLo = (uint32_t)(seed & 0xFFFFFFFFu);
    mdata.frameSeedHi = (uint32_t)(seed >> 32);

    // The key is n*stride + ph + 1 in a uint32; overflowing it would take
    // billions of trail steps, but fall back rather than wrap.
    if ((int64_t)fs.meteors.size() * mdata.stride >= (int64_t)UINT32_MAX) {
        MeteorsEffect::DrawRadialSnapshot(buffer, fs);
        return;
    }

    std::vector<int> cuts;
    ComputeRadialCuts(fs, cuts);

    std::vector<VulkanMeteorRadial> mp(fs.meteors.size());
    for (size_t i = 0; i < fs.meteors.size(); i++) {
        mp[i].x = fs.meteors[i].x;
        mp[i].y = fs.meteors[i].y;
        mp[i].dx = fs.meteors[i].dx;
        mp[i].dy = fs.meteors[i].dy;
        mp[i].hue = fs.meteors[i].hsv.hue;
        mp[i].sat = fs.meteors[i].hsv.saturation;
        mp[i].val = fs.meteors[i].hsv.value;
        mp[i].cut = cuts[i];
    }

    if (renderMeteorsRadialGPU(mdata, mp, buffer)) {
        return;
    }
    MeteorsEffect::DrawRadialSnapshot(buffer, fs);
}

#endif
