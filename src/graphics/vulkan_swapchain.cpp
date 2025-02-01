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

    void logPresentModes(const VulkanMappings& mappings, const Vector<VkPresentModeKHR>& present_modes) {
        for (const auto& present_mode : present_modes) {
            core::Logger::getInstance().debug(
                std::format("- {}", mappings.getPresentModeDescription(present_mode))
            );
        }
    }

    VulkanSwapchain::VulkanSwapchain(VulkanResources& resources) : resources_(resources) {}

    void VulkanSwapchain::createSwapchain(const core::Window& window) const {
        auto& res = resources_;
        if (res.swapchain != VK_NULL_HANDLE) {
            destroySwapchain();
        }

        if (res.physicalDevice == nullptr)
            throw std::runtime_error("Unable to create swapchain; physical device is null!");
        if (res.surface == nullptr)
            throw std::runtime_error("Unable to create swapchain; surface is null!");
        if (res.logicalDevice == nullptr)
            throw std::runtime_error("Unable to create swapchain; logical device is null!");

        const VulkanMappings mappings {};

        // Query swapchain support
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(res.physicalDevice, res.surface, &surfaceCapabilities);

        // Query surface format
        Vector<VkSurfaceFormatKHR> surfaceFormats;
        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(res.physicalDevice, res.surface, &surfaceFormatCount, nullptr);
        if (surfaceFormatCount != 0) {
            surfaceFormats.resize(surfaceFormatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(res.physicalDevice, res.surface, &surfaceFormatCount, surfaceFormats.data());
            if (log_is_trace_enabled()) {
                log_debug(std::format("Found {} surface formats:", surfaceFormatCount));
                logSurfaceFormat(mappings, surfaceFormats);
            } else {
                log_debug(std::format("Found {} surface formats", surfaceFormatCount));
            }
        } else {
            throw std::runtime_error("Failed to get surface formats!");
        }

        // Query presentation modes
        Vector<VkPresentModeKHR> presentModes; 
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(res.physicalDevice, res.surface, &presentModeCount, nullptr);
        if(presentModeCount != 0) {
            presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(res.physicalDevice, res.surface, &presentModeCount, presentModes.data());
            if (log_is_trace_enabled()) {
                log_debug(std::format("Found {} present modes:", presentModeCount));
                logPresentModes(mappings, presentModes);
            } else {
                log_debug(std::format("Found {} present modes", presentModeCount));
            }
        } else {
            throw std::runtime_error("Failed to get presentation modes!");
        }

        // Choose the best settings for the swapchain
        auto [format, colorSpace] = chooseSwapSurfaceFormat(surfaceFormats);
        const VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
        res.swapchainExtent = chooseSwapExtent(surfaceCapabilities, window);

        if (log_is_debug_enabled()) {
            log_debug(std::format("Picked format: {}", mappings.getFormatDescription(format)));
            log_debug(std::format("Picked present mode: {}", mappings.getPresentModeDescription(presentMode)));
        }

        // Create the swapchain
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = res.surface;
        createInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
        createInfo.imageFormat = format;
        createInfo.imageColorSpace = colorSpace;
        createInfo.imageExtent = res.swapchainExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = surfaceCapabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        VkSwapchainKHR swapchain = nullptr;
        if (vkCreateSwapchainKHR(res.logicalDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swapchain!");
        }
        if (swapchain == nullptr) {
            throw std::runtime_error("Failed to create swapchain! (swapchain is null)");
        }
        res.swapchain = swapchain;

        log_debug("Successfully created swapchain!");

        // Store image format
        res.swapchainImageFormat = createInfo.imageFormat;

        // Retrieve swapchain images
        vkGetSwapchainImagesKHR(res.logicalDevice, res.swapchain, &surfaceFormatCount, nullptr);
        res.swapchainImages.resize(surfaceFormatCount);
        vkGetSwapchainImagesKHR(res.logicalDevice, res.swapchain, &surfaceFormatCount, res.swapchainImages.data());

        // Create image views
        createImageViews();

        // Find suitable depth format
        res.depthFormat = findDepthFormat();
        if (res.depthFormat != VK_FORMAT_UNDEFINED && log_is_debug_enabled()) {
            log_debug(std::format("Picked depth format: {}", mappings.getDepthFormatDescription(res.depthFormat)));
        }
    }

    void VulkanSwapchain::destroySwapchain() const {
        auto& res = resources_;

        if (res.logicalDevice == VK_NULL_HANDLE) {
            log_warn("Unable to destroy swapchain; the logical device reference is invalid!");
            return;
        }

        vkDeviceWaitIdle(res.logicalDevice);

        if (!res.swapchainImages.empty()) {
            log_debug(std::format("Destroying {} image views.", res.swapchainImages.size()));
            for (auto const imageView : res.swapchainImageViews) {
                vkDestroyImageView(res.logicalDevice, imageView, nullptr);
            }
            res.swapchainImageViews.clear();
        } else {
            log_debug("No image views to destroy.");
        }

        if (res.swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(res.logicalDevice, res.swapchain, nullptr);
            log_debug("Destroyed Vulkan swapchain.");
            res.swapchain = VK_NULL_HANDLE;
        } else {
            log_debug("No Vulkan swapchain to destroy.");
        }

        res.swapchain = VK_NULL_HANDLE;
    }

    void VulkanSwapchain::createImageViews() const {
        auto& res = resources_;
        if (res.logicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Logical device is null. Cannot create image views.");
        }

        if (res.swapchainImages.empty()) {
            throw std::runtime_error("No images available to create image views.");
        }

        const auto imageCount = res.swapchainImages.size();
        res.swapchainImageViews.resize(imageCount);

        for (size_t i = 0; i < res.swapchainImages.size(); i++) {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = res.swapchainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = res.swapchainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(res.logicalDevice, &createInfo, nullptr, &res.swapchainImageViews[i]) != VK_SUCCESS) {
                // Release previous created image views to avoid memory leaks
                for (size_t j = 0; j < i; j++) {
                    vkDestroyImageView(res.logicalDevice, res.swapchainImageViews[j], nullptr);
                }
                log_error(std::format("Failed to create image view for image {}", i));
                throw std::runtime_error("Failed to create image views!");
            }
        }

        log_debug(std::format("Successfully created {} image views.", imageCount));
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

    VkFormat VulkanSwapchain::findDepthFormat() const {
        const Vector<VkFormat> candidates = {
            VK_FORMAT_D16_UNORM,
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_S8_UINT
        };

        const auto res = resources_;

        for (const auto format : candidates) {
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(res.physicalDevice, format, &formatProperties);

            if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                return format;
            }
        }

        throw std::runtime_error("Failed to find a suitable depth format!");
    }
}
