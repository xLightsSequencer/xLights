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

// GPU-RESIDENT, unlike every other Vulkan effect: the flame grid survives
// between frames in two ping-ponged device buffers instead of being uploaded
// and read back each frame.  That is not a micro-optimisation -- the temporal
// advance itself is trivial next to a round trip of the grid (a 534x534 model
// is 1.1MB each way), so an upload/readback design would be slower than just
// running the ISPC path on the CPU.  Mirrors MetalFireEffectData::Render in
// MetalFireEffect.mm.
//
// Cross-frame ordering is safe without extra synchronisation: Fire is Stateful
// (never frame-parallel), so successive frames reuse the same RenderBuffer, and
// getCommandBuffer() fences on any still-committed command buffer before
// beginning the next one.  The previous frame's advance has therefore always
// completed before this frame's advance reads its output.
//
// The buffers hang off the RenderBuffer's infoCache under a key well clear of
// the effect ids that share that map; ~RenderBuffer deletes the entry (before
// GPURenderUtils::cleanUp, so the device is still alive), and destroyBuffer
// queues to the deferred-free list, so a buffer an in-flight command buffer
// still references is not released early.
static constexpr int FIRE_GPU_CACHE_KEY = 0x51F13E;

class VulkanFireGpuCache : public EffectRenderCache {
public:
    VulkanFireGpuCache() = default;
    virtual ~VulkanFireGpuCache() {
        VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
        u.destroyBuffer(prev);
        u.destroyBuffer(next);
        u.destroyBuffer(lut);
    }

    // Ping-ponged: the advance reads prev and writes next, then they swap.
    VulkanBuffer prev;
    VulkanBuffer next;
    VulkanBuffer lut;
    size_t lutBytes = 0;
    int cells = 0;
    bool resident = false; // prev holds a grid the GPU itself advanced
};

static VulkanFireGpuCache* getGpuCache(RenderBuffer& buffer) {
    VulkanFireGpuCache* c = (VulkanFireGpuCache*)buffer.infoCache[FIRE_GPU_CACHE_KEY];
    if (c == nullptr) {
        c = new VulkanFireGpuCache();
        buffer.infoCache[FIRE_GPU_CACHE_KEY] = c;
    }
    return c;
}

VulkanFireEffect::VulkanFireEffect(int i) : FireEffect(i) {
}
VulkanFireEffect::~VulkanFireEffect() {
}

bool VulkanFireEffect::RenderFireGPU(RenderBuffer& buffer, FireFrameParams& params) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VulkanRenderBufferComputeData* rbcd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&buffer);
    if (rbcd == nullptr || buffer.IsDmxBuffer()
        || (buffer.BufferWi * buffer.BufferHt) < (int)u.bufferSizeThreshold) {
        // DMX needs the SetPixel channel translation; the CPU path handles it.
        return false;
    }
    if (u.fireAdvanceEffectFunction == VK_NULL_HANDLE || u.fireDrawEffectFunction == VK_NULL_HANDLE) {
        return false;
    }
    const int cells = params.maxMWi * params.maxMHt;
    const int npix = std::min((int)buffer.GetPixelCount(), buffer.BufferWi * buffer.BufferHt);
    if (cells < 1 || npix < 1) {
        return false;
    }

    // Buffers before command buffer (a grow can reset the command pool).
    VulkanBuffer& px = rbcd->getPixelBuffer();
    if (!px) {
        return false;
    }

    VulkanFireGpuCache* gpu = getGpuCache(buffer);
    const size_t gridBytes = (size_t)cells * sizeof(int32_t);
    if (gpu->cells != cells || !gpu->prev || !gpu->next) {
        u.destroyBuffer(gpu->prev);
        u.destroyBuffer(gpu->next);
        gpu->resident = false;
        gpu->cells = 0;
        if (!u.createSharedBuffer(gpu->prev, gridBytes, "FireGridA")) {
            return false;
        }
        if (!u.createSharedBuffer(gpu->next, gridBytes, "FireGridB")) {
            u.destroyBuffer(gpu->prev);
            return false;
        }
        gpu->cells = cells;
    }
    // Seed (or re-seed) the resident grid from the CPU's copy. After that the
    // GPU owns it and nothing is uploaded per frame.
    if (!gpu->resident || params.gridChangedOnCpu) {
        memcpy(gpu->prev.mapped, params.grid->data(), gridBytes);
        gpu->resident = true;
    }

    // Once the GPU has advanced the resident grid, the CPU's copy is stale.
    // Any bail-out from here on has to hand the real state back, or the CPU
    // path would resume from a grid several frames behind.  prev still holds
    // the pre-advance grid until the swap at the end, which is exactly what
    // the CPU needs to advance from.
    auto bailToCpu = [&]() {
        if (gpu->resident) {
            memcpy(params.grid->data(), gpu->prev.mapped, gridBytes);
            gpu->resident = false;
        }
        return false;
    };

    const size_t lutBytes = (size_t)FirePaletteSize() * sizeof(xlColor);
    if (!gpu->lut || gpu->lutBytes != lutBytes) {
        u.destroyBuffer(gpu->lut);
        gpu->lutBytes = 0;
        if (!u.createSharedBuffer(gpu->lut, lutBytes, "FireLut")) {
            return bailToCpu();
        }
        gpu->lutBytes = lutBytes;
    }
    // Cheap enough to just refresh; the hue shift can be value-curved.
    memcpy(gpu->lut.mapped, params.lut, lutBytes);

    VkCommandBuffer cb = rbcd->getCommandBuffer("-Fire");
    if (cb == VK_NULL_HANDLE) {
        return bailToCpu();
    }

    VulkanFireData fd = {};
    fd.width = params.maxMWi;
    fd.height = params.maxMHt;
    fd.curWi = params.curWi;
    fd.curHt = params.curHt;
    fd.bufferWi = buffer.BufferWi;
    fd.bufferHt = buffer.BufferHt;
    fd.npix = npix;
    fd.loc = params.loc;
    fd.step = params.step;
    fd.pad = 0;
    fd.frameSeedLo = (uint32_t)(params.frameSeed & 0xFFFFFFFFu);
    fd.frameSeedHi = (uint32_t)(params.frameSeed >> 32);

    // Advance: one thread per grid cell, reading last frame's grid.
    // Buffer order matches the kernel's binding declarations: prev=0, next=1.
    if (!rbcd->encodeEffectDispatch(cb, u.fireAdvanceEffectFunction, "FireAdvanceEffect",
                                    &fd, sizeof(fd), { gpu->prev.buffer, gpu->next.buffer },
                                    (uint32_t)params.maxMWi, (uint32_t)params.maxMHt)) {
        rbcd->abortCommandBuffer();
        return bailToCpu();
    }
    // Vulkan does not hazard-track between dispatches the way Metal does: the
    // draw reads the grid the advance just wrote.
    VulkanComputeUtilities::computeBarrier(cb);

    // Draw: one thread per displayed pixel, from the grid just written.
    // result=0, grid=1, lut=2.
    if (!rbcd->encodeEffectDispatch(cb, u.fireDrawEffectFunction, "FireDrawEffect",
                                    &fd, sizeof(fd), { px.buffer, gpu->next.buffer, gpu->lut.buffer },
                                    (uint32_t)params.curWi, (uint32_t)params.curHt)) {
        rbcd->abortCommandBuffer();
        return bailToCpu();
    }

    // Swap so next frame's advance reads what was just produced.  Not committed
    // here -- the blend pass commits this command buffer, as with the other
    // deferred Vulkan effects; nothing needs the grid on the CPU.
    std::swap(gpu->prev, gpu->next);

    // The whole grid was advanced, so the flame may have grown; the CPU
    // fallback's row bound must not clip it if it ever takes over again.
    params.liveRow = params.maxMHt - 1;
    return true;
}

#endif
