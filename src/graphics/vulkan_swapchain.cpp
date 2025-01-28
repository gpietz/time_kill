#include "vulkan_swapchain.hpp"
#include "vulkan_context.hpp"
#include "vulkan_mappings.hpp"
#include "core/logger.hpp"
#include "core/window.hpp"
#include <algorithm>

namespace time_kill::graphics {
    void logSurfaceFormat(const VulkanMappings& mappings, const Vector<VkSurfaceFormatKHR>& formats) {
        for (const auto&[format, _] : formats) {
            core::Logger::getInstance().debug(
                std::format("- {}", mappings.getFormatDescription(format))
            );
        }
    }
    //
    // void logPresentModes(const Vector<VkPresentModeKHR>& present_modes) {
    //     for (const auto& present_mode : present_modes) {
    //         core::Logger::getInstance().debug(
    //             std::format("- {}", present_mode)
    //         );
    //     }
    // }

    VulkanSwapchain::~VulkanSwapchain() {
        destroySwapchain();
    }

    void VulkanSwapchain::createSwapchain(const core::Window& window, const SharedPtr<VulkanResources>& resources) {
        if (swapchain_ != VK_NULL_HANDLE) {
            destroySwapchain();
        }

        const auto& res = resources;

        if (res->physicalDevice == nullptr)
            throw std::runtime_error("Unable to create swapchain; physical device is null!");
        if (res->surface == nullptr)
            throw std::runtime_error("Unable to create swapchain; surface is null!");
        if (res->logicalDevice == nullptr)
            throw std::runtime_error("Unable to create swapchain; logical device is null!");

        // Store logical device to enable swapchain destroy
        logicalDevice_ = res->logicalDevice;

        const VulkanMappings mappings {};

        // Query swapchain support
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(res->physicalDevice, res->surface, &surfaceCapabilities);

        // Query surface format
        Vector<VkSurfaceFormatKHR> surfaceFormats;
        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(res->physicalDevice, res->surface, &surfaceFormatCount, nullptr);
        if (surfaceFormatCount != 0) {
            surfaceFormats.resize(surfaceFormatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(res->physicalDevice, res->surface, &surfaceFormatCount, surfaceFormats.data());
            if (core::Logger::getInstance().isTraceEnabled()) {
                core::Logger::getInstance().debug(std::format("Found {} surface formats:", surfaceFormatCount));
                logSurfaceFormat(mappings, surfaceFormats);
            } else {
                core::Logger::getInstance().debug(std::format("Found {} surface formats", surfaceFormatCount));
            }
        } else {
            throw std::runtime_error("Failed to get surface formats!");
        }

        // Query presentation modes
        Vector<VkPresentModeKHR> presentModes; 
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(res->physicalDevice, res->surface, &presentModeCount, nullptr);
        if(presentModeCount != 0) {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(res->physicalDevice, res->surface, &presentModeCount, presentModes.data());
            core::Logger::getInstance().debug(std::format("Found {} present modes", presentModeCount));
            //logPresentModes(presentModes);
        } else {
            throw std::runtime_error("Failed to get presentation modes!");
        }

        // Choose the best settings for the swapchain
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(surfaceFormats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
        extent_ = chooseSwapExtent(surfaceCapabilities, window);

        // Create the swapchain
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = res->surface;
        createInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent_;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        VkSwapchainKHR swapchain = nullptr;
        if (vkCreateSwapchainKHR(res->logicalDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain!");
        }
        if (swapchain == nullptr) {
            throw std::runtime_error("Failed to create swapchain! (swapchain is null)");
        }
        swapchain_ = swapchain;

        core::Logger::getInstance().debug("Successfully created swapchain!");

        // Store image format
        imageFormat_ = createInfo.imageFormat;

        // Retrieve swapchain images
        vkGetSwapchainImagesKHR(logicalDevice_, swapchain_, &surfaceFormatCount, nullptr);
        images_.resize(surfaceFormatCount);
        vkGetSwapchainImagesKHR(logicalDevice_, swapchain_, &surfaceFormatCount, images_.data());

        // Create image views
        createImageViews();
    }

    void VulkanSwapchain::destroySwapchain() {
        if (logicalDevice_ == VK_NULL_HANDLE) {
            core::Logger::getInstance().warn("Unable to destroy swapchain; the logical device reference is invalid!");
            return;
        }

        vkDeviceWaitIdle(logicalDevice_);

        if (!images_.empty()) {
            core::Logger::getInstance().debug(std::format("Destroying {} image views.", images_.size()));
            for (auto const imageView : imageViews_) {
                vkDestroyImageView(logicalDevice_, imageView, nullptr);
            }
            imageViews_.clear();
        } else {
            core::Logger::getInstance().debug("No image views to destroy.");
        }

        if (swapchain_ != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(logicalDevice_, swapchain_, nullptr);
            core::Logger::getInstance().debug("Destroyed Vulkan swapchain.");
            swapchain_ = VK_NULL_HANDLE;
        } else {
            core::Logger::getInstance().debug("No Vulkan swapchain to destroy.");
        }

        swapchain_ = VK_NULL_HANDLE;
    }

    void VulkanSwapchain::createImageViews() {
        if (logicalDevice_ == VK_NULL_HANDLE) {
            throw std::runtime_error("Logical device is null. Cannot create image views.");
        }

        if (images_.empty()) {
            throw std::runtime_error("No images available to create image views.");
        }

        imageViews_.resize(images_.size());

        for (size_t i = 0; i < images_.size(); i++) {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = images_[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = imageFormat_;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(logicalDevice_, &createInfo, nullptr, &imageViews_[i]) != VK_SUCCESS) {
                // Release previous created image views to avoid memory leaks
                for (size_t j = 0; j < i; j++) {
                    vkDestroyImageView(logicalDevice_, imageViews_[j], nullptr);
                }
                core::Logger::getInstance().error(std::format("Failed to create image view for image {}", i));
                throw std::runtime_error("Failed to create image views!");
            }
        }

        core::Logger::getInstance().debug(std::format("Successfully created {} image views.", images_.size()));
    }

    VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const Vector<VkSurfaceFormatKHR>& availableFormats) {
        // Preferred format: SRGB with 8 bits per channel (B, G, R, A)
        constexpr VkSurfaceFormatKHR preferredFormat = {
            VK_FORMAT_B8G8R8A8_SRGB,          // Format
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR // Colour space
        };

        // Check whether the preferred format is available
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == preferredFormat.format &&
                availableFormat.colorSpace == preferredFormat.colorSpace) {
                return availableFormat;
                }
        }

        // If the preferred format is not available, use the first available format
        return availableFormats[0];
    }

    VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const Vector<VkPresentModeKHR>& availablePresentModes) {
        // Preferred mode: MAILBOX (Triple Buffering)
        constexpr VkPresentModeKHR preferredMode = VK_PRESENT_MODE_MAILBOX_KHR;

        // Check whether the preferred mode is available
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == preferredMode) {
                return availablePresentMode;
            }
        }

        // If the preferred mode is not available, use FIFO (V-Sync)
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const core::Window& window) {
        // If the current size is defined, use it
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }

        // Otherwise, select the size based on the window size
        int width = 0, height = 0;
        window.getFramebufferSize(width, height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        // Limit the size to the minimum and maximum value
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
