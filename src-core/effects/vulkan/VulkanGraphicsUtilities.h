#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Headless Vulkan graphics-pipeline foundation: render a fullscreen fragment
// pipeline into an offscreen RGBA8 image and land the pixels in a RenderBuffer.
// This is the load-bearing piece the native Vulkan Shader effect stands on (and
// the starting point for a future src-core/graphics UI port).  It reuses the
// compute backend's device / allocator (VulkanComputeUtilities::INSTANCE) but a
// dedicated graphics queue, and renders into its own image + host-visible
// readback buffer so it never shares queue-family ownership with the compute
// SSBOs.  Compiled only under HAVE_VULKAN.
#ifdef HAVE_VULKAN_SHADER

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#include <volk/volk.h>

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "VulkanComputeUtilities.h"
#include "../../render/RenderProfile.h"

class RenderBuffer;

// One in-flight shader frame's resources: offscreen render target + readback,
// UBO, input image, command buffer and fence.  Acquired from a pool for each
// submitted frame and recycled at completion, so a frame can stay in flight
// after Render() returns (see submitShaderFrame/completeShaderFrame) without
// its resources being reused underneath it.  Pooled targets are leaked at exit
// like the rest of the backend.
struct VulkanGraphicsTarget {
    uint32_t width = 0;
    uint32_t height = 0;
    VkImage image = VK_NULL_HANDLE;
    VmaAllocation imageAlloc = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VulkanBuffer readback;                 // host-visible, W*H*4
    VkCommandPool pool = VK_NULL_HANDLE;
    VkCommandBuffer cb = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    // Two-slot timestamp pool bracketing the submitted work.  The fence wait
    // measures GPU execution PLUS time queued behind other rows' submissions on
    // the shared graphics queue; the timestamp delta is execution alone, so the
    // difference between the two is the queueing.  Created lazily, only when
    // stats/profiling ask for it.
    VkQueryPool tsPool = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;  // shader-effect UBO/sampler set
    VkDescriptorSet descSet = VK_NULL_HANDLE;

    // Uniforms.  Owned here (not by the effect cache) so nothing tied to an
    // effect's cache lifetime is referenced by an in-flight frame.
    VulkanBuffer ubo;

    // Sampled input image (canvas/feedback pixels or the 128x1 audio FFT) and
    // its staging buffer.  The upload is recorded at the head of the render
    // command buffer — one submit per frame, not a separate round trip.
    uint32_t inW = 0, inH = 0;
    VkFormat inFmt = VK_FORMAT_UNDEFINED;
    VkImage inImage = VK_NULL_HANDLE;
    VmaAllocation inAlloc = VK_NULL_HANDLE;
    VkImageView inView = VK_NULL_HANDLE;
    VulkanBuffer staging;

    // In-flight bookkeeping, valid between submit and completion.
    bool inFlight = false;
    bool timestamped = false;
    uint8_t* dstPixels = nullptr;  // completion memcpy destination (null: skip)
    bool flipRows = false;         // bottom-row-first destination (renderToBuffer)
    GpuCommandBufferTag tag;       // profile attribution, captured at record time
    std::string statLabel;         // shader file for per-shader stats (stats-gated)
};

// Sampled-input description for one shader frame.  `data` is copied into the
// target's staging buffer during submit, so it only needs to live for the call.
struct VulkanShaderInput {
    uint32_t w = 0;
    uint32_t h = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    const void* data = nullptr;
    size_t byteSize = 0;
};

class VulkanGraphicsUtilities {
public:
    static VulkanGraphicsUtilities INSTANCE;

    // Graphics path is usable: compute backend up AND a graphics-capable queue
    // exists.  ensureInit() is idempotent (call_once inside).
    bool available();
    // Public entry: guarantees compute is up first, then builds the graphics
    // pipeline.  Safe from a render context (NOT from inside compute init).
    void ensureInit();
    // Builds the graphics pipeline assuming the compute device is already up.
    // Called from within VulkanComputeUtilities::doInit (device ready, but its
    // call_once is mid-flight, so we must not re-enter compute ensureInit).
    void initInline();

    // Build a fullscreen-triangle graphics pipeline from vertex + fragment
    // SPIR-V words against `layout` and the shared RGBA8 render pass.  Returns
    // VK_NULL_HANDLE on failure.
    VkPipeline buildPipeline(const uint32_t* vert, size_t vertBytes,
                             const uint32_t* frag, size_t fragBytes,
                             VkPipelineLayout layout);

    // A pipeline layout with an optional push-constant range (fragment stage)
    // and an optional descriptor-set layout (set 0).  Pass 0 / VK_NULL_HANDLE to
    // omit.  Owned/cached by the caller.
    VkPipelineLayout createPipelineLayout(uint32_t pushConstantBytes,
                                          VkDescriptorSetLayout setLayout);

    // The descriptor-set layout / pipeline layout the native Shader effect
    // builds its pipelines against: set 0 = { binding 0: UBO (uniforms),
    // binding 1: combined image sampler (input image; a 1x1 dummy when unused) },
    // both fragment stage.  One layout serves generative and image shaders.
    VkDescriptorSetLayout shaderSetLayout();
    VkPipelineLayout shaderPipelineLayout();

    // Submit one shader frame WITHOUT waiting: acquires a target, records the
    // (optional) input upload + fullscreen draw + readback copy into a single
    // command buffer, submits it, and returns the in-flight frame.  The frame
    // MUST be finished exactly once with completeShaderFrame() before
    // buffer.pixels is read — normally deferred through
    // VulkanRenderBufferComputeData::drainPendingShaderFrame() so the CPU is
    // not parked per frame and rows pipeline on the shared graphics queue
    // instead of serializing.  Returns nullptr on failure (nothing pending).
    VulkanGraphicsTarget* submitShaderFrame(RenderBuffer& buffer, VkPipeline pipeline,
                                            const void* uboData, uint32_t uboSize,
                                            const VulkanShaderInput* input,
                                            const std::string& statLabel = std::string());
    // Wait for the frame's fence, book stats/profile attribution, copy the
    // pixels to their destination, and recycle the target.  Safe to call with
    // nullptr.  Returns true when the fence signalled and the pixels landed.
    bool completeShaderFrame(VulkanGraphicsTarget* frame);

    // Render `pipeline` (a fullscreen draw) into buffer.pixels, synchronously.
    // Optional push constants (fragment stage) and a bound descriptor set
    // (set 0).  Handles the RenderBuffer's bottom-row-first convention.
    // Returns false (caller falls back) if the graphics path is unavailable or
    // a Vulkan call fails.
    bool renderToBuffer(RenderBuffer& buffer, VkPipeline pipeline, VkPipelineLayout layout,
                        const void* pushData, uint32_t pushBytes,
                        VkDescriptorSet descSet);

    // Phase-1 self-test: render the built-in UV-gradient shader at a small size
    // and confirm the corners came out as expected.  Logs PASS/FAIL.  Run
    // automatically at init when XL_VULKAN_GFXTEST is set.
    bool selfTest();

    // XL_VULKAN_SUBMITBENCH=1: time submit+fence round trips in isolation, to
    // separate the driver's floor from cost this code adds.  Logs to stderr.
    void submitLatencyBench();

    // Phase-2 in-binary proof: translate a push-constant-driven fragment shader
    // at runtime (glslang GLSL->SPIR-V), build a pipeline from the fresh SPIR-V,
    // and render it twice with different uniform values, confirming the uniform
    // reaches the shader.  Logs PASS/FAIL.  Run under XL_VULKAN_GFXTEST.
    bool runtimeShaderTest();

    VkRenderPass renderPass() const { return renderPass_; }

private:
    void doInit();
    VkShaderModule shaderModule(const uint32_t* words, size_t bytes);
    bool ensureTarget(VulkanGraphicsTarget& t, uint32_t w, uint32_t h);
    void destroyTarget(VulkanGraphicsTarget& t);
    // Pool of frame targets: acquired per submitted frame, returned at
    // completion.  Prefers a size-matching free target to avoid re-allocating
    // the render image every frame.
    VulkanGraphicsTarget* acquireTarget(uint32_t w, uint32_t h);
    void releaseTarget(VulkanGraphicsTarget* t);
    // Ensure the target's input image/staging match `input` and memcpy the data
    // into staging.  Returns the image view to bind (dummy when input==nullptr),
    // or VK_NULL_HANDLE on failure.
    VkImageView prepareInput(VulkanGraphicsTarget& t, const VulkanShaderInput* input);
    // Record the (optional) input upload + fullscreen draw + readback copy and
    // submit — does NOT wait.  Sets t.inFlight on success.
    bool recordAndSubmit(VulkanGraphicsTarget& t, uint32_t w, uint32_t h,
                         VkPipeline pipeline, VkPipelineLayout layout,
                         const void* pushData, uint32_t pushBytes, VkDescriptorSet descSet,
                         bool recordInputUpload);

    std::once_flag initOnce_;
    bool inited_ = false;
    bool ok_ = false;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    // ns per timestamp tick on the graphics queue family; 0 when it cannot
    // timestamp.  Kept separate from VulkanComputeUtilities::timestampPeriod,
    // which is validated against the (possibly different) compute family.
    float tsPeriod_ = 0.0f;

    std::mutex targetMutex_;
    std::vector<VulkanGraphicsTarget*> freeTargets_;

    // Built-in gradient test pipeline (lazy).
    VkPipelineLayout emptyLayout_ = VK_NULL_HANDLE;
    VkPipeline gradientPipeline_ = VK_NULL_HANDLE;

    // Native-Shader descriptor infrastructure (lazy).
    VkDescriptorSetLayout shaderSetLayout_ = VK_NULL_HANDLE;
    VkPipelineLayout shaderPipelineLayout_ = VK_NULL_HANDLE;
    VkSampler sampler_ = VK_NULL_HANDLE;
    // 1x1 opaque-black dummy sampled image bound at set0/binding1 for shaders
    // that declare no input sampler, so one descriptor layout serves all.
    VkImage dummyImage_ = VK_NULL_HANDLE;
    VmaAllocation dummyAlloc_ = VK_NULL_HANDLE;
    VkImageView dummyView_ = VK_NULL_HANDLE;
    std::once_flag shaderInfraOnce_;
    void ensureShaderInfra();
    void doInitShaderInfra();
    bool ensureDescriptor(VulkanGraphicsTarget& t);
};

#endif
