#pragma once

#include "prerequisites.hpp"
#include <vulkan/vulkan.h>

namespace time_kill::graphics {
    class VulkanResources {
    public:
        VulkanResources() = default;

        //=== Vulkan instance and devices
        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice logicalDevice = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;

        //=== Swapchain-related resources
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;
        VkExtent2D swapchainExtent = {};
        Vector<VkImage> swapchainImages;
        Vector<VkImageView> swapchainImageViews;

        //=== Depth Buffer
        VkFormat depthFormat = VK_FORMAT_UNDEFINED;
        VkImage depthImage = VK_NULL_HANDLE;
        VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
        VkImageView depthImageView = VK_NULL_HANDLE;

        //=== Render Pass
        VkRenderPass renderPass = VK_NULL_HANDLE;

        //=== Graphics Pipeline
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        VkPipelineLayout graphicsPipelineLayout = VK_NULL_HANDLE;
    };
}
