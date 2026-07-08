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

// Shared pipeline/descriptor state for the Vulkan graphics backend — the
// analogue of the Metal delegate's getPipelineState() cache.  Vulkan bakes
// primitive topology and blend state into the pipeline, so the cache keys on
// (shader set, topology, blending, smooth-points) instead of Metal's
// (name, blending).  One instance is shared by every canvas; all state lives
// on the shared VulkanComputeUtilities device.

#ifdef HAVE_VULKAN

#include <map>
#include <mutex>
#include <vector>

#include <volk.h>

#ifndef VMA_STATIC_VULKAN_FUNCTIONS
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#endif
#ifndef VMA_DYNAMIC_VULKAN_FUNCTIONS
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif
#include <vk_mem_alloc.h>

class VulkanPipelineCache {
public:
    enum class ShaderSet : uint8_t {
        SingleColor = 0,   // single_color.vert + color.frag
        MultiColor,        // multi_color.vert + color.frag
        IndexedColor,      // indexed_color.vert + color.frag (set 1 color table)
        Texture,           // texture.vert + texture.frag (linear sampler)
        TextureNearest,    // texture.vert + texture_nearest.frag
        TextureColor,      // texture.vert + texture_color.frag (font/alpha-mask)
        MeshSolid,         // mesh.vert + mesh_solid.frag
        MeshTexture,       // mesh.vert + mesh_texture.frag
        MeshWireframe      // mesh_wireframe.vert + mesh_solid.frag
    };
    static bool isMeshSet(ShaderSet s) { return s == ShaderSet::MeshSolid || s == ShaderSet::MeshTexture || s == ShaderSet::MeshWireframe; }

    static VulkanPipelineCache INSTANCE;

    // Lazy init on the shared VulkanComputeUtilities device.  Returns false
    // when Vulkan graphics is unavailable (no device, no swapchain extension,
    // or pipeline construction failed) — callers fall back to OpenGL.
    bool ensureInit();
    bool isReady() const { return ready; }

    VkFormat getColorFormat() const { return colorFormat; }
    VkFormat getDepthFormat() const { return depthFormat; }
    VkSampleCountFlagBits getSampleCount() const { return sampleCount; }
    // Render pass all graphics pipelines are built against; canvases create
    // their framebuffers with a compatible pass (MSAA color + depth, resolving
    // to the swapchain image — attachments [msaaColor, depth, resolve]).
    VkRenderPass getRenderPass() const { return renderPass; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    VkPipelineLayout getMeshPipelineLayout() const { return meshPipelineLayout; }

    // smoothPoints only applies to VK_PRIMITIVE_TOPOLOGY_POINT_LIST (selects
    // the point-smooth fragment shader).  depthTest enables depth test+write
    // (used for 3D viewports and mesh draws).  Returns VK_NULL_HANDLE on failure.
    VkPipeline getPipeline(ShaderSet set, VkPrimitiveTopology topology, bool blending, bool smoothPoints = false, bool depthTest = false);

    // Descriptor set management.  Set 0: sampled texture (+ immutable linear/
    // nearest samplers).  Set 1: indexed-color table SSBO.
    VkDescriptorSet allocateTextureSet(VkImageView view);
    VkDescriptorSet allocateColorTableSet(VkBuffer buffer, VkDeviceSize size);
    void updateColorTableSet(VkDescriptorSet set, VkBuffer buffer, VkDeviceSize size);
    void freeDescriptorSet(VkDescriptorSet set);

    // Deferred destruction: accumulators/textures can be deleted while the
    // frame that references them is still recording or executing.  Resources
    // queue here and are released by the canvas after its frame fence signals.
    void deferDestroyBuffer(VkBuffer buffer, VmaAllocation allocation);
    void deferDestroyImage(VkImage image, VmaAllocation allocation, VkImageView view, VkDescriptorSet set);
    void flushDeferredDestroy();

private:
    bool doInit();
    VkShaderModule createModule(const uint32_t* words, size_t sizeBytes);
    VkDescriptorSet allocateSet(VkDescriptorSetLayout layout);

    bool ready = false;
    bool initAttempted = false;
    std::mutex initMutex;
    std::mutex pipelineMutex;
    std::mutex descriptorMutex;

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_4_BIT;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipelineLayout meshPipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout textureSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout colorTableSetLayout = VK_NULL_HANDLE;
    VkSampler linearSampler = VK_NULL_HANDLE;
    VkSampler nearestSampler = VK_NULL_HANDLE;

    // Shader modules (index by ShaderModuleId in the .cpp)
    std::vector<VkShaderModule> modules;

    std::map<uint32_t, VkPipeline> pipelines;

    std::vector<VkDescriptorPool> descriptorPools;
    std::map<VkDescriptorSet, VkDescriptorPool> setPools;

    struct DeferredBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
    };
    struct DeferredImage {
        VkImage image;
        VmaAllocation allocation;
        VkImageView view;
        VkDescriptorSet set;
    };
    std::mutex deferredMutex;
    std::vector<DeferredBuffer> deferredBuffers;
    std::vector<DeferredImage> deferredImages;
};

#endif
