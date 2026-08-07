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

// Runtime-switchable Vulkan/OpenGL canvas.  Derives from xlGLCanvas so
// GRAPHICS_BASE_CLASS consumers keep a single compile-time base class; the
// backend is chosen once at startup (preference / XL_GRAPHICS_BACKEND env,
// gated on Vulkan availability).  When OpenGL is selected — or any Vulkan
// setup step fails — every override defers to the inherited GL path.

#ifdef HAVE_VULKAN

#include <vector>

#include "../opengl/xlGLCanvas.h"
#include "graphics/vulkan/IVulkanCanvas.h"

// VmaAllocation without pulling the heavy VMA header into this UI header
// (identical to VK_DEFINE_HANDLE; a repeated typedef of the same type is legal
// if vk_mem_alloc.h is also included in the same TU).
typedef struct VmaAllocation_T* VmaAllocation;

class xlVulkanCanvas : public xlGLCanvas, public IVulkanCanvas {
public:
    xlVulkanCanvas(wxWindow* parent,
                   wxWindowID id,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = 0,
                   const wxString& name = wxPanelNameStr,
                   bool only2d = true);
    xlVulkanCanvas(wxWindow* parent,
                   const wxGLAttributes& dispAttrs,
                   const wxString& name = wxPanelNameStr);
    virtual ~xlVulkanCanvas();

    // The one-time backend decision: preference/env, then Vulkan availability.
    static bool VulkanSelected();

    virtual void PrepareCanvas() override;
    virtual xlGraphicsContext* PrepareContextForDrawing() override;
    virtual xlGraphicsContext* PrepareContextForDrawing(const xlColor& bg) override;
    virtual void FinishDrawing(xlGraphicsContext* ctx, bool display = true) override;

    // Shadows of non-virtual xlGLCanvas methods: consumers' static type is
    // xlVulkanCanvas (GRAPHICS_BASE_CLASS), so these intercept and route.
    void SetCurrentGLContext();
    wxImage* GrabImage(wxSize size = wxSize(0, 0));
    void captureNextFrame(int w, int h);
    bool getFrameForExport(VideoWriterFrame& frame);
    std::string getName() const override { return xlGLCanvas::getName(); }
    double translateToBacking(double x) const override { return xlGLCanvas::translateToBacking(x); }

    // Virtual in both bases; consumer overrides remain the final overrider.
    virtual xlColor ClearBackgroundColor() const override { return xlGLCanvas::ClearBackgroundColor(); }
    virtual bool drawingUsingLogicalSize() const override { return true; }
    virtual bool RequiresDepthBuffer() const override { return false; }

    // IVulkanCanvas
    virtual VkCommandBuffer getFrameCommandBuffer() override { return frameCmd; }
    virtual VkExtent2D getFrameExtent() override { return swapExtent; }
    virtual VulkanPipelineCache* getPipelineCache() override;

protected:
    bool usingVulkan() const;
    bool initVulkan();
    bool createSwapchain();
    void destroySwapchain();
    void destroyVulkan();
    bool beginFrame(const xlColor& bg);

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkExtent2D swapExtent = { 0, 0 };
    std::vector<VkImage> swapImages;
    std::vector<VkImageView> swapViews;
    std::vector<VkFramebuffer> swapFramebuffers;
    // Shared MSAA color + depth targets (recreated with the swapchain).  The
    // MSAA color is absent when the device only supports 1x (no resolve).
    VkImage msaaImage = VK_NULL_HANDLE;
    VmaAllocation msaaAlloc = VK_NULL_HANDLE;
    VkImageView msaaView = VK_NULL_HANDLE;
    VkImage depthImage = VK_NULL_HANDLE;
    VmaAllocation depthAlloc = VK_NULL_HANDLE;
    VkImageView depthView = VK_NULL_HANDLE;
    std::vector<VkSemaphore> renderFinished; // per swapchain image
    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkFence frameFence = VK_NULL_HANDLE;
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    VkCommandBuffer frameCmd = VK_NULL_HANDLE;
    uint32_t currentImage = UINT32_MAX;
    bool frameActive = false;     // a Vulkan frame is recording
    bool swapchainStale = false;  // recreate before the next frame
    bool vulkanInited = false;
    bool vulkanFailed = false;    // permanent per-canvas fallback to GL

    // Readback/capture: a frame's resolved swapchain image is copied to this
    // host buffer inside FinishDrawing when a capture was requested.
    bool captureRequested = false;
    bool swapchainCanCapture = false; // surface supports TRANSFER_SRC
    VkBuffer captureBuffer = VK_NULL_HANDLE;
    VmaAllocation captureAlloc = VK_NULL_HANDLE;
    void* captureMapped = nullptr;
    VkDeviceSize captureBufferSize = 0;
    uint32_t captureW = 0;
    uint32_t captureH = 0;
    bool ensureCaptureBuffer(uint32_t w, uint32_t h);
    // Convert the last captured BGRA image to RGB at dst size (rescaling if
    // needed).  Returns false when no capture is available.
    bool captureToRGB(uint32_t dstW, uint32_t dstH, uint8_t* rgb);
};

#endif
