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

#include "VulkanComputeUtilities.h"

class RenderBuffer;

// A per-thread offscreen render target (image + framebuffer + readback), lazily
// sized to the largest buffer seen on that thread.  Leaked at exit like the rest
// of the backend.
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
    VkDescriptorPool descPool = VK_NULL_HANDLE;  // shader-effect UBO/sampler set
    VkDescriptorSet descSet = VK_NULL_HANDLE;
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

    // Render `pipeline` (built against shaderPipelineLayout) into buffer.pixels,
    // binding `ubo` at set0/binding0 and `inputView` (or the dummy) at
    // set0/binding1.  Returns false on failure.
    bool renderShader(RenderBuffer& buffer, VkPipeline pipeline, VkBuffer ubo, VkImageView inputView);

    // Upload `data` (byteSize bytes) into a per-thread sampled input image of the
    // given w/h/format, leaving it in SHADER_READ_ONLY_OPTIMAL, and return its
    // view for renderShader's inputView (RGBA8 for canvas/feedback shaders,
    // R32_SFLOAT 128x1 for audio).  VK_NULL_HANDLE on failure (caller uses dummy).
    VkImageView prepareInputImage(uint32_t w, uint32_t h, VkFormat format, const void* data, size_t byteSize);

    // Render `pipeline` (a fullscreen draw) into buffer.pixels.  Optional push
    // constants (fragment stage) and a bound descriptor set (set 0).  Handles the
    // RenderBuffer's bottom-row-first convention.  Returns false (caller falls
    // back) if the graphics path is unavailable or a Vulkan call fails.
    bool renderToBuffer(RenderBuffer& buffer, VkPipeline pipeline, VkPipelineLayout layout,
                        const void* pushData, uint32_t pushBytes,
                        VkDescriptorSet descSet);

    // Phase-1 self-test: render the built-in UV-gradient shader at a small size
    // and confirm the corners came out as expected.  Logs PASS/FAIL.  Run
    // automatically at init when XL_VULKAN_GFXTEST is set.
    bool selfTest();

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
    // Record+submit a fullscreen draw of `pipeline` into t.readback (w*h RGBA8,
    // top-row-first).  Blocks on the fence.  Shared by renderToBuffer/selfTest.
    bool renderCore(VulkanGraphicsTarget& t, uint32_t w, uint32_t h,
                    VkPipeline pipeline, VkPipelineLayout layout,
                    const void* pushData, uint32_t pushBytes, VkDescriptorSet descSet);

    std::once_flag initOnce_;
    bool inited_ = false;
    bool ok_ = false;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;

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
