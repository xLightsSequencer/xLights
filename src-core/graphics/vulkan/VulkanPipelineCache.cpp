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

#include "VulkanPipelineCache.h"

#include <spdlog/spdlog.h>

#include "VulkanFrameData.h"
#include "../../effects/vulkan/VulkanComputeUtilities.h"

#include "shaders/compiled/single_color.vert.spv.h"
#include "shaders/compiled/multi_color.vert.spv.h"
#include "shaders/compiled/indexed_color.vert.spv.h"
#include "shaders/compiled/texture.vert.spv.h"
#include "shaders/compiled/color.frag.spv.h"
#include "shaders/compiled/point_smooth.frag.spv.h"
#include "shaders/compiled/texture.frag.spv.h"
#include "shaders/compiled/texture_nearest.frag.spv.h"
#include "shaders/compiled/texture_color.frag.spv.h"
#include "shaders/compiled/mesh.vert.spv.h"
#include "shaders/compiled/mesh_wireframe.vert.spv.h"
#include "shaders/compiled/mesh_solid.frag.spv.h"
#include "shaders/compiled/mesh_texture.frag.spv.h"

#include "VulkanFrameData.h"
#include "VulkanMeshFrameData.h"

VulkanPipelineCache VulkanPipelineCache::INSTANCE;

enum ShaderModuleId : size_t {
    MOD_SINGLE_COLOR_VERT = 0,
    MOD_MULTI_COLOR_VERT,
    MOD_INDEXED_COLOR_VERT,
    MOD_TEXTURE_VERT,
    MOD_COLOR_FRAG,
    MOD_POINT_SMOOTH_FRAG,
    MOD_TEXTURE_FRAG,
    MOD_TEXTURE_NEAREST_FRAG,
    MOD_TEXTURE_COLOR_FRAG,
    MOD_MESH_VERT,
    MOD_MESH_WIREFRAME_VERT,
    MOD_MESH_SOLID_FRAG,
    MOD_MESH_TEXTURE_FRAG,
    MOD_COUNT
};

bool VulkanPipelineCache::ensureInit() {
    std::lock_guard<std::mutex> lock(initMutex);
    if (!initAttempted) {
        initAttempted = true;
        VulkanComputeUtilities::INSTANCE.ensureInit();
        VulkanComputeUtilities& vk = VulkanComputeUtilities::INSTANCE;
        if (vk.device == VK_NULL_HANDLE || vk.graphicsQueue == VK_NULL_HANDLE || !vk.swapchainExtensionEnabled) {
            spdlog::info("Vulkan graphics unavailable (device/graphics queue/swapchain missing), staying on OpenGL");
            return false;
        }
        ready = doInit();
        if (!ready) {
            spdlog::warn("Vulkan graphics pipeline setup failed, staying on OpenGL");
        }
    }
    return ready;
}

VkShaderModule VulkanPipelineCache::createModule(const uint32_t* words, size_t sizeBytes) {
    VkShaderModuleCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = sizeBytes;
    ci.pCode = words;
    VkShaderModule mod = VK_NULL_HANDLE;
    if (vkCreateShaderModule(VulkanComputeUtilities::INSTANCE.device, &ci, nullptr, &mod) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return mod;
}

bool VulkanPipelineCache::doInit() {
    VkDevice device = VulkanComputeUtilities::INSTANCE.device;

    // Clamp the MSAA sample count to what the device supports for both color
    // and depth (fall back to 1x = no MSAA).
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(VulkanComputeUtilities::INSTANCE.physicalDevice, &props);
        VkSampleCountFlags supported = props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;
        for (VkSampleCountFlagBits want : { VK_SAMPLE_COUNT_4_BIT, VK_SAMPLE_COUNT_2_BIT }) {
            if (sampleCount == want && !(supported & want)) {
                sampleCount = VK_SAMPLE_COUNT_1_BIT;
            }
        }
        if (!(supported & sampleCount)) {
            sampleCount = VK_SAMPLE_COUNT_1_BIT;
        }
    }
    const bool msaa = sampleCount != VK_SAMPLE_COUNT_1_BIT;

    // Render pass: MSAA color + depth, resolving to the swapchain image.
    // Attachment layout: [0]=color(MSAA), [1]=depth(MSAA), [2]=resolve target.
    // Without MSAA, [0] is the swapchain image directly and there is no resolve.
    std::vector<VkAttachmentDescription> attachments;
    VkAttachmentDescription color = {};
    color.format = colorFormat;
    color.samples = sampleCount;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = msaa ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = msaa ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments.push_back(color);

    VkAttachmentDescription depth = {};
    depth.format = depthFormat;
    depth.samples = sampleCount;
    depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments.push_back(depth);

    if (msaa) {
        VkAttachmentDescription resolve = {};
        resolve.format = colorFormat;
        resolve.samples = VK_SAMPLE_COUNT_1_BIT;
        resolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        resolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        resolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        resolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        resolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachments.push_back(resolve);
    }

    VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkAttachmentReference depthRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    VkAttachmentReference resolveRef = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;
    if (msaa) {
        subpass.pResolveAttachments = &resolveRef;
    }

    VkSubpassDependency dep = {};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.srcAccessMask = 0;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rpci = {};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = (uint32_t)attachments.size();
    rpci.pAttachments = attachments.data();
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;
    if (vkCreateRenderPass(device, &rpci, nullptr, &renderPass) != VK_SUCCESS) {
        return false;
    }

    // Samplers (immutable in the texture set layout, mirroring the constexpr
    // samplers in MetalShaders.metal).
    VkSamplerCreateInfo sci = {};
    sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.magFilter = VK_FILTER_LINEAR;
    sci.minFilter = VK_FILTER_LINEAR;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sci.maxLod = VK_LOD_CLAMP_NONE;
    if (vkCreateSampler(device, &sci, nullptr, &linearSampler) != VK_SUCCESS) {
        return false;
    }
    sci.magFilter = VK_FILTER_NEAREST; // Metal nearest: mag nearest, min linear
    if (vkCreateSampler(device, &sci, nullptr, &nearestSampler) != VK_SUCCESS) {
        return false;
    }

    // Set 0: sampled texture + immutable samplers.
    {
        VkDescriptorSetLayoutBinding bindings[3] = {};
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[1].pImmutableSamplers = &linearSampler;
        bindings[2].binding = 2;
        bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        bindings[2].descriptorCount = 1;
        bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[2].pImmutableSamplers = &nearestSampler;

        VkDescriptorSetLayoutCreateInfo lci = {};
        lci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        lci.bindingCount = 3;
        lci.pBindings = bindings;
        if (vkCreateDescriptorSetLayout(device, &lci, nullptr, &textureSetLayout) != VK_SUCCESS) {
            return false;
        }
    }

    // Set 1: indexed-color table SSBO.
    {
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo lci = {};
        lci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        lci.bindingCount = 1;
        lci.pBindings = &binding;
        if (vkCreateDescriptorSetLayout(device, &lci, nullptr, &colorTableSetLayout) != VK_SUCCESS) {
            return false;
        }
    }

    // Pipeline layout shared by all graphics pipelines: both sets + the
    // VulkanFrameData push constants (each shader statically uses only the
    // sets it needs, so unused sets never have to be bound).
    {
        VkDescriptorSetLayout layouts[2] = { textureSetLayout, colorTableSetLayout };
        VkPushConstantRange range = {};
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        range.offset = 0;
        range.size = sizeof(VulkanFrameData);

        VkPipelineLayoutCreateInfo pci = {};
        pci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pci.setLayoutCount = 2;
        pci.pSetLayouts = layouts;
        pci.pushConstantRangeCount = 1;
        pci.pPushConstantRanges = &range;
        if (vkCreatePipelineLayout(device, &pci, nullptr, &pipelineLayout) != VK_SUCCESS) {
            return false;
        }
    }

    // Mesh pipeline layout: just the texture set + the larger mesh push
    // constants (needs model/view matrices, so it can't share the 2D layout).
    {
        VkPushConstantRange range = {};
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        range.offset = 0;
        range.size = sizeof(VulkanMeshFrameData);

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(VulkanComputeUtilities::INSTANCE.physicalDevice, &props);
        if (props.limits.maxPushConstantsSize < range.size) {
            // Every desktop GPU exposes >= 256 bytes; if not, mesh draws stay
            // no-ops (models still show via the 2D node preview).
            spdlog::info("Vulkan graphics: maxPushConstantsSize {} < mesh {}, mesh rendering disabled",
                         props.limits.maxPushConstantsSize, (uint32_t)range.size);
            meshPipelineLayout = VK_NULL_HANDLE;
        } else {
            VkPipelineLayoutCreateInfo pci = {};
            pci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pci.setLayoutCount = 1;
            pci.pSetLayouts = &textureSetLayout;
            pci.pushConstantRangeCount = 1;
            pci.pPushConstantRanges = &range;
            if (vkCreatePipelineLayout(device, &pci, nullptr, &meshPipelineLayout) != VK_SUCCESS) {
                return false;
            }
        }
    }

    modules.resize(MOD_COUNT, VK_NULL_HANDLE);
    modules[MOD_MESH_VERT] = createModule(mesh_vert_spv, sizeof(mesh_vert_spv));
    modules[MOD_MESH_WIREFRAME_VERT] = createModule(mesh_wireframe_vert_spv, sizeof(mesh_wireframe_vert_spv));
    modules[MOD_MESH_SOLID_FRAG] = createModule(mesh_solid_frag_spv, sizeof(mesh_solid_frag_spv));
    modules[MOD_MESH_TEXTURE_FRAG] = createModule(mesh_texture_frag_spv, sizeof(mesh_texture_frag_spv));
    modules[MOD_SINGLE_COLOR_VERT] = createModule(single_color_vert_spv, sizeof(single_color_vert_spv));
    modules[MOD_MULTI_COLOR_VERT] = createModule(multi_color_vert_spv, sizeof(multi_color_vert_spv));
    modules[MOD_INDEXED_COLOR_VERT] = createModule(indexed_color_vert_spv, sizeof(indexed_color_vert_spv));
    modules[MOD_TEXTURE_VERT] = createModule(texture_vert_spv, sizeof(texture_vert_spv));
    modules[MOD_COLOR_FRAG] = createModule(color_frag_spv, sizeof(color_frag_spv));
    modules[MOD_POINT_SMOOTH_FRAG] = createModule(point_smooth_frag_spv, sizeof(point_smooth_frag_spv));
    modules[MOD_TEXTURE_FRAG] = createModule(texture_frag_spv, sizeof(texture_frag_spv));
    modules[MOD_TEXTURE_NEAREST_FRAG] = createModule(texture_nearest_frag_spv, sizeof(texture_nearest_frag_spv));
    modules[MOD_TEXTURE_COLOR_FRAG] = createModule(texture_color_frag_spv, sizeof(texture_color_frag_spv));
    for (VkShaderModule m : modules) {
        if (m == VK_NULL_HANDLE) {
            return false;
        }
    }
    return true;
}

VkPipeline VulkanPipelineCache::getPipeline(ShaderSet set, VkPrimitiveTopology topology, bool blending, bool smoothPoints, bool depthTest) {
    if (!ready) {
        return VK_NULL_HANDLE;
    }
    const bool mesh = isMeshSet(set);
    if (mesh) {
        if (meshPipelineLayout == VK_NULL_HANDLE) {
            return VK_NULL_HANDLE;
        }
        depthTest = true; // mesh always depth-tests
    }
    if (topology != VK_PRIMITIVE_TOPOLOGY_POINT_LIST) {
        smoothPoints = false;
    }
    uint32_t key = ((uint32_t)set << 9) | ((uint32_t)topology << 3) | (blending ? 4 : 0) | (smoothPoints ? 2 : 0) | (depthTest ? 1 : 0);

    std::lock_guard<std::mutex> lock(pipelineMutex);
    auto it = pipelines.find(key);
    if (it != pipelines.end()) {
        return it->second;
    }

    size_t vertMod;
    size_t fragMod;
    switch (set) {
    case ShaderSet::SingleColor:
        vertMod = MOD_SINGLE_COLOR_VERT;
        fragMod = smoothPoints ? MOD_POINT_SMOOTH_FRAG : MOD_COLOR_FRAG;
        break;
    case ShaderSet::MultiColor:
        vertMod = MOD_MULTI_COLOR_VERT;
        fragMod = smoothPoints ? MOD_POINT_SMOOTH_FRAG : MOD_COLOR_FRAG;
        break;
    case ShaderSet::IndexedColor:
        vertMod = MOD_INDEXED_COLOR_VERT;
        fragMod = smoothPoints ? MOD_POINT_SMOOTH_FRAG : MOD_COLOR_FRAG;
        break;
    case ShaderSet::Texture:
        vertMod = MOD_TEXTURE_VERT;
        fragMod = MOD_TEXTURE_FRAG;
        break;
    case ShaderSet::TextureNearest:
        vertMod = MOD_TEXTURE_VERT;
        fragMod = MOD_TEXTURE_NEAREST_FRAG;
        break;
    case ShaderSet::TextureColor:
        vertMod = MOD_TEXTURE_VERT;
        fragMod = MOD_TEXTURE_COLOR_FRAG;
        break;
    case ShaderSet::MeshSolid:
        vertMod = MOD_MESH_VERT;
        fragMod = MOD_MESH_SOLID_FRAG;
        break;
    case ShaderSet::MeshTexture:
        vertMod = MOD_MESH_VERT;
        fragMod = MOD_MESH_TEXTURE_FRAG;
        break;
    case ShaderSet::MeshWireframe:
        vertMod = MOD_MESH_WIREFRAME_VERT;
        fragMod = MOD_MESH_SOLID_FRAG;
        break;
    default:
        return VK_NULL_HANDLE;
    }

    VkPipelineShaderStageCreateInfo stages[2] = {};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = modules[vertMod];
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = modules[fragMod];
    stages[1].pName = "main";

    // Vertex input per shader set.  Slot 0 always carries positions; slot 1
    // carries the per-vertex aux stream (colors or texcoords).
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attrs;
    switch (set) {
    case ShaderSet::SingleColor:
        bindings.push_back({ 0, 12, VK_VERTEX_INPUT_RATE_VERTEX });
        attrs.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });
        break;
    case ShaderSet::MultiColor:
        bindings.push_back({ 0, 12, VK_VERTEX_INPUT_RATE_VERTEX });
        bindings.push_back({ 1, 4, VK_VERTEX_INPUT_RATE_VERTEX });
        attrs.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });
        attrs.push_back({ 1, 1, VK_FORMAT_R8G8B8A8_UNORM, 0 });
        break;
    case ShaderSet::IndexedColor:
        // Interleaved {float x,y,z; uint32 colorIdx} — matches the Metal
        // indexed accumulator's vertex layout.
        bindings.push_back({ 0, 16, VK_VERTEX_INPUT_RATE_VERTEX });
        attrs.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });
        attrs.push_back({ 1, 0, VK_FORMAT_R32_UINT, 12 });
        break;
    case ShaderSet::Texture:
    case ShaderSet::TextureNearest:
    case ShaderSet::TextureColor:
        bindings.push_back({ 0, 12, VK_VERTEX_INPUT_RATE_VERTEX });
        bindings.push_back({ 1, 8, VK_VERTEX_INPUT_RATE_VERTEX });
        attrs.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });
        attrs.push_back({ 2, 1, VK_FORMAT_R32G32_SFLOAT, 0 });
        break;
    case ShaderSet::MeshSolid:
    case ShaderSet::MeshTexture:
    case ShaderSet::MeshWireframe:
        // Interleaved {vec3 pos; vec3 normal; vec2 texcoord} = 32 bytes.
        bindings.push_back({ 0, 32, VK_VERTEX_INPUT_RATE_VERTEX });
        attrs.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });
        attrs.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, 12 });
        attrs.push_back({ 2, 0, VK_FORMAT_R32G32_SFLOAT, 24 });
        break;
    }

    VkPipelineVertexInputStateCreateInfo vin = {};
    vin.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vin.vertexBindingDescriptionCount = (uint32_t)bindings.size();
    vin.pVertexBindingDescriptions = bindings.data();
    vin.vertexAttributeDescriptionCount = (uint32_t)attrs.size();
    vin.pVertexAttributeDescriptions = attrs.data();

    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = topology;

    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rs = {};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = sampleCount;

    VkPipelineDepthStencilStateCreateInfo ds = {};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = depthTest ? VK_TRUE : VK_FALSE;
    ds.depthWriteEnable = depthTest ? VK_TRUE : VK_FALSE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkPipelineColorBlendAttachmentState blend = {};
    blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (blending) {
        blend.blendEnable = VK_TRUE;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    VkPipelineColorBlendStateCreateInfo cb = {};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &blend;

    VkDynamicState dynStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn = {};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates = dynStates;

    VkGraphicsPipelineCreateInfo pci = {};
    pci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pci.stageCount = 2;
    pci.pStages = stages;
    pci.pVertexInputState = &vin;
    pci.pInputAssemblyState = &ia;
    pci.pViewportState = &vp;
    pci.pRasterizationState = &rs;
    pci.pMultisampleState = &ms;
    pci.pDepthStencilState = &ds;
    pci.pColorBlendState = &cb;
    pci.pDynamicState = &dyn;
    pci.layout = mesh ? meshPipelineLayout : pipelineLayout;
    pci.renderPass = renderPass;
    pci.subpass = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult res = vkCreateGraphicsPipelines(VulkanComputeUtilities::INSTANCE.device, VK_NULL_HANDLE, 1, &pci, nullptr, &pipeline);
    if (res != VK_SUCCESS) {
        spdlog::error("Vulkan graphics: pipeline creation failed ({}) for set {}", (int)res, (int)set);
        return VK_NULL_HANDLE;
    }
    pipelines[key] = pipeline;
    return pipeline;
}

VkDescriptorSet VulkanPipelineCache::allocateSet(VkDescriptorSetLayout layout) {
    VkDevice device = VulkanComputeUtilities::INSTANCE.device;

    auto tryAlloc = [&](VkDescriptorPool pool) -> VkDescriptorSet {
        VkDescriptorSetAllocateInfo ai = {};
        ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        ai.descriptorPool = pool;
        ai.descriptorSetCount = 1;
        ai.pSetLayouts = &layout;
        VkDescriptorSet set = VK_NULL_HANDLE;
        if (vkAllocateDescriptorSets(device, &ai, &set) == VK_SUCCESS) {
            setPools[set] = pool;
        }
        return set;
    };

    if (!descriptorPools.empty()) {
        VkDescriptorSet set = tryAlloc(descriptorPools.back());
        if (set != VK_NULL_HANDLE) {
            return set;
        }
    }

    VkDescriptorPoolSize sizes[3] = {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 256 },
        { VK_DESCRIPTOR_TYPE_SAMPLER, 512 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 256 }
    };
    VkDescriptorPoolCreateInfo pci = {};
    pci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pci.maxSets = 512;
    pci.poolSizeCount = 3;
    pci.pPoolSizes = sizes;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    if (vkCreateDescriptorPool(device, &pci, nullptr, &pool) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    descriptorPools.push_back(pool);
    return tryAlloc(pool);
}

VkDescriptorSet VulkanPipelineCache::allocateTextureSet(VkImageView view) {
    std::lock_guard<std::mutex> lock(descriptorMutex);
    VkDescriptorSet set = allocateSet(textureSetLayout);
    if (set == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    VkDescriptorImageInfo info = {};
    info.imageView = view;
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    write.pImageInfo = &info;
    vkUpdateDescriptorSets(VulkanComputeUtilities::INSTANCE.device, 1, &write, 0, nullptr);
    return set;
}

VkDescriptorSet VulkanPipelineCache::allocateColorTableSet(VkBuffer buffer, VkDeviceSize size) {
    std::lock_guard<std::mutex> lock(descriptorMutex);
    VkDescriptorSet set = allocateSet(colorTableSetLayout);
    if (set == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    updateColorTableSet(set, buffer, size);
    return set;
}

void VulkanPipelineCache::updateColorTableSet(VkDescriptorSet set, VkBuffer buffer, VkDeviceSize size) {
    VkDescriptorBufferInfo info = {};
    info.buffer = buffer;
    info.offset = 0;
    info.range = size;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.pBufferInfo = &info;
    vkUpdateDescriptorSets(VulkanComputeUtilities::INSTANCE.device, 1, &write, 0, nullptr);
}

void VulkanPipelineCache::freeDescriptorSet(VkDescriptorSet set) {
    if (set == VK_NULL_HANDLE) {
        return;
    }
    std::lock_guard<std::mutex> lock(descriptorMutex);
    auto it = setPools.find(set);
    if (it != setPools.end()) {
        vkFreeDescriptorSets(VulkanComputeUtilities::INSTANCE.device, it->second, 1, &set);
        setPools.erase(it);
    }
}

void VulkanPipelineCache::deferDestroyBuffer(VkBuffer buffer, VmaAllocation allocation) {
    if (buffer == VK_NULL_HANDLE) {
        return;
    }
    std::lock_guard<std::mutex> lock(deferredMutex);
    deferredBuffers.push_back({ buffer, allocation });
}

void VulkanPipelineCache::deferDestroyImage(VkImage image, VmaAllocation allocation, VkImageView view, VkDescriptorSet set) {
    if (image == VK_NULL_HANDLE && view == VK_NULL_HANDLE && set == VK_NULL_HANDLE) {
        return;
    }
    std::lock_guard<std::mutex> lock(deferredMutex);
    deferredImages.push_back({ image, allocation, view, set });
}

void VulkanPipelineCache::flushDeferredDestroy() {
    std::vector<DeferredBuffer> buffers;
    std::vector<DeferredImage> images;
    {
        std::lock_guard<std::mutex> lock(deferredMutex);
        buffers.swap(deferredBuffers);
        images.swap(deferredImages);
    }
    VmaAllocator allocator = VulkanComputeUtilities::INSTANCE.allocator;
    VkDevice device = VulkanComputeUtilities::INSTANCE.device;
    for (auto& b : buffers) {
        vmaDestroyBuffer(allocator, b.buffer, b.allocation);
    }
    for (auto& i : images) {
        freeDescriptorSet(i.set);
        if (i.view != VK_NULL_HANDLE) {
            vkDestroyImageView(device, i.view, nullptr);
        }
        if (i.image != VK_NULL_HANDLE) {
            vmaDestroyImage(allocator, i.image, i.allocation);
        }
    }
}

#endif
