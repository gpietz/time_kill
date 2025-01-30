#pragma once

#include "graphics/vulkan_resources.hpp"

namespace time_kill::core {
    class Window;
}

namespace time_kill::graphics {
    class VulkanSwapchain {
    public:
        explicit VulkanSwapchain(VulkanResources& resources);
        ~VulkanSwapchain();

        void createSwapchain(const core::Window& window);
        void destroySwapchain();

    private:
        void createImageViews();

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const Vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const core::Window& window);

        VulkanResources& resources_;
    };
} // time_kill
