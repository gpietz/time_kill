#include "vulkan_tools.hpp"

namespace time_kill::graphics {
    bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    return true;
                }
            }
        }

        return false;
    }

    Vector<const char*> getRequiredExtensions(const bool enableValidationLayers) {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        Vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    QueueFamilyIndices VulkanTools::findQueueFamilies(
        VkInstance_T* instance,
        VkSurfaceKHR_T* surface,
        VkPhysicalDevice_T* device
    ) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = std::make_optional(i);
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (glfwGetPhysicalDevicePresentationSupport(instance, device, i) == GLFW_TRUE) {
                indices.presentFamily = std::make_optional(i);
            }

            if (indices.isComplete()) {
                break;
            }
        }

        return indices;
    }

    String VulkanTools::getDeviceName(VkPhysicalDevice_T* device) {
        VkPhysicalDeviceProperties deviceProperties = {};
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        return deviceProperties.deviceName;
    }

    void VulkanTools::queueWaitIdle(VkQueue_T* queue) {
        if (queue != VK_NULL_HANDLE) {
            vkQueueWaitIdle(queue);
        }
    }
}
