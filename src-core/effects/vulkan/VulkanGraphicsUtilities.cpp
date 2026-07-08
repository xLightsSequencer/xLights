/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef HAVE_VULKAN_SHADER

#include "VulkanGraphicsUtilities.h"

#include <mutex>
#include <cstring>
#include <spdlog/spdlog.h>

#include "../../render/RenderBuffer.h"
#include "../../utils/Color.h"

#include "VulkanShaderTranslate.h"

#include "shaders/compiled/fullscreen.vert.spv.h"
#include "shaders/compiled/gradient_test.frag.spv.h"

VulkanGraphicsUtilities VulkanGraphicsUtilities::INSTANCE;

static constexpr VkFormat kTargetFormat = VK_FORMAT_R8G8B8A8_UNORM; // matches xlColor r,g,b,a

bool VulkanGraphicsUtilities::available() {
    ensureInit();
    return ok_;
}

void VulkanGraphicsUtilities::ensureInit() {
    // Guarantee the compute device is up first (one-directional: graphics
    // depends on compute, never the reverse), then build the graphics pipeline.
    VulkanComputeUtilities::INSTANCE.ensureInit();
    initInline();
}

void VulkanGraphicsUtilities::initInline() {
    std::call_once(initOnce_, [this]() { doInit(); });
}

void VulkanGraphicsUtilities::doInit() {
    inited_ = true;
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    // NOTE: do NOT call u.ensureInit() here — initInline() can run from inside
    // compute's own doInit (call_once mid-flight); the device/enabled state is
    // already set by that point.
    if (!u.enabled || u.graphicsQueue == VK_NULL_HANDLE || u.device == VK_NULL_HANDLE) {
        spdlog::info("Vulkan graphics pipeline unavailable (no graphics-capable queue); Shader effect stays on CPU/GL");
        return;
    }

    // Render pass: one RGBA8 color attachment, cleared, left in TRANSFER_SRC so
    // it can be copied straight to the readback buffer.
    VkAttachmentDescription color = {};
    color.format = kTargetFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;

    // External->subpass (acquire) and subpass->external (release to transfer).
    VkSubpassDependency deps[2] = {};
    deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    deps[0].dstSubpass = 0;
    deps[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[0].srcAccessMask = 0;
    deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[1].srcSubpass = 0;
    deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[1].dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    VkRenderPassCreateInfo rpci = {};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 1;
    rpci.pAttachments = &color;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &subpass;
    rpci.dependencyCount = 2;
    rpci.pDependencies = deps;
    if (vkCreateRenderPass(u.device, &rpci, nullptr, &renderPass_) != VK_SUCCESS) {
        spdlog::error("Vulkan graphics: vkCreateRenderPass failed");
        return;
    }

    emptyLayout_ = createPipelineLayout(0, VK_NULL_HANDLE);
    gradientPipeline_ = buildPipeline(fullscreen_vert_spv, sizeof(fullscreen_vert_spv),
                                      gradient_test_frag_spv, sizeof(gradient_test_frag_spv),
                                      emptyLayout_);
    ok_ = (emptyLayout_ != VK_NULL_HANDLE && gradientPipeline_ != VK_NULL_HANDLE);
    if (ok_) {
        spdlog::info("Vulkan graphics pipeline ready (graphics queue family {})", u.graphicsQueueFamilyIndex);
        if (getenv("XL_VULKAN_GFXTEST") != nullptr) {
            selfTest();
            runtimeShaderTest();
        }
    }
}

VkShaderModule VulkanGraphicsUtilities::shaderModule(const uint32_t* words, size_t bytes) {
    VkShaderModuleCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = bytes;
    ci.pCode = words;
    VkShaderModule m = VK_NULL_HANDLE;
    if (vkCreateShaderModule(VulkanComputeUtilities::INSTANCE.device, &ci, nullptr, &m) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return m;
}

VkPipelineLayout VulkanGraphicsUtilities::createPipelineLayout(uint32_t pushConstantBytes, VkDescriptorSetLayout setLayout) {
    VkPushConstantRange pcr = {};
    pcr.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pcr.offset = 0;
    pcr.size = pushConstantBytes;

    VkPipelineLayoutCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    ci.setLayoutCount = (setLayout != VK_NULL_HANDLE) ? 1 : 0;
    ci.pSetLayouts = (setLayout != VK_NULL_HANDLE) ? &setLayout : nullptr;
    ci.pushConstantRangeCount = (pushConstantBytes > 0) ? 1 : 0;
    ci.pPushConstantRanges = (pushConstantBytes > 0) ? &pcr : nullptr;

    VkPipelineLayout layout = VK_NULL_HANDLE;
    if (vkCreatePipelineLayout(VulkanComputeUtilities::INSTANCE.device, &ci, nullptr, &layout) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return layout;
}

VkPipeline VulkanGraphicsUtilities::buildPipeline(const uint32_t* vert, size_t vertBytes,
                                                  const uint32_t* frag, size_t fragBytes,
                                                  VkPipelineLayout layout) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    if (renderPass_ == VK_NULL_HANDLE || layout == VK_NULL_HANDLE) {
        return VK_NULL_HANDLE;
    }
    VkShaderModule vs = shaderModule(vert, vertBytes);
    VkShaderModule fs = shaderModule(frag, fragBytes);
    if (vs == VK_NULL_HANDLE || fs == VK_NULL_HANDLE) {
        if (vs) vkDestroyShaderModule(u.device, vs, nullptr);
        if (fs) vkDestroyShaderModule(u.device, fs, nullptr);
        return VK_NULL_HANDLE;
    }

    VkPipelineShaderStageCreateInfo stages[2] = {};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vs;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fs;
    stages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vi = {};  // attribute-less
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vp = {};  // dynamic — size set at record time
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
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState cba = {};
    cba.blendEnable = VK_FALSE;
    cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo cb = {};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &cba;

    VkDynamicState dyn[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo ds = {};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    ds.dynamicStateCount = 2;
    ds.pDynamicStates = dyn;

    VkGraphicsPipelineCreateInfo pci = {};
    pci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pci.stageCount = 2;
    pci.pStages = stages;
    pci.pVertexInputState = &vi;
    pci.pInputAssemblyState = &ia;
    pci.pViewportState = &vp;
    pci.pRasterizationState = &rs;
    pci.pMultisampleState = &ms;
    pci.pColorBlendState = &cb;
    pci.pDynamicState = &ds;
    pci.layout = layout;
    pci.renderPass = renderPass_;
    pci.subpass = 0;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult r = vkCreateGraphicsPipelines(u.device, VK_NULL_HANDLE, 1, &pci, nullptr, &pipeline);
    vkDestroyShaderModule(u.device, vs, nullptr);
    vkDestroyShaderModule(u.device, fs, nullptr);
    if (r != VK_SUCCESS) {
        spdlog::error("Vulkan graphics: vkCreateGraphicsPipelines failed ({})", (int)r);
        return VK_NULL_HANDLE;
    }
    return pipeline;
}

void VulkanGraphicsUtilities::destroyTarget(VulkanGraphicsTarget& t) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    if (t.framebuffer) vkDestroyFramebuffer(u.device, t.framebuffer, nullptr);
    if (t.view) vkDestroyImageView(u.device, t.view, nullptr);
    if (t.image) vmaDestroyImage(u.allocator, t.image, t.imageAlloc);
    if (t.readback) u.destroyBuffer(t.readback);
    // pool/cb/fence are reused across sizes; keep them.
    t.framebuffer = VK_NULL_HANDLE;
    t.view = VK_NULL_HANDLE;
    t.image = VK_NULL_HANDLE;
    t.imageAlloc = VK_NULL_HANDLE;
    t.width = t.height = 0;
}

bool VulkanGraphicsUtilities::ensureTarget(VulkanGraphicsTarget& t, uint32_t w, uint32_t h) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    if (t.pool == VK_NULL_HANDLE) {
        VkCommandPoolCreateInfo pci = {};
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pci.queueFamilyIndex = u.graphicsQueueFamilyIndex;
        if (vkCreateCommandPool(u.device, &pci, nullptr, &t.pool) != VK_SUCCESS) return false;
        VkCommandBufferAllocateInfo ai = {};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool = t.pool;
        ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = 1;
        if (vkAllocateCommandBuffers(u.device, &ai, &t.cb) != VK_SUCCESS) return false;
        VkFenceCreateInfo fci = {};
        fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        if (vkCreateFence(u.device, &fci, nullptr, &t.fence) != VK_SUCCESS) return false;
    }
    if (t.image != VK_NULL_HANDLE && t.width == w && t.height == h) {
        return true;
    }
    destroyTarget(t);

    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = kTargetFormat;
    ici.extent = { w, h, 1 };
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo aci = {};
    aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    if (vmaCreateImage(u.allocator, &ici, &aci, &t.image, &t.imageAlloc, nullptr) != VK_SUCCESS) {
        t.image = VK_NULL_HANDLE;
        return false;
    }

    VkImageViewCreateInfo vci = {};
    vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vci.image = t.image;
    vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vci.format = kTargetFormat;
    vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vci.subresourceRange.levelCount = 1;
    vci.subresourceRange.layerCount = 1;
    if (vkCreateImageView(u.device, &vci, nullptr, &t.view) != VK_SUCCESS) {
        destroyTarget(t);
        return false;
    }

    VkFramebufferCreateInfo fci = {};
    fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fci.renderPass = renderPass_;
    fci.attachmentCount = 1;
    fci.pAttachments = &t.view;
    fci.width = w;
    fci.height = h;
    fci.layers = 1;
    if (vkCreateFramebuffer(u.device, &fci, nullptr, &t.framebuffer) != VK_SUCCESS) {
        destroyTarget(t);
        return false;
    }

    if (!u.createSharedBuffer(t.readback, (size_t)w * h * 4, "ShaderReadback")) {
        destroyTarget(t);
        return false;
    }
    t.width = w;
    t.height = h;
    return true;
}

bool VulkanGraphicsUtilities::renderCore(VulkanGraphicsTarget& target, uint32_t w, uint32_t h,
                                         VkPipeline pipeline, VkPipelineLayout layout,
                                         const void* pushData, uint32_t pushBytes, VkDescriptorSet descSet) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    vkResetCommandPool(u.device, target.pool, 0);
    VkCommandBufferBeginInfo bi = {};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(target.cb, &bi);

    VkClearValue clear = {};
    clear.color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    VkRenderPassBeginInfo rpbi = {};
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.renderPass = renderPass_;
    rpbi.framebuffer = target.framebuffer;
    rpbi.renderArea.extent = { w, h };
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = &clear;
    vkCmdBeginRenderPass(target.cb, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(target.cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    VkViewport viewport = { 0.0f, 0.0f, (float)w, (float)h, 0.0f, 1.0f };
    vkCmdSetViewport(target.cb, 0, 1, &viewport);
    VkRect2D scissor = { { 0, 0 }, { w, h } };
    vkCmdSetScissor(target.cb, 0, 1, &scissor);
    if (descSet != VK_NULL_HANDLE) {
        vkCmdBindDescriptorSets(target.cb, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descSet, 0, nullptr);
    }
    if (pushData != nullptr && pushBytes > 0) {
        vkCmdPushConstants(target.cb, layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, pushBytes, pushData);
    }
    vkCmdDraw(target.cb, 3, 1, 0, 0);
    vkCmdEndRenderPass(target.cb);

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = { w, h, 1 };
    vkCmdCopyImageToBuffer(target.cb, target.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           target.readback.buffer, 1, &region);

    VkBufferMemoryBarrier bmb = {};
    bmb.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bmb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    bmb.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    bmb.buffer = target.readback.buffer;
    bmb.size = VK_WHOLE_SIZE;
    vkCmdPipelineBarrier(target.cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                         0, 0, nullptr, 1, &bmb, 0, nullptr);

    vkEndCommandBuffer(target.cb);

    VkSubmitInfo si = {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &target.cb;
    {
        std::mutex& m = u.graphicsSubmitMutex();
        std::lock_guard<std::mutex> lock(m);
        if (vkQueueSubmit(u.graphicsQueue, 1, &si, target.fence) != VK_SUCCESS) return false;
    }
    vkWaitForFences(u.device, 1, &target.fence, VK_TRUE, UINT64_MAX);
    vkResetFences(u.device, 1, &target.fence);
    return true;
}

bool VulkanGraphicsUtilities::renderToBuffer(RenderBuffer& buffer, VkPipeline pipeline, VkPipelineLayout layout,
                                             const void* pushData, uint32_t pushBytes, VkDescriptorSet descSet) {
    ensureInit();
    if (!ok_ || pipeline == VK_NULL_HANDLE) return false;
    const uint32_t w = (uint32_t)buffer.BufferWi;
    const uint32_t h = (uint32_t)buffer.BufferHt;
    uint8_t* dst = (uint8_t*)buffer.GetPixels();  // xlColor is r,g,b,a (4 bytes)
    if (w == 0 || h == 0 || dst == nullptr) return false;

    static thread_local VulkanGraphicsTarget target;
    if (!ensureTarget(target, w, h)) return false;
    if (!renderCore(target, w, h, pipeline, layout, pushData, pushBytes, descSet)) return false;

    // Vulkan framebuffer origin is top-left; RenderBuffer is bottom-row-first
    // (GL convention), so flip rows on the way out.
    const uint8_t* src = (const uint8_t*)target.readback.mapped;
    for (uint32_t y = 0; y < h; y++) {
        std::memcpy(dst + (size_t)(h - 1 - y) * w * 4, src + (size_t)y * w * 4, (size_t)w * 4);
    }
    return true;
}

void VulkanGraphicsUtilities::ensureShaderInfra() {
    if (shaderPipelineLayout_ != VK_NULL_HANDLE) return;
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;

    VkDescriptorSetLayoutBinding b[2] = {};
    b[0].binding = 0; b[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // The generated vertex stage also reads the UBO (RENDERSIZE/XL_OFFSET/XL_ZOOM).
    b[0].descriptorCount = 1; b[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    b[1].binding = 1; b[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    b[1].descriptorCount = 1; b[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkDescriptorSetLayoutCreateInfo slci = {};
    slci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    slci.bindingCount = 2;
    slci.pBindings = b;
    if (vkCreateDescriptorSetLayout(u.device, &slci, nullptr, &shaderSetLayout_) != VK_SUCCESS) return;
    shaderPipelineLayout_ = createPipelineLayout(0, shaderSetLayout_);

    VkSamplerCreateInfo sci = {};
    sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.magFilter = VK_FILTER_LINEAR;
    sci.minFilter = VK_FILTER_LINEAR;
    sci.addressModeU = sci.addressModeV = sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sci.maxLod = VK_LOD_CLAMP_NONE;
    vkCreateSampler(u.device, &sci, nullptr, &sampler_);

    // 1x1 dummy sampled image (contents irrelevant — only bound when a shader
    // declares no input sampler; just needs a valid SHADER_READ_ONLY layout).
    VkImageCreateInfo ici = {};
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = kTargetFormat;
    ici.extent = { 1, 1, 1 };
    ici.mipLevels = 1; ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VmaAllocationCreateInfo aci = {};
    aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    if (vmaCreateImage(u.allocator, &ici, &aci, &dummyImage_, &dummyAlloc_, nullptr) == VK_SUCCESS) {
        VkImageViewCreateInfo vci = {};
        vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vci.image = dummyImage_;
        vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vci.format = kTargetFormat;
        vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vci.subresourceRange.levelCount = 1;
        vci.subresourceRange.layerCount = 1;
        vkCreateImageView(u.device, &vci, nullptr, &dummyView_);
        // One-time layout transition UNDEFINED -> SHADER_READ_ONLY_OPTIMAL.
        VkCommandPoolCreateInfo pci = {};
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.queueFamilyIndex = u.graphicsQueueFamilyIndex;
        VkCommandPool pool = VK_NULL_HANDLE;
        vkCreateCommandPool(u.device, &pci, nullptr, &pool);
        VkCommandBufferAllocateInfo cbi = {};
        cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbi.commandPool = pool; cbi.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; cbi.commandBufferCount = 1;
        VkCommandBuffer cb = VK_NULL_HANDLE;
        vkAllocateCommandBuffers(u.device, &cbi, &cb);
        VkCommandBufferBeginInfo bi = {};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cb, &bi);
        VkImageMemoryBarrier imb = {};
        imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imb.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imb.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imb.srcQueueFamilyIndex = imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imb.image = dummyImage_;
        imb.subresourceRange = vci.subresourceRange;
        imb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &imb);
        vkEndCommandBuffer(cb);
        VkSubmitInfo si = {};
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.commandBufferCount = 1; si.pCommandBuffers = &cb;
        {
            std::mutex& m = u.graphicsSubmitMutex();
            std::lock_guard<std::mutex> lock(m);
            vkQueueSubmit(u.graphicsQueue, 1, &si, VK_NULL_HANDLE);
            vkQueueWaitIdle(u.graphicsQueue);
        }
        vkDestroyCommandPool(u.device, pool, nullptr);
    }
}

bool VulkanGraphicsUtilities::ensureDescriptor(VulkanGraphicsTarget& t) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    if (t.descSet != VK_NULL_HANDLE) return true;
    VkDescriptorPoolSize sizes[2] = {};
    sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; sizes[0].descriptorCount = 1;
    sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; sizes[1].descriptorCount = 1;
    VkDescriptorPoolCreateInfo pci = {};
    pci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pci.maxSets = 1;
    pci.poolSizeCount = 2;
    pci.pPoolSizes = sizes;
    if (vkCreateDescriptorPool(u.device, &pci, nullptr, &t.descPool) != VK_SUCCESS) return false;
    VkDescriptorSetAllocateInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    ai.descriptorPool = t.descPool;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts = &shaderSetLayout_;
    return vkAllocateDescriptorSets(u.device, &ai, &t.descSet) == VK_SUCCESS;
}

VkDescriptorSetLayout VulkanGraphicsUtilities::shaderSetLayout() { ensureShaderInfra(); return shaderSetLayout_; }
VkPipelineLayout VulkanGraphicsUtilities::shaderPipelineLayout() { ensureShaderInfra(); return shaderPipelineLayout_; }

bool VulkanGraphicsUtilities::renderShader(RenderBuffer& buffer, VkPipeline pipeline, VkBuffer ubo, VkImageView inputView) {
    ensureInit();
    if (!ok_ || pipeline == VK_NULL_HANDLE) return false;
    ensureShaderInfra();
    if (shaderSetLayout_ == VK_NULL_HANDLE) return false;
    const uint32_t w = (uint32_t)buffer.BufferWi;
    const uint32_t h = (uint32_t)buffer.BufferHt;
    uint8_t* dst = (uint8_t*)buffer.GetPixels();
    if (w == 0 || h == 0 || dst == nullptr) return false;

    static thread_local VulkanGraphicsTarget target;
    if (!ensureTarget(target, w, h) || !ensureDescriptor(target)) return false;

    VkDescriptorBufferInfo dbi = { ubo, 0, VK_WHOLE_SIZE };
    VkDescriptorImageInfo dii = {};
    dii.sampler = sampler_;
    dii.imageView = (inputView != VK_NULL_HANDLE) ? inputView : dummyView_;
    dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet w2[2] = {};
    w2[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w2[0].dstSet = target.descSet; w2[0].dstBinding = 0; w2[0].descriptorCount = 1;
    w2[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; w2[0].pBufferInfo = &dbi;
    w2[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    w2[1].dstSet = target.descSet; w2[1].dstBinding = 1; w2[1].descriptorCount = 1;
    w2[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; w2[1].pImageInfo = &dii;
    vkUpdateDescriptorSets(VulkanComputeUtilities::INSTANCE.device, 2, w2, 0, nullptr);

    if (!renderCore(target, w, h, pipeline, shaderPipelineLayout_, nullptr, 0, target.descSet)) return false;

    const uint8_t* src = (const uint8_t*)target.readback.mapped;
    for (uint32_t y = 0; y < h; y++) {
        std::memcpy(dst + (size_t)(h - 1 - y) * w * 4, src + (size_t)y * w * 4, (size_t)w * 4);
    }
    return true;
}

VkImageView VulkanGraphicsUtilities::prepareInputImage(uint32_t w, uint32_t h, VkFormat format, const void* data, size_t byteSize) {
    ensureInit();
    if (!ok_ || w == 0 || h == 0 || data == nullptr || byteSize == 0) return VK_NULL_HANDLE;
    ensureShaderInfra();
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;

    struct InputImg {
        uint32_t w = 0, h = 0;
        VkFormat fmt = VK_FORMAT_UNDEFINED;
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation alloc = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VulkanBuffer staging;
        VkCommandPool pool = VK_NULL_HANDLE;
        VkCommandBuffer cb = VK_NULL_HANDLE;
        VkFence fence = VK_NULL_HANDLE;
    };
    static thread_local InputImg t;

    if (t.pool == VK_NULL_HANDLE) {
        VkCommandPoolCreateInfo pci = {};
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pci.queueFamilyIndex = u.graphicsQueueFamilyIndex;
        if (vkCreateCommandPool(u.device, &pci, nullptr, &t.pool) != VK_SUCCESS) return VK_NULL_HANDLE;
        VkCommandBufferAllocateInfo ai = {};
        ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool = t.pool; ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount = 1;
        vkAllocateCommandBuffers(u.device, &ai, &t.cb);
        VkFenceCreateInfo fci = {};
        fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vkCreateFence(u.device, &fci, nullptr, &t.fence);
    }
    if (t.image == VK_NULL_HANDLE || t.w != w || t.h != h || t.fmt != format) {
        if (t.view) vkDestroyImageView(u.device, t.view, nullptr);
        if (t.image) vmaDestroyImage(u.allocator, t.image, t.alloc);
        t.view = VK_NULL_HANDLE; t.image = VK_NULL_HANDLE;
        VkImageCreateInfo ici = {};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.format = format;
        ici.extent = { w, h, 1 };
        ici.mipLevels = 1; ici.arrayLayers = 1;
        ici.samples = VK_SAMPLE_COUNT_1_BIT;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VmaAllocationCreateInfo aci = {};
        aci.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        if (vmaCreateImage(u.allocator, &ici, &aci, &t.image, &t.alloc, nullptr) != VK_SUCCESS) {
            t.image = VK_NULL_HANDLE;
            return VK_NULL_HANDLE;
        }
        VkImageViewCreateInfo vci = {};
        vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vci.image = t.image;
        vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vci.format = format;
        vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vci.subresourceRange.levelCount = 1;
        vci.subresourceRange.layerCount = 1;
        vkCreateImageView(u.device, &vci, nullptr, &t.view);
        t.w = w; t.h = h; t.fmt = format;
    }
    if (!t.staging || t.staging.size < byteSize) {
        if (t.staging) u.destroyBuffer(t.staging);
        if (!u.createSharedBuffer(t.staging, byteSize, "ShaderInputStaging")) return VK_NULL_HANDLE;
    }
    std::memcpy(t.staging.mapped, data, byteSize);

    vkResetCommandPool(u.device, t.pool, 0);
    VkCommandBufferBeginInfo bi = {};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(t.cb, &bi);

    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.levelCount = 1; range.layerCount = 1;
    auto barrier = [&](VkImageLayout oldL, VkImageLayout newL, VkAccessFlags src, VkAccessFlags dst,
                       VkPipelineStageFlags srcS, VkPipelineStageFlags dstS) {
        VkImageMemoryBarrier imb = {};
        imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imb.oldLayout = oldL; imb.newLayout = newL;
        imb.srcQueueFamilyIndex = imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imb.image = t.image; imb.subresourceRange = range;
        imb.srcAccessMask = src; imb.dstAccessMask = dst;
        vkCmdPipelineBarrier(t.cb, srcS, dstS, 0, 0, nullptr, 0, nullptr, 1, &imb);
    };
    barrier(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = { w, h, 1 };
    vkCmdCopyBufferToImage(t.cb, t.staging.buffer, t.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    barrier(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    vkEndCommandBuffer(t.cb);

    VkSubmitInfo si = {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1; si.pCommandBuffers = &t.cb;
    {
        std::mutex& m = u.graphicsSubmitMutex();
        std::lock_guard<std::mutex> lock(m);
        if (vkQueueSubmit(u.graphicsQueue, 1, &si, t.fence) != VK_SUCCESS) return VK_NULL_HANDLE;
    }
    vkWaitForFences(u.device, 1, &t.fence, VK_TRUE, UINT64_MAX);
    vkResetFences(u.device, 1, &t.fence);
    return t.view;
}

bool VulkanGraphicsUtilities::runtimeShaderTest() {
    if (!ok_) return false;
    const char* frag =
        "#version 450\n"
        "layout(location=0) in vec2 vUV;\n"
        "layout(location=0) out vec4 o;\n"
        "layout(push_constant) uniform P { float t; } pc;\n"
        "void main(){ o = vec4(vUV, pc.t, 1.0); }\n";
    std::vector<uint32_t> fspv;
    std::string err;
    if (!VulkanShaderTranslate::ToSpirv(frag, VulkanShaderTranslate::Stage::Fragment, fspv, err)) {
        spdlog::error("Vulkan runtime-shader test: translate failed: {}", err);
        return false;
    }
    VkPipelineLayout layout = createPipelineLayout(16, VK_NULL_HANDLE);
    VkPipeline pipe = buildPipeline(fullscreen_vert_spv, sizeof(fullscreen_vert_spv),
                                    fspv.data(), fspv.size() * sizeof(uint32_t), layout);
    if (pipe == VK_NULL_HANDLE) {
        spdlog::error("Vulkan runtime-shader test: pipeline build failed");
        return false;
    }
    const uint32_t w = 16, h = 16;
    static thread_local VulkanGraphicsTarget t;
    auto blueAtCenter = [&](float tval) -> int {
        if (!ensureTarget(t, w, h) || !renderCore(t, w, h, pipe, layout, &tval, sizeof(tval), VK_NULL_HANDLE))
            return -1;
        const uint8_t* p = (const uint8_t*)t.readback.mapped + ((size_t)(h / 2) * w + (w / 2)) * 4;
        return p[2];  // blue = pc.t * 255
    };
    int lo = blueAtCenter(0.25f);
    int hi = blueAtCenter(0.75f);
    bool pass = (lo >= 0 && hi >= 0 && hi - lo > 100);  // ~64 vs ~191
    spdlog::info("Vulkan runtime-shader test: blue(t=.25)={} blue(t=.75)={} -> {} ({} SPIR-V words)",
                 lo, hi, pass ? "PASS" : "FAIL", fspv.size());
    vkDestroyPipeline(VulkanComputeUtilities::INSTANCE.device, pipe, nullptr);
    vkDestroyPipelineLayout(VulkanComputeUtilities::INSTANCE.device, layout, nullptr);
    return pass;
}

bool VulkanGraphicsUtilities::selfTest() {
    // Assumes init already ran (called at the tail of doInit, or after
    // ensureInit); does NOT re-enter init.
    if (!ok_) {
        spdlog::warn("Vulkan graphics self-test skipped (pipeline unavailable)");
        return false;
    }
    const uint32_t w = 16, h = 16;
    static thread_local VulkanGraphicsTarget t;
    if (!ensureTarget(t, w, h) || !renderCore(t, w, h, gradientPipeline_, emptyLayout_, nullptr, 0, VK_NULL_HANDLE)) {
        spdlog::error("Vulkan graphics self-test: render failed");
        return false;
    }
    // gradient_test.frag emits (R=u, G=v).  Image is top-row-first: top-left is
    // ~(0,0), bottom-right ~(near 255).  Just check the gradient direction.
    const uint8_t* p = (const uint8_t*)t.readback.mapped;
    auto px = [&](uint32_t x, uint32_t y) { return p + ((size_t)y * w + x) * 4; };
    const uint8_t* tl = px(0, 0);
    const uint8_t* br = px(w - 1, h - 1);
    bool pass = tl[0] < 32 && tl[1] < 32 && br[0] > 200 && br[1] > 200;
    spdlog::info("Vulkan graphics self-test: TL=({},{},{}) BR=({},{},{}) -> {}",
                 tl[0], tl[1], tl[2], br[0], br[1], br[2], pass ? "PASS" : "FAIL");
    return pass;
}

#endif
