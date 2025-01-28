#pragma once

#include <vulkan/vulkan.h>

namespace time_kill::graphics {
    class VulkanResources {
    public:
        explicit VulkanResources();

        VkInstance instance;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
    };
}
