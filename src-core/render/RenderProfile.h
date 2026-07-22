#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// XL_RENDER_PROFILE=1 diagnostic: per-row / per-effect render timing.  The
// counters here are written only by the single thread running a RenderJob's
// current slice (no atomics), except gpuWaitNs which is attributed through the
// thread-local pointer below from GPURenderUtils::waitForRenderCompletion.

#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>

inline uint64_t xlProfNs(std::chrono::steady_clock::time_point a,
                         std::chrono::steady_clock::time_point b) {
    return (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
}

struct RenderJobProfile {
    // Per-stage inclusive wall time (ns).
    uint64_t effectNs = 0;      // Σ of the per-effect map below - CPU-side only.
                                // For a GPU effect this is just the dispatch
                                // encode; the kernel's cost is in gpuBusyNs.
    uint64_t blurZoomNs = 0;    // HandleLayerBlurZoom
    uint64_t transitionNs = 0;  // HandleLayerTransitions
    uint64_t blendNs = 0;       // CalcOutput
    // The output stage, split: `getColors` is the per-node scatter of final
    // colors into seqData; `setColors` is the reverse gather, the preload of
    // seqData back into a buffer layer for "blend with existing" rows.  They
    // were one counter, which hid that the two costs are not the same size.
    uint64_t getColorsNs = 0;
    uint64_t setColorsNs = 0;
    uint64_t gpuWaitNs = 0;     // parked in GPURenderUtils::waitForRenderCompletion
    uint64_t suspendedNs = 0;   // suspended awaiting an upstream frame
    uint64_t sliceNs = 0;       // active wall time across all slices

    uint64_t frames = 0;        // frames actually rendered
    uint64_t slices = 0;        // ProcessSlice entries
    uint64_t suspends = 0;      // suspension count

    // GPU execution, attributed back to the effect that encoded the work (see
    // GpuCommandBufferTag).  gpuBusyNs is Σ of per-command-buffer GPU windows,
    // NOT wall time: buffers from different rows overlap on the GPU, so this
    // can exceed the batch wall.  Rank effects by it; don't read it as a budget.
    uint64_t gpuBusyNs = 0;
    uint64_t gpuSharedNs = 0;   // portion of gpuBusyNs on buffers carrying >1 stage
    uint64_t gpuCbs = 0;        // command buffers attributed

    std::map<std::string, uint64_t> perEffectNs;
    std::map<std::string, uint64_t> perEffectCount;
    std::map<std::string, uint64_t> perEffectGpuNs;
    std::map<std::string, uint64_t> perEffectGpuCbs;

    void addEffect(const std::string& effectName, uint64_t ns) {
        effectNs += ns;
        perEffectNs[effectName] += ns;
        perEffectCount[effectName] += 1;
    }

    // Unlike every other counter here, this one is not written only by the slice
    // thread: a command buffer is waited by whoever needs its pixels, which can
    // be a parallel_for worker of this job rendering a sibling per-model buffer.
    // Only ever reached when profiling is on, so the lock costs nothing when off.
    std::mutex gpuMtx;
    void addGpu(const char* who, uint64_t ns, bool shared) {
        std::lock_guard<std::mutex> lk(gpuMtx);
        gpuBusyNs += ns;
        gpuCbs += 1;
        if (shared) {
            gpuSharedNs += ns;
        }
        perEffectGpuNs[who] += ns;
        perEffectGpuCbs[who] += 1;
    }

    void merge(const RenderJobProfile& o) {
        effectNs += o.effectNs;
        blurZoomNs += o.blurZoomNs;
        transitionNs += o.transitionNs;
        blendNs += o.blendNs;
        getColorsNs += o.getColorsNs;
        setColorsNs += o.setColorsNs;
        gpuWaitNs += o.gpuWaitNs;
        suspendedNs += o.suspendedNs;
        sliceNs += o.sliceNs;
        frames += o.frames;
        slices += o.slices;
        suspends += o.suspends;
        gpuBusyNs += o.gpuBusyNs;
        gpuSharedNs += o.gpuSharedNs;
        gpuCbs += o.gpuCbs;
        for (const auto& e : o.perEffectNs) {
            perEffectNs[e.first] += e.second;
        }
        for (const auto& e : o.perEffectCount) {
            perEffectCount[e.first] += e.second;
        }
        for (const auto& e : o.perEffectGpuNs) {
            perEffectGpuNs[e.first] += e.second;
        }
        for (const auto& e : o.perEffectGpuCbs) {
            perEffectGpuCbs[e.first] += e.second;
        }
    }

    uint64_t outputNs() const {
        return getColorsNs + setColorsNs;
    }
    uint64_t stageSumNs() const {
        return effectNs + blurZoomNs + transitionNs + blendNs + outputNs();
    }
    // A row's own timeline: active work plus the time it sat suspended waiting
    // on upstream (excludes pool-scheduling latency between slices).
    uint64_t wallNs() const {
        return sliceNs + suspendedNs;
    }
};

// Points at the profile of the RenderJob whose slice this thread is currently
// running (set for the slice's duration, otherwise null).  Nested parallel_for
// workers never see it set - the calling slice thread's inclusive stage timer
// still captures their wall time.
inline thread_local RenderJobProfile* tlsRenderProfile = nullptr;

// RAII stage timer.  Constructed with a target counter, or nullptr to disarm -
// when disarmed it never reads the clock, so profiling-off sites cost nothing
// beyond a null check.
struct StageTimer {
    uint64_t* target;
    std::chrono::steady_clock::time_point t0;
    explicit StageTimer(uint64_t* t) : target(t) {
        if (target != nullptr) {
            t0 = std::chrono::steady_clock::now();
        }
    }
    ~StageTimer() {
        if (target != nullptr) {
            *target += xlProfNs(t0, std::chrono::steady_clock::now());
        }
    }
    StageTimer(const StageTimer&) = delete;
    StageTimer& operator=(const StageTimer&) = delete;
};

// ---------------------------------------------------------------------------
// GPU attribution.
//
// A GPU effect only *encodes* its dispatch on the render thread; the kernel runs
// later and the CPU blocks for it in waitForRenderCompletion.  Timing the encode
// (what perEffectNs does) therefore reports a GPU effect as nearly free, and the
// real cost lands in the row's gpuWait bucket with no idea which effect caused
// it.  That under-reports a heavy kernel by orders of magnitude.
//
// So: stamp each command buffer with the effect that opened it, and when the
// buffer completes charge its GPU execution window back to that effect.
//
// The thread that waits on a command buffer is not always the thread that
// encoded into it (a slice thread can blur+wait a buffer a per-model worker
// opened), so the tag has to live on the command buffer, captured at encode
// time and consumed at completion.

// Set for the duration of an effect's Render() on whichever thread runs it -
// including per-model parallel_for workers, which tlsRenderProfile deliberately
// does not reach.  Null when profiling is off, which is the only check any of
// the GPU backends needs.
struct GpuEffectTag {
    RenderJobProfile* profile = nullptr;
    const char* effectName = nullptr;  // RenderableEffect::Name(), stable for the app's life
};
inline thread_local GpuEffectTag tlsGpuEffect;

struct GpuEffectScope {
    GpuEffectTag saved;
    bool armed;
    GpuEffectScope(RenderJobProfile* p, const char* name) : armed(p != nullptr) {
        if (armed) {
            saved = tlsGpuEffect;
            tlsGpuEffect.profile = p;
            tlsGpuEffect.effectName = name;
        }
    }
    ~GpuEffectScope() {
        if (armed) {
            tlsGpuEffect = saved;
        }
    }
    GpuEffectScope(const GpuEffectScope&) = delete;
    GpuEffectScope& operator=(const GpuEffectScope&) = delete;
};

// Stable names for GPU work that no effect owns.  Compared by pointer, so every
// caller must land on the same literal - hence the lookup rather than the raw
// postfix, which Vulkan passes as the effect name.
inline const char* xlGpuStageName(const std::string& postfix) {
    if (postfix == "-Blur") return "(gpu blur)";
    if (postfix == "-RotoZoom") return "(gpu rotozoom)";
    if (postfix == "-Transition") return "(gpu transition)";
    if (postfix == "-Blend") return "(gpu blend)";
    if (postfix == "-Map") return "(gpu map)";
    return "(gpu other)";
}

// Attribution state carried by one command buffer.  Owned by the backend's
// per-RenderBuffer compute data (Metal: MetalRenderBufferComputeData, Vulkan:
// VulkanRenderBufferComputeData), reset whenever the buffer is retired.
struct GpuCommandBufferTag {
    RenderJobProfile* profile = nullptr;
    const char* owner = nullptr;
    bool shared = false;  // a later stage piled onto the effect's buffer

    // Call for every getCommandBuffer() that hands back a usable buffer.
    void note(const std::string& postfix) {
        RenderJobProfile* p = tlsGpuEffect.profile;
        const char* who = tlsGpuEffect.effectName;
        if (p == nullptr) {
            // Not inside an effect render: blur / rotozoom / transition / blend
            // encoding on the slice thread, which does carry tlsRenderProfile.
            p = tlsRenderProfile;
            if (p == nullptr) {
                return;  // profiling off
            }
            who = xlGpuStageName(postfix);
        }
        if (profile == nullptr) {
            profile = p;
            owner = who;
        } else if (owner != who) {
            shared = true;
        }
    }
    bool armed() const {
        return profile != nullptr;
    }
    void complete(uint64_t gpuNs) {
        if (profile != nullptr) {
            profile->addGpu(owner, gpuNs, shared);
        }
        reset();
    }
    void reset() {
        profile = nullptr;
        owner = nullptr;
        shared = false;
    }
};
