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

#include "VulkanComputeUtilities.h"
#include "VulkanGraphicsUtilities.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

#include <spdlog/spdlog.h>

#include "../../render/PixelBuffer.h"
#include "../../render/RenderBuffer.h"
#include "VulkanEffectDataTypes.h"

#include "shaders/compiled/TentBlurH.spv.h"
#include "shaders/compiled/TentBlurV.spv.h"
#include "shaders/compiled/RotoZoomBlank.spv.h"
#include "shaders/compiled/RotoZoomRotateX.spv.h"
#include "shaders/compiled/RotoZoomRotateXClaim.spv.h"
#include "shaders/compiled/RotoZoomRotateY.spv.h"
#include "shaders/compiled/RotoZoomRotateYClaim.spv.h"
#include "shaders/compiled/RotoZoomRotateZ.spv.h"
#include "shaders/compiled/RotoZoomRotateZClaim.spv.h"

#include "shaders/compiled/GetColorsForNodes.spv.h"
#include "shaders/compiled/PutColorsForNodes.spv.h"
#include "shaders/compiled/AdjustHSV.spv.h"
#include "shaders/compiled/ApplySparkles.spv.h"
#include "shaders/compiled/AdjustBrightnessContrast.spv.h"
#include "shaders/compiled/AdjustBrightnessLevel.spv.h"
#include "shaders/compiled/FirstLayerFade.spv.h"
#include "shaders/compiled/NonAlphaFade.spv.h"
#include "shaders/compiled/Blend_Normal.spv.h"
#include "shaders/compiled/Blend_Effect1_2.spv.h"
#include "shaders/compiled/Blend_Mask1.spv.h"
#include "shaders/compiled/Blend_Mask2.spv.h"
#include "shaders/compiled/Blend_Unmask1.spv.h"
#include "shaders/compiled/Blend_Unmask2.spv.h"
#include "shaders/compiled/Blend_TrueUnmask1.spv.h"
#include "shaders/compiled/Blend_TrueUnmask2.spv.h"
#include "shaders/compiled/Blend_Shadow_1on2.spv.h"
#include "shaders/compiled/Blend_Shadow_2on1.spv.h"
#include "shaders/compiled/Blend_Layered.spv.h"
#include "shaders/compiled/Blend_Averaged.spv.h"
#include "shaders/compiled/Blend_Reveal12.spv.h"
#include "shaders/compiled/Blend_Reveal21.spv.h"
#include "shaders/compiled/Blend_Additive.spv.h"
#include "shaders/compiled/Blend_Subtractive.spv.h"
#include "shaders/compiled/Blend_Max.spv.h"
#include "shaders/compiled/Blend_Min.spv.h"
#include "shaders/compiled/Blend_AsBrightness.spv.h"
#include "shaders/compiled/Blend_Highlight.spv.h"
#include "shaders/compiled/Blend_HighlightVibrant.spv.h"
#include "shaders/compiled/Blend_BottomTop.spv.h"
#include "shaders/compiled/Blend_LeftRight.spv.h"

#include "shaders/compiled/Transition_wipe.spv.h"
#include "shaders/compiled/Transition_clock.spv.h"
#include "shaders/compiled/Transition_fromMiddle.spv.h"
#include "shaders/compiled/Transition_squareExplode.spv.h"
#include "shaders/compiled/Transition_circleExplode.spv.h"
#include "shaders/compiled/Transition_blinds.spv.h"
#include "shaders/compiled/Transition_blend.spv.h"
#include "shaders/compiled/Transition_slideChecks.spv.h"
#include "shaders/compiled/Transition_slideBars.spv.h"
#include "shaders/compiled/Transition_shatter.spv.h"
#include "shaders/compiled/Transition_star.spv.h"
#include "shaders/compiled/Transition_pinwheel.spv.h"
#include "shaders/compiled/Transition_bowTie.spv.h"
#include "shaders/compiled/Transition_blobs.spv.h"
#include "shaders/compiled/Transition_fold.spv.h"
#include "shaders/compiled/Transition_zoom.spv.h"
#include "shaders/compiled/Transition_circularSwirl.spv.h"
#include "shaders/compiled/Transition_doorway.spv.h"
#include "shaders/compiled/Transition_swap.spv.h"
#include "shaders/compiled/Transition_circles.spv.h"
#include "shaders/compiled/Transition_dissolve.spv.h"

#include "shaders/compiled/BarsEffect.spv.h"
#include "shaders/compiled/ColorWashEffect.spv.h"
#include "shaders/compiled/ShockwaveEffect.spv.h"
#include "shaders/compiled/FanEffect.spv.h"
#include "shaders/compiled/SpiralsEffect.spv.h"
#include "shaders/compiled/GalaxyEffect.spv.h"
#include "shaders/compiled/CirclesEffect.spv.h"
#include "shaders/compiled/PlasmaEffect.spv.h"
#include "shaders/compiled/ButterflyEffect.spv.h"
#include "shaders/compiled/PinwheelEffect.spv.h"
#include "shaders/compiled/KaleidoscopeEffect.spv.h"
#include "shaders/compiled/WarpEffect.spv.h"
#include "shaders/compiled/TreeEffect.spv.h"
#include "shaders/compiled/ShimmerEffect.spv.h"
#include "shaders/compiled/CandleEffect.spv.h"
#include "shaders/compiled/WaveEffect.spv.h"
#include "shaders/compiled/GarlandsEffect.spv.h"
#include "shaders/compiled/FillEffect.spv.h"
#include "shaders/compiled/MeteorsEffect.spv.h"
#include "shaders/compiled/TwinkleEffect.spv.h"
#include "shaders/compiled/LifeEffect.spv.h"

#include "../../render/DissolveTransitionPattern.h"

static xlvk::uchar4 toUchar4(const xlColor& c) {
    return { c.red, c.green, c.blue, c.alpha };
}

VulkanComputeUtilities VulkanComputeUtilities::INSTANCE;

static bool envSet(const char* name) {
    return getenv(name) != nullptr;
}
static long envLong(const char* name, long def) {
    const char* v = getenv(name);
    if (v == nullptr) {
        return def;
    }
    char* end = nullptr;
    long r = strtol(v, &end, 10);
    return end == v ? def : r;
}

VulkanComputeUtilities::VulkanComputeUtilities() {
}

VulkanComputeUtilities::~VulkanComputeUtilities() {
    // Deliberately no teardown (matches the Metal backend): the process is
    // exiting and render threads may still hold command buffers.  Validation
    // runs get a clean shutdown via the atexit hook installed in doInit.
}

void VulkanComputeUtilities::ensureInit() {
    std::call_once(initFlag, [this]() { doInit(); });
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* userData) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::error("Vulkan validation: {}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn("Vulkan validation: {}", data->pMessage);
    } else {
        spdlog::info("Vulkan validation: {}", data->pMessage);
    }
    return VK_FALSE;
}

bool VulkanComputeUtilities::createInstance(bool wantValidation) {
    uint32_t loaderVersion = volkGetInstanceVersion();
    if (loaderVersion < VK_API_VERSION_1_1) {
        spdlog::info("Vulkan compute disabled: loader/ICD only supports 1.0");
        return false;
    }

    std::vector<const char*> layers;
    std::vector<const char*> extensions;
    // XL_VULKAN_GPUAV enables GPU-Assisted Validation, which instruments the
    // compute shaders to bounds-check every buffer access at runtime (catches
    // out-of-bounds scatter that standard validation cannot see).  Slow — only
    // for diagnosing shader memory bugs.
    const bool wantGpuAv = envSet("XL_VULKAN_GPUAV");
    if (wantValidation) {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> props(count);
        vkEnumerateInstanceLayerProperties(&count, props.data());
        for (auto& p : props) {
            if (strcmp(p.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
                layers.push_back("VK_LAYER_KHRONOS_validation");
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                validation = true;
                break;
            }
        }
        if (!validation) {
            spdlog::warn("XL_VULKAN_VALIDATE set but VK_LAYER_KHRONOS_validation not available");
        }
    }

    // Surface extensions for the on-screen Vulkan graphics backend.  Optional:
    // a headless/compute-only ICD still works, the graphics canvas just stays
    // on its OpenGL fallback.
    {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> props(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data());
        auto hasExt = [&props](const char* name) {
            for (auto& p : props) {
                if (strcmp(p.extensionName, name) == 0) {
                    return true;
                }
            }
            return false;
        };
        if (hasExt(VK_KHR_SURFACE_EXTENSION_NAME)) {
            bool platformSurface = false;
#if defined(_WIN32)
            for (const char* ext : { "VK_KHR_win32_surface" }) {
#else
            for (const char* ext : { "VK_KHR_xlib_surface", "VK_KHR_xcb_surface", "VK_KHR_wayland_surface" }) {
#endif
                if (hasExt(ext)) {
                    extensions.push_back(ext);
                    platformSurface = true;
                }
            }
            if (platformSurface) {
                extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
                surfaceExtensionsEnabled = true;
            }
        }
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "xLights";
    appInfo.pEngineName = "xLightsEffectCompute";
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &appInfo;
    ci.enabledLayerCount = (uint32_t)layers.size();
    ci.ppEnabledLayerNames = layers.data();

    VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT };
    VkValidationFeaturesEXT valFeatures = {};
    if (validation && wantGpuAv) {
        extensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
        valFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        valFeatures.enabledValidationFeatureCount = 1;
        valFeatures.pEnabledValidationFeatures = enables;
        ci.pNext = &valFeatures;
    }
    ci.enabledExtensionCount = (uint32_t)extensions.size();
    ci.ppEnabledExtensionNames = extensions.data();

    VkResult res = vkCreateInstance(&ci, nullptr, &instance);
    if (res != VK_SUCCESS) {
        spdlog::info("Vulkan compute disabled: vkCreateInstance failed ({})", (int)res);
        return false;
    }
    volkLoadInstanceOnly(instance);

    if (validation) {
        VkDebugUtilsMessengerCreateInfoEXT mi = {};
        mi.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        mi.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        mi.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        mi.pfnUserCallback = vulkanDebugCallback;
        vkCreateDebugUtilsMessengerEXT(instance, &mi, nullptr, &debugMessenger);
    }
    return true;
}

bool VulkanComputeUtilities::pickPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (count == 0) {
        spdlog::info("Vulkan compute disabled: no physical devices");
        return false;
    }
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());

    const bool allowCpu = envSet("XL_VULKAN_ALLOW_CPU");
    const char* wanted = getenv("XL_VULKAN_DEVICE");

    auto computeQueueFamily = [](VkPhysicalDevice d) -> int {
        uint32_t qc = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qc, nullptr);
        std::vector<VkQueueFamilyProperties> qf(qc);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qc, qf.data());
        int anyCompute = -1;
        for (uint32_t i = 0; i < qc; i++) {
            if (qf[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                // Prefer an async-compute family (no graphics) so effect
                // rendering doesn't contend with the compositor's queue.
                if (!(qf[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                    return (int)i;
                }
                if (anyCompute < 0) {
                    anyCompute = (int)i;
                }
            }
        }
        return anyCompute;
    };

    // For the graphics backend: any graphics-capable family (present support
    // is verified per-surface at swapchain creation).  Compute-only devices
    // stay usable for effects; graphics just remains disabled on them.
    auto graphicsQueueFamily = [](VkPhysicalDevice d) -> int {
        uint32_t qc = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qc, nullptr);
        std::vector<VkQueueFamilyProperties> qf(qc);
        vkGetPhysicalDeviceQueueFamilyProperties(d, &qc, qf.data());
        for (uint32_t i = 0; i < qc; i++) {
            if (qf[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                return (int)i;
            }
        }
        return -1;
    };

    int bestScore = -1;
    for (uint32_t idx = 0; idx < count; idx++) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(devices[idx], &props);
        if (props.apiVersion < VK_API_VERSION_1_1) {
            continue;
        }
        int qfi = computeQueueFamily(devices[idx]);
        if (qfi < 0) {
            continue;
        }
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU && !allowCpu) {
            continue;
        }

        int score;
        switch (props.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   score = 4; break;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: score = 3; break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    score = 2; break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:            score = 1; break;
        default:                                     score = 2; break;
        }
        if (wanted != nullptr) {
            // XL_VULKAN_DEVICE: numeric index or device-name substring.
            char* end = nullptr;
            long widx = strtol(wanted, &end, 10);
            bool match = (end != wanted && *end == '\0') ? (widx == (long)idx)
                                                         : (strstr(props.deviceName, wanted) != nullptr);
            if (!match) {
                continue;
            }
            score += 100;
        }
        if (score > bestScore) {
            bestScore = score;
            physicalDevice = devices[idx];
            queueFamilyIndex = (uint32_t)qfi;
            deviceType = props.deviceType;
            deviceName = props.deviceName;
            graphicsFamilyCandidate = graphicsQueueFamily(devices[idx]);
        }
    }
    if (physicalDevice == VK_NULL_HANDLE) {
        spdlog::info("Vulkan compute disabled: no usable device (set XL_VULKAN_ALLOW_CPU=1 to permit CPU implementations like lavapipe)");
        return false;
    }
    VkPhysicalDeviceProperties chosen;
    vkGetPhysicalDeviceProperties(physicalDevice, &chosen);
    storageBufferAlignment = std::max((VkDeviceSize)1, chosen.limits.minStorageBufferOffsetAlignment);
    return true;
}

bool VulkanComputeUtilities::createDeviceAndQueue() {
    float priority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    VkDeviceQueueCreateInfo qi = {};
    qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qi.queueFamilyIndex = queueFamilyIndex;
    qi.queueCount = 1;
    qi.pQueuePriorities = &priority;
    queueInfos.push_back(qi);
    if (graphicsFamilyCandidate >= 0 && (uint32_t)graphicsFamilyCandidate != queueFamilyIndex) {
        qi.queueFamilyIndex = (uint32_t)graphicsFamilyCandidate;
        queueInfos.push_back(qi);
    }

    // VK_KHR_swapchain for the on-screen graphics backend.
    std::vector<const char*> deviceExtensions;
    if (surfaceExtensionsEnabled && graphicsFamilyCandidate >= 0) {
        uint32_t count = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> props(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, props.data());
        for (auto& p : props) {
            if (strcmp(p.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
                deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
                swapchainExtensionEnabled = true;
                break;
            }
        }
    }

    VkPhysicalDeviceFeatures features = {};
    VkDeviceCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount = (uint32_t)queueInfos.size();
    ci.pQueueCreateInfos = queueInfos.data();
    ci.pEnabledFeatures = &features;
    ci.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    ci.ppEnabledExtensionNames = deviceExtensions.data();

    VkResult res = vkCreateDevice(physicalDevice, &ci, nullptr, &device);
    if (res != VK_SUCCESS) {
        spdlog::info("Vulkan compute disabled: vkCreateDevice failed ({})", (int)res);
        return false;
    }
    volkLoadDevice(device);
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
    if (graphicsFamilyCandidate >= 0) {
        graphicsQueueFamilyIndex = (uint32_t)graphicsFamilyCandidate;
        vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
    }
    return queue != VK_NULL_HANDLE;
}

bool VulkanComputeUtilities::createAllocator() {
    VmaVulkanFunctions functions = {};
    VmaAllocatorCreateInfo ci = {};
    ci.physicalDevice = physicalDevice;
    ci.device = device;
    ci.instance = instance;
    ci.vulkanApiVersion = VK_API_VERSION_1_1;
    VkResult res = vmaImportVulkanFunctionsFromVolk(&ci, &functions);
    if (res != VK_SUCCESS) {
        return false;
    }
    ci.pVulkanFunctions = &functions;
    res = vmaCreateAllocator(&ci, &allocator);
    return res == VK_SUCCESS;
}

void VulkanComputeUtilities::doInit() {
    // Must never touch libvulkan when disabled so XL_NO_GPU_COMPUTE gives a
    // pure-CPU run for A/B determinism testing.
    if (envSet("XL_NO_GPU_COMPUTE")) {
        return;
    }
    if (volkInitialize() != VK_SUCCESS) {
        spdlog::info("Vulkan compute disabled: no Vulkan loader");
        return;
    }
    if (!createInstance(envSet("XL_VULKAN_VALIDATE"))) {
        return;
    }
    if (!pickPhysicalDevice() || !createDeviceAndQueue() || !createAllocator()) {
        return;
    }
    enabled = true;
    if (!buildPipelines()) {
        spdlog::info("Vulkan compute disabled: pipeline creation failed");
        enabled = false;
        return;
    }
    spdlog::info("Vulkan compute enabled: {} (type {}, queue family {})", deviceName, (int)deviceType, queueFamilyIndex);
    if (getenv("XL_VULKAN_GFXTEST") != nullptr) {
        // Bring up + self-test the graphics-pipeline foundation for headless
        // verification.  initInline (not ensureInit) since we're inside this
        // object's call_once and must not re-enter it.
        VulkanGraphicsUtilities::INSTANCE.initInline();
    }

    if (envSet("XL_GPU_STATS")) {
        // Proof that GPU paths actually ran: below-threshold test sequences
        // silently fall back to CPU and would otherwise look like a pass.
        atexit([]() {
            VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
            fprintf(stderr, "XL_GPU_STATS: blur=%llu rotozoom=%llu transitions=%llu blend=%llu effect=%llu setup=%llu blurcall=%llu\n",
                    (unsigned long long)u.statBlur.load(), (unsigned long long)u.statRotoZoom.load(),
                    (unsigned long long)u.statTransition.load(), (unsigned long long)u.statBlend.load(),
                    (unsigned long long)u.statEffect.load(),
                    (unsigned long long)u.statSetup.load(), (unsigned long long)u.statBlurCall.load());
        });
    }

    if (validation) {
        // Give validation runs a clean shutdown so leak checks are meaningful.
        atexit([]() {
            VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
            if (u.enabled) {
                u.enabled = false;
                vkDeviceWaitIdle(u.device);
                for (VkPipeline* p : { &u.tentBlurHFunction, &u.tentBlurVFunction,
                                       &u.xrotateFunction, &u.yrotateFunction, &u.zrotateFunction,
                                       &u.xrotateClaimFunction, &u.yrotateClaimFunction, &u.zrotateClaimFunction,
                                       &u.rotateBlankFunction,
                                       &u.getColorsFunction, &u.putColorsFunction, &u.adjustHSVFunction,
                                       &u.applySparklesFunction, &u.brightnessContrastFunction,
                                       &u.brightnessLevelFunction, &u.firstLayerFadeFunction,
                                       &u.nonAlphaFadeFunction, &u.barsEffectFunction,
                                       &u.colorWashEffectFunction,
                                       &u.shockwaveEffectFunction, &u.fanEffectFunction,
                                       &u.spiralsEffectFunction, &u.galaxyEffectFunction,
                                       &u.circlesEffectFunction, &u.plasmaEffectFunction,
                                       &u.butterflyEffectFunction, &u.pinwheelEffectFunction,
                                       &u.kaleidoscopeEffectFunction, &u.warpEffectFunction,
                                       &u.treeEffectFunction, &u.shimmerEffectFunction,
                                       &u.candleEffectFunction, &u.waveEffectFunction,
                                       &u.garlandsEffectFunction, &u.fillEffectFunction, &u.meteorsEffectFunction,
                                       &u.twinkleEffectFunction, &u.lifeEffectFunction }) {
                    if (*p != VK_NULL_HANDLE) {
                        vkDestroyPipeline(u.device, *p, nullptr);
                        *p = VK_NULL_HANDLE;
                    }
                }
                // Mix_Effect2 shares Mix_Effect1's pipeline — destroy each
                // distinct pipeline once.
                for (auto& bf : u.blendFunctions) {
                    if (bf.first != MixTypes::Mix_Effect2 && bf.second->function != VK_NULL_HANDLE) {
                        vkDestroyPipeline(u.device, bf.second->function, nullptr);
                    }
                    bf.second->function = VK_NULL_HANDLE;
                }
                for (auto& tr : u.transitions) {
                    if (tr.second->function != VK_NULL_HANDLE) {
                        vkDestroyPipeline(u.device, tr.second->function, nullptr);
                        tr.second->function = VK_NULL_HANDLE;
                    }
                }
                u.destroyBuffer(u.dissolveBuffer);
                if (u.pipelineLayout != VK_NULL_HANDLE) {
                    vkDestroyPipelineLayout(u.device, u.pipelineLayout, nullptr);
                }
                if (u.dsLayout != VK_NULL_HANDLE) {
                    vkDestroyDescriptorSetLayout(u.device, u.dsLayout, nullptr);
                }
                u.destroyBuffer(u.dummyBuffer);
                // Device/instance/allocator are deliberately NOT destroyed:
                // per-model GPU data can legitimately outlive the render
                // context at process exit (matching Metal's no-teardown
                // policy), and the vkDestroyDevice outstanding-object sweep
                // would flag those as false leaks.  In-run validation is
                // unaffected.
            }
        });
    }
}

int VulkanComputeUtilities::gpuCoreCount() {
    ensureInit();
    if (!enabled) {
        return 0;
    }
    long override = envLong("XL_VULKAN_CONCURRENCY", -1);
    if (override >= 0) {
        return (int)std::min(override, (long)256);
    }
    switch (deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return 16;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return 8;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        // lavapipe executes on CPU threads that would fight the render pool.
        return 2;
    default:
        return 4;
    }
}

void VulkanComputeUtilities::setObjectName(uint64_t handle, VkObjectType type, const std::string& name) {
    if (validation && vkSetDebugUtilsObjectNameEXT != nullptr) {
        VkDebugUtilsObjectNameInfoEXT ni = {};
        ni.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        ni.objectType = type;
        ni.objectHandle = handle;
        ni.pObjectName = name.c_str();
        vkSetDebugUtilsObjectNameEXT(device, &ni);
    }
}

bool VulkanComputeUtilities::createSharedBuffer(VulkanBuffer& b, size_t size, const std::string& name) {
    if (!enabled) {
        return false;
    }
    VkBufferCreateInfo bi = {};
    bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bi.size = size;
    // UNIFORM_BUFFER too, so shared buffers can also back the Shader effect's UBO.
    bi.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo ai = {};
    // HOST_CACHED preference matters: the CPU effect code reads these pixels
    // heavily and uncached write-combined memory makes those reads crawl.
    ai.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    ai.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    ai.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    ai.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

    VmaAllocationInfo info = {};
    if (vmaCreateBuffer(allocator, &bi, &ai, &b.buffer, &b.allocation, &info) != VK_SUCCESS) {
        b = VulkanBuffer();
        return false;
    }
    b.mapped = info.pMappedData;
    b.size = size;
    setObjectName((uint64_t)b.buffer, VK_OBJECT_TYPE_BUFFER, name);
    return true;
}

bool VulkanComputeUtilities::createDeviceBuffer(VulkanBuffer& b, size_t size, const std::string& name) {
    if (!enabled) {
        return false;
    }
    VkBufferCreateInfo bi = {};
    bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bi.size = size;
    // UNIFORM_BUFFER too, so shared buffers can also back the Shader effect's UBO.
    bi.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo ai = {};
    ai.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    if (vmaCreateBuffer(allocator, &bi, &ai, &b.buffer, &b.allocation, nullptr) != VK_SUCCESS) {
        b = VulkanBuffer();
        return false;
    }
    b.mapped = nullptr;
    b.size = size;
    setObjectName((uint64_t)b.buffer, VK_OBJECT_TYPE_BUFFER, name);
    return true;
}

void VulkanComputeUtilities::destroyBuffer(VulkanBuffer& b) {
    if (b.buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, b.buffer, b.allocation);
    }
    b = VulkanBuffer();
}

VkPipeline VulkanComputeUtilities::createComputePipeline(const uint32_t* spirv, size_t sizeBytes, const char* name) {
    VkShaderModuleCreateInfo smi = {};
    smi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smi.codeSize = sizeBytes;
    smi.pCode = spirv;
    VkShaderModule module = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device, &smi, nullptr, &module) != VK_SUCCESS) {
        spdlog::error("Vulkan compute: failed to create shader module {}", name);
        return VK_NULL_HANDLE;
    }
    VkComputePipelineCreateInfo pi = {};
    pi.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pi.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pi.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pi.stage.module = module;
    pi.stage.pName = "main";
    pi.layout = pipelineLayout;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult res = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pi, nullptr, &pipeline);
    vkDestroyShaderModule(device, module, nullptr);
    if (res != VK_SUCCESS) {
        spdlog::error("Vulkan compute: failed to create pipeline {} ({})", name, (int)res);
        return VK_NULL_HANDLE;
    }
    setObjectName((uint64_t)pipeline, VK_OBJECT_TYPE_PIPELINE, name);
    return pipeline;
}

bool VulkanComputeUtilities::buildPipelines() {
    VkDescriptorSetLayoutBinding bindings[NUM_BINDINGS] = {};
    for (uint32_t i = 0; i < NUM_BINDINGS; i++) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo li = {};
    li.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    li.bindingCount = NUM_BINDINGS;
    li.pBindings = bindings;
    if (vkCreateDescriptorSetLayout(device, &li, nullptr, &dsLayout) != VK_SUCCESS) {
        return false;
    }
    VkPushConstantRange pcr = {};
    pcr.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pcr.offset = 0;
    pcr.size = PUSH_CONSTANT_SIZE;
    VkPipelineLayoutCreateInfo pli = {};
    pli.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pli.setLayoutCount = 1;
    pli.pSetLayouts = &dsLayout;
    pli.pushConstantRangeCount = 1;
    pli.pPushConstantRanges = &pcr;
    if (vkCreatePipelineLayout(device, &pli, nullptr, &pipelineLayout) != VK_SUCCESS) {
        return false;
    }
    if (!createDeviceBuffer(dummyBuffer, 16, "DummyBinding")) {
        return false;
    }

#define XLVK_PIPELINE(member, header) \
    member = createComputePipeline(header##_spv, sizeof(header##_spv), #header); \
    if (member == VK_NULL_HANDLE) return false;

    XLVK_PIPELINE(tentBlurHFunction, TentBlurH)
    XLVK_PIPELINE(tentBlurVFunction, TentBlurV)
    XLVK_PIPELINE(rotateBlankFunction, RotoZoomBlank)
    XLVK_PIPELINE(xrotateFunction, RotoZoomRotateX)
    XLVK_PIPELINE(xrotateClaimFunction, RotoZoomRotateXClaim)
    XLVK_PIPELINE(yrotateFunction, RotoZoomRotateY)
    XLVK_PIPELINE(yrotateClaimFunction, RotoZoomRotateYClaim)
    XLVK_PIPELINE(zrotateFunction, RotoZoomRotateZ)
    XLVK_PIPELINE(zrotateClaimFunction, RotoZoomRotateZClaim)

    XLVK_PIPELINE(getColorsFunction, GetColorsForNodes)
    XLVK_PIPELINE(putColorsFunction, PutColorsForNodes)
    XLVK_PIPELINE(adjustHSVFunction, AdjustHSV)
    XLVK_PIPELINE(applySparklesFunction, ApplySparkles)
    XLVK_PIPELINE(brightnessContrastFunction, AdjustBrightnessContrast)
    XLVK_PIPELINE(brightnessLevelFunction, AdjustBrightnessLevel)
    XLVK_PIPELINE(firstLayerFadeFunction, FirstLayerFade)
    XLVK_PIPELINE(nonAlphaFadeFunction, NonAlphaFade)

    XLVK_PIPELINE(barsEffectFunction, BarsEffect)
    XLVK_PIPELINE(colorWashEffectFunction, ColorWashEffect)
    XLVK_PIPELINE(shockwaveEffectFunction, ShockwaveEffect)
    XLVK_PIPELINE(fanEffectFunction, FanEffect)
    XLVK_PIPELINE(spiralsEffectFunction, SpiralsEffect)
    XLVK_PIPELINE(galaxyEffectFunction, GalaxyEffect)
    XLVK_PIPELINE(circlesEffectFunction, CirclesEffect)
    XLVK_PIPELINE(plasmaEffectFunction, PlasmaEffect)
    XLVK_PIPELINE(butterflyEffectFunction, ButterflyEffect)
    XLVK_PIPELINE(pinwheelEffectFunction, PinwheelEffect)
    XLVK_PIPELINE(kaleidoscopeEffectFunction, KaleidoscopeEffect)
    XLVK_PIPELINE(warpEffectFunction, WarpEffect)
    XLVK_PIPELINE(treeEffectFunction, TreeEffect)
    XLVK_PIPELINE(shimmerEffectFunction, ShimmerEffect)
    XLVK_PIPELINE(candleEffectFunction, CandleEffect)
    XLVK_PIPELINE(waveEffectFunction, WaveEffect)
    XLVK_PIPELINE(garlandsEffectFunction, GarlandsEffect)
    XLVK_PIPELINE(fillEffectFunction, FillEffect)
    XLVK_PIPELINE(meteorsEffectFunction, MeteorsEffect)
    XLVK_PIPELINE(twinkleEffectFunction, TwinkleEffect)
    XLVK_PIPELINE(lifeEffectFunction, LifeEffect)

#define XLVK_BLEND(mix, header, ...) \
    { \
        VkPipeline p = createComputePipeline(header##_spv, sizeof(header##_spv), #header); \
        if (p == VK_NULL_HANDLE) return false; \
        blendFunctions[MixTypes::mix] = new BlendFunctionInfo(p, #header, ##__VA_ARGS__); \
    }

    XLVK_BLEND(Mix_Normal, Blend_Normal)
    XLVK_BLEND(Mix_Effect1, Blend_Effect1_2)
    // Effect1 and Effect2 share the kernel; mixTypeData selects the direction
    blendFunctions[MixTypes::Mix_Effect2] = new BlendFunctionInfo(blendFunctions[MixTypes::Mix_Effect1]->function, "Blend_Effect1_2", 1);
    XLVK_BLEND(Mix_Mask1, Blend_Mask1)
    XLVK_BLEND(Mix_Mask2, Blend_Mask2)
    XLVK_BLEND(Mix_Unmask1, Blend_Unmask1)
    XLVK_BLEND(Mix_Unmask2, Blend_Unmask2)
    XLVK_BLEND(Mix_TrueUnmask1, Blend_TrueUnmask1)
    XLVK_BLEND(Mix_TrueUnmask2, Blend_TrueUnmask2)
    XLVK_BLEND(Mix_Shadow_1on2, Blend_Shadow_1on2)
    XLVK_BLEND(Mix_Shadow_2on1, Blend_Shadow_2on1)
    XLVK_BLEND(Mix_Layered, Blend_Layered)
    XLVK_BLEND(Mix_Average, Blend_Averaged)
    XLVK_BLEND(Mix_1_reveals_2, Blend_Reveal12)
    XLVK_BLEND(Mix_2_reveals_1, Blend_Reveal21)
    XLVK_BLEND(Mix_Additive, Blend_Additive)
    XLVK_BLEND(Mix_Subtractive, Blend_Subtractive)
    XLVK_BLEND(Mix_Max, Blend_Max)
    XLVK_BLEND(Mix_Min, Blend_Min)
    XLVK_BLEND(Mix_AsBrightness, Blend_AsBrightness)
    XLVK_BLEND(Mix_Highlight, Blend_Highlight)
    XLVK_BLEND(Mix_Highlight_Vibrant, Blend_HighlightVibrant)
    XLVK_BLEND(Mix_BottomTop, Blend_BottomTop, 0, true)
    XLVK_BLEND(Mix_LeftRight, Blend_LeftRight, 0, true)
#undef XLVK_BLEND

    transitions[""] = new TransitionInfo(0);
    transitions["None"] = new TransitionInfo(0);
    transitions["Fade"] = new TransitionInfo(0);

#define XLVK_TRANSITION(name, header, type, ...) \
    { \
        VkPipeline p = createComputePipeline(header##_spv, sizeof(header##_spv), #header); \
        if (p == VK_NULL_HANDLE) return false; \
        transitions[name] = new TransitionInfo(p, type, ##__VA_ARGS__); \
    }

    XLVK_TRANSITION("Wipe", Transition_wipe, 1)
    XLVK_TRANSITION("Clock", Transition_clock, 1)
    XLVK_TRANSITION("From Middle", Transition_fromMiddle, 1)
    XLVK_TRANSITION("Circle Explode", Transition_circleExplode, 1)
    XLVK_TRANSITION("Square Explode", Transition_squareExplode, 1)
    XLVK_TRANSITION("Blend", Transition_blend, 1)
    XLVK_TRANSITION("Slide Checks", Transition_slideChecks, 1)
    XLVK_TRANSITION("Slide Bars", Transition_slideBars, 1)
    XLVK_TRANSITION("Blinds", Transition_blinds, 1)

    XLVK_TRANSITION("Shatter", Transition_shatter, 2, true)
    XLVK_TRANSITION("Star", Transition_star, 2)
    XLVK_TRANSITION("Pinwheel", Transition_pinwheel, 2, true)
    XLVK_TRANSITION("Bow Tie", Transition_bowTie, 2)
    XLVK_TRANSITION("Blobs", Transition_blobs, 2)
    XLVK_TRANSITION("Fold", Transition_fold, 2)
    XLVK_TRANSITION("Zoom", Transition_zoom, 2)
    XLVK_TRANSITION("Circular Swirl", Transition_circularSwirl, 2, true)
    XLVK_TRANSITION("Doorway", Transition_doorway, 2)
    XLVK_TRANSITION("Swap", Transition_swap, 2)
    XLVK_TRANSITION("Circles", Transition_circles, 2, true)
    XLVK_TRANSITION("Dissolve", Transition_dissolve, 3, true)
#undef XLVK_TRANSITION
#undef XLVK_PIPELINE

    int bufferSize = DissolvePatternWidth * DissolvePatternHeight;
    if (!createSharedBuffer(dissolveBuffer, bufferSize, "DissolveTransitonPattern")) {
        return false;
    }
    memcpy(dissolveBuffer.mapped, DissolveTransitonPattern, bufferSize);
    return true;
}

void VulkanComputeUtilities::computeBarrier(VkCommandBuffer cb) {
    VkMemoryBarrier mb = {};
    mb.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    mb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    mb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT |
                       VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cb,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 1, &mb, 0, nullptr, 0, nullptr);
}

// ---------------------------------------------------------------------------
// VulkanPixelBufferComputeData

VulkanPixelBufferComputeData::VulkanPixelBufferComputeData() {
}
VulkanPixelBufferComputeData::~VulkanPixelBufferComputeData() {
    VulkanComputeUtilities::INSTANCE.destroyBuffer(sparkleBuffer);
    VulkanComputeUtilities::INSTANCE.destroyBuffer(tmpBufferBlend);
}

bool VulkanPixelBufferComputeData::doTransitions(PixelBufferClass* pixelBuffer, int layer, RenderBuffer* prevRB) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    PixelBufferClass::LayerInfo* li = pixelBuffer->layers[layer];
    int ms = li->BufferHt * li->BufferWi;
    if (ms < (int)u.bufferSizeThreshold) {
        li->maskSize = 0; // start with empty mask
        return false;
    }
    if (li->inMaskFactor < 1.0 || li->outMaskFactor < 1.0) {
        VulkanRenderBufferComputeData* bd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&li->buffer);
        if (!bd) {
            li->maskSize = 0;
            return false;
        }
        if (ms > li->maskMaxSize) {
            if (bd->isCommitted()) {
                bd->waitForCompletion();
            }
            u.destroyBuffer(bd->maskBuffer);
            li->maskMaxSize = ms;
            // round up: the kernels write the byte mask through 32-bit words
            if (!u.createSharedBuffer(bd->maskBuffer, ((size_t)ms + 3) & ~(size_t)3,
                                      li->buffer.GetModelName() + "MaskBuffer-" + std::to_string(layer))) {
                li->maskMaxSize = 0;
                li->maskSize = 0;
                return false;
            }
            li->mask = static_cast<uint8_t*>(bd->maskBuffer.mapped);
        }
    }
    li->maskSize = 0; // start with empty mask

    const auto& tiIn = u.transitions.find(li->inTransitionType);
    if (tiIn == u.transitions.end()) {
        return false;
    }
    const auto& tiOut = u.transitions.find(li->outTransitionType);
    if (tiOut == u.transitions.end()) {
        return false;
    }

    TransitionData data;
    data.width = li->BufferWi;
    data.height = li->BufferHt;
    data.hasPrev = prevRB != nullptr ? 1 : 0;
    data.pWidth = 0;
    data.pHeight = 0;
    if (prevRB) {
        data.pWidth = prevRB->BufferWi;
        data.pHeight = prevRB->BufferHt;
    }
    if (li->inMaskFactor < 1.0) {
        data.progress = li->inMaskFactor;
        data.adjust = li->inTransitionAdjust;
        data.reverse = li->inTransitionReverse ? 1 : 0;
        data.isOut = 0;
        if (li->InTransitionAdjustValueCurve.IsActive()) {
            data.adjust = li->InTransitionAdjustValueCurve.GetOutputValueAt(li->inMaskFactor, li->buffer.GetStartTimeMS(), li->buffer.GetEndTimeMS());
        }
        if (tiIn->second->reversed) {
            data.progress = 1.0f - li->inMaskFactor;
        }
        if (tiIn->second->type == 1) {
            li->maskSize = li->BufferHt * li->BufferWi;
            if (!doMap(tiIn->second->function, data, &li->buffer)) {
                return false;
            }
        } else if (tiIn->second->type == 2) {
            if (!doTransition(tiIn->second->function, data, &li->buffer, prevRB)) {
                return false;
            }
        } else if (tiIn->second->type == 3) {
            if (!doTransition(tiIn->second->function, data, &li->buffer, u.dissolveBuffer.buffer)) {
                return false;
            }
        }
    }
    if (li->outMaskFactor < 1.0) {
        data.progress = li->outMaskFactor;
        data.adjust = li->outTransitionAdjust;
        data.reverse = li->outTransitionReverse ? 1 : 0;
        data.isOut = 1;
        if (li->OutTransitionAdjustValueCurve.IsActive()) {
            data.adjust = li->OutTransitionAdjustValueCurve.GetOutputValueAt(li->outMaskFactor, li->buffer.GetStartTimeMS(), li->buffer.GetEndTimeMS());
        }
        if (tiOut->second->reversed) {
            data.progress = 1.0f - li->outMaskFactor;
        }
        if (tiOut->second->type == 1) {
            li->maskSize = li->BufferHt * li->BufferWi;
            if (!doMap(tiOut->second->function, data, &li->buffer)) {
                return false;
            }
        } else if (tiOut->second->type == 2) {
            if (!doTransition(tiOut->second->function, data, &li->buffer, prevRB)) {
                return false;
            }
        } else if (tiOut->second->type == 3) {
            if (!doTransition(tiOut->second->function, data, &li->buffer, u.dissolveBuffer.buffer)) {
                return false;
            }
        }
    }
    u.statTransition++;
    return true;
}

bool VulkanPixelBufferComputeData::doMap(VkPipeline f, TransitionData& data, RenderBuffer* buffer) {
    VulkanRenderBufferComputeData* bd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(buffer);
    if (!bd->maskBuffer) {
        return false;
    }
    VkCommandBuffer cb = bd->getCommandBuffer("-Map");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    VulkanComputeUtilities::computeBarrier(cb);
    return bd->encodeDispatch(cb, f, "Map", &data, sizeof(data),
                              { bd->maskBuffer.buffer }, data.width, data.height);
}

bool VulkanPixelBufferComputeData::doTransition(VkPipeline f, TransitionData& data, RenderBuffer* buffer, RenderBuffer* prevRB) {
    VkBuffer bufferPrev = VK_NULL_HANDLE;
    if (prevRB) {
        VulkanRenderBufferComputeData* prevCD = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(prevRB);
        if (prevCD) {
            // prevRB was written by its own (already-committed) command buffer.
            // Vulkan does not serialize cross-command-buffer execution the way
            // Metal does, so fence-wait it before this transition reads it.
            prevCD->waitForCompletion();
            bufferPrev = prevCD->getPixelBuffer().buffer;
        }
        if (bufferPrev == VK_NULL_HANDLE) {
            return false;
        }
    }
    return doTransition(f, data, buffer, bufferPrev);
}

bool VulkanPixelBufferComputeData::doTransition(VkPipeline f, TransitionData& data, RenderBuffer* buffer, VkBuffer prev) {
    VulkanRenderBufferComputeData* bd = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(buffer);
    // Buffers before command buffer — see blur() for why the order matters.
    VulkanBuffer& bufferResult = bd->getPixelBuffer();
    VulkanBuffer& bufferCopy = bd->getPixelBufferCopy();
    if (!bufferResult || !bufferCopy) {
        return false;
    }
    if (prev == VK_NULL_HANDLE) {
        prev = bufferCopy.buffer;
    }
    VkCommandBuffer cb = bd->getCommandBuffer("-Transition");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    VulkanComputeUtilities::computeBarrier(cb);
    VkBufferCopy region = { 0, 0, (VkDeviceSize)(data.width * data.height * 4) };
    vkCmdCopyBuffer(cb, bufferResult.buffer, bufferCopy.buffer, 1, &region);
    VulkanComputeUtilities::computeBarrier(cb);
    return bd->encodeDispatch(cb, f, "Transition", &data, sizeof(data),
                              { bufferResult.buffer, bufferCopy.buffer, prev }, data.width, data.height);
}

void VulkanPixelBufferComputeData::fillLayerBlendingData(LayerBlendingData& data, PixelBufferClass::LayerInfo* layer) {
    data.nodeCount = layer->buffer.GetNodeCount();
    data.bufferHi = layer->buffer.BufferHt;
    data.bufferWi = layer->buffer.BufferWi;
    data.useMask = layer->maskSize > 0 ? 1 : 0;
    data.hueAdjust = layer->outputHueAdjust;
    data.valueAdjust = layer->outputValueAdjust;
    data.saturationAdjust = layer->outputSaturationAdjust;
    data.outputSparkleCount = layer->outputSparkleCount;
    data.contrast = layer->contrast;
    data.brightness = layer->outputBrightnessAdjust;
    data.isChromaKey = layer->isChromaKey ? 1 : 0;
    data.chromaSensitivity = layer->chromaSensitivity;
    data.chromaColor = toUchar4(layer->chromaKeyColour);
    data.effectMixThreshold = layer->outputEffectMixThreshold;
    data.effectMixVaries = layer->effectMixVaries ? 1 : 0;
    data.brightnessLevel = layer->brightnessLevel ? 1 : 0;
    data.mixTypeData = 0;
    data.fadeFactor = layer->fadeFactor;
    data.sparkleColor = toUchar4(layer->sparklesColour);
}

bool VulkanPixelBufferComputeData::doBlendLayers(PixelBufferClass* pixelBuffer, int effectPeriod, const std::vector<bool>& validLayers, int saveLayer, bool saveToPixels) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    if (pixelBuffer->layers[saveLayer]->buffer.GetNodeCount() < (int)u.bufferSizeThreshold) {
        return false;
    }
    for (int l = validLayers.size() - 1; l >= 0; --l) {
        if (validLayers[l]) {
            if (u.blendFunctions.find(pixelBuffer->layers[l]->mixType) == u.blendFunctions.end()) {
                return false;
            }
        }
    }

    if (!pixelBuffer->sparklesVector.empty()) {
        if (pixelBuffer->sparkles == &pixelBuffer->sparklesVector[0] && sparkleBuffer) {
            // buffer was re-initialized; the old GPU copy is stale
            u.destroyBuffer(sparkleBuffer);
        }
        if (!sparkleBuffer) {
            // round up to a whole number of words: the kernel reads uint16
            // pairs through 32-bit words
            size_t len = ((pixelBuffer->sparklesVector.size() * sizeof(uint16_t)) + 3) & ~(size_t)3;
            if (!u.createSharedBuffer(sparkleBuffer, len, pixelBuffer->GetModelName() + "SparkleBuffer")) {
                return false;
            }
            memcpy(sparkleBuffer.mapped, &pixelBuffer->sparklesVector[0], pixelBuffer->sparklesVector.size() * sizeof(uint16_t));
            pixelBuffer->sparkles = static_cast<uint16_t*>(sparkleBuffer.mapped);
        }
    }
    if (!tmpBufferBlend) {
        size_t len = pixelBuffer->layers[saveLayer]->buffer.GetNodeCount() * sizeof(uint32_t);
        if (!u.createSharedBuffer(tmpBufferBlend, len, pixelBuffer->GetModelName() + "-BlendBuffer")) {
            return false;
        }
    }
    VulkanRenderBufferComputeData* slRMRB = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&pixelBuffer->layers[saveLayer]->buffer);
    if (!slRMRB) {
        return false;
    }
    if (slRMRB->isCommitted()) {
        slRMRB->waitForCompletion();
    }

    // Acquire every buffer needed BEFORE the command buffer (grow paths can
    // reset it — see blur()).
    struct LayerBits {
        PixelBufferClass::LayerInfo* layer;
        VulkanRenderBufferComputeData* cd;
        VkBuffer blend;
        VkBuffer pixels;
        VkBuffer indexes;
        VkBuffer mask;
    };
    std::vector<LayerBits> bits;
    for (int l = validLayers.size() - 1; l >= 0; --l) {
        if (!validLayers[l]) {
            continue;
        }
        auto layer = pixelBuffer->layers[l];
        VulkanRenderBufferComputeData* layerCD = VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(&layer->buffer);
        if (!layerCD) {
            return false;
        }
        VulkanBuffer& blendBuf = layerCD->getBlendBuffer();
        VulkanBuffer& pixels = layerCD->getPixelBuffer();
        VulkanBuffer& indexes = layerCD->getIndexBuffer();
        if (!blendBuf || !pixels || !indexes) {
            return false;
        }
        VkBuffer mask = VK_NULL_HANDLE;
        if (layer->maskSize > 0) {
            // The transition mask was built by the CPU transition path —
            // upload it (kept in a per-layer staging buffer that outlives
            // the command buffer).
            VulkanBuffer& mu = layerCD->cpuMaskUpload;
            size_t len = ((size_t)layer->maskSize + 3) & ~(size_t)3;
            if (mu.size < len) {
                u.destroyBuffer(layerCD->cpuMaskUpload);
                if (!u.createSharedBuffer(layerCD->cpuMaskUpload, len, pixelBuffer->GetModelName() + "-CPUMaskUpload")) {
                    return false;
                }
            }
            memcpy(layerCD->cpuMaskUpload.mapped, layer->mask, layer->maskSize);
            mask = layerCD->cpuMaskUpload.buffer;
        }
        bits.push_back({ layer, layerCD, blendBuf.buffer, pixels.buffer, indexes.buffer, mask });
    }

    // The saveToPixels PutColors pass needs the save layer's owner, pixel and
    // index buffers.  Acquire them BEFORE the command buffer: getOwnerBuffer()
    // and getPixelBuffer() have grow paths that call waitForCompletion(), which
    // would reset the blend command buffer mid-encode (only reachable via
    // canvas mode's nested saveToPixels blend, where the save layer may not be
    // among the blended input layers gathered above).
    VkBuffer saveOwner = VK_NULL_HANDLE;
    VkBuffer savePixels = VK_NULL_HANDLE;
    VkBuffer saveIndexes = VK_NULL_HANDLE;
    if (saveToPixels) {
        VulkanBuffer& o = slRMRB->getOwnerBuffer();
        VulkanBuffer& p = slRMRB->getPixelBuffer();
        VulkanBuffer& ix = slRMRB->getIndexBuffer();
        if (!o || !p || !ix) {
            return false;
        }
        saveOwner = o.buffer;
        savePixels = p.buffer;
        saveIndexes = ix.buffer;
    }

    VkCommandBuffer cb = slRMRB->getCommandBuffer("-Blend");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }

    static const bool noGpuSparkles = (getenv("XL_NO_GPU_SPARKLES") != nullptr);

    // first load the pixel data into the buffers for blending on each layer
    for (auto& lb : bits) {
        auto layer = lb.layer;
        LayerBlendingData data;
        fillLayerBlendingData(data, layer);

        // first, we grab the color for the node from the buffer for the layer
        VulkanComputeUtilities::computeBarrier(cb);
        if (!slRMRB->encodeDispatch(cb, u.getColorsFunction, "GetColors", &data, sizeof(data),
                                    { lb.blend, lb.pixels, lb.mask, lb.indexes }, data.nodeCount, 0)) {
            return false;
        }
        if (layer->needsHSVAdjust) {
            VulkanComputeUtilities::computeBarrier(cb);
            if (!slRMRB->encodeDispatch(cb, u.adjustHSVFunction, "AdjustHSV", &data, sizeof(data),
                                        { lb.blend }, data.nodeCount, 0)) {
                return false;
            }
        }
        if (!noGpuSparkles &&
            (layer->use_music_sparkle_count ||
             layer->sparkle_count > 0 ||
             layer->outputSparkleCount > 0)) {
            VulkanComputeUtilities::computeBarrier(cb);
            if (!slRMRB->encodeDispatch(cb, u.applySparklesFunction, "ApplySparkles", &data, sizeof(data),
                                        { lb.blend, sparkleBuffer.buffer }, data.nodeCount, 0)) {
                return false;
            }
        }
        if (layer->contrast != 0 || layer->outputBrightnessAdjust != 100) {
            VulkanComputeUtilities::computeBarrier(cb);
            if (!slRMRB->encodeDispatch(cb, u.brightnessContrastFunction, "ApplyBrightnessContrast", &data, sizeof(data),
                                        { lb.blend }, data.nodeCount, 0)) {
                return false;
            }
        }
        if (layer->brightnessLevel) {
            VulkanComputeUtilities::computeBarrier(cb);
            if (!slRMRB->encodeDispatch(cb, u.brightnessLevelFunction, "ApplyBrightnessLevel", &data, sizeof(data),
                                        { lb.blend }, data.nodeCount, 0)) {
                return false;
            }
        }
    }

    // now all the pixels are loaded and adjusted, now start the blending
    bool first = true;
    for (auto& lb : bits) {
        auto layer = lb.layer;
        LayerBlendingData data;
        fillLayerBlendingData(data, layer);

        if (first) {
            first = false;
            VulkanComputeUtilities::computeBarrier(cb);
            if (!slRMRB->encodeDispatch(cb, u.firstLayerFadeFunction, "ApplyFadeBottomLayer", &data, sizeof(data),
                                        { tmpBufferBlend.buffer, lb.blend }, data.nodeCount, 0)) {
                return false;
            }
        } else {
            if (!layer->buffer.allowAlpha && layer->fadeFactor != 1.0) {
                // need to fade the first here as we're not mixing anything
                VulkanComputeUtilities::computeBarrier(cb);
                if (!slRMRB->encodeDispatch(cb, u.nonAlphaFadeFunction, "ApplyNonAlphaFade", &data, sizeof(data),
                                            { lb.blend }, data.nodeCount, 0)) {
                    return false;
                }
            }
            auto& f = u.blendFunctions[layer->mixType];
            data.mixTypeData = f->mixTypeData;
            VulkanComputeUtilities::computeBarrier(cb);
            if (!slRMRB->encodeDispatch(cb, f->function, f->name.c_str(), &data, sizeof(data),
                                        { tmpBufferBlend.buffer, lb.blend, f->needIndexes ? lb.indexes : VK_NULL_HANDLE },
                                        data.nodeCount, 0)) {
                return false;
            }
        }
    }
    if (saveToPixels) {
        auto layer = pixelBuffer->layers[saveLayer];
        LayerBlendingData data;
        fillLayerBlendingData(data, layer);
        data.useMask = 0;

        // All save-layer buffers were pre-acquired above (no grow resets cb).
        VulkanComputeUtilities::computeBarrier(cb);
        if (!slRMRB->encodeDispatch(cb, u.putColorsFunction, "PutColors", &data, sizeof(data),
                                    { savePixels, tmpBufferBlend.buffer, VK_NULL_HANDLE,
                                      saveIndexes, saveOwner },
                                    data.nodeCount, 0)) {
            return false;
        }
    }
    // The blend command buffer (slRMRB) reads every input layer's pixel
    // buffer, which was written by that layer's own separate command buffer.
    // Metal serializes command-buffer execution in commit order, so the
    // already-committed layer work is guaranteed to finish first; Vulkan gives
    // no such ordering guarantee, so we must fence-wait the input layers
    // BEFORE submitting the blend, not after.
    for (int ii = (pixelBuffer->numLayers - 1); ii >= 0; --ii) {
        if (!validLayers[ii]) {
            continue;
        }
        if (ii != saveLayer) {
            GPURenderUtils::waitForRenderCompletion(&pixelBuffer->layers[ii]->buffer);
        }
    }
    slRMRB->commit();
    slRMRB->waitForCompletion();
    u.statBlend++;

    // XL_BLENDSUM=1: dump per-layer node-color and final blend checksums to
    // stderr so two runs can be diffed to the first divergent blend stage.
    static const bool blendSum = (getenv("XL_BLENDSUM") != nullptr);
    if (blendSum) {
        auto fnv = [](const uint8_t* d, size_t n) {
            uint64_t h = 1469598103934665603ULL;
            for (size_t i = 0; i < n; i++) {
                h ^= d[i];
                h *= 1099511628211ULL;
            }
            return h;
        };
        for (auto& lb : bits) {
            VulkanBuffer& bb = lb.cd->getBlendBuffer();
            int l = 0;
            for (int ii = 0; ii < (int)pixelBuffer->layers.size(); ii++) {
                if (pixelBuffer->layers[ii] == lb.layer) {
                    l = ii;
                }
            }
            fprintf(stderr, "BSUM f=%d m=%s l=%d h=%016llx\n", effectPeriod, pixelBuffer->GetModelName().c_str(), l,
                    (unsigned long long)fnv((const uint8_t*)bb.mapped, lb.layer->buffer.GetNodeCount() * 4));
        }
        fprintf(stderr, "BSUM f=%d m=%s l=FINAL h=%016llx\n", effectPeriod, pixelBuffer->GetModelName().c_str(),
                (unsigned long long)fnv((const uint8_t*)tmpBufferBlend.mapped, pixelBuffer->layers[saveLayer]->buffer.GetNodeCount() * 4));
    }

    xlColor* colors = (xlColor*)(tmpBufferBlend.mapped);
    int nc = pixelBuffer->layers[saveLayer]->buffer.GetNodeCount();
    for (int x = 0; x < nc; x++) {
        pixelBuffer->layers[saveLayer]->buffer.Nodes[x]->SetColor(colors[x]);
    }
    return true;
}

// ---------------------------------------------------------------------------
// VulkanRenderBufferComputeData

std::atomic<uint32_t> VulkanRenderBufferComputeData::commandBufferCount(0);
#define MAX_COMMANDBUFFER_COUNT 256

VulkanRenderBufferComputeData::VulkanRenderBufferComputeData(RenderBuffer* rb, VulkanPixelBufferComputeData* pbd, int l) :
    pixelBufferData(pbd), renderBuffer(rb), layer(l) {
}

VulkanRenderBufferComputeData::~VulkanRenderBufferComputeData() {
    pixelBufferData = nullptr;
    // Unlike Metal (ARC keeps resources alive until the command buffer
    // drains), Vulkan destruction is immediate — drain first.
    if (recording || committed) {
        waitForCompletion();
    }
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    u.destroyBuffer(pixelBuffer);
    u.destroyBuffer(pixelBufferCopy);
    u.destroyBuffer(blendBuffer);
    u.destroyBuffer(indexBuffer);
    u.destroyBuffer(ownerBuffer);
    u.destroyBuffer(rotoOwnerBuffer);
    u.destroyBuffer(maskBuffer);
    u.destroyBuffer(cpuMaskUpload);
    u.destroyBuffer(paramArena);
    for (VkDescriptorPool p : descriptorPools) {
        vkDestroyDescriptorPool(u.device, p, nullptr);
    }
    if (fence != VK_NULL_HANDLE) {
        vkDestroyFence(u.device, fence, nullptr);
    }
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(u.device, commandPool, nullptr);
    }
}

VkDescriptorSet VulkanRenderBufferComputeData::allocateDescriptorSet() {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    for (;;) {
        if (activePool < descriptorPools.size()) {
            VkDescriptorSetAllocateInfo ai = {};
            ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            ai.descriptorPool = descriptorPools[activePool];
            ai.descriptorSetCount = 1;
            ai.pSetLayouts = &u.dsLayout;
            VkDescriptorSet set = VK_NULL_HANDLE;
            VkResult res = vkAllocateDescriptorSets(u.device, &ai, &set);
            if (res == VK_SUCCESS) {
                return set;
            }
            if (res != VK_ERROR_OUT_OF_POOL_MEMORY && res != VK_ERROR_FRAGMENTED_POOL) {
                return VK_NULL_HANDLE;
            }
            ++activePool;
            continue;
        }
        VkDescriptorPoolSize ps = {};
        ps.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        ps.descriptorCount = 128 * VulkanComputeUtilities::NUM_BINDINGS;
        VkDescriptorPoolCreateInfo pi = {};
        pi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pi.maxSets = 128;
        pi.poolSizeCount = 1;
        pi.pPoolSizes = &ps;
        VkDescriptorPool pool = VK_NULL_HANDLE;
        if (vkCreateDescriptorPool(u.device, &pi, nullptr, &pool) != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }
        descriptorPools.push_back(pool);
        activePool = descriptorPools.size() - 1;
    }
}

void VulkanRenderBufferComputeData::resetDescriptorPools() {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    for (VkDescriptorPool p : descriptorPools) {
        vkResetDescriptorPool(u.device, p, 0);
    }
    activePool = 0;
    paramArenaOffset = 0;
}

bool VulkanRenderBufferComputeData::stageParams(const void* data, size_t size, VkBuffer& outBuf, VkDeviceSize& outOffset) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    // Fixed 64KB arena: each effect uploads one param struct (<=~1KB) per
    // command buffer, so this never fills.  Creating the buffer lazily is safe
    // during recording (it doesn't touch the command buffer); growing it would
    // require freeing the old buffer that earlier dispatches in this command
    // buffer still reference, so we never grow mid-recording — an oversized
    // request just falls back to CPU.
    static const VkDeviceSize ARENA_SIZE = 64 * 1024;
    if (!paramArena) {
        if (!u.createSharedBuffer(paramArena, ARENA_SIZE, renderBuffer->GetModelName() + "-ParamArena")) {
            return false;
        }
    }
    VkDeviceSize align = u.storageBufferAlignment;
    VkDeviceSize off = (paramArenaOffset + align - 1) & ~(align - 1);
    if (off + size > paramArena.size) {
        return false;
    }
    memcpy(static_cast<uint8_t*>(paramArena.mapped) + off, data, size);
    outBuf = paramArena.buffer;
    outOffset = off;
    paramArenaOffset = off + size;
    return true;
}

bool VulkanRenderBufferComputeData::encodeEffectDispatch(VkCommandBuffer cb, VkPipeline pipeline, const char* label,
                                                         const void* params, uint32_t paramsSize,
                                                         std::initializer_list<VkBuffer> buffers,
                                                         uint32_t gridW, uint32_t gridH) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VkBuffer paramBuf = VK_NULL_HANDLE;
    VkDeviceSize paramOff = 0;
    if (!stageParams(params, paramsSize, paramBuf, paramOff)) {
        return false;
    }
    VkDescriptorSet set = allocateDescriptorSet();
    if (set == VK_NULL_HANDLE) {
        return false;
    }
    VkDescriptorBufferInfo bufferInfos[VulkanComputeUtilities::NUM_BINDINGS];
    VkWriteDescriptorSet writes[VulkanComputeUtilities::NUM_BINDINGS];
    uint32_t i = 0;
    for (VkBuffer b : buffers) {
        bufferInfos[i] = { b != VK_NULL_HANDLE ? b : u.dummyBuffer.buffer, 0, VK_WHOLE_SIZE };
        ++i;
    }
    for (; i < VulkanComputeUtilities::NUM_BINDINGS - 1; i++) {
        bufferInfos[i] = { u.dummyBuffer.buffer, 0, VK_WHOLE_SIZE };
    }
    // binding 5 = params SSBO at its arena offset
    bufferInfos[VulkanComputeUtilities::NUM_BINDINGS - 1] = { paramBuf, paramOff, paramsSize };
    for (i = 0; i < VulkanComputeUtilities::NUM_BINDINGS; i++) {
        writes[i] = {};
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = set;
        writes[i].dstBinding = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[i].pBufferInfo = &bufferInfos[i];
    }
    vkUpdateDescriptorSets(u.device, VulkanComputeUtilities::NUM_BINDINGS, writes, 0, nullptr);

    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_COMPUTE, u.pipelineLayout, 0, 1, &set, 0, nullptr);
    if (gridH == 0) {
        vkCmdDispatch(cb, (gridW + 63) / 64, 1, 1);
    } else {
        vkCmdDispatch(cb, (gridW + 7) / 8, (gridH + 7) / 8, 1);
    }
    u.statEffect++;
    return true;
}

bool VulkanRenderBufferComputeData::encodeDispatch(VkCommandBuffer cb, VkPipeline pipeline, const char* label,
                                                   const void* push, uint32_t pushSize,
                                                   std::initializer_list<VkBuffer> buffers,
                                                   uint32_t gridW, uint32_t gridH) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    VkDescriptorSet set = allocateDescriptorSet();
    if (set == VK_NULL_HANDLE) {
        return false;
    }
    VkDescriptorBufferInfo bufferInfos[VulkanComputeUtilities::NUM_BINDINGS];
    VkWriteDescriptorSet writes[VulkanComputeUtilities::NUM_BINDINGS];
    uint32_t i = 0;
    for (VkBuffer b : buffers) {
        bufferInfos[i] = { b != VK_NULL_HANDLE ? b : u.dummyBuffer.buffer, 0, VK_WHOLE_SIZE };
        ++i;
    }
    for (; i < VulkanComputeUtilities::NUM_BINDINGS; i++) {
        bufferInfos[i] = { u.dummyBuffer.buffer, 0, VK_WHOLE_SIZE };
    }
    for (i = 0; i < VulkanComputeUtilities::NUM_BINDINGS; i++) {
        writes[i] = {};
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = set;
        writes[i].dstBinding = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[i].pBufferInfo = &bufferInfos[i];
    }
    vkUpdateDescriptorSets(u.device, VulkanComputeUtilities::NUM_BINDINGS, writes, 0, nullptr);

    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_COMPUTE, u.pipelineLayout, 0, 1, &set, 0, nullptr);
    if (push != nullptr && pushSize > 0) {
        vkCmdPushConstants(cb, u.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, pushSize, push);
    }
    // Kernels bounds-check (vkCmdDispatch rounds up; there is no Metal-style
    // exact-grid dispatch).  gridH == 0 selects the 1-D 64-wide form.
    if (gridH == 0) {
        vkCmdDispatch(cb, (gridW + 63) / 64, 1, 1);
    } else {
        vkCmdDispatch(cb, (gridW + 7) / 8, (gridH + 7) / 8, 1);
    }
    return true;
}

VulkanRenderBufferComputeData* VulkanRenderBufferComputeData::getVulkanRenderBufferComputeData(RenderBuffer* b) {
    return static_cast<VulkanRenderBufferComputeData*>(b->gpuRenderData);
}

bool VulkanRenderBufferComputeData::ensureCommandInfra() {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    if (commandPool != VK_NULL_HANDLE) {
        return true;
    }
    VkCommandPoolCreateInfo pi = {};
    pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pi.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    pi.queueFamilyIndex = u.queueFamilyIndex;
    if (vkCreateCommandPool(u.device, &pi, nullptr, &commandPool) != VK_SUCCESS) {
        return false;
    }
    VkCommandBufferAllocateInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = commandPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(u.device, &ai, &commandBuffer) != VK_SUCCESS) {
        vkDestroyCommandPool(u.device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
        return false;
    }
    VkFenceCreateInfo fi = {};
    fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if (vkCreateFence(u.device, &fi, nullptr, &fence) != VK_SUCCESS) {
        vkDestroyCommandPool(u.device, commandPool, nullptr);
        commandPool = VK_NULL_HANDLE;
        commandBuffer = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

VkCommandBuffer VulkanRenderBufferComputeData::getCommandBuffer(const std::string& postfix) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    if (!u.enabled) {
        return VK_NULL_HANDLE;
    }
    if (recording && committed) {
        // This should not happen.  If we get here, some work was sent
        // to the GPU, but then nothing asked for the result so the
        // work was irrelevant.   That would need to be tracked down.
        waitForCompletion();
    }
    if (!recording) {
        uint32_t max = MAX_COMMANDBUFFER_COUNT - 4;
        if (u.prioritizeGraphics()) {
            // use a lower command buffer count if the GPU is needed for frontend
            max = 64;
        }
        if (commandBufferCount.fetch_add(1) > max) {
            --commandBufferCount;
            static std::atomic<long> s_cbNil{0};
            long n = ++s_cbNil;
            if ((n & (n - 1)) == 0) { // powers of two, avoid log spam
                fprintf(stderr, "XLDBG: getCommandBuffer over-limit fallback count=%ld\n", n);
            }
            return VK_NULL_HANDLE;
        }
        if (!ensureCommandInfra()) {
            --commandBufferCount;
            return VK_NULL_HANDLE;
        }
        VkCommandBufferBeginInfo bi = {};
        bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (vkBeginCommandBuffer(commandBuffer, &bi) != VK_SUCCESS) {
            --commandBufferCount;
            return VK_NULL_HANDLE;
        }
        u.setObjectName((uint64_t)commandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER,
                        renderBuffer->GetModelName() + "-" + std::to_string(layer) + postfix);
        recording = true;
    }
    return commandBuffer;
}

void VulkanRenderBufferComputeData::abortCommandBuffer() {
    if (recording && !committed) {
        vkEndCommandBuffer(commandBuffer);
        vkResetCommandPool(VulkanComputeUtilities::INSTANCE.device, commandPool, 0);
        resetDescriptorPools();
        recording = false;
        --commandBufferCount;
    }
}

void VulkanRenderBufferComputeData::commit() {
    if (recording && !committed) {
        VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo si = {};
        si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.commandBufferCount = 1;
        si.pCommandBuffers = &commandBuffer;
        VkResult res;
        {
            std::unique_lock<std::mutex> lock(u.queueMutex);
            res = vkQueueSubmit(u.queue, 1, &si, fence);
        }
        if (res != VK_SUCCESS) {
            if (res == VK_ERROR_DEVICE_LOST) {
                spdlog::error("Vulkan compute: device lost on submit — disabling GPU backend for this session");
                u.enabled = false;
            }
            // Treat as an empty submit: no fence will signal, so mark the
            // fence signaled state by resetting to a clean non-pending pool.
            vkResetCommandPool(u.device, commandPool, 0);
            resetDescriptorPools();
            recording = false;
            --commandBufferCount;
            return;
        }
        committed = true;
    }
}

void VulkanRenderBufferComputeData::waitForCompletion() {
    if (recording || committed) {
        VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
        commit();
        if (committed) {
            VkResult res = vkWaitForFences(u.device, 1, &fence, VK_TRUE, UINT64_MAX);
            if (res == VK_ERROR_DEVICE_LOST) {
                spdlog::error("Vulkan compute: device lost on wait — disabling GPU backend for this session");
                u.enabled = false;
            }
            vkResetFences(u.device, 1, &fence);
            vkResetCommandPool(u.device, commandPool, 0);
            resetDescriptorPools();
            recording = false;
            committed = false;
            --commandBufferCount;
        }
    }
}

VulkanBuffer& VulkanRenderBufferComputeData::getBlendBuffer() {
    size_t len = renderBuffer->GetNodeCount() * sizeof(uint32_t);
    if (blendBuffer && blendBuffer.size < len) {
        if (recording || committed) {
            waitForCompletion();
        }
        VulkanComputeUtilities::INSTANCE.destroyBuffer(blendBuffer);
    }
    if (!blendBuffer) {
        VulkanComputeUtilities::INSTANCE.createSharedBuffer(blendBuffer, len, renderBuffer->GetModelName() + "-WorkBuffer" + std::to_string(layer));
    }
    return blendBuffer;
}

VulkanBuffer& VulkanRenderBufferComputeData::getIndexBuffer() {
    return indexBuffer;
}

// pixel -> owning node table for PutColorsForNodes.  Multiple nodes can map
// to the same buffer pixel (group buffers especially); an ungated GPU scatter
// lets whichever thread lands last win, which made canvas-preload output
// non-deterministic run to run.  The owner is the last node in Nodes order
// covering the pixel, matching the serial CPU loop's last-node-wins result.
VulkanBuffer& VulkanRenderBufferComputeData::getOwnerBuffer() {
    int pixelCount = renderBuffer->GetPixelCount();
    if (!ownerBuffer || ownerSize < pixelCount) {
        if (recording || committed) {
            waitForCompletion();
        }
        VulkanComputeUtilities::INSTANCE.destroyBuffer(ownerBuffer);
        if (!VulkanComputeUtilities::INSTANCE.createSharedBuffer(ownerBuffer, pixelCount * sizeof(int32_t), renderBuffer->GetModelName() + "OwnerBuffer")) {
            ownerSize = 0;
            return ownerBuffer;
        }
        ownerSize = pixelCount;
        ownerStale = true;
    }
    if (ownerStale) {
        int32_t* owner = static_cast<int32_t*>(ownerBuffer.mapped);
        std::fill(owner, owner + pixelCount, -1);
        int32_t idx = 0;
        for (auto& n : renderBuffer->Nodes) {
            for (auto& c : n->Coords) {
                if (c.bufY >= 0 && c.bufY < renderBuffer->BufferHt &&
                    c.bufX >= 0 && c.bufX < renderBuffer->BufferWi) {
                    int32_t pidx = c.bufY * renderBuffer->BufferWi + c.bufX;
                    if (pidx < pixelCount) {
                        owner[pidx] = idx;
                    }
                }
            }
            ++idx;
        }
        ownerStale = false;
    }
    return ownerBuffer;
}

VulkanBuffer& VulkanRenderBufferComputeData::getPixelBufferCopy() {
    if (!pixelBufferCopy) {
        size_t bufferSize = std::max((size_t)renderBuffer->GetPixelCount(), pixelBufferSize) * 4;
        VulkanComputeUtilities::INSTANCE.createDeviceBuffer(pixelBufferCopy, bufferSize, renderBuffer->GetModelName() + "PixelBufferCopy");
    }
    return pixelBufferCopy;
}

VulkanBuffer& VulkanRenderBufferComputeData::getRotoOwnerBuffer() {
    int pixelCount = renderBuffer->BufferWi * renderBuffer->BufferHt;
    if (!rotoOwnerBuffer || rotoOwnerSize < pixelCount) {
        if (recording || committed) {
            waitForCompletion();
        }
        VulkanComputeUtilities::INSTANCE.destroyBuffer(rotoOwnerBuffer);
        if (VulkanComputeUtilities::INSTANCE.createDeviceBuffer(rotoOwnerBuffer, pixelCount * sizeof(int32_t), renderBuffer->GetModelName() + "RotoOwnerBuffer")) {
            rotoOwnerSize = pixelCount;
        } else {
            rotoOwnerSize = 0;
        }
    }
    return rotoOwnerBuffer;
}

void VulkanRenderBufferComputeData::bufferResized() {
    if (pixelBuffer && pixelBufferSize < renderBuffer->GetPixelCount()) {
        //buffer needs to get bigger
        getPixelBuffer(false);
    }
    ownerStale = true;
    int indexCount = renderBuffer->Nodes.size();
    for (auto& n : renderBuffer->Nodes) {
        if (n->Coords.size() > 1) {
            indexCount += n->Coords.size() + 1;
        }
    }
    if (indexesSize < indexCount) {
        if (recording || committed) {
            waitForCompletion();
        }
        VulkanComputeUtilities::INSTANCE.destroyBuffer(indexBuffer);
        indexes = nullptr;
        if (!VulkanComputeUtilities::INSTANCE.createSharedBuffer(indexBuffer, indexCount * sizeof(int32_t), renderBuffer->GetModelName() + "IndexBuffer")) {
            indexesSize = 0;
            return;
        }
        indexes = static_cast<int32_t*>(indexBuffer.mapped);
        indexesSize = indexCount;
    }
    if (indexes == nullptr) {
        return;
    }
    int idx = 0;
    int extraIdx = renderBuffer->Nodes.size();
    for (auto& n : renderBuffer->Nodes) {
        if (n->Coords.size() > 1) {
            indexes[idx] = extraIdx | 0x80000000;
            int countIdx = extraIdx++;
            indexes[countIdx] = n->Coords.size();
            for (auto& c : n->Coords) {
                if (c.bufY < 0 || c.bufY >= renderBuffer->BufferHt ||
                    c.bufX < 0 || c.bufX >= renderBuffer->BufferWi) {
                    indexes[countIdx] -= 1;
                } else {
                    int32_t pidx = c.bufY * renderBuffer->BufferWi + c.bufX;
                    indexes[extraIdx++] = pidx;
                }
            }
        } else if (n->Coords.empty()) {
            // Node with zero coords — treat the same as a node whose single
            // coord is out-of-bounds (sentinel -1); dereferencing Coords[0]
            // on an empty vector crashed on the iPad.
            indexes[idx] = -1;
        } else if (n->Coords[0].bufY < 0 || n->Coords[0].bufY >= renderBuffer->BufferHt ||
                   n->Coords[0].bufX < 0 || n->Coords[0].bufX >= renderBuffer->BufferWi) {
            indexes[idx] = -1;
        } else {
            int32_t pidx = n->Coords[0].bufY * renderBuffer->BufferWi + n->Coords[0].bufX;
            indexes[idx] = pidx;
        }
        ++idx;
    }
}

VulkanBuffer& VulkanRenderBufferComputeData::getPixelBuffer(bool sendToGPU) {
    if (pixelBufferSize < renderBuffer->GetPixelCount()) {
        // Never replace a buffer the GPU may still be reading/writing.
        if (recording || committed) {
            waitForCompletion();
        }
        VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
        size_t bufferSize = renderBuffer->GetPixelCount() * 4;
        VulkanBuffer newBuffer;
        if (!u.createSharedBuffer(newBuffer, bufferSize, renderBuffer->GetModelName() + "PixelBuffer")) {
            // Allocation failed: point the render buffer back at its own
            // vector so CPU rendering stays correct, and report no buffer.
            u.destroyBuffer(pixelBuffer);
            u.destroyBuffer(pixelBufferCopy);
            pixelBufferSize = 0;
            renderBuffer->pixels = renderBuffer->pixelVector.data();
            return pixelBuffer;
        }
        // copy from the old buffer (which renderBuffer->pixels points into) before reassigning it
        memcpy(newBuffer.mapped, renderBuffer->pixels, pixelBufferSize == 0 ? bufferSize : pixelBufferSize * 4);
        u.destroyBuffer(pixelBufferCopy);
        u.destroyBuffer(pixelBuffer);
        pixelBufferSize = renderBuffer->pixelVector.size();
        pixelBuffer = newBuffer;
        renderBuffer->pixels = static_cast<xlColor*>(pixelBuffer.mapped);
    }
    return pixelBuffer;
}

bool VulkanRenderBufferComputeData::blur(int radius) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    if ((renderBuffer->BufferHt < (radius * 2)) || (renderBuffer->BufferWi < (radius * 2)) ||
        ((uint32_t)(renderBuffer->BufferWi * renderBuffer->BufferHt) < u.bufferSizeThreshold)) {
        // Smallish buffer, overhead of sending to GPU will be more than the gain
        return false;
    }
    // tent blur is closest to what is implemented on the C++/CPU side; the
    // separable TentBlurH/V kernels are bit-exact on every run with the same
    // kernel width and clamp edge handling.  Ping-pong through
    // pixelBufferCopy so only two passes are needed.
    int kernelWidth = (radius - 1) * 2 - 1;
    if (kernelWidth < 3) {
        // matches MPSImageTent with kernelWidth 1 — an identity filter
        return true;
    }
    // Acquire buffers BEFORE the command buffer: a buffer-grow path calls
    // waitForCompletion(), which would end/reset a command buffer handed out
    // just above it (Metal's buffer getters never touch the command buffer,
    // so its ordering is safe — ours is not).
    VulkanBuffer& px = getPixelBuffer();
    VulkanBuffer& tmp = getPixelBufferCopy();
    if (!px || !tmp) {
        return false;
    }
    VkCommandBuffer cb = getCommandBuffer("-Blur");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }

    TentBlurData data;
    data.width = renderBuffer->BufferWi;
    data.height = renderBuffer->BufferHt;
    data.halfK = (kernelWidth - 1) / 2;

    // {dst, src} per pass
    VkBuffer bufs[2][2] = { { tmp.buffer, px.buffer }, { px.buffer, tmp.buffer } };
    VkPipeline fns[2] = { u.tentBlurHFunction, u.tentBlurVFunction };
    for (int pass = 0; pass < 2; pass++) {
        VulkanComputeUtilities::computeBarrier(cb);
        if (!encodeDispatch(cb, fns[pass], pass == 0 ? "TentBlurH" : "TentBlurV",
                            &data, sizeof(data), { bufs[pass][0], bufs[pass][1] },
                            data.width, data.height)) {
            return false;
        }
    }
    u.statBlur++;
    static const bool blurSync = (getenv("XLDBG_BLURSYNC") != nullptr);
    if (blurSync) {
        commit();
        waitForCompletion();
    }
    return true;
}

bool VulkanRenderBufferComputeData::rotoZoom(GPURenderUtils::RotoZoomSettings& settings) {
    if ((renderBuffer->BufferWi * renderBuffer->BufferHt) < 256) {
        // Smallish buffer, overhead of sending to GPU will be more than the gain
        return false;
    }
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;

    RotoZoomData data;
    data.width = renderBuffer->BufferWi;
    data.height = renderBuffer->BufferHt;

    data.offset = settings.offset;
    data.xrotation = settings.xrotation;
    data.xpivot = settings.xpivot;
    data.yrotation = settings.yrotation;
    data.ypivot = settings.ypivot;
    data.zrotation = settings.zrotation;
    data.zoom = settings.zoom;
    data.zoomquality = settings.zoomquality;
    data.pivotpointx = settings.pivotpointx;
    data.pivotpointy = settings.pivotpointy;

    if (!getPixelBuffer()) {
        return false;
    }
    if (!getPixelBufferCopy()) {
        return false;
    }
    for (auto& c : settings.rotationorder) {
        switch (c) {
        case 'X':
            if (data.xrotation != 0 && data.xrotation != 360) {
                callRotoZoomFunction(u.xrotateFunction, u.xrotateClaimFunction, data);
            }
            break;
        case 'Y':
            if (data.yrotation != 0 && data.yrotation != 360) {
                callRotoZoomFunction(u.yrotateFunction, u.yrotateClaimFunction, data);
            }
            break;
        case 'Z':
            if (data.zrotation != 0.0 || data.zoom != 1.0) {
                callRotoZoomFunction(u.zrotateFunction, u.zrotateClaimFunction, data);
            }
            break;
        }
    }
    return true;
}

bool VulkanRenderBufferComputeData::callRotoZoomFunction(VkPipeline function, VkPipeline claimFunction, RotoZoomData& data) {
    VulkanComputeUtilities& u = VulkanComputeUtilities::INSTANCE;
    // Buffers before command buffer — see blur() for why the order matters.
    VulkanBuffer& bufferResult = getPixelBuffer();
    VulkanBuffer& bufferCopy = getPixelBufferCopy();
    VulkanBuffer& owner = getRotoOwnerBuffer();
    if (!bufferResult || !bufferCopy || !owner) {
        return false;
    }
    VkCommandBuffer cb = getCommandBuffer("-RotoZoom");
    if (cb == VK_NULL_HANDLE) {
        return false;
    }
    int pixelCount = data.width * data.height;

    // snapshot the current pixels, and fill the claim buffer with -1
    // (0xFF bytes) so the claim pass can atomicMax the winning source
    // index into it
    VulkanComputeUtilities::computeBarrier(cb);
    VkBufferCopy region = { 0, 0, (VkDeviceSize)(data.width * data.height * 4) };
    vkCmdCopyBuffer(cb, bufferResult.buffer, bufferCopy.buffer, 1, &region);
    vkCmdFillBuffer(cb, owner.buffer, 0, pixelCount * sizeof(int32_t), 0xFFFFFFFFu);

    VulkanComputeUtilities::computeBarrier(cb);
    if (!encodeDispatch(cb, u.rotateBlankFunction, "RotoZoomBlank",
                        &data, sizeof(data), { bufferResult.buffer }, data.width, data.height)) {
        return false;
    }

    // claim pass: record the highest source index per destination so the
    // write pass has a deterministic winner for colliding pixels
    VulkanComputeUtilities::computeBarrier(cb);
    if (!encodeDispatch(cb, claimFunction, "RotoZoomClaim",
                        &data, sizeof(data), { owner.buffer }, data.width, data.height)) {
        return false;
    }

    VulkanComputeUtilities::computeBarrier(cb);
    if (!encodeDispatch(cb, function, "RotoZoomWrite",
                        &data, sizeof(data), { bufferResult.buffer, bufferCopy.buffer, owner.buffer },
                        data.width, data.height)) {
        return false;
    }
    u.statRotoZoom++;
    return true;
}

#endif
