#pragma once

#include "prerequisites.hpp"
#include <vulkan/vulkan.h>
#include <unordered_map>

namespace time_kill::graphics {
    class VulkanMappings {
    public:
        VulkanMappings();

        String getFormatDescription(VkFormat format) const;

    private:
        std::unordered_map<VkFormat, String> formatMap_;
    };
}
