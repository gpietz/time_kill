#pragma once

#include "prerequisites.hpp"
#include <vulkan/vulkan.h>
#include <unordered_map>

namespace time_kill::graphics {
    class VulkanMappings {
    public:
        VulkanMappings();

        [[nodiscard]] String getFormatDescription(VkFormat format) const;
        [[nodiscard]] String getPresentModeDescription(VkPresentModeKHR presentMode) const;

    private:
        std::unordered_map<VkFormat, String> formatMap_;
        std::unordered_map<VkPresentModeKHR, String> presentModeMap_;
    };
}
