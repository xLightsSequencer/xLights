/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/
#pragma once

// Vulkan analogue of MetalComputeUtilities.hpp.  Only included by the
// vulkan/ translation units, which are themselves compiled out without
// HAVE_VULKAN.
#ifdef HAVE_VULKAN

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <volk.h>
#ifndef VMA_STATIC_VULKAN_FUNCTIONS
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#endif
#ifndef VMA_DYNAMIC_VULKAN_FUNCTIONS
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif
#include <vk_mem_alloc.h>

#include "GPURenderUtils.h"
#include "../../render/PixelBuffer.h"

class PixelBufferClass;
class RenderBuffer;

// A VkBuffer plus its VMA allocation.  "Shared" buffers are persistently
// mapped host-visible|coherent memory (mapped != nullptr) — the analogue of
// MTLResourceStorageModeShared; "device" buffers are GPU-local scratch
// (the analogue of MTLResourceStorageModePrivate).
struct VulkanBuffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    void* mapped = nullptr;
    size_t size = 0;

    explicit operator bool() const { return buffer != VK_NULL_HANDLE; }
};

class VulkanRenderBufferComputeData;

class VulkanPixelBufferComputeData {
public:
    VulkanPixelBufferComputeData();
    ~VulkanPixelBufferComputeData();

    bool doTransitions(PixelBufferClass* pixelBuffer, int layer, RenderBuffer* prevRB);
    bool doBlendLayers(PixelBufferClass* pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels);

    bool doMap(VkPipeline f, struct TransitionData& data, RenderBuffer* buffer);
    bool doTransition(VkPipeline f, struct TransitionData& data, RenderBuffer* buffer, RenderBuffer* prevRB);
    bool doTransition(VkPipeline f, struct TransitionData& data, RenderBuffer* buffer, VkBuffer prev);

    VulkanBuffer sparkleBuffer;
    size_t sparkleBufferCount = 0;
    VulkanBuffer tmpBufferBlend;

private:
    static void fillLayerBlendingData(struct LayerBlendingData& data, PixelBufferClass::LayerInfo* layer);
};

class VulkanRenderBufferComputeData {
public:
    VulkanRenderBufferComputeData(RenderBuffer* rb, VulkanPixelBufferComputeData* pixelBufferData, int l);
    ~VulkanRenderBufferComputeData();

    void bufferResized();

    VulkanPixelBufferComputeData* pixelBufferData;

    static VulkanRenderBufferComputeData* getVulkanRenderBufferComputeData(RenderBuffer*);

    // Returns the per-RenderBuffer command buffer in the recording state, or
    // VK_NULL_HANDLE when over the global budget (callers fall back to CPU).
    VkCommandBuffer getCommandBuffer(const std::string& postfix = "");
    void abortCommandBuffer();

    // Encode one compute dispatch: allocate+write a descriptor set (unused
    // bindings get the dummy buffer), push constants, dispatch.  gridH == 0
    // selects the 1-D form (64-wide workgroups); otherwise 8x8 2-D.  The
    // analogue of one Metal compute-encoder block.  Public because the
    // per-model blend/transition orchestration encodes into this buffer's
    // command stream.
    bool encodeDispatch(VkCommandBuffer cb, VkPipeline pipeline, const char* label,
                        const void* push, uint32_t pushSize,
                        std::initializer_list<VkBuffer> buffers,
                        uint32_t gridW, uint32_t gridH);

    // Per-effect dispatch: the effect parameter structs exceed the 128-byte
    // push-constant guarantee, so they are staged into a per-command-buffer
    // host-visible arena and bound as a read-only SSBO at binding 5.  The
    // positional `buffers` go to bindings 0..n (binding 0 is the result).
    bool encodeEffectDispatch(VkCommandBuffer cb, VkPipeline pipeline, const char* label,
                              const void* params, uint32_t paramsSize,
                              std::initializer_list<VkBuffer> buffers,
                              uint32_t gridW, uint32_t gridH);
    // Copy `size` bytes into the arena (growing it if needed), returning the
    // buffer + aligned offset.  Lifetime is the current command buffer.
    bool stageParams(const void* data, size_t size, VkBuffer& outBuf, VkDeviceSize& outOffset);

    VulkanBuffer& getPixelBuffer(bool sendToGPU = true);
    VulkanBuffer& getPixelBufferCopy();
    VulkanBuffer& getIndexBuffer();
    VulkanBuffer& getBlendBuffer();
    VulkanBuffer& getOwnerBuffer();
    VulkanBuffer& getRotoOwnerBuffer();

    void commit();
    bool isCommitted() { return committed; }
    // True when this frame's effect left GPU work queued (an open, unsubmitted
    // command buffer).  The render cache uses this to avoid draining that queue
    // (a pipeline-breaking readback) just to store a cheap-to-recompute effect.
    bool hasOpenCommandBuffer() { return commandBuffer != VK_NULL_HANDLE && !committed; }
    void waitForCompletion();

    // Retire a buffer that is being replaced (grown). If a command buffer is in
    // flight/recording it may still reference the old buffer, so instead of
    // draining the GPU (waitForCompletion) just to free it, hand it to the
    // deferred-free list and destroy it once this render buffer's command buffer
    // next completes. Frees immediately when nothing is in flight. Mirrors the
    // implicit lifetime extension Metal gets from ARC. `b` is cleared.
    void retireBuffer(VulkanBuffer& b);

    bool blur(int radius);
    // Integer box blur (small-blur case).  Only runs when the buffer already
    // has an open command buffer (its effect rendered on the GPU this frame);
    // it appends to that command buffer, so there is no upload/download bounce.
    // Returns false otherwise -> the bit-identical CPU box blur handles it.
    bool boxBlur(int d, int u);
    bool rotoZoom(GPURenderUtils::RotoZoomSettings& settings);

    VulkanBuffer maskBuffer;

    // Transition-mask staging for masks built by the CPU transition path
    // (kept alive until the command buffer completes — Metal relies on ARC
    // for this, Vulkan needs explicit ownership).
    VulkanBuffer cpuMaskUpload;

private:
    bool ensureCommandInfra();
    bool callRotoZoomFunction(VkPipeline function, VkPipeline claimFunction, struct RotoZoomData& data);
    VkDescriptorSet allocateDescriptorSet();
    void resetDescriptorPools();
    // XL_RENDER_PROFILE only: bracket the command buffer with timestamp queries
    // so its GPU execution can be charged back to the effect that opened it.
    bool ensureTimestampPool();

    RenderBuffer* renderBuffer;
    int layer;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkQueryPool timestampPool = VK_NULL_HANDLE;
    bool timestamped = false;   // this command buffer carries the query pair
    GpuCommandBufferTag cbTag;  // inert (null profile) when profiling is off
    std::vector<VkDescriptorPool> descriptorPools;
    size_t activePool = 0;
    VulkanBuffer paramArena;
    VkDeviceSize paramArenaOffset = 0;
    bool recording = false;
    bool committed = false;

    VulkanBuffer pixelBuffer;
    VulkanBuffer pixelBufferCopy;
    size_t pixelBufferSize = 0;
    VulkanBuffer blendBuffer;
    VulkanBuffer indexBuffer;
    int32_t* indexes = nullptr;
    int indexesSize = 0;
    // pixel index -> node index that deterministically owns the pixel (the
    // last node in Nodes order covering it).  PutColorsForNodes gates its
    // scatter on this so concurrent GPU threads never race for a shared
    // pixel; rebuilt lazily after any geometry change (see bufferResized).
    VulkanBuffer ownerBuffer;
    int ownerSize = 0;
    bool ownerStale = true;
    // per-dispatch scratch for the rotozoom claim pass (pixel -> winning
    // source index); refilled to -1 before every rotation, so no staleness
    // tracking is needed.
    VulkanBuffer rotoOwnerBuffer;
    int rotoOwnerSize = 0;

    // Buffers replaced while a command buffer was in flight; freed once that
    // command buffer completes (see retireBuffer / waitForCompletion).
    std::vector<VulkanBuffer> pendingFree;

    static std::atomic<uint32_t> commandBufferCount;
};

class VulkanComputeUtilities {
public:
    VulkanComputeUtilities();
    ~VulkanComputeUtilities();

    // Lazy bring-up: unlike Metal (device created at static init), volk must
    // dlopen the loader, so all Vulkan work waits for the first real call.
    bool computeEnabled() {
        ensureInit();
        return enabled;
    }
    bool prioritizeGraphics() {
        return pg;
    }
    void prioritizeGraphics(bool p) {
        pg = p;
    }

    // ~Concurrent effect renders the GPU can absorb, for sizing the render
    // pool (0 if Vulkan unusable).  Vulkan exposes no core count, so this is
    // a device-type heuristic; XL_VULKAN_CONCURRENCY overrides.
    int gpuCoreCount();

    bool createSharedBuffer(VulkanBuffer& b, size_t size, const std::string& name);
    bool createDeviceBuffer(VulkanBuffer& b, size_t size, const std::string& name);
    void destroyBuffer(VulkanBuffer& b);

    // Compute→compute execution+memory barrier between two dispatches in the
    // same command buffer.  Metal hazard-tracks this implicitly; Vulkan does
    // not, so every pair of dependent dispatches needs one.
    static void computeBarrier(VkCommandBuffer cb);

    // One layout for every kernel: 6 positional storage-buffer bindings
    // (kernels use bindings 0..n like Metal's setBuffer atIndex:1..; unused
    // bindings get the dummy buffer) plus a 128-byte push-constant range
    // (the analogue of Metal setBytes; guaranteed minimum on all devices).
    static constexpr uint32_t NUM_BINDINGS = 6;
    static constexpr uint32_t PUSH_CONSTANT_SIZE = 128;
    VkDescriptorSetLayout dsLayout = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VulkanBuffer dummyBuffer;

    VkPipeline createComputePipeline(const uint32_t* spirv, size_t sizeBytes, const char* name);

    VkPipeline tentBlurHFunction = VK_NULL_HANDLE;
    VkPipeline tentBlurVFunction = VK_NULL_HANDLE;
    VkPipeline boxBlurFunction = VK_NULL_HANDLE;
    VkPipeline xrotateFunction = VK_NULL_HANDLE;
    VkPipeline yrotateFunction = VK_NULL_HANDLE;
    VkPipeline zrotateFunction = VK_NULL_HANDLE;
    VkPipeline xrotateClaimFunction = VK_NULL_HANDLE;
    VkPipeline yrotateClaimFunction = VK_NULL_HANDLE;
    VkPipeline zrotateClaimFunction = VK_NULL_HANDLE;
    VkPipeline rotateBlankFunction = VK_NULL_HANDLE;

    // Per-effect kernels (Phase D)
    VkPipeline barsEffectFunction = VK_NULL_HANDLE;
    VkPipeline colorWashEffectFunction = VK_NULL_HANDLE;
    VkPipeline shockwaveEffectFunction = VK_NULL_HANDLE;
    VkPipeline fanEffectFunction = VK_NULL_HANDLE;
    VkPipeline spiralsEffectFunction = VK_NULL_HANDLE;
    VkPipeline galaxyEffectFunction = VK_NULL_HANDLE;
    VkPipeline circlesEffectFunction = VK_NULL_HANDLE;
    VkPipeline plasmaEffectFunction = VK_NULL_HANDLE;
    VkPipeline butterflyEffectFunction = VK_NULL_HANDLE;
    VkPipeline pinwheelEffectFunction = VK_NULL_HANDLE;
    VkPipeline kaleidoscopeEffectFunction = VK_NULL_HANDLE;
    VkPipeline warpEffectFunction = VK_NULL_HANDLE;
    VkPipeline treeEffectFunction = VK_NULL_HANDLE;
    VkPipeline shimmerEffectFunction = VK_NULL_HANDLE;
    VkPipeline candleEffectFunction = VK_NULL_HANDLE;
    VkPipeline waveEffectFunction = VK_NULL_HANDLE;
    VkPipeline garlandsEffectFunction = VK_NULL_HANDLE;
    VkPipeline fillEffectFunction = VK_NULL_HANDLE;
    VkPipeline meteorsEffectFunction = VK_NULL_HANDLE;
    VkPipeline twinkleEffectFunction = VK_NULL_HANDLE;
    VkPipeline lifeEffectFunction = VK_NULL_HANDLE;

    VkPipeline getColorsFunction = VK_NULL_HANDLE;
    VkPipeline putColorsFunction = VK_NULL_HANDLE;
    VkPipeline adjustHSVFunction = VK_NULL_HANDLE;
    VkPipeline applySparklesFunction = VK_NULL_HANDLE;
    VkPipeline brightnessContrastFunction = VK_NULL_HANDLE;
    VkPipeline brightnessLevelFunction = VK_NULL_HANDLE;
    VkPipeline firstLayerFadeFunction = VK_NULL_HANDLE;
    VkPipeline nonAlphaFadeFunction = VK_NULL_HANDLE;

    class BlendFunctionInfo {
    public:
        BlendFunctionInfo(VkPipeline fn, const char* name, int mtd = 0, bool needIndexes = false) :
            function(fn), name(name), mixTypeData(mtd), needIndexes(needIndexes) {}
        ~BlendFunctionInfo() = default;

        VkPipeline function;
        std::string name;
        int mixTypeData;
        bool needIndexes;
    };
    std::map<MixTypes, BlendFunctionInfo*> blendFunctions;

    class TransitionInfo {
    public:
        explicit TransitionInfo(int t) : function(VK_NULL_HANDLE), type(t), reversed(false) {}
        TransitionInfo(VkPipeline fn, int t, bool r = false) : function(fn), type(t), reversed(r) {}
        ~TransitionInfo() = default;
        VkPipeline function;
        int type; // 0 = fade/none, 1 = mask/map, 2 = two-buffer, 3 = dissolve
        bool reversed;
    };
    std::map<std::string, TransitionInfo*> transitions;
    VulkanBuffer dissolveBuffer;

    bool enabled = false;
    std::atomic<bool> pg{false};
    uint32_t bufferSizeThreshold = 2048;

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    uint32_t queueFamilyIndex = 0;
    // Graphics/present queue for the on-screen Vulkan graphics backend.  The
    // compute queue above deliberately prefers an async-compute family, so
    // presentation gets its own graphics-capable queue.  On single-family
    // devices this aliases the compute queue — guard submissions with
    // graphicsSubmitMutex(), which then maps to queueMutex.  VK_NULL_HANDLE
    // when the instance lacks surface extensions or the device has no
    // graphics family (compute-only accelerator).
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamilyIndex = 0;
    bool surfaceExtensionsEnabled = false;
    bool swapchainExtensionEnabled = false;
    std::mutex graphicsQueueMutex;
    std::mutex& graphicsSubmitMutex() { return graphicsQueue == queue ? queueMutex : graphicsQueueMutex; }
    VmaAllocator allocator = VK_NULL_HANDLE;
    VkPhysicalDeviceType deviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
    std::string deviceName;
    // minStorageBufferOffsetAlignment — arena sub-allocations for effect
    // param structs (bound as SSBOs) must start on this boundary.
    VkDeviceSize storageBufferAlignment = 256;
    // ns per timestamp tick; 0 when the compute queue cannot timestamp, which
    // turns off XL_RENDER_PROFILE's per-effect GPU attribution on this device.
    float timestampPeriod = 0.0f;

    // All vkQueueSubmit calls are serialized on this (queues are externally
    // synchronized); command recording happens lock-free in per-RenderBuffer
    // command buffers.
    std::mutex queueMutex;

    // XL_GPU_STATS=1 dispatch counters, dumped at exit.
    std::atomic<uint64_t> statBlur{0};
    std::atomic<uint64_t> statBoxBlur{0};
    std::atomic<uint64_t> statRotoZoom{0};
    std::atomic<uint64_t> statTransition{0};
    std::atomic<uint64_t> statBlend{0};
    std::atomic<uint64_t> statSetup{0};
    std::atomic<uint64_t> statBlurCall{0};
    std::atomic<uint64_t> statEffect{0};

    void setObjectName(uint64_t handle, VkObjectType type, const std::string& name);

    void ensureInit();

    static VulkanComputeUtilities INSTANCE;

private:
    void doInit();
    bool createInstance(bool wantValidation);
    bool pickPhysicalDevice();
    bool createDeviceAndQueue();
    bool createAllocator();
    bool buildPipelines();

    std::once_flag initFlag;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    bool validation = false;
    // Graphics-capable queue family found by pickPhysicalDevice, consumed by
    // createDeviceAndQueue (-1 = compute-only device).
    int graphicsFamilyCandidate = -1;
};

#endif
