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

#if defined(_WIN32) && !defined(VK_USE_PLATFORM_WIN32_KHR)
// Must precede any volk.h include in this TU so the Win32 surface entry
// points are declared.
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "xlVulkanCanvas.h"

#include <wx/image.h>

#include <spdlog/spdlog.h>

#include "media/VideoWriter.h"
#include "settings/XLightsConfigAdapter.h"

#ifndef VMA_STATIC_VULKAN_FUNCTIONS
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#endif
#ifndef VMA_DYNAMIC_VULKAN_FUNCTIONS
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif
#include <vk_mem_alloc.h>

#include "graphics/vulkan/VulkanPipelineCache.h"
#include "graphics/vulkan/xlVulkanGraphicsContext.h"
#include "effects/vulkan/VulkanComputeUtilities.h"

xlVulkanCanvas::xlVulkanCanvas(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                               const wxSize& size, long style, const wxString& name, bool only2d) :
    xlGLCanvas(parent, id, pos, size, style, name, only2d) {
}

xlVulkanCanvas::xlVulkanCanvas(wxWindow* parent, const wxGLAttributes& dispAttrs, const wxString& name) :
    xlGLCanvas(parent, dispAttrs, name) {
}

xlVulkanCanvas::~xlVulkanCanvas() {
    destroyVulkan();
}

bool xlVulkanCanvas::VulkanSelected() {
    static int cached = -1;
    if (cached == -1) {
        cached = 0;
        // Persisted with the rest of xLights' settings (settings.json via the
        // config adapter), set from Preferences > Other > "Preview graphics".
        std::string pref = GetXLightsConfig()->Read("xLightsGraphicsBackend", "OpenGL");
        bool want = wxString(pref).CmpNoCase("Vulkan") == 0;
        const char* env = getenv("XL_GRAPHICS_BACKEND");
        if (env != nullptr) {
            want = wxString(env).CmpNoCase("Vulkan") == 0;
        }
        if (want) {
            if (VulkanPipelineCache::INSTANCE.ensureInit()) {
                cached = 1;
                spdlog::info("Graphics backend: Vulkan ({})", VulkanComputeUtilities::INSTANCE.deviceName);
            } else {
                spdlog::warn("Vulkan graphics backend requested but unavailable, using OpenGL");
            }
        }
    }
    return cached == 1;
}

bool xlVulkanCanvas::usingVulkan() const {
    return !vulkanFailed && VulkanSelected();
}

VulkanPipelineCache* xlVulkanCanvas::getPipelineCache() {
    return &VulkanPipelineCache::INSTANCE;
}

void xlVulkanCanvas::PrepareCanvas() {
    if (usingVulkan()) {
        mIsInitialized = true;
        return;
    }
    xlGLCanvas::PrepareCanvas();
}

void xlVulkanCanvas::SetCurrentGLContext() {
    if (usingVulkan()) {
        return;
    }
    xlGLCanvas::SetCurrentGLContext();
}

bool xlVulkanCanvas::ensureCaptureBuffer(uint32_t w, uint32_t h) {
    VkDeviceSize need = (VkDeviceSize)w * h * 4;
    if (need <= captureBufferSize && captureBuffer != VK_NULL_HANDLE) {
        return true;
    }
    VulkanComputeUtilities& vk = VulkanComputeUtilities::INSTANCE;
    if (captureBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(vk.allocator, captureBuffer, captureAlloc);
        captureBuffer = VK_NULL_HANDLE;
    }
    VkBufferCreateInfo bci = {};
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = need;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VmaAllocationCreateInfo aci = {};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VmaAllocationInfo info = {};
    if (vmaCreateBuffer(vk.allocator, &bci, &aci, &captureBuffer, &captureAlloc, &info) != VK_SUCCESS) {
        captureBuffer = VK_NULL_HANDLE;
        captureBufferSize = 0;
        return false;
    }
    captureMapped = info.pMappedData;
    captureBufferSize = need;
    return true;
}

bool xlVulkanCanvas::captureToRGB(uint32_t dstW, uint32_t dstH, uint8_t* rgb) {
    if (captureMapped == nullptr || captureW == 0 || captureH == 0) {
        return false;
    }
    // The swapchain is B8G8R8A8_UNORM; emit RGB.  Rescale (nearest) when the
    // requested size differs from the captured (window) size.
    const uint8_t* src = (const uint8_t*)captureMapped;
    for (uint32_t y = 0; y < dstH; y++) {
        uint32_t sy = dstH == captureH ? y : (uint32_t)((uint64_t)y * captureH / dstH);
        if (sy >= captureH) sy = captureH - 1;
        const uint8_t* srow = src + (size_t)sy * captureW * 4;
        uint8_t* drow = rgb + (size_t)y * dstW * 3;
        for (uint32_t x = 0; x < dstW; x++) {
            uint32_t sx = dstW == captureW ? x : (uint32_t)((uint64_t)x * captureW / dstW);
            if (sx >= captureW) sx = captureW - 1;
            const uint8_t* p = srow + (size_t)sx * 4;
            drow[x * 3 + 0] = p[2]; // R (from BGRA)
            drow[x * 3 + 1] = p[1]; // G
            drow[x * 3 + 2] = p[0]; // B
        }
    }
    return true;
}

wxImage* xlVulkanCanvas::GrabImage(wxSize size) {
    if (!usingVulkan()) {
        return xlGLCanvas::GrabImage(size);
    }
    if (!initVulkan() || !swapchainCanCapture) {
        return nullptr;
    }
    captureRequested = true;
    render();               // full repaint through the Vulkan path; FinishDrawing copies
    captureRequested = false;
    if (captureMapped == nullptr || captureW == 0) {
        return nullptr;
    }
    int dstW = size.GetWidth() > 0 ? size.GetWidth() : (int)captureW;
    int dstH = size.GetHeight() > 0 ? size.GetHeight() : (int)captureH;
    wxImage* img = new wxImage(dstW, dstH);
    if (!captureToRGB((uint32_t)dstW, (uint32_t)dstH, img->GetData())) {
        delete img;
        return nullptr;
    }
    return img;
}

void xlVulkanCanvas::captureNextFrame(int w, int h) {
    if (usingVulkan()) {
        captureRequested = true;
    } else {
        xlGLCanvas::captureNextFrame(w, h);
    }
}

bool xlVulkanCanvas::getFrameForExport(VideoWriterFrame& frame) {
    if (!usingVulkan()) {
        return xlGLCanvas::getFrameForExport(frame);
    }
    // The paint that ran between captureNextFrame() and here left the pixels
    // in the capture buffer.
    if (frame.rgbBuffer == nullptr || captureMapped == nullptr) {
        return false;
    }
    size_t need = (size_t)frame.width * frame.height * 3;
    if ((size_t)frame.rgbBufferSize < need) {
        return false;
    }
    return captureToRGB((uint32_t)frame.width, (uint32_t)frame.height, frame.rgbBuffer);
}

bool xlVulkanCanvas::initVulkan() {
    if (vulkanInited) {
        return true;
    }
    VulkanComputeUtilities& vk = VulkanComputeUtilities::INSTANCE;

#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hinstance = ::GetModuleHandle(nullptr);
    sci.hwnd = (HWND)GetHWND();
    if (vkCreateWin32SurfaceKHR == nullptr ||
        vkCreateWin32SurfaceKHR(vk.instance, &sci, nullptr, &surface) != VK_SUCCESS) {
        spdlog::warn("Vulkan graphics: surface creation failed for {}, falling back to OpenGL", getName());
        vulkanFailed = true;
        return false;
    }
#else
    // Linux surface plumbing (XID via GTK) is a follow-up; the runtime switch
    // keeps these canvases on the OpenGL path until then.
    vulkanFailed = true;
    return false;
#endif

    VkBool32 presentSupported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(vk.physicalDevice, vk.graphicsQueueFamilyIndex, surface, &presentSupported);
    if (presentSupported != VK_TRUE) {
        spdlog::warn("Vulkan graphics: queue family cannot present to {}, falling back to OpenGL", getName());
        destroyVulkan();
        vulkanFailed = true;
        return false;
    }

    // Ensure the swapchain can use the format all pipelines were built for.
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physicalDevice, surface, &formatCount, formats.data());
    bool formatOk = false;
    for (auto& f : formats) {
        if (f.format == VulkanPipelineCache::INSTANCE.getColorFormat() && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            formatOk = true;
            break;
        }
    }
    if (!formatOk) {
        spdlog::warn("Vulkan graphics: surface does not support the pipeline color format, falling back to OpenGL");
        destroyVulkan();
        vulkanFailed = true;
        return false;
    }

    VkCommandPoolCreateInfo pci = {};
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pci.queueFamilyIndex = vk.graphicsQueueFamilyIndex;
    if (vkCreateCommandPool(vk.device, &pci, nullptr, &cmdPool) != VK_SUCCESS) {
        destroyVulkan();
        vulkanFailed = true;
        return false;
    }
    VkCommandBufferAllocateInfo cai = {};
    cai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cai.commandPool = cmdPool;
    cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cai.commandBufferCount = 1;
    vkAllocateCommandBuffers(vk.device, &cai, &frameCmd);

    VkSemaphoreCreateInfo semci = {};
    semci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(vk.device, &semci, nullptr, &imageAvailable);
    VkFenceCreateInfo fci = {};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(vk.device, &fci, nullptr, &frameFence);

    if (!createSwapchain()) {
        destroyVulkan();
        vulkanFailed = true;
        return false;
    }
    vulkanInited = true;
    return true;
}

bool xlVulkanCanvas::createSwapchain() {
    VulkanComputeUtilities& vk = VulkanComputeUtilities::INSTANCE;
    VulkanPipelineCache& cache = VulkanPipelineCache::INSTANCE;

    VkSurfaceCapabilitiesKHR caps = {};
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.physicalDevice, surface, &caps) != VK_SUCCESS) {
        return false;
    }
    VkExtent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX) {
        extent.width = (uint32_t)std::max(1.0, translateToBacking(GetClientSize().GetWidth()));
        extent.height = (uint32_t)std::max(1.0, translateToBacking(GetClientSize().GetHeight()));
    }
    extent.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, extent.width));
    extent.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, extent.height));
    if (extent.width == 0 || extent.height == 0) {
        // Window minimized/hidden — nothing to present to right now.
        return false;
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
        imageCount = caps.maxImageCount;
    }

    VkSwapchainCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = surface;
    sci.minImageCount = imageCount;
    sci.imageFormat = cache.getColorFormat();
    sci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    sci.imageExtent = extent;
    sci.imageArrayLayers = 1;
    // TRANSFER_SRC (when supported) lets GrabImage / video export copy the
    // presented image back to the CPU.
    swapchainCanCapture = (caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) != 0;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (swapchainCanCapture ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0);
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = swapchain;

    VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
    VkResult res = vkCreateSwapchainKHR(vk.device, &sci, nullptr, &newSwapchain);
    destroySwapchain();
    if (res != VK_SUCCESS) {
        spdlog::warn("Vulkan graphics: swapchain creation failed ({}) for {}", (int)res, getName());
        return false;
    }
    swapchain = newSwapchain;
    swapExtent = extent;

    uint32_t count = 0;
    vkGetSwapchainImagesKHR(vk.device, swapchain, &count, nullptr);
    swapImages.resize(count);
    vkGetSwapchainImagesKHR(vk.device, swapchain, &count, swapImages.data());

    const bool msaa = cache.getSampleCount() != VK_SAMPLE_COUNT_1_BIT;

    // Shared MSAA color + depth targets (one each; the frame is synchronous).
    auto createTarget = [&](VkFormat fmt, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageAspectFlags aspect,
                            VkImage& img, VmaAllocation& alloc, VkImageView& view) -> bool {
        VkImageCreateInfo ici = {};
        ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        ici.imageType = VK_IMAGE_TYPE_2D;
        ici.format = fmt;
        ici.extent = { extent.width, extent.height, 1 };
        ici.mipLevels = 1;
        ici.arrayLayers = 1;
        ici.samples = samples;
        ici.tiling = VK_IMAGE_TILING_OPTIMAL;
        ici.usage = usage;
        ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VmaAllocationCreateInfo aci = {};
        aci.usage = VMA_MEMORY_USAGE_AUTO;
        aci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        if (vmaCreateImage(vk.allocator, &ici, &aci, &img, &alloc, nullptr) != VK_SUCCESS) {
            return false;
        }
        VkImageViewCreateInfo vci = {};
        vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vci.image = img;
        vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vci.format = fmt;
        vci.subresourceRange = { aspect, 0, 1, 0, 1 };
        return vkCreateImageView(vk.device, &vci, nullptr, &view) == VK_SUCCESS;
    };

    if (msaa && !createTarget(cache.getColorFormat(), cache.getSampleCount(),
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                              VK_IMAGE_ASPECT_COLOR_BIT, msaaImage, msaaAlloc, msaaView)) {
        return false;
    }
    if (!createTarget(cache.getDepthFormat(), cache.getSampleCount(),
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
                      VK_IMAGE_ASPECT_DEPTH_BIT, depthImage, depthAlloc, depthView)) {
        return false;
    }

    swapViews.resize(count, VK_NULL_HANDLE);
    swapFramebuffers.resize(count, VK_NULL_HANDLE);
    renderFinished.resize(count, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < count; i++) {
        VkImageViewCreateInfo vci = {};
        vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vci.image = swapImages[i];
        vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vci.format = cache.getColorFormat();
        vci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        if (vkCreateImageView(vk.device, &vci, nullptr, &swapViews[i]) != VK_SUCCESS) {
            return false;
        }
        // Attachment order matches the render pass: [color(MSAA), depth, resolve]
        // with MSAA; [color(=swapchain), depth] without.
        VkImageView atts[3];
        uint32_t attCount;
        if (msaa) {
            atts[0] = msaaView;
            atts[1] = depthView;
            atts[2] = swapViews[i];
            attCount = 3;
        } else {
            atts[0] = swapViews[i];
            atts[1] = depthView;
            attCount = 2;
        }
        VkFramebufferCreateInfo fci = {};
        fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fci.renderPass = cache.getRenderPass();
        fci.attachmentCount = attCount;
        fci.pAttachments = atts;
        fci.width = extent.width;
        fci.height = extent.height;
        fci.layers = 1;
        if (vkCreateFramebuffer(vk.device, &fci, nullptr, &swapFramebuffers[i]) != VK_SUCCESS) {
            return false;
        }
        VkSemaphoreCreateInfo semci = {};
        semci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if (vkCreateSemaphore(vk.device, &semci, nullptr, &renderFinished[i]) != VK_SUCCESS) {
            return false;
        }
    }
    swapchainStale = false;
    return true;
}

void xlVulkanCanvas::destroySwapchain() {
    VulkanComputeUtilities& vk = VulkanComputeUtilities::INSTANCE;
    for (VkFramebuffer f : swapFramebuffers) {
        if (f != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(vk.device, f, nullptr);
        }
    }
    swapFramebuffers.clear();
    for (VkImageView v : swapViews) {
        if (v != VK_NULL_HANDLE) {
            vkDestroyImageView(vk.device, v, nullptr);
        }
    }
    swapViews.clear();
    for (VkSemaphore s : renderFinished) {
        if (s != VK_NULL_HANDLE) {
            vkDestroySemaphore(vk.device, s, nullptr);
        }
    }
    renderFinished.clear();
    swapImages.clear();
    if (msaaView != VK_NULL_HANDLE) {
        vkDestroyImageView(vk.device, msaaView, nullptr);
        msaaView = VK_NULL_HANDLE;
    }
    if (msaaImage != VK_NULL_HANDLE) {
        vmaDestroyImage(vk.allocator, msaaImage, msaaAlloc);
        msaaImage = VK_NULL_HANDLE;
        msaaAlloc = VK_NULL_HANDLE;
    }
    if (depthView != VK_NULL_HANDLE) {
        vkDestroyImageView(vk.device, depthView, nullptr);
        depthView = VK_NULL_HANDLE;
    }
    if (depthImage != VK_NULL_HANDLE) {
        vmaDestroyImage(vk.allocator, depthImage, depthAlloc);
        depthImage = VK_NULL_HANDLE;
        depthAlloc = VK_NULL_HANDLE;
    }
    if (swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(vk.device, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }
}

void xlVulkanCanvas::destroyVulkan() {
    VulkanComputeUtilities& vk = VulkanComputeUtilities::INSTANCE;
    if (vk.device == VK_NULL_HANDLE) {
        return;
    }
    if (vulkanInited || surface != VK_NULL_HANDLE) {
        // Frames are synchronous (fence-waited in FinishDrawing) so the only
        // wait needed here is for safety on teardown paths.
        vkDeviceWaitIdle(vk.device);
    }
    destroySwapchain();
    if (imageAvailable != VK_NULL_HANDLE) {
        vkDestroySemaphore(vk.device, imageAvailable, nullptr);
        imageAvailable = VK_NULL_HANDLE;
    }
    if (frameFence != VK_NULL_HANDLE) {
        vkDestroyFence(vk.device, frameFence, nullptr);
        frameFence = VK_NULL_HANDLE;
    }
    if (cmdPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(vk.device, cmdPool, nullptr);
        cmdPool = VK_NULL_HANDLE;
        frameCmd = VK_NULL_HANDLE;
    }
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(vk.instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }
    if (captureBuffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(vk.allocator, captureBuffer, captureAlloc);
        captureBuffer = VK_NULL_HANDLE;
        captureMapped = nullptr;
        captureBufferSize = 0;
    }
    VulkanPipelineCache::INSTANCE.flushDeferredDestroy();
    vulkanInited = false;
}

bool xlVulkanCanvas::beginFrame(const xlColor& bg) {
    VulkanComputeUtilities& vk = VulkanComputeUtilities::INSTANCE;

    if (swapchainStale || mWindowResized || swapchain == VK_NULL_HANDLE) {
        mWindowResized = false;
        if (!createSwapchain()) {
            return false;
        }
    }

    VkResult res = vkAcquireNextImageKHR(vk.device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &currentImage);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
        if (!createSwapchain()) {
            return false;
        }
        res = vkAcquireNextImageKHR(vk.device, swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &currentImage);
    }
    if (res != VK_SUCCESS) {
        return false;
    }

    vkResetCommandBuffer(frameCmd, 0);
    VkCommandBufferBeginInfo bi = {};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(frameCmd, &bi);

    VkClearValue clears[2] = {};
    clears[0].color = { { bg.red / 255.0f, bg.green / 255.0f, bg.blue / 255.0f, bg.alpha / 255.0f } };
    clears[1].depthStencil = { 1.0f, 0 };
    VkRenderPassBeginInfo rbi = {};
    rbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rbi.renderPass = VulkanPipelineCache::INSTANCE.getRenderPass();
    rbi.framebuffer = swapFramebuffers[currentImage];
    rbi.renderArea = { { 0, 0 }, swapExtent };
    rbi.clearValueCount = 2;
    rbi.pClearValues = clears;
    vkCmdBeginRenderPass(frameCmd, &rbi, VK_SUBPASS_CONTENTS_INLINE);
    frameActive = true;
    return true;
}

xlGraphicsContext* xlVulkanCanvas::PrepareContextForDrawing() {
    return PrepareContextForDrawing(ClearBackgroundColor());
}

xlGraphicsContext* xlVulkanCanvas::PrepareContextForDrawing(const xlColor& bg) {
    if (!usingVulkan()) {
        return xlGLCanvas::PrepareContextForDrawing(bg);
    }
    if (!initVulkan()) {
        // First failure flips this canvas to the GL path permanently.
        return xlGLCanvas::PrepareContextForDrawing(bg);
    }
    if (!beginFrame(bg)) {
        return nullptr;
    }
    return new xlVulkanGraphicsContext(this);
}

void xlVulkanCanvas::FinishDrawing(xlGraphicsContext* ctx, bool display) {
    if (!frameActive) {
        xlGLCanvas::FinishDrawing(ctx, display);
        return;
    }
    frameActive = false;
    VulkanComputeUtilities& vk = VulkanComputeUtilities::INSTANCE;

    vkCmdEndRenderPass(frameCmd);

    // Capture: copy the just-rendered swapchain image (now in PRESENT_SRC) to
    // the host capture buffer before presenting.
    bool didCapture = false;
    if (captureRequested && swapchainCanCapture && ensureCaptureBuffer(swapExtent.width, swapExtent.height)) {
        VkImageMemoryBarrier toSrc = {};
        toSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        toSrc.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        toSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        toSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        toSrc.image = swapImages[currentImage];
        toSrc.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        toSrc.srcAccessMask = 0;
        toSrc.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(frameCmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &toSrc);

        VkBufferImageCopy region = {};
        region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        region.imageExtent = { swapExtent.width, swapExtent.height, 1 };
        vkCmdCopyImageToBuffer(frameCmd, swapImages[currentImage], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, captureBuffer, 1, &region);

        VkImageMemoryBarrier toPresent = toSrc;
        toPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        toPresent.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        toPresent.dstAccessMask = 0;
        vkCmdPipelineBarrier(frameCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0, 0, nullptr, 0, nullptr, 1, &toPresent);
        captureW = swapExtent.width;
        captureH = swapExtent.height;
    }
    captureRequested = false;

    vkEndCommandBuffer(frameCmd);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si = {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &imageAvailable;
    si.pWaitDstStageMask = &waitStage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &frameCmd;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &renderFinished[currentImage];

    VkResult res;
    {
        std::lock_guard<std::mutex> lock(vk.graphicsSubmitMutex());
        res = vkQueueSubmit(vk.graphicsQueue, 1, &si, frameFence);
        if (res == VK_SUCCESS) {
            // The rendered image is presented even when display is false —
            // capture-only frames are handled with offscreen targets in a
            // follow-up; skipping the present would leak the acquired image.
            VkPresentInfoKHR pi = {};
            pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            pi.waitSemaphoreCount = 1;
            pi.pWaitSemaphores = &renderFinished[currentImage];
            pi.swapchainCount = 1;
            pi.pSwapchains = &swapchain;
            pi.pImageIndices = &currentImage;
            VkResult pres = vkQueuePresentKHR(vk.graphicsQueue, &pi);
            if (pres == VK_ERROR_OUT_OF_DATE_KHR || pres == VK_SUBOPTIMAL_KHR) {
                swapchainStale = true;
            }
        }
    }
    if (res == VK_SUCCESS) {
        // Synchronous frame end: makes accumulator/texture destruction and
        // mapped-buffer reuse trivially safe.  Revisit for pipelined frames.
        vkWaitForFences(vk.device, 1, &frameFence, VK_TRUE, UINT64_MAX);
        vkResetFences(vk.device, 1, &frameFence);
    }
    delete ctx;
    VulkanPipelineCache::INSTANCE.flushDeferredDestroy();
}

#endif
