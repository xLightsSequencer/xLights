#include "VulkanBackend.h"
#include <iostream>
#include <set>
#include <algorithm>
#include <fstream>
#include "textured_shaders.h"
// Include platform-specific Vulkan surface helpers
#if defined(_WIN32)
#  include <vulkan/vulkan_win32.h>
#  include <windows.h>
#else
#  include <vulkan/vulkan_xlib.h>
#endif

namespace gfx {

// forward declaration for helper defined later in this file
static bool create_buffer(VkDevice device, VkPhysicalDevice physical, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/) {
    std::cerr << "[Vulkan] " << (pCallbackData ? pCallbackData->pMessage : "") << std::endl;
    return VK_FALSE;
}

bool VulkanBackend::QueueIndexedDrawBuffer(VkBuffer buffer, VkDeviceMemory memory, uint32_t vertexCount, PrimitiveType prim, float pointSize) {
    if (!initialized_) return false;
    PendingIndexedDrawEntry e;
    e.buffer = buffer;
    e.memory = memory;
    e.vertexCount = vertexCount;
    e.prim = prim;
    e.pointSize = pointSize;
    e.dstImageIndex = UINT32_MAX;
    pending_indexed_draws_.push_back(e);
    return true;
}

bool VulkanBackend::QueueIndexedDraw(const void* vertexColorData, VkDeviceSize dataSize, uint32_t vertexCount, PrimitiveType prim, float pointSize) {
    if (!initialized_ || vertexColorData == nullptr || dataSize == 0) return false;
    VkBuffer buf = VK_NULL_HANDLE;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    if (!CreateBuffer(dataSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buf, mem)) {
        return false;
    }
    if (!UpdateBufferData(mem, 0, vertexColorData, dataSize)) {
        DestroyBuffer(buf, mem);
        return false;
    }
    // Ownership transferred to pending draws
    return QueueIndexedDrawBuffer(buf, mem, vertexCount, prim, pointSize);
}

bool VulkanBackend::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &outBuffer, VkDeviceMemory &outMemory) {
    if (!device_ || !physical_device_) return false;
    return ::gfx::create_buffer(device_, physical_device_, size, usage, properties, outBuffer, outMemory);
}

bool VulkanBackend::UpdateBufferData(VkDeviceMemory memory, VkDeviceSize offset, const void* data, VkDeviceSize size) {
    if (!device_) return false;
    void* mapped = nullptr;
    if (vkMapMemory(device_, memory, offset, size, 0, &mapped) != VK_SUCCESS) return false;
    memcpy(mapped, data, (size_t)size);
    vkUnmapMemory(device_, memory);
    return true;
}

bool VulkanBackend::CreateColorPipelineFromBytes(const std::vector<uint32_t> &vertSpv, const std::vector<uint32_t> &fragSpv) {
    if (vertSpv.empty() || fragSpv.empty()) return false;
    VkShaderModule vertModule = createShaderModuleFromWords(device_, vertSpv);
    VkShaderModule fragModule = createShaderModuleFromWords(device_, fragSpv);
    if (vertModule == VK_NULL_HANDLE || fragModule == VK_NULL_HANDLE) return false;

    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertModule;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragModule;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

    // Vertex input binding (pos x,y,z + color r,g,b,a)
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(float) * 7; // 3 + 4
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> attributeDescs(2);
    attributeDescs[0].binding = 0; attributeDescs[0].location = 0; attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT; attributeDescs[0].offset = 0;
    attributeDescs[1].binding = 0; attributeDescs[1].location = 1; attributeDescs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; attributeDescs[1].offset = sizeof(float)*3;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescs.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f; viewport.y = 0.0f;
    viewport.width = (float)swapchain_extent_.width;
    viewport.height = (float)swapchain_extent_.height;
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;

    VkRect2D scissor{}; scissor.offset = {0,0}; scissor.extent = swapchain_extent_;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1; viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1; viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // no descriptors for now
    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &colorPipelineLayout_) != VK_SUCCESS) {
        vkDestroyShaderModule(device_, fragModule, nullptr);
        vkDestroyShaderModule(device_, vertModule, nullptr);
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = colorPipelineLayout_;
    pipelineInfo.renderPass = render_pass_;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &colorPipeline_) != VK_SUCCESS) {
        vkDestroyPipelineLayout(device_, colorPipelineLayout_, nullptr);
        colorPipelineLayout_ = VK_NULL_HANDLE;
        vkDestroyShaderModule(device_, fragModule, nullptr);
        vkDestroyShaderModule(device_, vertModule, nullptr);
        return false;
    }

    vkDestroyShaderModule(device_, fragModule, nullptr);
    vkDestroyShaderModule(device_, vertModule, nullptr);

    return true;
}

void VulkanBackend::DestroyBuffer(VkBuffer buffer, VkDeviceMemory memory) {
    if (!device_) return;
    if (buffer != VK_NULL_HANDLE) vkDestroyBuffer(device_, buffer, nullptr);
    if (memory != VK_NULL_HANDLE) vkFreeMemory(device_, memory, nullptr);
}

int VulkanBackend::StartReadback(uint32_t imageIndex) {
    if (!initialized_) return -1;
    if (imageIndex == UINT32_MAX) {
        if (current_image_index_ != UINT32_MAX) imageIndex = current_image_index_;
        else imageIndex = 0;
    }
    if (imageIndex >= swapchain_images_.size()) return -1;

    uint32_t w = swapchain_extent_.width;
    uint32_t h = swapchain_extent_.height;
    VkDeviceSize bufSize = (VkDeviceSize)w * (VkDeviceSize)h * 4;

    // create staging buffer (TRANSFER_DST, host visible)
    VkBuffer stagingBuf = VK_NULL_HANDLE;
    VkDeviceMemory stagingMem = VK_NULL_HANDLE;
    if (!::gfx::create_buffer(device_, physical_device_, bufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuf, stagingMem)) {
        return -1;
    }

    // allocate command buffer
    VkCommandBufferAllocateInfo ai{};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandPool = command_pool_;
    ai.commandBufferCount = 1;
    VkCommandBuffer cmd;
    if (vkAllocateCommandBuffers(device_, &ai, &cmd) != VK_SUCCESS) {
        vkDestroyBuffer(device_, stagingBuf, nullptr);
        vkFreeMemory(device_, stagingMem, nullptr);
        return -1;
    }

    VkCommandBufferBeginInfo bi{}; bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &bi);

    VkImage srcImage = swapchain_images_[imageIndex];

    // transition src image to TRANSFER_SRC
    VkImageMemoryBarrier barrierSrc{};
    barrierSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierSrc.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrierSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrierSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierSrc.image = srcImage;
    barrierSrc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierSrc.subresourceRange.baseMipLevel = 0;
    barrierSrc.subresourceRange.levelCount = 1;
    barrierSrc.subresourceRange.baseArrayLayer = 0;
    barrierSrc.subresourceRange.layerCount = 1;
    barrierSrc.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrierSrc.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierSrc);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0; // tightly packed
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0,0,0};
    region.imageExtent = { w, h, 1 };

    vkCmdCopyImageToBuffer(cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuf, 1, &region);

    // transition back to PRESENT_SRC
    VkImageMemoryBarrier barrierToPresent{};
    barrierToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierToPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrierToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrierToPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToPresent.image = srcImage;
    barrierToPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierToPresent.subresourceRange.baseMipLevel = 0;
    barrierToPresent.subresourceRange.levelCount = 1;
    barrierToPresent.subresourceRange.baseArrayLayer = 0;
    barrierToPresent.subresourceRange.layerCount = 1;
    barrierToPresent.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrierToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierToPresent);

    vkEndCommandBuffer(cmd);

    // create fence
    VkFenceCreateInfo fi{}; fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; fi.flags = 0;
    VkFence fence = VK_NULL_HANDLE;
    if (vkCreateFence(device_, &fi, nullptr, &fence) != VK_SUCCESS) {
        vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);
        vkDestroyBuffer(device_, stagingBuf, nullptr);
        vkFreeMemory(device_, stagingMem, nullptr);
        return -1;
    }

    VkSubmitInfo si{}; si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
    if (vkQueueSubmit(graphics_queue_, 1, &si, fence) != VK_SUCCESS) {
        vkDestroyFence(device_, fence, nullptr);
        vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);
        vkDestroyBuffer(device_, stagingBuf, nullptr);
        vkFreeMemory(device_, stagingMem, nullptr);
        return -1;
    }

    // keep request
    auto req = std::make_unique<ReadbackRequest>();
    req->stagingBuffer = stagingBuf;
    req->stagingMemory = stagingMem;
    req->fence = fence;
    req->imageIndex = imageIndex;
    req->width = w;
    req->height = h;
    req->size = bufSize;

    int id = static_cast<int>(readback_requests_.size());
    readback_requests_.push_back(std::move(req));

    // free command buffer now (fence ensures completion holds resources until done)
    vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);

    return id;
}

bool VulkanBackend::TryGetReadback(int requestId, std::vector<uint8_t> &outPixels) {
    if (requestId < 0 || static_cast<size_t>(requestId) >= readback_requests_.size()) return false;
    auto &reqPtr = readback_requests_[requestId];
    if (!reqPtr) return false;
    ReadbackRequest* req = reqPtr.get();

    VkResult r = vkGetFenceStatus(device_, req->fence);
    if (r == VK_NOT_READY) return false;
    if (r != VK_SUCCESS) return false;

    // fence signaled, map staging buffer
    void* mapped = nullptr;
    if (vkMapMemory(device_, req->stagingMemory, 0, req->size, 0, &mapped) != VK_SUCCESS) return false;
    uint8_t *src = reinterpret_cast<uint8_t*>(mapped);
    outPixels.resize((size_t)req->width * (size_t)req->height * 4);
    memcpy(outPixels.data(), src, (size_t)req->size);
    vkUnmapMemory(device_, req->stagingMemory);

    // cleanup
    if (req->stagingBuffer != VK_NULL_HANDLE) vkDestroyBuffer(device_, req->stagingBuffer, nullptr);
    if (req->stagingMemory != VK_NULL_HANDLE) vkFreeMemory(device_, req->stagingMemory, nullptr);
    if (req->fence != VK_NULL_HANDLE) vkDestroyFence(device_, req->fence, nullptr);

    // remove request slot
    readback_requests_[requestId].reset();
    return true;
}

bool VulkanBackend::ReadSwapchainImagePixels(std::vector<uint8_t> &outPixels, uint32_t imageIndex) {
    if (!initialized_) return false;
    if (imageIndex == UINT32_MAX) {
        if (current_image_index_ != UINT32_MAX) imageIndex = current_image_index_;
        else imageIndex = 0;
    }
    if (imageIndex >= swapchain_images_.size()) return false;

    VkImage srcImage = swapchain_images_[imageIndex];
    uint32_t w = swapchain_extent_.width;
    uint32_t h = swapchain_extent_.height;
    VkDeviceSize imageSize = (VkDeviceSize)w * (VkDeviceSize)h * 4;

    // Create a linear tiled destination image compatible with CPU readback
    VkImageCreateInfo imgInfo{};
    imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.extent.width = w;
    imgInfo.extent.height = h;
    imgInfo.extent.depth = 1;
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imgInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage dstImage;
    if (vkCreateImage(device_, &imgInfo, nullptr, &dstImage) != VK_SUCCESS) return false;

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device_, dstImage, &memReq);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memProperties);

    uint32_t memTypeIndex = 0xffffffffu;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((memReq.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            memTypeIndex = i; break;
        }
    }
    if (memTypeIndex == 0xffffffffu) {
        vkDestroyImage(device_, dstImage, nullptr);
        return false;
    }

    VkMemoryAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize = memReq.size;
    alloc.memoryTypeIndex = memTypeIndex;
    VkDeviceMemory dstMemory;
    if (vkAllocateMemory(device_, &alloc, nullptr, &dstMemory) != VK_SUCCESS) {
        vkDestroyImage(device_, dstImage, nullptr);
        return false;
    }
    vkBindImageMemory(device_, dstImage, dstMemory, 0);

    // Create command buffer to copy swapchain image to linear image
    VkCommandBufferAllocateInfo ai{}; ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandPool = command_pool_; ai.commandBufferCount = 1;
    VkCommandBuffer cmd; vkAllocateCommandBuffers(device_, &ai, &cmd);
    VkCommandBufferBeginInfo bi{}; bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &bi);

    // Transition src image to TRANSFER_SRC
    VkImageMemoryBarrier barrierSrc{};
    barrierSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierSrc.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrierSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrierSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierSrc.image = srcImage;
    barrierSrc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierSrc.subresourceRange.baseMipLevel = 0;
    barrierSrc.subresourceRange.levelCount = 1;
    barrierSrc.subresourceRange.baseArrayLayer = 0;
    barrierSrc.subresourceRange.layerCount = 1;
    barrierSrc.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    barrierSrc.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierSrc);

    // Transition dst image to TRANSFER_DST
    VkImageMemoryBarrier barrierDst{};
    barrierDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrierDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierDst.image = dstImage;
    barrierDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierDst.subresourceRange.baseMipLevel = 0;
    barrierDst.subresourceRange.levelCount = 1;
    barrierDst.subresourceRange.baseArrayLayer = 0;
    barrierDst.subresourceRange.layerCount = 1;
    barrierDst.srcAccessMask = 0;
    barrierDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierDst);

    VkImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; copyRegion.srcSubresource.baseArrayLayer = 0; copyRegion.srcSubresource.layerCount = 1; copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.srcOffset = {0,0,0};
    copyRegion.dstSubresource = copyRegion.srcSubresource;
    copyRegion.dstOffset = {0,0,0};
    copyRegion.extent = { w, h, 1 };
    vkCmdCopyImage(cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    // Transition dst image to GENERAL so we can map
    VkImageMemoryBarrier barrierDstGen{};
    barrierDstGen.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierDstGen.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierDstGen.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrierDstGen.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierDstGen.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierDstGen.image = dstImage;
    barrierDstGen.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierDstGen.subresourceRange.baseMipLevel = 0;
    barrierDstGen.subresourceRange.levelCount = 1;
    barrierDstGen.subresourceRange.baseArrayLayer = 0;
    barrierDstGen.subresourceRange.layerCount = 1;
    barrierDstGen.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierDstGen.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierDstGen);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo si{}; si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
    if (vkQueueSubmit(graphics_queue_, 1, &si, VK_NULL_HANDLE) != VK_SUCCESS) {
        vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);
        vkDestroyImage(device_, dstImage, nullptr);
        vkFreeMemory(device_, dstMemory, nullptr);
        return false;
    }
    vkQueueWaitIdle(graphics_queue_);
    vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);

    // Map image memory and read pixels
    VkImageSubresource subres{}; subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; subres.arrayLayer = 0; subres.mipLevel = 0;
    VkSubresourceLayout layout; vkGetImageSubresourceLayout(device_, dstImage, &subres, &layout);

    void* mapped = nullptr;
    if (vkMapMemory(device_, dstMemory, 0, VK_WHOLE_SIZE, 0, &mapped) != VK_SUCCESS) {
        vkDestroyImage(device_, dstImage, nullptr);
        vkFreeMemory(device_, dstMemory, nullptr);
        return false;
    }

    uint8_t *src = reinterpret_cast<uint8_t*>(mapped) + layout.offset;
    outPixels.resize((size_t)w * (size_t)h * 4);

    // The linear layout may include rowPitch padding.
    for (uint32_t y = 0; y < h; ++y) {
        memcpy(outPixels.data() + (size_t)y * w * 4, src + (size_t)y * layout.rowPitch, w * 4);
    }

    vkUnmapMemory(device_, dstMemory);
    vkDestroyImage(device_, dstImage, nullptr);
    vkFreeMemory(device_, dstMemory, nullptr);

    return true;
}

// Helper to create shader module from uint32_t SPV word vector
VkShaderModule VulkanBackend::createShaderModuleFromWords(VkDevice device, const std::vector<uint32_t>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

VulkanBackend::VulkanBackend() {
    // allocate buffers_to_destroy_ for frames in flight once frames_in_flight_ is set in Init
}

VulkanBackend::~VulkanBackend() {
    Shutdown();
}

bool VulkanBackend::Init(void* native_window, void* native_display, uint32_t frames_in_flight) {
    if (!native_window) return false;
    native_window_ = native_window;
    native_display_ = native_display;
    frames_in_flight_ = std::max<uint32_t>(1, frames_in_flight);

    buffers_to_destroy_.resize(frames_in_flight_);

    // Initialize volk loader
    if (volkInitialize() != VK_SUCCESS) {
        std::cerr << "volkInitialize failed" << std::endl;
        return false;
    }

    if (!create_instance()) return false;

    volkLoadInstance(instance_);

    if (!create_surface()) return false;

    if (!pick_physical_device()) return false;

    if (!create_logical_device()) return false;

    volkLoadDevice(device_);

    if (!create_swapchain()) return false;
    if (!create_image_views()) return false;
    if (!create_render_pass()) return false;
    if (!create_framebuffers()) return false;
    if (!create_command_pool_and_buffers()) return false;
    if (!create_sync_objects()) return false;

    initialized_ = true;
    return true;
}

bool VulkanBackend::CreateTexturedPipelineFromBytes(const std::vector<uint32_t> &vertSpv, const std::vector<uint32_t> &fragSpv) {
    if (vertSpv.empty() || fragSpv.empty()) return false;
    VkShaderModule vertModule = createShaderModuleFromWords(device_, vertSpv);
    VkShaderModule fragModule = createShaderModuleFromWords(device_, fragSpv);
    if (vertModule == VK_NULL_HANDLE || fragModule == VK_NULL_HANDLE) return false;

    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertModule;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragModule;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

    // Vertex input binding (pos x,y,z, uv u,v)
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(float) * 5;
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> attributeDescs(2);
    attributeDescs[0].binding = 0; attributeDescs[0].location = 0; attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT; attributeDescs[0].offset = 0;
    attributeDescs[1].binding = 0; attributeDescs[1].location = 1; attributeDescs[1].format = VK_FORMAT_R32G32_SFLOAT; attributeDescs[1].offset = sizeof(float)*3;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescs.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f; viewport.y = 0.0f;
    viewport.width = (float)swapchain_extent_.width;
    viewport.height = (float)swapchain_extent_.height;
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;

    VkRect2D scissor{}; scissor.offset = {0,0}; scissor.extent = swapchain_extent_;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1; viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1; viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Descriptor set layout for combined image sampler
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
        std::cerr << "failed to create descriptor set layout!" << std::endl;
        return false;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
    // Add push constant for destination rect (vec4: x,y,w,h)
    VkPushConstantRange pushRange{};
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushRange.offset = 0;
    pushRange.size = sizeof(float) * 4;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;

    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
        std::cerr << "failed to create pipeline layout!" << std::endl;
        return false;
    }

    // Create a descriptor pool for texture descriptor sets (create later in command pool setup too)
    // We'll create a small pool here, but create_command_pool_and_buffers will create a proper one.
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 16;
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 16;
    if (vkCreateDescriptorPool(device_, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        // not fatal; descriptor pool may be created later when textures exist
        descriptorPool_ = VK_NULL_HANDLE;
    }

    // Create a simple quad vertex buffer (6 vertices, pos.xyz + uv.xy)
    {
        float quadVerts[6 * 5] = {
            // x,y,z, u,v  - unit quad [0,1] in xy; vertex shader will map to dst rect
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f
        };
        VkDeviceSize vbSize = sizeof(quadVerts);
        if (!::gfx::create_buffer(device_, physical_device_, vbSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, quadVertexBuffer_, quadVertexMemory_)) {
            std::cerr << "failed to create quad vertex buffer" << std::endl;
        } else {
            void* mapped = nullptr;
            vkMapMemory(device_, quadVertexMemory_, 0, vbSize, 0, &mapped);
            memcpy(mapped, quadVerts, vbSize);
            vkUnmapMemory(device_, quadVertexMemory_);
        }
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout_;
    pipelineInfo.renderPass = render_pass_;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS) {
        std::cerr << "failed to create graphics pipeline!" << std::endl;
        return false;
    }

    vkDestroyShaderModule(device_, fragModule, nullptr);
    vkDestroyShaderModule(device_, vertModule, nullptr);

    // create default sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(device_, &samplerInfo, nullptr, &sampler_) != VK_SUCCESS) {
        std::cerr << "failed to create sampler" << std::endl;
        sampler_ = VK_NULL_HANDLE;
    }

    return true;
}

void VulkanBackend::QueueCopyTextureToSwapchain(int textureId, uint32_t dstX, uint32_t dstY, uint32_t dstW, uint32_t dstH) {
    if (textureId < 0 || textureId >= static_cast<int>(gpu_textures_.size())) return;
    VulkanBackend::PendingCopy pc;
    pc.srcTexture = gpu_textures_[textureId].get();
    pc.dstX = dstX; pc.dstY = dstY; pc.dstW = dstW; pc.dstH = dstH;
    // dstImageIndex will be set at frame begin; use UINT32_MAX to indicate any
    pc.dstImageIndex = UINT32_MAX;
    pending_copies_.push_back(pc);
}

int VulkanBackend::CreateTextureFromPixels(const uint8_t* pixels, uint32_t w, uint32_t h) {
    if (!device_ || !physical_device_) return -1;
    // create staging buffer
    VkDeviceSize imageSize = w * h * 4;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    if (!::gfx::create_buffer(device_, physical_device_, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory)) {
        std::cerr << "failed to create staging buffer" << std::endl;
        return -1;
    }

    void* data;
    vkMapMemory(device_, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vkUnmapMemory(device_, stagingBufferMemory);

    // create image
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = w;
    imageInfo.extent.height = h;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage image;
    if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        std::cerr << "failed to create image" << std::endl;
        vkDestroyBuffer(device_, stagingBuffer, nullptr);
        vkFreeMemory(device_, stagingBufferMemory, nullptr);
        return -1;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memProperties);
    uint32_t memoryTypeIndex = 0xffffffffu;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            memoryTypeIndex = i; break;
        }
    }
    if (memoryTypeIndex == 0xffffffffu) {
        std::cerr << "failed to find memory type for image" << std::endl;
        vkDestroyImage(device_, image, nullptr);
        vkDestroyBuffer(device_, stagingBuffer, nullptr);
        vkFreeMemory(device_, stagingBufferMemory, nullptr);
        return -1;
    }
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    VkDeviceMemory imageMemory;
    if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        std::cerr << "failed to allocate image memory" << std::endl;
        vkDestroyImage(device_, image, nullptr);
        vkDestroyBuffer(device_, stagingBuffer, nullptr);
        vkFreeMemory(device_, stagingBufferMemory, nullptr);
        return -1;
    }
    vkBindImageMemory(device_, image, imageMemory, 0);

    // Create a command buffer for copy
    VkCommandBufferAllocateInfo allocInfoCmd{};
    allocInfoCmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfoCmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfoCmd.commandPool = command_pool_;
    allocInfoCmd.commandBufferCount = 1;

    VkCommandBuffer cmd;
    if (vkAllocateCommandBuffers(device_, &allocInfoCmd, &cmd) != VK_SUCCESS) {
        std::cerr << "failed to allocate command buffer for texture upload" << std::endl;
        vkDestroyImage(device_, image, nullptr);
        vkDestroyBuffer(device_, stagingBuffer, nullptr);
        vkFreeMemory(device_, stagingBufferMemory, nullptr);
        vkFreeMemory(device_, imageMemory, nullptr);
        return -1;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    // Transition image layout to DST optimal
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Copy buffer to image
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0,0,0};
    region.imageExtent = { w, h, 1 };

    vkCmdCopyBufferToImage(cmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition image to SHADER_READ_ONLY_OPTIMAL
    VkImageMemoryBarrier barrier2 = barrier;
    barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    if (vkQueueSubmit(graphics_queue_, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        std::cerr << "failed to submit staging command buffer" << std::endl;
        vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);
        vkDestroyImage(device_, image, nullptr);
        vkDestroyBuffer(device_, stagingBuffer, nullptr);
        vkFreeMemory(device_, stagingBufferMemory, nullptr);
        vkFreeMemory(device_, imageMemory, nullptr);
        return -1;
    }
    vkQueueWaitIdle(graphics_queue_);
    vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);

    // cleanup staging
    vkDestroyBuffer(device_, stagingBuffer, nullptr);
    vkFreeMemory(device_, stagingBufferMemory, nullptr);

    // create image view
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device_, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        std::cerr << "failed to create texture image view" << std::endl;
        vkDestroyImage(device_, image, nullptr);
        vkFreeMemory(device_, imageMemory, nullptr);
        return -1;
    }

    auto tex = std::make_unique<GpuTexture>();
    tex->image = image;
    tex->memory = imageMemory;
    tex->view = imageView;
    tex->format = VK_FORMAT_R8G8B8A8_UNORM;
    tex->width = w;
    tex->height = h;

    // keep a CPU-side copy for fallback composition
    tex->cpu_pixels.assign(pixels, pixels + (size_t)w * (size_t)h * 4);
    // keep a CPU-side copy for fallback composition
    tex->cpu_pixels.assign(pixels, pixels + (size_t)w * (size_t)h * 4);
    gpu_textures_.push_back(std::move(tex));
    return static_cast<int>(gpu_textures_.size() - 1);
}

// Helper: create buffer and allocate memory
bool create_buffer(VkDevice device, VkPhysicalDevice physical, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical, &memProperties);

    auto findMemoryType = [&](uint32_t typeFilter, VkMemoryPropertyFlags props) -> uint32_t {
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & props) == props) {
                return i;
            }
        }
        return 0xffffffffu;
    };

    uint32_t memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    if (memoryTypeIndex == 0xffffffffu) return false;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        return false;
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
    return true;
}

void VulkanBackend::Shutdown() {
    if (!initialized_) return;

    vkDeviceWaitIdle(device_);

    cleanup_swapchain();

    for (size_t i = 0; i < frames_in_flight_; ++i) {
        if (render_finished_semaphores_[i] != VK_NULL_HANDLE) vkDestroySemaphore(device_, render_finished_semaphores_[i], nullptr);
        if (image_available_semaphores_[i] != VK_NULL_HANDLE) vkDestroySemaphore(device_, image_available_semaphores_[i], nullptr);
        if (in_flight_fences_[i] != VK_NULL_HANDLE) vkDestroyFence(device_, in_flight_fences_[i], nullptr);
    }

    if (command_pool_ != VK_NULL_HANDLE) vkDestroyCommandPool(device_, command_pool_, nullptr);

    if (device_ != VK_NULL_HANDLE) vkDestroyDevice(device_, nullptr);
    if (surface_ != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance_, surface_, nullptr);
    if (instance_ != VK_NULL_HANDLE) vkDestroyInstance(instance_, nullptr);

    initialized_ = false;
}

void VulkanBackend::ClearColor(float r, float g, float b, float a) {
    clear_color_[0] = r; clear_color_[1] = g; clear_color_[2] = b; clear_color_[3] = a;
}

bool VulkanBackend::BeginFrame() {
    if (!initialized_) return false;

    vkWaitForFences(device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);
    vkResetFences(device_, 1, &in_flight_fences_[current_frame_]);

    uint32_t imageIndex = 0;
    VkResult res = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, image_available_semaphores_[current_frame_], VK_NULL_HANDLE, &imageIndex);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        // TODO: handle resize
        return false;
    } else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
        std::cerr << "Failed to acquire swapchain image" << std::endl;
        return false;
    }
    current_image_index_ = imageIndex;

    // Record command buffer for this image: clear and any pending copies
    VkCommandBuffer cmd = command_buffers_[imageIndex];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkResetCommandBuffer(cmd, 0);
    vkBeginCommandBuffer(cmd, &beginInfo);

    // Compose CPU pending draws into backbuffer (still done on CPU)
    uint32_t fbw = swapchain_extent_.width;
    uint32_t fbh = swapchain_extent_.height;
    size_t pixelCount = (size_t)fbw * (size_t)fbh;
    cpu_backbuffer_pixels_.assign(pixelCount * 4, 0);

    // clear to clear_color_
    uint8_t cr = static_cast<uint8_t>(clear_color_[0] * 255.0f);
    uint8_t cg = static_cast<uint8_t>(clear_color_[1] * 255.0f);
    uint8_t cb = static_cast<uint8_t>(clear_color_[2] * 255.0f);
    uint8_t ca = static_cast<uint8_t>(clear_color_[3] * 255.0f);
    for (size_t i = 0; i < pixelCount; ++i) {
        cpu_backbuffer_pixels_[i*4 + 0] = cr;
        cpu_backbuffer_pixels_[i*4 + 1] = cg;
        cpu_backbuffer_pixels_[i*4 + 2] = cb;
        cpu_backbuffer_pixels_[i*4 + 3] = ca;
    }

    for (const auto &pd : pending_draws_) {
        // simple blit (no scaling or filtering) with alpha overwrite
        for (uint32_t y = 0; y < pd.srcH && y < pd.dstH; ++y) {
            for (uint32_t x = 0; x < pd.srcW && x < pd.dstW; ++x) {
                uint32_t dstX = pd.dstX + x;
                uint32_t dstY = pd.dstY + y;
                if (dstX >= fbw || dstY >= fbh) continue;
                size_t dstIdx = ((size_t)dstY * fbw + dstX) * 4;
                size_t srcIdx = ((size_t)y * pd.srcW + x) * 4;
                cpu_backbuffer_pixels_[dstIdx + 0] = pd.pixels[srcIdx + 0];
                cpu_backbuffer_pixels_[dstIdx + 1] = pd.pixels[srcIdx + 1];
                cpu_backbuffer_pixels_[dstIdx + 2] = pd.pixels[srcIdx + 2];
                cpu_backbuffer_pixels_[dstIdx + 3] = pd.pixels[srcIdx + 3];
            }
        }
    }

    // If we have a textured pipeline and quad vertex buffer, render using GPU pipeline
    if (graphicsPipeline_ != VK_NULL_HANDLE && quadVertexBuffer_ != VK_NULL_HANDLE && descriptorPool_ != VK_NULL_HANDLE) {
        // Begin render pass
        VkClearValue clearValue{};
        clearValue.color.float32[0] = clear_color_[0];
        clearValue.color.float32[1] = clear_color_[1];
        clearValue.color.float32[2] = clear_color_[2];
        clearValue.color.float32[3] = clear_color_[3];

        VkRenderPassBeginInfo rpbi{};
        rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpbi.renderPass = render_pass_;
        rpbi.framebuffer = swapchain_framebuffers_[imageIndex];
        rpbi.renderArea.offset = {0,0};
        rpbi.renderArea.extent = swapchain_extent_;
        rpbi.clearValueCount = 1;
        rpbi.pClearValues = &clearValue;

        vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

        // Bind pipeline and vertex buffer
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);
        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, &quadVertexBuffer_, offsets);

        // Draw queued GPU texture copies using per-texture descriptor sets and push constants
        for (auto &pc : pending_copies_) {
            if (pc.dstImageIndex != UINT32_MAX && pc.dstImageIndex != imageIndex) continue;
            pc.dstImageIndex = imageIndex;
            if (!pc.srcTexture) continue;

            // Allocate descriptor set for texture if needed
            if (pc.srcTexture->descriptorSet == VK_NULL_HANDLE) {
                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = descriptorPool_;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &descriptorSetLayout_;
                VkDescriptorSet ds;
                if (vkAllocateDescriptorSets(device_, &allocInfo, &ds) == VK_SUCCESS) {
                    VkDescriptorImageInfo imgInfo{};
                    imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imgInfo.imageView = pc.srcTexture->view;
                    imgInfo.sampler = sampler_;
                    VkWriteDescriptorSet write{};
                    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    write.dstSet = ds;
                    write.dstBinding = 0;
                    write.dstArrayElement = 0;
                    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    write.descriptorCount = 1;
                    write.pImageInfo = &imgInfo;
                    vkUpdateDescriptorSets(device_, 1, &write, 0, nullptr);
                    pc.srcTexture->descriptorSet = ds;
                }
            }

            if (pc.srcTexture->descriptorSet != VK_NULL_HANDLE) {
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &pc.srcTexture->descriptorSet, 0, nullptr);

                float dstRect[4];
                dstRect[0] = static_cast<float>(pc.dstX);
                dstRect[1] = static_cast<float>(pc.dstY);
                dstRect[2] = static_cast<float>(pc.dstW);
                dstRect[3] = static_cast<float>(pc.dstH);
                vkCmdPushConstants(cmd, pipelineLayout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(dstRect), &dstRect);

                vkCmdDraw(cmd, 6, 1, 0, 0);
            }
        }

        // Draw any pending indexed draws (vertexColor interleaved buffers) using color pipeline if available
        if (colorPipeline_ != VK_NULL_HANDLE) {
            for (auto &entry : pending_indexed_draws_) {
                if (entry.dstImageIndex != UINT32_MAX && entry.dstImageIndex != imageIndex) continue;
                // Bind color pipeline
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, colorPipeline_);
                VkDeviceSize offsets[1] = {0};
                vkCmdBindVertexBuffers(cmd, 0, 1, &entry.buffer, offsets);
                // Only triangle topology supported for now
                vkCmdDraw(cmd, entry.vertexCount, 1, 0, 0);
                // Schedule buffer destruction after submission
                buffers_to_destroy_[current_frame_].push_back(std::make_pair(entry.buffer, entry.memory));
            }
            // Clear pending draws (we've transferred ownership of buffers to destroy list)
            pending_indexed_draws_.clear();
        }

        vkCmdEndRenderPass(cmd);

        // Transition swap image to PRESENT_SRC_KHR
        VkImage swapImage = swapchain_images_[imageIndex];
        VkImageMemoryBarrier barrierToPresent{};
        barrierToPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierToPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrierToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrierToPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierToPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierToPresent.image = swapImage;
        barrierToPresent.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrierToPresent.subresourceRange.baseMipLevel = 0;
        barrierToPresent.subresourceRange.levelCount = 1;
        barrierToPresent.subresourceRange.baseArrayLayer = 0;
        barrierToPresent.subresourceRange.layerCount = 1;
        barrierToPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrierToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierToPresent);

        vkEndCommandBuffer(cmd);

        // submit
        VkSemaphore waitSemaphores[] = { image_available_semaphores_[current_frame_] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { render_finished_semaphores_[current_frame_] };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphics_queue_, 1, &submitInfo, in_flight_fences_[current_frame_]) != VK_SUCCESS) {
            std::cerr << "failed to submit draw command buffer!" << std::endl;
            return false;
        }

        // After submit, destroy any buffers scheduled for previous frames that are no longer in flight
        // buffers_to_destroy_ is sized by frames_in_flight_. We clear the slot corresponding to the frame we are reusing.
        size_t destroy_index = current_frame_;
        // Destroy previous contents if any
        for (auto &p : buffers_to_destroy_[destroy_index]) {
            if (p.first != VK_NULL_HANDLE) vkDestroyBuffer(device_, p.first, nullptr);
            if (p.second != VK_NULL_HANDLE) vkFreeMemory(device_, p.second, nullptr);
        }
        buffers_to_destroy_[destroy_index].clear();

        // clear pending copies for the image we just submitted
        pending_copies_.erase(std::remove_if(pending_copies_.begin(), pending_copies_.end(), [imageIndex](const PendingCopy &pc) { return pc.dstImageIndex == imageIndex; }), pending_copies_.end());

        return true;
    } else {
        // Fallback: upload CPU backbuffer into swapchain image using staging buffer (previous behavior)
        VkDeviceSize imageSize = fbw * fbh * 4;
        VkBuffer stagingBuf; VkDeviceMemory stagingMem;
        if (!::gfx::create_buffer(device_, physical_device_, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuf, stagingMem)) {
            std::cerr << "failed to create staging buffer for backbuffer" << std::endl;
            vkEndCommandBuffer(cmd);
            return false;
        }
        void* mapped; vkMapMemory(device_, stagingMem, 0, imageSize, 0, &mapped);
        memcpy(mapped, cpu_backbuffer_pixels_.data(), (size_t)imageSize);
        vkUnmapMemory(device_, stagingMem);

        // Transition swap image to TRANSFER_DST_OPTIMAL
        VkImage swapImage = swapchain_images_[imageIndex];
        VkImageMemoryBarrier barrierDst{};
        barrierDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrierDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrierDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrierDst.image = swapImage;
        barrierDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrierDst.subresourceRange.baseMipLevel = 0;
        barrierDst.subresourceRange.levelCount = 1;
        barrierDst.subresourceRange.baseArrayLayer = 0;
        barrierDst.subresourceRange.layerCount = 1;
        barrierDst.srcAccessMask = 0;
        barrierDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierDst);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0,0,0};
        region.imageExtent = { fbw, fbh, 1 };

        vkCmdCopyBufferToImage(cmd, stagingBuf, swapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // Transition swap image to PRESENT_SRC
        VkImageMemoryBarrier barrierToPresent = barrierDst;
        barrierToPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrierToPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrierToPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrierToPresent.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierToPresent);

        vkEndCommandBuffer(cmd);

        // cleanup staging
        vkDestroyBuffer(device_, stagingBuf, nullptr);
        vkFreeMemory(device_, stagingMem, nullptr);
        // submit
        VkSemaphore waitSemaphores[] = { image_available_semaphores_[current_frame_] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSemaphore signalSemaphores[] = { render_finished_semaphores_[current_frame_] };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphics_queue_, 1, &submitInfo, in_flight_fences_[current_frame_]) != VK_SUCCESS) {
            std::cerr << "failed to submit draw command buffer!" << std::endl;
            return false;
        }
        // clear pending copies for the image we just submitted
        pending_copies_.erase(std::remove_if(pending_copies_.begin(), pending_copies_.end(), [imageIndex](const PendingCopy &pc) { return pc.dstImageIndex == imageIndex; }), pending_copies_.end());

        return true;
    }
}

bool VulkanBackend::EndFrame() {
    if (!initialized_) return false;
    if (current_image_index_ == UINT32_MAX) {
        // nothing to present
        return false;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    VkSemaphore signalSemaphores[] = { render_finished_semaphores_[current_frame_] };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapchains[] = { swapchain_ };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;

    // Present the image we acquired in BeginFrame
    presentInfo.pImageIndices = &current_image_index_;

    VkResult res = vkQueuePresentKHR(present_queue_, &presentInfo);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        // swapchain recreation required
        return false;
    } else if (res != VK_SUCCESS) {
        std::cerr << "failed to present swapchain image" << std::endl;
        return false;
    }
    current_frame_ = (current_frame_ + 1) % frames_in_flight_;
    current_image_index_ = UINT32_MAX;
    return true;
}

void VulkanBackend::Resize(int width, int height) {
    framebuffer_resized_ = true;
    width_ = width;
    height_ = height;
}

// Private helpers (simplified implementations follow)

bool VulkanBackend::create_instance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanBackend";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "NoEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    std::vector<const char*> extensions;
    // We need the surface extension plus the platform-specific surface extension
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(_WIN32)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
        std::cerr << "failed to create instance!" << std::endl;
        return false;
    }

    return true;
}

bool VulkanBackend::create_surface() {
    // Create platform-specific surface using native handles
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = static_cast<HWND>(native_window_);
    createInfo.hinstance = GetModuleHandle(nullptr);
    if (vkCreateWin32SurfaceKHR(instance_, &createInfo, nullptr, &surface_) != VK_SUCCESS) {
        std::cerr << "failed to create Win32 window surface!" << std::endl;
        return false;
    }
#else
    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = static_cast<Display*>(native_display_);
    createInfo.window = static_cast<Window>(reinterpret_cast<uintptr_t>(native_window_));
    if (vkCreateXlibSurfaceKHR(instance_, &createInfo, nullptr, &surface_) != VK_SUCCESS) {
        std::cerr << "failed to create Xlib window surface!" << std::endl;
        return false;
    }
#endif
    return true;
}

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

static QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& qf : queueFamilies) {
        if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }
        ++i;
    }
    return indices;
}

bool VulkanBackend::pick_physical_device() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
    if (deviceCount == 0) {
        std::cerr << "failed to find GPUs with Vulkan support!" << std::endl;
        return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

    for (const auto& dev : devices) {
        QueueFamilyIndices indices = find_queue_families(dev, surface_);
        if (indices.isComplete()) {
            physical_device_ = dev;
            break;
        }
    }

    if (physical_device_ == VK_NULL_HANDLE) {
        std::cerr << "failed to find a suitable GPU!" << std::endl;
        return false;
    }

    return true;
}

bool VulkanBackend::create_logical_device() {
    QueueFamilyIndices indices = find_queue_families(physical_device_, surface_);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t qf : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = qf;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physical_device_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
        std::cerr << "failed to create logical device!" << std::endl;
        return false;
    }

    vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphics_queue_);
    vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &present_queue_);

    return true;
}

bool VulkanBackend::create_swapchain() {
    // Query support details (simplified)
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &formatCount, formats.data());

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device_, surface_, &presentModeCount, presentModes.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    VkExtent2D extent = capabilities.currentExtent;
    if (extent.width == UINT32_MAX) {
        // Query native window size for fallback – only implemented for Win32 here
#if defined(_WIN32)
        // Avoid pulling in windows.h here; treat native_window_ as HWND-sized pointer and use a conservative default.
        // If platform requires exact size query, include windows.h in the TU that needs it.
        extent.width = 640;
        extent.height = 480;
#else
        extent.width = 640;
        extent.height = 480;
#endif
    }

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = find_queue_families(physical_device_, surface_);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapchain_) != VK_SUCCESS) {
        std::cerr << "failed to create swap chain!" << std::endl;
        return false;
    }

    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
    swapchain_images_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapchain_images_.data());

    swapchain_image_format_ = surfaceFormat.format;
    swapchain_extent_ = extent;

    return true;
}

bool VulkanBackend::create_image_views() {
    swapchain_image_views_.resize(swapchain_images_.size());
    for (size_t i = 0; i < swapchain_images_.size(); ++i) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchain_images_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchain_image_format_;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device_, &createInfo, nullptr, &swapchain_image_views_[i]) != VK_SUCCESS) {
            std::cerr << "failed to create image views!" << std::endl;
            return false;
        }
    }
    return true;
}

bool VulkanBackend::create_render_pass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain_image_format_;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, &render_pass_) != VK_SUCCESS) {
        std::cerr << "failed to create render pass!" << std::endl;
        return false;
    }

    return true;
}

// Helper to create shader module from SPV bytes
VkShaderModule createShaderModule(VkDevice device, const std::vector<uint8_t>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

bool VulkanBackend::CreateTexturedPipeline(const std::string &vertSpvPath, const std::string &fragSpvPath) {
    // Load SPV files
    auto loadFile = [](const std::string &path) -> std::vector<uint8_t> {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) return {};
        size_t fileSize = (size_t)file.tellg();
        std::vector<uint8_t> buffer(fileSize);
        file.seekg(0);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        file.close();
        return buffer;
    };

    auto vertCode = loadFile(vertSpvPath);
    auto fragCode = loadFile(fragSpvPath);
    if (vertCode.empty() || fragCode.empty()) return false;

    VkShaderModule vertModule = createShaderModule(device_, vertCode);
    VkShaderModule fragModule = createShaderModule(device_, fragCode);
    if (vertModule == VK_NULL_HANDLE || fragModule == VK_NULL_HANDLE) return false;

    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertModule;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragModule;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertStage, fragStage };

    // Vertex input binding (pos x,y,z, uv u,v)
    VkVertexInputBindingDescription bindingDesc{};
    bindingDesc.binding = 0;
    bindingDesc.stride = sizeof(float) * 5;
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> attributeDescs(2);
    attributeDescs[0].binding = 0; attributeDescs[0].location = 0; attributeDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT; attributeDescs[0].offset = 0;
    attributeDescs[1].binding = 0; attributeDescs[1].location = 1; attributeDescs[1].format = VK_FORMAT_R32G32_SFLOAT; attributeDescs[1].offset = sizeof(float)*3;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescs.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescs.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f; viewport.y = 0.0f;
    viewport.width = (float)swapchain_extent_.width;
    viewport.height = (float)swapchain_extent_.height;
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;

    VkRect2D scissor{}; scissor.offset = {0,0}; scissor.extent = swapchain_extent_;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1; viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1; viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Descriptor set layout for combined image sampler
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
        std::cerr << "failed to create descriptor set layout!" << std::endl;
        return false;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;

    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
        std::cerr << "failed to create pipeline layout!" << std::endl;
        return false;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout_;
    pipelineInfo.renderPass = render_pass_;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS) {
        std::cerr << "failed to create graphics pipeline!" << std::endl;
        return false;
    }

    vkDestroyShaderModule(device_, fragModule, nullptr);
    vkDestroyShaderModule(device_, vertModule, nullptr);

    return true;
}

bool VulkanBackend::create_framebuffers() {
    swapchain_framebuffers_.resize(swapchain_image_views_.size());

    for (size_t i = 0; i < swapchain_image_views_.size(); ++i) {
        VkImageView attachments[] = { swapchain_image_views_[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass_;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchain_extent_.width;
        framebufferInfo.height = swapchain_extent_.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &swapchain_framebuffers_[i]) != VK_SUCCESS) {
            std::cerr << "failed to create framebuffer!" << std::endl;
            return false;
        }
    }

    return true;
}

bool VulkanBackend::create_command_pool_and_buffers() {
    QueueFamilyIndices indices = find_queue_families(physical_device_, surface_);
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    if (vkCreateCommandPool(device_, &poolInfo, nullptr, &command_pool_) != VK_SUCCESS) {
        std::cerr << "failed to create command pool!" << std::endl;
        return false;
    }

    command_buffers_.resize(swapchain_images_.size());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

    if (vkAllocateCommandBuffers(device_, &allocInfo, command_buffers_.data()) != VK_SUCCESS) {
        std::cerr << "failed to allocate command buffers!" << std::endl;
        return false;
    }

    // create descriptor pool for textures (will be small)
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = static_cast<uint32_t>(std::max<size_t>(gpu_textures_.size() + 4, 16));

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = &poolSize;
    descriptorPoolInfo.maxSets = static_cast<uint32_t>(std::max<size_t>(gpu_textures_.size() + 4, 16));

    if (vkCreateDescriptorPool(device_, &descriptorPoolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        std::cerr << "failed to create descriptor pool" << std::endl;
        descriptorPool_ = VK_NULL_HANDLE;
    }

    // create quad vertex buffer (unit quad in pixel coordinates will be supplied per-draw via push constants)
    struct Vertex { float x,y,z,u,v; };
    Vertex quad[6] = {
        {0,0,0, 0.0f,0.0f}, {0,1,0, 0.0f,1.0f}, {1,1,0, 1.0f,1.0f},
        {1,1,0, 1.0f,1.0f}, {1,0,0, 1.0f,0.0f}, {0,0,0, 0.0f,0.0f}
    };
    VkDeviceSize bufferSize = sizeof(quad);
    VkBuffer staging;
    VkDeviceMemory stagingMem;
    if (!::gfx::create_buffer(device_, physical_device_, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, stagingMem)) {
        std::cerr << "failed to create quad staging buffer" << std::endl;
    } else {
        void* data; vkMapMemory(device_, stagingMem, 0, bufferSize, 0, &data); memcpy(data, quad, (size_t)bufferSize); vkUnmapMemory(device_, stagingMem);
        // create vertex buffer
        if (create_buffer(device_, physical_device_, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quadVertexBuffer_, quadVertexMemory_)) {
            // copy staging to vertex buffer
            VkCommandBufferAllocateInfo ai{}; ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandPool = command_pool_; ai.commandBufferCount = 1;
            VkCommandBuffer cmd; vkAllocateCommandBuffers(device_, &ai, &cmd);
            VkCommandBufferBeginInfo bi{}; bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &bi);
            VkBufferCopy copy{}; copy.srcOffset = 0; copy.dstOffset = 0; copy.size = bufferSize;
            vkCmdCopyBuffer(cmd, staging, quadVertexBuffer_, 1, &copy);
            vkEndCommandBuffer(cmd);
            VkSubmitInfo si{}; si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
            vkQueueSubmit(graphics_queue_, 1, &si, VK_NULL_HANDLE); vkQueueWaitIdle(graphics_queue_);
            vkFreeCommandBuffers(device_, command_pool_, 1, &cmd);
        }
        vkDestroyBuffer(device_, staging, nullptr); vkFreeMemory(device_, stagingMem, nullptr);
    }

    // If we have bundled SPIR-V available at compile time, try to create the textured pipeline
    // by default we look for embedded arrays in textured_shaders.cpp
#if __has_include("textured_shaders.h")
#include "textured_shaders.h"
    try {
        auto vert = get_textured_vert_spv();
        auto frag = get_textured_frag_spv();
        if (!vert.empty() && !frag.empty()) {
            CreateTexturedPipelineFromBytes(vert, frag);
        }
    } catch(...) {
        // ignore if header not available
    }
#endif

    return true;
}

bool VulkanBackend::create_sync_objects() {
    image_available_semaphores_.resize(frames_in_flight_);
    render_finished_semaphores_.resize(frames_in_flight_);
    in_flight_fences_.resize(frames_in_flight_);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < frames_in_flight_; ++i) {
        if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &image_available_semaphores_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &render_finished_semaphores_[i]) != VK_SUCCESS ||
            vkCreateFence(device_, &fenceInfo, nullptr, &in_flight_fences_[i]) != VK_SUCCESS) {
            std::cerr << "failed to create synchronization objects for a frame!" << std::endl;
            return false;
        }
    }

    return true;
}

void VulkanBackend::cleanup_swapchain() {
    for (auto fb : swapchain_framebuffers_) {
        if (fb != VK_NULL_HANDLE) vkDestroyFramebuffer(device_, fb, nullptr);
    }
    swapchain_framebuffers_.clear();

    if (render_pass_ != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device_, render_pass_, nullptr);
        render_pass_ = VK_NULL_HANDLE;
    }

    for (auto iv : swapchain_image_views_) {
        if (iv != VK_NULL_HANDLE) vkDestroyImageView(device_, iv, nullptr);
    }
    swapchain_image_views_.clear();

    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }

    swapchain_images_.clear();
}

} // namespace gfx
