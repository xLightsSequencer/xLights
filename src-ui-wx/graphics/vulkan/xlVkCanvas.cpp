/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Vulkan canvas implementation.
// Requires: Vulkan SDK (vulkan/vulkan.h, libvulkan), shaderc (for runtime GLSL compilation).
// On Windows: VK_KHR_win32_surface extension.
// On Linux:   VK_KHR_xlib_surface or VK_KHR_xcb_surface extension.

// Redefine the opaque-pointer typedefs from the header so that the real vulkan.h
// typedefs (which are VK_DEFINE_HANDLE / VK_DEFINE_NON_DISPATCHABLE_HANDLE macros)
// take precedence here.  We must undef before including vulkan.h.
#undef VkInstance
#undef VkPhysicalDevice
#undef VkDevice
#undef VkQueue
#undef VkSurfaceKHR
#undef VkSwapchainKHR
#undef VkRenderPass
#undef VkFramebuffer
#undef VkCommandPool
#undef VkCommandBuffer
#undef VkSemaphore
#undef VkFence
#undef VkImage
#undef VkImageView
#undef VkDeviceMemory
#undef VkBuffer
#undef VkDescriptorPool
#undef VkDescriptorSetLayout
#undef VkPipelineLayout
#undef VkPipeline

#include <vulkan/vulkan.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif

#include <wx/image.h>
#include <wx/msgdlg.h>
#include <log.h>

#include "xlVkCanvas.h"
#include "xlVkGraphicsContext.h"
#include "UtilFunctions.h"

// ---------------------------------------------------------------------------
// Static members
// ---------------------------------------------------------------------------
VkSharedState xlVkCanvas::s_shared;
int           xlVkCanvas::s_canvasCount = 0;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static void VkCheck(VkResult r, const char* where) {
    if (r != VK_SUCCESS) {
        auto logger = spdlog::get("xlights");
        if (logger) logger->error("Vulkan error {} at {}", (int)r, where);
    }
}
#define VKC(expr) VkCheck((expr), #expr)

static uint32_t FindMemoryType(VkPhysicalDevice pd, uint32_t typeFilter, VkMemoryPropertyFlags props) {
    VkPhysicalDeviceMemoryProperties mp;
    vkGetPhysicalDeviceMemoryProperties(pd, &mp);
    for (uint32_t i = 0; i < mp.memoryTypeCount; ++i) {
        if ((typeFilter & (1u << i)) && (mp.memoryTypes[i].propertyFlags & props) == props)
            return i;
    }
    return UINT32_MAX;
}

static bool CreateBuffer(VkDevice device, VkPhysicalDevice pd,
                         VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props,
                         VkBuffer& buf, VkDeviceMemory& mem) {
    VkBufferCreateInfo bci{};
    bci.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size        = size;
    bci.usage       = usage;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bci, nullptr, &buf) != VK_SUCCESS) return false;

    VkMemoryRequirements mr;
    vkGetBufferMemoryRequirements(device, buf, &mr);

    VkMemoryAllocateInfo mai{};
    mai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize  = mr.size;
    mai.memoryTypeIndex = FindMemoryType(pd, mr.memoryTypeBits, props);
    if (mai.memoryTypeIndex == UINT32_MAX) return false;
    if (vkAllocateMemory(device, &mai, nullptr, &mem) != VK_SUCCESS) return false;
    vkBindBufferMemory(device, buf, mem, 0);
    return true;
}

static VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool pool) {
    VkCommandBufferAllocateInfo ai{};
    ai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandPool        = pool;
    ai.commandBufferCount = 1;
    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(device, &ai, &cmd);
    VkCommandBufferBeginInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &bi);
    return cmd;
}

static void EndSingleTimeCommands(VkDevice device, VkCommandPool pool,
                                   VkQueue queue, VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);
    VkSubmitInfo si{};
    si.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &cmd;
    vkQueueSubmit(queue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device, pool, 1, &cmd);
}

static void TransitionImageLayout(VkDevice device, VkCommandPool pool, VkQueue queue,
                                   VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                                   VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT) {
    VkCommandBuffer cmd = BeginSingleTimeCommands(device, pool);
    VkImageMemoryBarrier barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = aspect;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = 0;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    EndSingleTimeCommands(device, pool, queue, cmd);
}

// ---------------------------------------------------------------------------
// Event table
// ---------------------------------------------------------------------------
BEGIN_EVENT_TABLE(xlVkCanvas, wxPanel)
    EVT_SIZE(xlVkCanvas::Resized)
    EVT_ERASE_BACKGROUND(xlVkCanvas::OnEraseBackground)
END_EVENT_TABLE()

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
xlVkCanvas::xlVkCanvas(wxWindow* parent, wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style, const wxString& name, bool only2d)
    : wxPanel(parent, id, pos, size, style | wxWANTS_CHARS, name), is3d(!only2d)
    , mName(name.ToStdString()) {
    mWindowWidth  = size.GetWidth()  > 0 ? size.GetWidth()  : 100;
    mWindowHeight = size.GetHeight() > 0 ? size.GetHeight() : 100;
    ++s_canvasCount;
}

xlVkCanvas::~xlVkCanvas() {
    if (s_shared.device) {
        vkDeviceWaitIdle(s_shared.device);
    }
    CleanupSwapchain();
    if (mSurface) {
        vkDestroySurfaceKHR(s_shared.instance, mSurface, nullptr);
        mSurface = nullptr;
    }
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (mImageAvailable[i]) vkDestroySemaphore(s_shared.device, mImageAvailable[i], nullptr);
        if (mRenderFinished[i]) vkDestroySemaphore(s_shared.device, mRenderFinished[i], nullptr);
        if (mInFlightFences[i]) vkDestroyFence(s_shared.device, mInFlightFences[i], nullptr);
    }
    if (s_shared.commandPool && mCmdBufs[0]) {
        vkFreeCommandBuffers(s_shared.device, s_shared.commandPool,
                             MAX_FRAMES_IN_FLIGHT, mCmdBufs);
    }
    --s_canvasCount;
    if (s_canvasCount == 0) {
        xlVkGraphicsContext::DestroyPipelines();
        DestroySharedState();
    }
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void*) {
    auto logger = spdlog::get("xlights");
    if (!logger) return VK_FALSE;
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        logger->error("[Vulkan] {}", data->pMessage);
    else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        logger->warn("[Vulkan] {}", data->pMessage);
    else
        logger->debug("[Vulkan] {}", data->pMessage);
    return VK_FALSE;
}

bool xlVkCanvas::InitializeInstance() {
    if (s_shared.instance) return true;  // already done

    auto logger = spdlog::get("xlights");

    // -----------------------------------------------------------------------
    // 1. Create VkInstance
    // -----------------------------------------------------------------------
    std::vector<const char*> instanceExts = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef __linux__
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
    };
#ifndef NDEBUG
    instanceExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    std::vector<const char*> layers;
#ifndef NDEBUG
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (auto& lp : availableLayers) {
        if (std::string(lp.layerName) == validationLayer) {
            layers.push_back(validationLayer);
            break;
        }
    }
#endif

    VkApplicationInfo appInfo{};
    appInfo.sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "xLights";
    appInfo.apiVersion       = VK_API_VERSION_1_0;

    VkInstanceCreateInfo ici{};
    ici.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo        = &appInfo;
    ici.enabledExtensionCount   = (uint32_t)instanceExts.size();
    ici.ppEnabledExtensionNames = instanceExts.data();
    ici.enabledLayerCount       = (uint32_t)layers.size();
    ici.ppEnabledLayerNames     = layers.data();

    if (vkCreateInstance(&ici, nullptr, &s_shared.instance) != VK_SUCCESS) {
        if (logger) logger->error("xlVkCanvas: failed to create VkInstance");
        return false;
    }

    // -----------------------------------------------------------------------
    // 2. Pick physical device (no surface needed yet)
    // -----------------------------------------------------------------------
    uint32_t pdCount = 0;
    vkEnumeratePhysicalDevices(s_shared.instance, &pdCount, nullptr);
    if (pdCount == 0) {
        if (logger) logger->error("xlVkCanvas: no Vulkan physical devices found");
        return false;
    }
    std::vector<VkPhysicalDevice> pds(pdCount);
    vkEnumeratePhysicalDevices(s_shared.instance, &pdCount, pds.data());

    auto pickDevice = [&](VkPhysicalDeviceType preferred) -> VkPhysicalDevice {
        for (auto pd : pds) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(pd, &props);
            if (props.deviceType == preferred) {
                uint32_t extCount = 0;
                vkEnumerateDeviceExtensionProperties(pd, nullptr, &extCount, nullptr);
                std::vector<VkExtensionProperties> exts(extCount);
                vkEnumerateDeviceExtensionProperties(pd, nullptr, &extCount, exts.data());
                for (auto& e : exts) {
                    if (std::string(e.extensionName) == VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                        return pd;
                }
            }
        }
        return nullptr;
    };

    s_shared.physicalDevice = pickDevice(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
    if (!s_shared.physicalDevice)
        s_shared.physicalDevice = pickDevice(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
    if (!s_shared.physicalDevice)
        s_shared.physicalDevice = pds[0];

    return true;
}

bool xlVkCanvas::InitializeSharedState(VkSurfaceKHR firstSurface) {
    // Instance + physical device must already be set up by InitializeInstance().
    if (!s_shared.instance || !s_shared.physicalDevice) return false;
    if (s_shared.device) return true;  // logical device already created

    auto logger = spdlog::get("xlights");

    // Rebuild validation layer list (same logic as InitializeInstance so the
    // logical device is created with the same layers as the instance).
    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    std::vector<const char*> layers;
#ifndef NDEBUG
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    for (auto& lp : availableLayers) {
        if (std::string(lp.layerName) == validationLayer) {
            layers.push_back(validationLayer);
            break;
        }
    }
#endif

    // -----------------------------------------------------------------------
    // 3. Find queue families using the first surface
    // -----------------------------------------------------------------------
    uint32_t qfCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(s_shared.physicalDevice, &qfCount, nullptr);
    std::vector<VkQueueFamilyProperties> qfProps(qfCount);
    vkGetPhysicalDeviceQueueFamilyProperties(s_shared.physicalDevice, &qfCount, qfProps.data());

    s_shared.graphicsFamily = UINT32_MAX;
    s_shared.presentFamily  = UINT32_MAX;
    for (uint32_t i = 0; i < qfCount; ++i) {
        if (qfProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            s_shared.graphicsFamily = i;

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(s_shared.physicalDevice, i, firstSurface, &presentSupport);
        if (presentSupport)
            s_shared.presentFamily = i;

        if (s_shared.graphicsFamily != UINT32_MAX && s_shared.presentFamily != UINT32_MAX)
            break;
    }
    if (s_shared.graphicsFamily == UINT32_MAX || s_shared.presentFamily == UINT32_MAX) {
        if (logger) logger->error("xlVkCanvas: no suitable queue families");
        return false;
    }

    // -----------------------------------------------------------------------
    // 4. Create logical device
    // -----------------------------------------------------------------------
    float qPriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> qcis;
    std::vector<uint32_t> uniqueFamilies = {s_shared.graphicsFamily};
    if (s_shared.presentFamily != s_shared.graphicsFamily)
        uniqueFamilies.push_back(s_shared.presentFamily);

    for (uint32_t fam : uniqueFamilies) {
        VkDeviceQueueCreateInfo qci{};
        qci.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qci.queueFamilyIndex = fam;
        qci.queueCount       = 1;
        qci.pQueuePriorities = &qPriority;
        qcis.push_back(qci);
    }

    VkPhysicalDeviceFeatures devFeatures{};
    devFeatures.wideLines = VK_TRUE;     // for drawPoints size >1 via wide lines fallback

    const char* swapchainExt = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    VkDeviceCreateInfo dci{};
    dci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount    = (uint32_t)qcis.size();
    dci.pQueueCreateInfos       = qcis.data();
    dci.enabledExtensionCount   = 1;
    dci.ppEnabledExtensionNames = &swapchainExt;
    dci.pEnabledFeatures        = &devFeatures;
    dci.enabledLayerCount       = (uint32_t)layers.size();
    dci.ppEnabledLayerNames     = layers.data();

    if (vkCreateDevice(s_shared.physicalDevice, &dci, nullptr, &s_shared.device) != VK_SUCCESS) {
        if (logger) logger->error("xlVkCanvas: failed to create VkDevice");
        return false;
    }

    vkGetDeviceQueue(s_shared.device, s_shared.graphicsFamily, 0, &s_shared.graphicsQueue);
    vkGetDeviceQueue(s_shared.device, s_shared.presentFamily,  0, &s_shared.presentQueue);

    // -----------------------------------------------------------------------
    // 5. Command pool
    // -----------------------------------------------------------------------
    VkCommandPoolCreateInfo cpci{};
    cpci.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.queueFamilyIndex = s_shared.graphicsFamily;
    cpci.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(s_shared.device, &cpci, nullptr, &s_shared.commandPool) != VK_SUCCESS) {
        if (logger) logger->error("xlVkCanvas: failed to create command pool");
        return false;
    }

    // -----------------------------------------------------------------------
    // 6. Descriptor pool (generous limits; shared across all canvases)
    // -----------------------------------------------------------------------
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = 4096;
    poolSizes[1].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = 256;

    VkDescriptorPoolCreateInfo dpci{};
    dpci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    dpci.maxSets       = 4096 + 256;
    dpci.poolSizeCount = (uint32_t)poolSizes.size();
    dpci.pPoolSizes    = poolSizes.data();
    if (vkCreateDescriptorPool(s_shared.device, &dpci, nullptr, &s_shared.descriptorPool) != VK_SUCCESS) {
        if (logger) logger->error("xlVkCanvas: failed to create descriptor pool");
        return false;
    }

    // -----------------------------------------------------------------------
    // 7. Descriptor set layouts
    // -----------------------------------------------------------------------
    // Layout A: single combined image sampler (for texture draw calls)
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding         = 0;
        binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo dsci{};
        dsci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dsci.bindingCount = 1;
        dsci.pBindings    = &binding;
        vkCreateDescriptorSetLayout(s_shared.device, &dsci, nullptr, &s_shared.textureDSL);
    }

    // Layout B: UBO (binding 0, vertex) + sampler (binding 1, fragment) for mesh shaders
    {
        VkDescriptorSetLayoutBinding bindings[2]{};
        bindings[0].binding        = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags     = VK_SHADER_STAGE_VERTEX_BIT;
        bindings[1].binding        = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags     = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo dsci{};
        dsci.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dsci.bindingCount = 2;
        dsci.pBindings    = bindings;
        vkCreateDescriptorSetLayout(s_shared.device, &dsci, nullptr, &s_shared.meshDSL);
    }

    // -----------------------------------------------------------------------
    // 8. Pipeline layouts
    // -----------------------------------------------------------------------
    VkPushConstantRange pcRange{};
    pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pcRange.offset     = 0;
    pcRange.size       = sizeof(VkPushConstants);

    // Basic layout (no descriptor sets)
    {
        VkPipelineLayoutCreateInfo plci{};
        plci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plci.pushConstantRangeCount = 1;
        plci.pPushConstantRanges    = &pcRange;
        vkCreatePipelineLayout(s_shared.device, &plci, nullptr, &s_shared.basicLayout);
    }
    // Texture layout (textureDSL at set 0)
    {
        VkPipelineLayoutCreateInfo plci{};
        plci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plci.setLayoutCount         = 1;
        plci.pSetLayouts            = &s_shared.textureDSL;
        plci.pushConstantRangeCount = 1;
        plci.pPushConstantRanges    = &pcRange;
        vkCreatePipelineLayout(s_shared.device, &plci, nullptr, &s_shared.textureLayout);
    }
    // Mesh layout (meshDSL at set 0)
    {
        VkPipelineLayoutCreateInfo plci{};
        plci.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        plci.setLayoutCount         = 1;
        plci.pSetLayouts            = &s_shared.meshDSL;
        plci.pushConstantRangeCount = 1;
        plci.pPushConstantRanges    = &pcRange;
        vkCreatePipelineLayout(s_shared.device, &plci, nullptr, &s_shared.meshLayout);
    }

    if (logger) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(s_shared.physicalDevice, &props);
        logger->info("xlVkCanvas: initialized with GPU: {}", props.deviceName);
    }
    return true;
}

void xlVkCanvas::DestroySharedState() {
    auto& s = s_shared;
    if (!s.device) return;
    vkDeviceWaitIdle(s.device);

    if (s.meshLayout)    vkDestroyPipelineLayout(s.device, s.meshLayout,    nullptr);
    if (s.textureLayout) vkDestroyPipelineLayout(s.device, s.textureLayout, nullptr);
    if (s.basicLayout)   vkDestroyPipelineLayout(s.device, s.basicLayout,   nullptr);
    if (s.meshDSL)       vkDestroyDescriptorSetLayout(s.device, s.meshDSL,    nullptr);
    if (s.textureDSL)    vkDestroyDescriptorSetLayout(s.device, s.textureDSL, nullptr);
    if (s.descriptorPool) vkDestroyDescriptorPool(s.device, s.descriptorPool, nullptr);
    if (s.commandPool)   vkDestroyCommandPool(s.device, s.commandPool, nullptr);
    vkDestroyDevice(s.device, nullptr);
    vkDestroyInstance(s.instance, nullptr);
    s = {};
}

bool xlVkCanvas::CreateSurface() {
#ifdef _WIN32
    HWND hwnd = (HWND)GetHandle();
    HINSTANCE hinstance = GetModuleHandle(nullptr);

    VkWin32SurfaceCreateInfoKHR sci{};
    sci.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hwnd      = hwnd;
    sci.hinstance = hinstance;
    return vkCreateWin32SurfaceKHR(s_shared.instance, &sci, nullptr, &mSurface) == VK_SUCCESS;
#elif defined(__linux__)
    Display* display = (Display*)wxGetDisplay();
    Window   window  = (Window)GetHandle();
    VkXlibSurfaceCreateInfoKHR sci{};
    sci.sType  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    sci.dpy    = display;
    sci.window = window;
    return vkCreateXlibSurfaceKHR(s_shared.instance, &sci, nullptr, &mSurface) == VK_SUCCESS;
#else
    spdlog::error("xlVkCanvas: unsupported platform for surface creation");
    return false;
#endif
}

static VkFormat ChooseDepthFormat(VkPhysicalDevice pd) {
    const VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
    };
    for (VkFormat fmt : candidates) {
        VkFormatProperties fp;
        vkGetPhysicalDeviceFormatProperties(pd, fmt, &fp);
        if (fp.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            return fmt;
    }
    return VK_FORMAT_D16_UNORM;
}

bool xlVkCanvas::CreateSwapchain() {
    // Capabilities
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_shared.physicalDevice, mSurface, &caps);

    // Choose surface format: prefer BGRA8_UNORM / SRGB_NONLINEAR
    uint32_t fmtCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(s_shared.physicalDevice, mSurface, &fmtCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(fmtCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(s_shared.physicalDevice, mSurface, &fmtCount, formats.data());

    VkSurfaceFormatKHR chosen = formats[0];
    for (auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosen = f;
            break;
        }
    }
    mSwapFormat = (VkFormatOpaque)chosen.format;

    // Choose present mode: prefer MAILBOX (lowest latency without tearing)
    uint32_t pmCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(s_shared.physicalDevice, mSurface, &pmCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(pmCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(s_shared.physicalDevice, mSurface, &pmCount, presentModes.data());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (auto pm : presentModes) {
        if (pm == VK_PRESENT_MODE_MAILBOX_KHR) { presentMode = pm; break; }
    }

    // Extent
    VkExtent2D extent;
    if (caps.currentExtent.width != UINT32_MAX) {
        extent = caps.currentExtent;
    } else {
        extent.width  = std::clamp((uint32_t)mWindowWidth,  caps.minImageExtent.width,  caps.maxImageExtent.width);
        extent.height = std::clamp((uint32_t)mWindowHeight, caps.minImageExtent.height, caps.maxImageExtent.height);
    }
    if (extent.width == 0 || extent.height == 0) return false;

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0) imageCount = std::min(imageCount, caps.maxImageCount);

    VkSwapchainCreateInfoKHR sci{};
    sci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface          = mSurface;
    sci.minImageCount    = imageCount;
    sci.imageFormat      = chosen.format;
    sci.imageColorSpace  = chosen.colorSpace;
    sci.imageExtent      = extent;
    sci.imageArrayLayers = 1;
    sci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    uint32_t families[2] = {s_shared.graphicsFamily, s_shared.presentFamily};
    if (s_shared.graphicsFamily != s_shared.presentFamily) {
        sci.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        sci.queueFamilyIndexCount = 2;
        sci.pQueueFamilyIndices   = families;
    } else {
        sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    sci.preTransform   = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode    = presentMode;
    sci.clipped        = VK_TRUE;

    if (vkCreateSwapchainKHR(s_shared.device, &sci, nullptr, &mSwapchain) != VK_SUCCESS)
        return false;

    // Retrieve images
    uint32_t imgCount = 0;
    vkGetSwapchainImagesKHR(s_shared.device, mSwapchain, &imgCount, nullptr);
    mSwapImages.resize(imgCount);
    vkGetSwapchainImagesKHR(s_shared.device, mSwapchain, &imgCount, mSwapImages.data());

    // Create image views
    mSwapImageViews.resize(imgCount);
    for (uint32_t i = 0; i < imgCount; ++i) {
        VkImageViewCreateInfo ivci{};
        ivci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ivci.image                           = mSwapImages[i];
        ivci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        ivci.format                          = chosen.format;
        ivci.components                      = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
        ivci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        ivci.subresourceRange.baseMipLevel   = 0;
        ivci.subresourceRange.levelCount     = 1;
        ivci.subresourceRange.baseArrayLayer = 0;
        ivci.subresourceRange.layerCount     = 1;
        vkCreateImageView(s_shared.device, &ivci, nullptr, &mSwapImageViews[i]);
    }

    mImagesInFlight.assign(imgCount, VK_NULL_HANDLE);
    return true;
}

bool xlVkCanvas::CreateDepthResources() {
    if (!RequiresDepthBuffer() && !is3d)
        return true;

    mDepthFormat = (VkFormatOpaque)ChooseDepthFormat(s_shared.physicalDevice);

    VkImageCreateInfo ici{};
    ici.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType     = VK_IMAGE_TYPE_2D;
    ici.format        = (VkFormat)mDepthFormat;
    ici.extent        = {(uint32_t)mWindowWidth, (uint32_t)mWindowHeight, 1};
    ici.mipLevels     = 1;
    ici.arrayLayers   = 1;
    ici.samples       = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling        = VK_IMAGE_TILING_OPTIMAL;
    ici.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (vkCreateImage(s_shared.device, &ici, nullptr, &mDepthImage) != VK_SUCCESS)
        return false;

    VkMemoryRequirements mr;
    vkGetImageMemoryRequirements(s_shared.device, mDepthImage, &mr);
    VkMemoryAllocateInfo mai{};
    mai.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize  = mr.size;
    mai.memoryTypeIndex = FindMemoryType(s_shared.physicalDevice, mr.memoryTypeBits,
                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(s_shared.device, &mai, nullptr, &mDepthMemory) != VK_SUCCESS) return false;
    vkBindImageMemory(s_shared.device, mDepthImage, mDepthMemory, 0);

    VkImageViewCreateInfo ivci{};
    ivci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image                           = mDepthImage;
    ivci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    ivci.format                          = (VkFormat)mDepthFormat;
    ivci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    ivci.subresourceRange.levelCount     = 1;
    ivci.subresourceRange.layerCount     = 1;
    vkCreateImageView(s_shared.device, &ivci, nullptr, &mDepthView);

    TransitionImageLayout(s_shared.device, s_shared.commandPool, s_shared.graphicsQueue,
                          mDepthImage, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                          VK_IMAGE_ASPECT_DEPTH_BIT);
    return true;
}

bool xlVkCanvas::CreateRenderPass() {
    bool useDepth = (RequiresDepthBuffer() || is3d) && mDepthImage != nullptr;

    VkAttachmentDescription colorAtt{};
    colorAtt.format         = (VkFormat)mSwapFormat;
    colorAtt.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAtt.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAtt.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAtt.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAtt.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAtt.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAtt{};
    depthAtt.format         = (VkFormat)mDepthFormat;
    depthAtt.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAtt.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAtt.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAtt.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAtt.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAtt.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthRef{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorRef;
    if (useDepth) subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dep{};
    dep.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass    = 0;
    dep.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | (useDepth ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : 0);

    std::vector<VkAttachmentDescription> atts = {colorAtt};
    if (useDepth) atts.push_back(depthAtt);

    VkRenderPassCreateInfo rpci{};
    rpci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = (uint32_t)atts.size();
    rpci.pAttachments    = atts.data();
    rpci.subpassCount    = 1;
    rpci.pSubpasses      = &subpass;
    rpci.dependencyCount = 1;
    rpci.pDependencies   = &dep;

    return vkCreateRenderPass(s_shared.device, &rpci, nullptr, &mRenderPass) == VK_SUCCESS;
}

bool xlVkCanvas::CreateFramebuffers() {
    bool useDepth = (RequiresDepthBuffer() || is3d) && mDepthImage != nullptr;
    mFramebuffers.resize(mSwapImageViews.size());
    for (size_t i = 0; i < mSwapImageViews.size(); ++i) {
        std::vector<VkImageView> attachments = {mSwapImageViews[i]};
        if (useDepth) attachments.push_back(mDepthView);

        VkFramebufferCreateInfo fbci{};
        fbci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbci.renderPass      = mRenderPass;
        fbci.attachmentCount = (uint32_t)attachments.size();
        fbci.pAttachments    = attachments.data();
        fbci.width           = (uint32_t)mWindowWidth;
        fbci.height          = (uint32_t)mWindowHeight;
        fbci.layers          = 1;
        if (vkCreateFramebuffer(s_shared.device, &fbci, nullptr, &mFramebuffers[i]) != VK_SUCCESS)
            return false;
    }
    return true;
}

bool xlVkCanvas::AllocateCommandBuffers() {
    VkCommandBufferAllocateInfo ai{};
    ai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool        = s_shared.commandPool;
    ai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    return vkAllocateCommandBuffers(s_shared.device, &ai, mCmdBufs) == VK_SUCCESS;
}

bool xlVkCanvas::CreateSyncObjects() {
    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fci{};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(s_shared.device, &sci, nullptr, &mImageAvailable[i]) != VK_SUCCESS) return false;
        if (vkCreateSemaphore(s_shared.device, &sci, nullptr, &mRenderFinished[i]) != VK_SUCCESS) return false;
        if (vkCreateFence    (s_shared.device, &fci, nullptr, &mInFlightFences[i]) != VK_SUCCESS) return false;
    }
    return true;
}

bool xlVkCanvas::EnsureInitialized() {
    if (mIsInitialized) return true;

    // Phase 1: create VkInstance + pick GPU (no surface needed).
    if (!InitializeInstance()) {
        spdlog::error("xlVkCanvas: InitializeInstance failed");
        return false;
    }
    // Phase 2: create the per-canvas surface (needs instance from phase 1).
    if (!CreateSurface()) {
        spdlog::error("xlVkCanvas: CreateSurface failed");
        return false;
    }
    // Phase 3: create logical device, queues, pools, layouts (needs surface for
    //          present-queue family query). No-op for subsequent canvases.
    if (!InitializeSharedState(mSurface)) {
        spdlog::error("xlVkCanvas: InitializeSharedState failed");
        return false;
    }
    if (!CreateSwapchain()) {
        spdlog::error("xlVkCanvas: CreateSwapchain failed");
        return false;
    }
    if (!CreateDepthResources()) {
        spdlog::warn("xlVkCanvas: CreateDepthResources failed (no depth buffer)");
    }
    if (!CreateRenderPass()) {
        spdlog::error("xlVkCanvas: CreateRenderPass failed");
        return false;
    }
    if (!xlVkGraphicsContext::InitializePipelines(mRenderPass)) {
        spdlog::error("xlVkCanvas: InitializePipelines failed");
        return false;
    }
    if (!CreateFramebuffers()) {
        spdlog::error("xlVkCanvas: CreateFramebuffers failed");
        return false;
    }
    if (!AllocateCommandBuffers()) {
        spdlog::error("xlVkCanvas: AllocateCommandBuffers failed");
        return false;
    }
    if (!CreateSyncObjects()) {
        spdlog::error("xlVkCanvas: CreateSyncObjects failed");
        return false;
    }

    mIsInitialized = true;
    return true;
}

void xlVkCanvas::CleanupSwapchain() {
    if (!s_shared.device) return;
    for (auto fb : mFramebuffers) vkDestroyFramebuffer(s_shared.device, fb, nullptr);
    mFramebuffers.clear();
    if (mRenderPass) { vkDestroyRenderPass(s_shared.device, mRenderPass, nullptr); mRenderPass = nullptr; }
    for (auto iv : mSwapImageViews) vkDestroyImageView(s_shared.device, iv, nullptr);
    mSwapImageViews.clear();
    if (mDepthView)   { vkDestroyImageView  (s_shared.device, mDepthView,   nullptr); mDepthView   = nullptr; }
    if (mDepthImage)  { vkDestroyImage      (s_shared.device, mDepthImage,  nullptr); mDepthImage  = nullptr; }
    if (mDepthMemory) { vkFreeMemory        (s_shared.device, mDepthMemory, nullptr); mDepthMemory = nullptr; }
    if (mSwapchain)   { vkDestroySwapchainKHR(s_shared.device, mSwapchain,  nullptr); mSwapchain   = nullptr; }
}

void xlVkCanvas::RecreateSwapchain() {
    if (!s_shared.device) return;
    vkDeviceWaitIdle(s_shared.device);
    CleanupSwapchain();
    xlVkGraphicsContext::DestroyPipelines();
    CreateSwapchain();
    CreateDepthResources();
    CreateRenderPass();
    xlVkGraphicsContext::InitializePipelines(mRenderPass);
    CreateFramebuffers();
    mImagesInFlight.assign(mSwapImages.size(), VK_NULL_HANDLE);
}

// ---------------------------------------------------------------------------
// Canvas event / lifecycle
// ---------------------------------------------------------------------------
void xlVkCanvas::Resized(wxSizeEvent& evt) {
    mWindowWidth  = evt.GetSize().GetWidth();
    mWindowHeight = evt.GetSize().GetHeight();
    mWindowResized = true;
    if (mIsInitialized && mWindowWidth > 0 && mWindowHeight > 0) {
        RecreateSwapchain();
    }
    Refresh();
}

double xlVkCanvas::contentScale() const {
    return GetContentScaleFactor();
}
double xlVkCanvas::translateToBacking(double x) const {
    return GetContentScaleFactor() * x;
}
double xlVkCanvas::mapLogicalToAbsolute(double x) const {
    if (drawingUsingLogicalSize()) return x;
    return translateToBacking(x);
}

void xlVkCanvas::PrepareCanvas() {
    EnsureInitialized();
}

xlGraphicsContext* xlVkCanvas::PrepareContextForDrawing() {
    return PrepareContextForDrawing(ClearBackgroundColor());
}

xlGraphicsContext* xlVkCanvas::PrepareContextForDrawing(const xlColor& bg) {
    if (!EnsureInitialized()) return nullptr;
    if (mWindowWidth == 0 || mWindowHeight == 0) return nullptr;

    // Wait for the previous use of this frame slot
    vkWaitForFences(s_shared.device, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(s_shared.device, mSwapchain, UINT64_MAX,
                                            mImageAvailable[mCurrentFrame],
                                            VK_NULL_HANDLE, &mCurrentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapchain();
        return PrepareContextForDrawing(bg);
    }

    // If this image was still in flight from a previous frame, wait on its fence
    if (mImagesInFlight[mCurrentImageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(s_shared.device, 1, &mImagesInFlight[mCurrentImageIndex], VK_TRUE, UINT64_MAX);
    }
    mImagesInFlight[mCurrentImageIndex] = mInFlightFences[mCurrentFrame];
    vkResetFences(s_shared.device, 1, &mInFlightFences[mCurrentFrame]);

    // Begin command buffer
    VkCommandBuffer cmd = mCmdBufs[mCurrentFrame];
    VkCommandBufferBeginInfo bi{};
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &bi);

    // Begin render pass
    VkClearValue clears[2];
    clears[0].color = {{bg.red / 255.0f, bg.green / 255.0f, bg.blue / 255.0f, bg.alpha / 255.0f}};
    clears[1].depthStencil = {1.0f, 0};

    bool useDepth = (RequiresDepthBuffer() || is3d) && mDepthImage != nullptr;
    VkRenderPassBeginInfo rpbi{};
    rpbi.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.renderPass        = mRenderPass;
    rpbi.framebuffer       = mFramebuffers[mCurrentImageIndex];
    rpbi.renderArea.offset = {0, 0};
    rpbi.renderArea.extent = {(uint32_t)mWindowWidth, (uint32_t)mWindowHeight};
    rpbi.clearValueCount   = useDepth ? 2u : 1u;
    rpbi.pClearValues      = clears;
    vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    // Set dynamic viewport and scissor
    VkViewport vp{};
    vp.x        = 0.0f;
    vp.y        = 0.0f;
    vp.width    = (float)mWindowWidth;
    vp.height   = (float)mWindowHeight;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &vp);

    VkRect2D scissor{{0, 0}, {(uint32_t)mWindowWidth, (uint32_t)mWindowHeight}};
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    return new xlVkGraphicsContext(this);
}

void xlVkCanvas::FinishDrawing(xlGraphicsContext* ctx, bool display) {
    auto* vkCtx = dynamic_cast<xlVkGraphicsContext*>(ctx);
    if (vkCtx) vkCtx->EndFrame();
    delete ctx;

    VkCommandBuffer cmd = mCmdBufs[mCurrentFrame];
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si{};
    si.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount   = 1;
    si.pWaitSemaphores      = &mImageAvailable[mCurrentFrame];
    si.pWaitDstStageMask    = &waitStage;
    si.commandBufferCount   = 1;
    si.pCommandBuffers      = &cmd;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores    = &mRenderFinished[mCurrentFrame];
    vkQueueSubmit(s_shared.graphicsQueue, 1, &si, mInFlightFences[mCurrentFrame]);

    if (display) {
        VkPresentInfoKHR pi{};
        pi.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores    = &mRenderFinished[mCurrentFrame];
        pi.swapchainCount     = 1;
        pi.pSwapchains        = &mSwapchain;
        pi.pImageIndices      = &mCurrentImageIndex;
        VkResult r = vkQueuePresentKHR(s_shared.presentQueue, &pi);
        if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR) {
            RecreateSwapchain();
        }
    }

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// ---------------------------------------------------------------------------
// GrabImage / frame export
// ---------------------------------------------------------------------------
uint8_t* xlVkCanvas::ReadbackFrame(int& outW, int& outH) {
    if (!mIsInitialized) return nullptr;
    outW = (int)mWindowWidth;
    outH = (int)mWindowHeight;
    if (outW == 0 || outH == 0) return nullptr;

    VkDeviceSize size = outW * outH * 4;
    VkBuffer     stagingBuf;
    VkDeviceMemory stagingMem;
    if (!CreateBuffer(s_shared.device, s_shared.physicalDevice, size,
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      stagingBuf, stagingMem)) return nullptr;

    VkImage srcImage = mSwapImages[mCurrentImageIndex];

    // Transition to transfer src
    TransitionImageLayout(s_shared.device, s_shared.commandPool, s_shared.graphicsQueue,
                          srcImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    // Copy image to buffer
    VkCommandBuffer cmd = BeginSingleTimeCommands(s_shared.device, s_shared.commandPool);
    VkBufferImageCopy region{};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {(uint32_t)outW, (uint32_t)outH, 1};
    vkCmdCopyImageToBuffer(cmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuf, 1, &region);
    EndSingleTimeCommands(s_shared.device, s_shared.commandPool, s_shared.graphicsQueue, cmd);

    // Transition back
    TransitionImageLayout(s_shared.device, s_shared.commandPool, s_shared.graphicsQueue,
                          srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // Map and copy
    void* mapped;
    vkMapMemory(s_shared.device, stagingMem, 0, size, 0, &mapped);
    uint8_t* pixels = new uint8_t[size];
    memcpy(pixels, mapped, (size_t)size);
    vkUnmapMemory(s_shared.device, stagingMem);

    vkDestroyBuffer(s_shared.device, stagingBuf, nullptr);
    vkFreeMemory   (s_shared.device, stagingMem, nullptr);

    return pixels; // BGRA format
}

wxImage* xlVkCanvas::GrabImage(wxSize size) {
    int w, h;
    uint8_t* bgra = ReadbackFrame(w, h);
    if (!bgra) return nullptr;

    unsigned char* rgb = (unsigned char*)malloc(w * h * 3);
    if (!rgb) { delete[] bgra; return nullptr; }

    // Convert BGRA → RGB and vertically flip
    for (int y = 0; y < h; ++y) {
        const uint8_t* src = bgra + (size_t)(h - 1 - y) * w * 4;
        unsigned char* dst = rgb  + (size_t)y * w * 3;
        for (int x = 0; x < w; ++x, src += 4, dst += 3) {
            dst[0] = src[2]; // R
            dst[1] = src[1]; // G
            dst[2] = src[0]; // B
        }
    }
    delete[] bgra;
    return new wxImage(w, h, rgb, false);
}

bool xlVkCanvas::getFrameForExport(int w, int h, AVFrame*, uint8_t* buffer, int bufferSize) {
    bool padW = (w % 2), padH = (h % 2);
    int wPad = padW ? w + 1 : w;
    int hPad = padH ? h + 1 : h;
    if (bufferSize < wPad * 3 * hPad) return false;

    int fw, fh;
    uint8_t* bgra = ReadbackFrame(fw, fh);
    if (!bgra) return false;

    unsigned char* dst = buffer;
    if (padH) { memset(dst, 0, wPad * 3); dst += wPad * 3; }
    for (int y = h - 1; y >= 0; --y) {
        const uint8_t* src = bgra + (size_t)y * fw * 4;
        for (int x = 0; x < w; ++x, src += 4, dst += 3) {
            dst[0] = src[2]; // R
            dst[1] = src[1]; // G
            dst[2] = src[0]; // B
        }
        if (padW) { dst[0] = dst[1] = dst[2] = 0; dst += 3; }
    }
    delete[] bgra;
    return true;
}
