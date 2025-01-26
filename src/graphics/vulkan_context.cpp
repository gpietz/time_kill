#include "vulkan_context.hpp"
#include "vulkan_tools.hpp"
#include "core/logger.hpp"
#include <algorithm>
#include <ranges>
#include <map>
#include <iostream>
#include <set>

const std::vector<const char*> ValidationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> DeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

namespace time_kill::graphics {
    VulkanContext::VulkanContext(const core::Window& window, const bool debugEnabled)
        : debugEnabled_(debugEnabled), instance_(nullptr), debugMessenger_(nullptr),
          surface_(nullptr), physicalDevice_(nullptr), logicalDevice_(nullptr),
          graphicsQueue_(nullptr), presentQueue_(nullptr)
    {
        if (!glfwVulkanSupported()) {
            throw std::runtime_error("Vulkan is not supported by GLFW");
        }

        createInstance(window);
        createDebugMessenger(window);
        createSurface(window);
        pickPhysicalDevice(window);
        createLogicalDevice(window);
    }

    VulkanContext::~VulkanContext() {
        if (logicalDevice_ != nullptr) {
            queuesWaitIdle();
            vkDestroyDevice(logicalDevice_, nullptr);
            logicalDevice_ = nullptr;
        }
        if (surface_ != nullptr) {
            vkDestroySurfaceKHR(instance_, surface_, nullptr);
            surface_ = nullptr;
        }
        if (debugEnabled_ && debugMessenger_ != nullptr) {
            cleanupDebugMessenger();
            debugMessenger_ = nullptr;
        }
        if (instance_ != nullptr) {
            vkDestroyInstance(instance_, nullptr);
            instance_ = nullptr;
        }
    }

    void VulkanContext::queuesWaitIdle() const {
        // Wait for the end of all operations in the graphics queue
        VulkanTools::queueWaitIdle(graphicsQueue_);
        // Wait for the end of all operations in the present queue
        VulkanTools::queueWaitIdle(presentQueue_);
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    ) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    void VulkanContext::createDebugMessenger(const core::Window& window) {
        if (!debugEnabled_) {
            return;
        }
        if (instance_ == nullptr) {
            throw std::runtime_error("Vulkan instance is not available!");
        }

        // Load the function pointer
        PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
        vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT")
        );
        if (!vkCreateDebugUtilsMessengerEXT) {
            throw std::runtime_error("Failed to load vkCreateDebugUtilsMessengerEXT!");
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                               | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = this;

        if (vkCreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create debug messenger!");
        }

        core::Logger::getInstance().info("Created Vulkan debug messenger successfully.");
    }

    void VulkanContext::cleanupDebugMessenger() {
        const auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT")
        );

        if (vkDestroyDebugUtilsMessengerEXT != nullptr) {
            vkDestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
        }

        debugMessenger_ = nullptr;
    }

    void VulkanContext::createInstance(const core::Window& window) {
        if (debugEnabled_ && !checkValidationLayerSupport(ValidationLayers)) {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "TimeKill";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Retrieve GLFW extensions
        uint32_t extensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        logGlfwVulkanExtensions(extensionCount, glfwExtensions);

        // Enable extensions (aka getRequiredExtensions)
        Vector<const char*> extensions(glfwExtensions, glfwExtensions + extensionCount);
        if (debugEnabled_) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Validation Layers (Debug)
        if (debugEnabled_) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
            createInfo.ppEnabledLayerNames = ValidationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (const auto result = vkCreateInstance(&createInfo, nullptr, &instance_); result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance (vkCreateInstance failed)");
        }

        core::Logger::getInstance().debug("Created Vulkan instance successfully.");
    }

    void VulkanContext::createSurface(const core::Window& window) {
        // Use GLFW to crate a Vulkan surface
        if (glfwCreateWindowSurface(instance_, window.window_.get(), nullptr, &surface_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }

        core::Logger::getInstance().info("Vulkan surface created successfully.");
    }

    void VulkanContext::pickPhysicalDevice(const core::Window &window) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

        std::multimap<int, VkPhysicalDevice> candidates;
        for (const auto& device : devices) {
            int score = rateDeviceSuitability(device, window);
            candidates.insert(std::make_pair(score, device));
        }

        // Select the best device (highest score)
        if (candidates.rbegin()->first == 0) {
            physicalDevice_ = candidates.begin()->second;
            const auto deviceName = VulkanTools::getDeviceName(physicalDevice_);
            core::Logger::getInstance().info("Vulkan physical device found: " + deviceName);
        } else {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    int VulkanContext::rateDeviceSuitability(VkPhysicalDevice device, const core::Window& window) const {
        // Properties
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        // Features
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        int score = 0;

        // Prefer dedicated GPUs
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        // Consider maximum texture size (could be relevant for your application)
        score += static_cast<int>(deviceProperties.limits.maxImageDimension2D);

        // Geometry shader support is important
        if (!deviceFeatures.geometryShader) {
            return 0;
        }

        // Check the support for required queue families
        if (const QueueFamilyIndices indices =
            VulkanTools::findQueueFamilies(instance_, surface_, device); !indices.isComplete()) {
            return 0;
        }

        // Check whether the GPU supports the required extensions
        if (!checkDeviceExtensionSupport(device)) {
            return 0;
        }

        // Check swapchain support
        SwapchainSupportDetails swapChainSupport = querySwapchainSupport(device, window);
        if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
            return 0;
        }

        // Consider memory size (can be important for applications with large textures or models)
        VkPhysicalDeviceMemoryProperties memoryProperties = {};
        vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
        uint64_t deviceMemorySize = 0;
        for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
            if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                deviceMemorySize += memoryProperties.memoryHeaps[i].size;
            }
        }
        score += static_cast<int>(deviceMemorySize / (1024 * 1024)); // Memory in MB

        // Support for certain features (e.g. tessellation shader, anisotropy)
        if (deviceFeatures.tessellationShader) {
            score += 500;
        }
        if (deviceFeatures.samplerAnisotropy) {
            score += 500;
        }

        // Support for modern Vulkan features (optional)
        VkPhysicalDeviceFeatures2  supportedFeatures = {};
        supportedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceFeatures2(device, &supportedFeatures);

        if (supportedFeatures.features.multiViewport) {
            score += 500; // Bonus for multi-viewport support
        }

        // Support for ray tracing (optional)
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures = {};
        rayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

        VkPhysicalDeviceFeatures2 features2 = {};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = &rayTracingFeatures;
        vkGetPhysicalDeviceFeatures2(device, &features2);

        if (rayTracingFeatures.rayTracingPipeline) {
            score += 1000;
        }

        return score;
    }

    void VulkanContext::createLogicalDevice(const core::Window& window) {
        if (physicalDevice_ == nullptr) {
            throw std::runtime_error("Physical device not selected!");
        }

        const auto deviceName = VulkanTools::getDeviceName(physicalDevice_);

        const auto [graphicsFamily, presentFamily] =
            VulkanTools::findQueueFamilies(instance_, surface_, physicalDevice_);

        if (!graphicsFamily.has_value() || !presentFamily.has_value()) {
            throw std::runtime_error("Failed to find required queue families!");
        }

        // Create a set of unique queue families
        Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { graphicsFamily.value(), presentFamily.value() };

        // Define queue priorities
        float queuePriority = 1.0f;

        // Create queue create info for each unique queue family
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        //  Specify device features
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.geometryShader = VK_TRUE;

        // Create logical device info
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        // Enable device extensions
        createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = DeviceExtensions.data();

        // Enable validation layers (if debug is enabled)
        if (debugEnabled_) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
            createInfo.ppEnabledLayerNames  = ValidationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        // Create the logical device
        if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &logicalDevice_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device for GPU: " +  deviceName);
        }
        if (logicalDevice_ == nullptr) {
            throw std::runtime_error("Failed to create logical device! (logicalDevice isn't present)");
        }

        // Retrieve queue handles
        vkGetDeviceQueue(logicalDevice_, graphicsFamily.value(), 0, &graphicsQueue_);
        vkGetDeviceQueue(logicalDevice_, presentFamily.value(), 0, &presentQueue_);

        if (graphicsQueue_ == nullptr) {
            throw std::runtime_error("Failed to create graphics queue!");
        }
        if (presentQueue_ == nullptr) {
            throw std::runtime_error("Failed to create presenting queue!");
        }

        core::Logger::getInstance().debug("Created logical device for GPU: " + deviceName);
    }

    void VulkanContext::logGlfwVulkanExtensions(const uint32_t extensionCount, const char** glfwExtensions) {
        if (extensionCount == 0) {
            core::Logger::getInstance().info("GLFW requires no Vulkan instance extensions.");
            return;
        }

        if (!glfwExtensions) {
            throw std::runtime_error("GLFW extensions not supported (nullptr)");
        }

        core::Logger::getInstance().info("GLFW extension count: " + std::to_string(extensionCount));

        String extensions;
        std::ranges::for_each(std::views::iota(0u, extensionCount), [&](const auto& item) {
            extensions += glfwExtensions[item];
            if (item < extensionCount - 1) {
                extensions += ", ";
            }
        });

        core::Logger::getInstance().debug("GLFW required Vulkan extensions: " + extensions);
    }

    bool VulkanContext::checkDeviceExtensionSupport(const VkPhysicalDevice device) {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        Vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<String> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    SwapchainSupportDetails VulkanContext::querySwapchainSupport(
        const VkPhysicalDevice device,
        const core::Window& window
    ) const {
        SwapchainSupportDetails details = {};

        // Get surface capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

        // Retrieve supported Surface formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);
        if (formatCount == 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
        }

        // Retrieve supported presentation modes
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
        if (presentModeCount == 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
        }

        return details;
    }
}
