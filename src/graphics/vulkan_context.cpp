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
        : debugEnabled_(debugEnabled), debugMessenger_(nullptr)
    {
        if (!glfwVulkanSupported()) {
            throw std::runtime_error("Vulkan is not supported by GLFW");
        }

        resources_ = std::make_shared<VulkanResources>();

        createInstance(window);
        createDebugMessenger(window);
        createSurface(window);
        pickPhysicalDevice(window);
        createLogicalDevice(window);

        swapchain_ = std::make_shared<VulkanSwapchain>();
        swapchain_->createSwapchain(window, resources_);
    }

    VulkanContext::~VulkanContext() {
        // FIXME This must be done in VulkanResources?
        // if (logicalDevice_ != nullptr) {
        //     queuesWaitIdle();
        //     vkDestroyDevice(logicalDevice_, nullptr);
        //     logicalDevice_ = nullptr;
        // }
        // if (surface_ != nullptr) {
        //     vkDestroySurfaceKHR(instance_, surface_, nullptr);
        //     surface_ = nullptr;
        // }
        // if (debugEnabled_ && debugMessenger_ != nullptr) {
        //     cleanupDebugMessenger();
        //     debugMessenger_ = nullptr;
        // }
        // if (instance_ != nullptr) {
        //     vkDestroyInstance(instance_, nullptr);
        //     instance_ = nullptr;
        // }
    }

    void VulkanContext::queuesWaitIdle() const {
        const auto res = resources_;

        // Wait for the end of all operations in the graphics queue
        VulkanTools::queueWaitIdle(res->graphicsQueue);
        // Wait for the end of all operations in the present queue
        VulkanTools::queueWaitIdle(res->presentQueue);
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

        const auto res = resources_;

        if (res->instance == nullptr) {
            throw std::runtime_error("Vulkan instance is not available!");
        }

        // Load the function pointer
        const auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(res->instance, "vkCreateDebugUtilsMessengerEXT")
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

        if (vkCreateDebugUtilsMessengerEXT(res->instance, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create debug messenger!");
        }

        core::Logger::getInstance().info("Created Vulkan debug messenger successfully.");
    }

    void VulkanContext::cleanupDebugMessenger() {
        const auto res = resources_;

        const auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(res->instance, "vkDestroyDebugUtilsMessengerEXT")
        );

        if (vkDestroyDebugUtilsMessengerEXT != nullptr) {
            vkDestroyDebugUtilsMessengerEXT(res->instance, debugMessenger_, nullptr);
        }

        debugMessenger_ = nullptr;
    }

    void VulkanContext::createInstance(const core::Window& window) const {
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

        const auto res = resources_;

        if (const auto result = vkCreateInstance(&createInfo, nullptr, &res->instance); result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance (vkCreateInstance failed)");
        }

        core::Logger::getInstance().debug("Created Vulkan instance successfully.");
    }

    void VulkanContext::createSurface(const core::Window& window) const {
        // Use GLFW to crate a Vulkan surface
        if (const auto res = resources_;
            glfwCreateWindowSurface(res->instance, window.window_.get(), nullptr, &res->surface) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }

        core::Logger::getInstance().info("Vulkan surface created successfully.");
    }

    void VulkanContext::pickPhysicalDevice(const core::Window &window) const {
        const auto res = resources_;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(res->instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(res->instance, &deviceCount, devices.data());

        std::multimap<int, VkPhysicalDevice> candidates;
        for (const auto& device : devices) {
            int score = rateDeviceSuitability(device, window);
            candidates.insert(std::make_pair(score, device));
        }

        // Select the best device (highest score)
        if (candidates.rbegin()->first == 0) {
            res->physicalDevice = candidates.begin()->second;
            const auto deviceName = VulkanTools::getDeviceName(res->physicalDevice);
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

        const auto res = resources_;

        // Check the support for required queue families
        if (const QueueFamilyIndices indices =
            VulkanTools::findQueueFamilies(res->instance, res->surface, device); !indices.isComplete()) {
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

    void VulkanContext::createLogicalDevice(const core::Window& window) const {
        const auto res = resources_;

        if (res->physicalDevice == nullptr) {
            throw std::runtime_error("Physical device not selected!");
        }

        const auto deviceName = VulkanTools::getDeviceName(res->physicalDevice);

        const auto [graphicsFamily, presentFamily] =
            VulkanTools::findQueueFamilies(res->instance, res->surface, res->physicalDevice);

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
        if (vkCreateDevice(res->physicalDevice, &createInfo, nullptr, &res->logicalDevice) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device for GPU: " +  deviceName);
        }
        if (res->logicalDevice == nullptr) {
            throw std::runtime_error("Failed to create logical device! (logicalDevice isn't present)");
        }

        // Retrieve queue handles
        vkGetDeviceQueue(res->logicalDevice, graphicsFamily.value(), 0, &res->graphicsQueue);
        vkGetDeviceQueue(res->logicalDevice, presentFamily.value(), 0, &res->presentQueue);

        if (res->graphicsQueue == nullptr) {
            throw std::runtime_error("Failed to create graphics queue!");
        }
        if (res->presentQueue == nullptr) {
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

        const auto res = resources_;

        // Get surface capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, res->surface, &details.capabilities);

        // Retrieve supported Surface formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, res->surface, &formatCount, nullptr);
        if (formatCount == 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, res->surface, &formatCount, details.formats.data());
        }

        // Retrieve supported presentation modes
        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, res->surface, &presentModeCount, nullptr);
        if (presentModeCount == 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, res->surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }
}
