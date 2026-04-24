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

#include <string>
#include <vector>
#include <cstdint>

#include <wx/panel.h>

#include "graphics/xlGraphicsContext.h"
#include "utils/Color.h"

extern "C" {
    struct AVFrame;
}

class wxImage;

// Forward declarations for Vulkan types — xlVkCanvas.cpp includes vulkan.h directly.
// Using VK_DEFINE_HANDLE-style opaque pointers so the header stays Vulkan-free.
struct VkInstance_T;        using VkInstance        = VkInstance_T*;
struct VkPhysicalDevice_T;  using VkPhysicalDevice  = VkPhysicalDevice_T*;
struct VkDevice_T;          using VkDevice          = VkDevice_T*;
struct VkQueue_T;           using VkQueue           = VkQueue_T*;
struct VkSurfaceKHR_T;      using VkSurfaceKHR      = VkSurfaceKHR_T*;
struct VkSwapchainKHR_T;    using VkSwapchainKHR    = VkSwapchainKHR_T*;
struct VkRenderPass_T;      using VkRenderPass      = VkRenderPass_T*;
struct VkFramebuffer_T;     using VkFramebuffer     = VkFramebuffer_T*;
struct VkCommandPool_T;     using VkCommandPool     = VkCommandPool_T*;
struct VkCommandBuffer_T;   using VkCommandBuffer   = VkCommandBuffer_T*;
struct VkSemaphore_T;       using VkSemaphore       = VkSemaphore_T*;
struct VkFence_T;           using VkFence           = VkFence_T*;
struct VkImage_T;           using VkImage           = VkImage_T*;
struct VkImageView_T;       using VkImageView       = VkImageView_T*;
struct VkDeviceMemory_T;    using VkDeviceMemory    = VkDeviceMemory_T*;
struct VkBuffer_T;          using VkBuffer          = VkBuffer_T*;
struct VkDescriptorPool_T;  using VkDescriptorPool  = VkDescriptorPool_T*;
struct VkDescriptorSetLayout_T; using VkDescriptorSetLayout = VkDescriptorSetLayout_T*;
struct VkPipelineLayout_T;  using VkPipelineLayout  = VkPipelineLayout_T*;
struct VkPipeline_T;        using VkPipeline        = VkPipeline_T*;

// VkFormat is an enum in vulkan.h; expose as uint32_t here.
using VkFormatOpaque = uint32_t;


struct VkSharedState {
    VkInstance        instance        = nullptr;
    VkPhysicalDevice  physicalDevice  = nullptr;
    VkDevice          device          = nullptr;
    VkQueue           graphicsQueue   = nullptr;
    VkQueue           presentQueue    = nullptr;
    uint32_t          graphicsFamily  = UINT32_MAX;
    uint32_t          presentFamily   = UINT32_MAX;
    VkCommandPool     commandPool     = nullptr;
    VkDescriptorPool  descriptorPool  = nullptr;
    // Descriptor set layouts shared by all canvases/contexts
    VkDescriptorSetLayout textureDSL  = nullptr; // set=0: combined sampler
    VkDescriptorSetLayout meshDSL     = nullptr; // set=0: UBO (NM), set=1: sampler (optional)
    // Pipeline layouts (push_constant + descriptor sets)
    VkPipelineLayout  basicLayout     = nullptr; // no descriptor sets
    VkPipelineLayout  textureLayout   = nullptr; // textureDSL
    VkPipelineLayout  meshLayout      = nullptr; // meshDSL
};


class xlVkCanvas : public wxPanel {
public:
    xlVkCanvas(wxWindow* parent,
               wxWindowID id        = wxID_ANY,
               const wxPoint& pos   = wxDefaultPosition,
               const wxSize& size   = wxDefaultSize,
               long style           = 0,
               const wxString& name = wxPanelNameStr,
               bool only2d          = true);
    virtual ~xlVkCanvas();

    int    getWidth()  const { return (int)mWindowWidth; }
    int    getHeight() const { return (int)mWindowHeight; }

    const std::string& getName() const { return mName; }


    double translateToBacking(double x)   const;
    double mapLogicalToAbsolute(double x) const;

    virtual xlColor ClearBackgroundColor() const { return xlBLACK; }
    virtual bool    drawingUsingLogicalSize() const { return true; }
    virtual bool    RequiresDepthBuffer()     const { return false; }

    // Frame capture (for video export)
    void     captureNextFrame(int w, int h) { mCaptureW = w; mCaptureH = h; mCapture = true; }
    bool     getFrameForExport(int w, int h, AVFrame*, uint8_t* buffer, int bufferSize);
    wxImage* GrabImage(wxSize size = wxSize(0, 0));

    virtual void render() {}

    // Canvas lifecycle (mirror GL / Metal API)
    virtual void PrepareCanvas();
    virtual xlGraphicsContext* PrepareContextForDrawing();
    virtual xlGraphicsContext* PrepareContextForDrawing(const xlColor& bg);
    virtual void FinishDrawing(xlGraphicsContext* ctx, bool display = true);

    void Resized(wxSizeEvent& evt);

    // Access shared Vulkan state for graphics context use.
    static VkSharedState& GetShared() { return s_shared; }

    // Per-canvas accessors used by xlVkGraphicsContext
    VkRenderPass  getRenderPass()       const { return mRenderPass; }
    VkCommandBuffer getCurrentCmdBuf()  const { return mCmdBufs[mCurrentFrame]; }
    uint32_t        getCurrentFrame()   const { return mCurrentFrame; }
    bool Is3D() const {
        return is3d;
    }
    double          contentScale()      const;

    // Readback: copy current swapchain image to a CPU-accessible buffer.
    // Returns a malloc'd RGBA buffer (caller frees) or nullptr on failure.
    uint8_t* ReadbackFrame(int& outW, int& outH);

    bool IsCoreProfile() {
        return true;
    }

protected:
    DECLARE_EVENT_TABLE()

    void OnEraseBackground(wxEraseEvent& evt) {}

    // Vulkan setup helpers
    bool EnsureInitialized();
    bool CreateSurface();
    bool CreateSwapchain();
    bool CreateDepthResources();
    bool CreateRenderPass();
    bool CreateFramebuffers();
    bool AllocateCommandBuffers();
    bool CreateSyncObjects();
    void CleanupSwapchain();
    void RecreateSwapchain();

    static bool InitializeInstance();
    static bool InitializeSharedState(VkSurfaceKHR surface);
    static void DestroySharedState();

    size_t mWindowWidth   = 0;
    size_t mWindowHeight  = 0;
    int    mWindowResized = false;
    bool   mIsInitialized = false;
    bool is3d = false;
    std::string mName;

    // Per-canvas Vulkan objects
    VkSurfaceKHR     mSurface    = nullptr;
    VkSwapchainKHR   mSwapchain  = nullptr;
    VkFormatOpaque   mSwapFormat = 0;
    VkFormatOpaque   mDepthFormat = 0;
    std::vector<VkImage>     mSwapImages;
    std::vector<VkImageView> mSwapImageViews;
    VkImage       mDepthImage   = nullptr;
    VkDeviceMemory mDepthMemory = nullptr;
    VkImageView    mDepthView   = nullptr;
    VkRenderPass   mRenderPass  = nullptr;
    std::vector<VkFramebuffer> mFramebuffers;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    VkCommandBuffer mCmdBufs[MAX_FRAMES_IN_FLIGHT]          = {};
    VkSemaphore     mImageAvailable[MAX_FRAMES_IN_FLIGHT]   = {};
    VkSemaphore     mRenderFinished[MAX_FRAMES_IN_FLIGHT]   = {};
    VkFence         mInFlightFences[MAX_FRAMES_IN_FLIGHT]   = {};
    std::vector<VkFence> mImagesInFlight; // tracks which fence protects each image

    uint32_t mCurrentFrame       = 0;
    uint32_t mCurrentImageIndex  = 0;

    bool mCapture = false;
    int  mCaptureW = 0;
    int  mCaptureH = 0;

    // Shared Vulkan state (one instance/device for all canvases)
    static VkSharedState s_shared;
    static int           s_canvasCount;
};
