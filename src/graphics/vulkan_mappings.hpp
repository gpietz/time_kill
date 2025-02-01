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
        [[nodiscard]] String getDepthFormatDescription(VkFormat) const;
        [[nodiscard]] String getShaderStageDescription(VkShaderStageFlagBits stage) const;

    private:
        std::unordered_map<VkFormat, String> formatMap_;
        std::unordered_map<VkPresentModeKHR, String> presentModeMap_;
        std::unordered_map<VkFormat, String> depthFormatMap_;
        std::unordered_map<VkShaderStageFlagBits, String> shaderStageMap_;
    };
}
