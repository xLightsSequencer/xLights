#pragma once

// Minimal Vulkan backend using volk loader.
// Cross-platform (Windows + Linux) swapchain support using native surfaces.
// The backend exposes a very small OpenGL-like API: Init(native_window), Shutdown(), ClearColor(), BeginFrame(), EndFrame().

#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#if defined(_WIN32)
#  ifndef VK_USE_PLATFORM_WIN32_KHR
#    define VK_USE_PLATFORM_WIN32_KHR
#  endif
#endif
#include <volk/volk.h>
#if !defined(_WIN32)
#  include <X11/Xlib.h>
#endif
#include <array>
#include <optional>
#include <vector>
#include <string>
#include <memory>

namespace gfx {

class VulkanBackend {
public:
    VulkanBackend();
    ~VulkanBackend();

    // Initialize the backend with a native window handle. On Windows, pass HWND as native_window and nullptr for native_display.
    // On Linux with X11, pass (Window) native_window and (Display*) native_display.
    bool Init(void* native_window, void* native_display = nullptr, uint32_t frames_in_flight = 2);

    // Cleanup
    void Shutdown();

    // Set clear color (0..1)
    void ClearColor(float r, float g, float b, float a = 1.0f);

    // Start a frame. Returns true if ready to record draw calls.
    bool BeginFrame();

    // End frame and present.
    bool EndFrame();

    // Resize must be called when window size changes (framebuffer resized).
    void Resize(int width, int height);

    // Texture helpers (public so higher-level contexts can upload/queue textures)
    // Create a GPU texture from pixel data (RGBA8). Returns texture id or -1 on failure.
    int CreateTextureFromPixels(const uint8_t* pixels, uint32_t w, uint32_t h);

    // Enqueue a copy of a GPU texture to the next acquired swapchain image (dst rectangle).
    void QueueCopyTextureToSwapchain(int textureId, uint32_t dstX, uint32_t dstY, uint32_t dstW, uint32_t dstH);

    bool IsInitialized() const { return initialized_; }
    // Read pixels from a swapchain image into CPU memory (RGBA8). If imageIndex is UINT32_MAX,
    // use the currently acquired image if available, otherwise the first image.
    bool ReadSwapchainImagePixels(std::vector<uint8_t> &outPixels, uint32_t imageIndex = UINT32_MAX);
    // Async readback API using staging buffers.
    // StartReadback returns a request id >=0 on success, or -1 on failure.
    int StartReadback(uint32_t imageIndex = UINT32_MAX);
    // Try to retrieve completed readback. Returns true and fills outPixels when ready; false if not ready or invalid id.
    bool TryGetReadback(int requestId, std::vector<uint8_t> &outPixels);

    // Buffer helpers: create a VkBuffer + VkDeviceMemory pair and update their contents.
    bool CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &outBuffer, VkDeviceMemory &outMemory);
    bool UpdateBufferData(VkDeviceMemory memory, VkDeviceSize offset, const void* data, VkDeviceSize size);
    void DestroyBuffer(VkBuffer buffer, VkDeviceMemory memory);

private:
    struct ReadbackRequest {
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
        VkFence fence = VK_NULL_HANDLE;
        uint32_t imageIndex = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        VkDeviceSize size = 0;
    };
    std::vector<std::unique_ptr<ReadbackRequest>> readback_requests_;

    bool create_instance();
    bool create_surface();
    bool pick_physical_device();
    bool create_logical_device();
    bool create_swapchain();
    bool create_image_views();
    bool create_render_pass();
    bool create_framebuffers();
    bool create_sync_objects();
    bool create_command_pool_and_buffers();
    void cleanup_swapchain();

    VkShaderModule createShaderModuleFromWords(VkDevice device, const std::vector<uint32_t>& code);

    // Texture helpers
    struct GpuTexture {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        uint32_t width = 0;
        uint32_t height = 0;
        VkSampler sampler = VK_NULL_HANDLE;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        // CPU-side copy of pixels (RGBA8) to support CPU composition fallback
        std::vector<uint8_t> cpu_pixels;
    };
    // (texture helpers declared public earlier)

    struct PendingCopy { GpuTexture* srcTexture; uint32_t dstX, dstY, dstW, dstH; uint32_t dstImageIndex; };

    std::vector<std::unique_ptr<GpuTexture>> gpu_textures_;
    std::vector<PendingCopy> pending_copies_;
    // Draw queue for CPU-side composition
    struct PendingDrawPixels { const uint8_t* pixels; uint32_t srcW, srcH; uint32_t dstX, dstY, dstW, dstH; float brightness; uint8_t alpha; };
    std::vector<PendingDrawPixels> pending_draws_;

    // CPU backbuffer for composition (RGBA8)
    std::vector<uint8_t> cpu_backbuffer_pixels_;

    // Pipeline and descriptors for textured quad rendering
    VkDescriptorSetLayout descriptorSetLayout_ = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;
    // Optional color pipeline for vertex-colored draws
    VkPipelineLayout colorPipelineLayout_ = VK_NULL_HANDLE;
    VkPipeline colorPipeline_ = VK_NULL_HANDLE;
    VkSampler sampler_ = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool_ = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> textureDescriptorSets_;
    VkBuffer quadVertexBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory quadVertexMemory_ = VK_NULL_HANDLE;

    // Create graphics pipeline from SPIR-V files. Returns true on success.
    bool CreateTexturedPipeline(const std::string &vertSpvPath, const std::string &fragSpvPath);
    // Create textured pipeline from embedded SPIR-V byte arrays
    bool CreateTexturedPipelineFromBytes(const std::vector<uint32_t> &vertSpv, const std::vector<uint32_t> &fragSpv);
    // Create a simple color pipeline (vertex pos + color attribute). Returns true on success.
    bool CreateColorPipelineFromBytes(const std::vector<uint32_t> &vertSpv, const std::vector<uint32_t> &fragSpv);

    enum PrimitiveType { PRIM_POINTS = 0, PRIM_TRIANGLES = 1, PRIM_LINES = 2 };
    // Queue an interleaved vertex/color buffer (pos.xyz + color.rgba as floats) for drawing.
    // vertexColorData: pointer to float array with (3+4)=7 floats per vertex.
    bool QueueIndexedDraw(const void* vertexColorData, VkDeviceSize dataSize, uint32_t vertexCount, PrimitiveType prim, float pointSize);
    // Queue an already-created VkBuffer (and its VkDeviceMemory) for drawing. Backend will take ownership and destroy after use.
    bool QueueIndexedDrawBuffer(VkBuffer buffer, VkDeviceMemory memory, uint32_t vertexCount, PrimitiveType prim, float pointSize);

    // Buffers scheduled for destruction when a frame fence signals (size = frames_in_flight_)
    std::vector<std::vector<std::pair<VkBuffer, VkDeviceMemory>>> buffers_to_destroy_;
    // Pending indexed draws enqueued by contexts
    struct PendingIndexedDrawEntry { VkBuffer buffer; VkDeviceMemory memory; uint32_t vertexCount; PrimitiveType prim; float pointSize; uint32_t dstImageIndex; };
    std::vector<PendingIndexedDrawEntry> pending_indexed_draws_;

private:
    void* native_window_ = nullptr;
    void* native_display_ = nullptr; // X11 Display* on Linux, nullptr on Windows
    uint32_t width_ = 0;
    uint32_t height_ = 0;

    VkInstance instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSurfaceKHR surface_ = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat swapchain_image_format_ = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchain_extent_ = {};
    std::vector<VkImage> swapchain_images_;
    std::vector<VkImageView> swapchain_image_views_;
    std::vector<VkFramebuffer> swapchain_framebuffers_;

    VkRenderPass render_pass_ = VK_NULL_HANDLE;

    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers_;

    // Sync per-frame
    uint32_t frames_in_flight_ = 2;
    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> in_flight_fences_;
    size_t current_frame_ = 0;
    uint32_t current_image_index_ = UINT32_MAX;

    std::array<float,4> clear_color_ = {0.0f, 0.0f, 0.0f, 1.0f};

    bool framebuffer_resized_ = false;
    bool initialized_ = false;
};

} // namespace gfx
