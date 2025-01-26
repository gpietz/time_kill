#pragma once

#include "core/window.hpp"
#include <vulkan/vulkan.h>

namespace time_kill::graphics {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);

    //! Function that will return the required list of extensions based on whether
    //! validation layers are enabled or not.
    Vector<const char*> getRequiredExtensions(bool enableValidationLayers);

    class VulkanTools {
    public:
        static QueueFamilyIndices findQueueFamilies(
            VkInstance_T* instance,
            VkSurfaceKHR_T* surface,
            VkPhysicalDevice_T* device
        );

        static String getDeviceName(VkPhysicalDevice_T* device);
        static void queueWaitIdle(VkQueue_T* queue);
    };
}
