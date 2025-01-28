#pragma once

#include "prerequisites.hpp"
#include "graphics/vulkan_resources.hpp"

namespace time_kill::core {
    class Window;
}

namespace time_kill::graphics {
    class VulkanSwapchain {
    public:
        ~VulkanSwapchain();

        void createSwapchain(const core::Window& window, const SharedPtr<VulkanResources>& resources);
        void destroySwapchain();

    private:
        void createImageViews();

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const Vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const core::Window& window);

        VkDevice logicalDevice_ = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain_ = nullptr;
        Vector<VkImage> images_;
        Vector<VkImageView> imageViews_;
        VkFormat imageFormat_ = VK_FORMAT_UNDEFINED;
        VkExtent2D extent_ = { 0, 0 };
    };
} // time_kill
