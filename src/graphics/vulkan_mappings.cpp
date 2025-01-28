#include "vulkan_mappings.hpp"

namespace time_kill::graphics {
    VulkanMappings::VulkanMappings() {
        formatMap_ = {
            {VK_FORMAT_UNDEFINED, "VK_FORMAT_UNDEFINED"},
            {VK_FORMAT_R4G4_UNORM_PACK8, "VK_FORMAT_R4G4_UNORM_PACK8"},
            {VK_FORMAT_R4G4B4A4_UNORM_PACK16, "VK_FORMAT_R4G4B4A4_UNORM_PACK16"},
            {VK_FORMAT_B4G4R4A4_UNORM_PACK16, "VK_FORMAT_B4G4R4A4_UNORM_PACK16"},
            {VK_FORMAT_R5G6B5_UNORM_PACK16, "VK_FORMAT_R5G6B5_UNORM_PACK16"},
            {VK_FORMAT_B5G6R5_UNORM_PACK16, "VK_FORMAT_B5G6R5_UNORM_PACK16"},
            {VK_FORMAT_R5G5B5A1_UNORM_PACK16, "VK_FORMAT_R5G5B5A1_UNORM_PACK16"},
            {VK_FORMAT_B5G5R5A1_UNORM_PACK16, "VK_FORMAT_B5G5R5A1_UNORM_PACK16"},
            {VK_FORMAT_A1R5G5B5_UNORM_PACK16, "VK_FORMAT_A1R5G5B5_UNORM_PACK16"},
            {VK_FORMAT_R8_UNORM, "VK_FORMAT_R8_UNORM"},
            {VK_FORMAT_R8_SNORM, "VK_FORMAT_R8_SNORM"},
            {VK_FORMAT_R8_USCALED, "VK_FORMAT_R8_USCALED"},
            {VK_FORMAT_R8_SSCALED, "VK_FORMAT_R8_SSCALED"},
            {VK_FORMAT_R8_UINT, "VK_FORMAT_R8_UINT"},
            {VK_FORMAT_R8_SINT, "VK_FORMAT_R8_SINT"},
            {VK_FORMAT_R8_SRGB, "VK_FORMAT_R8_SRGB"},
            {VK_FORMAT_R8G8_UNORM, "VK_FORMAT_R8G8_UNORM"},
            {VK_FORMAT_R8G8_SNORM, "VK_FORMAT_R8G8_SNORM"},
            {VK_FORMAT_R8G8_USCALED, "VK_FORMAT_R8G8_USCALED"},
            {VK_FORMAT_R8G8_SSCALED, "VK_FORMAT_R8G8_SSCALED"},
            {VK_FORMAT_R8G8_UINT, "VK_FORMAT_R8G8_UINT"},
            {VK_FORMAT_R8G8_SINT, "VK_FORMAT_R8G8_SINT"},
            {VK_FORMAT_R8G8_SRGB, "VK_FORMAT_R8G8_SRGB"},
            {VK_FORMAT_R8G8B8_UNORM, "VK_FORMAT_R8G8B8_UNORM"},
            {VK_FORMAT_R8G8B8_SNORM, "VK_FORMAT_R8G8B8_SNORM"},
            {VK_FORMAT_R8G8B8_USCALED, "VK_FORMAT_R8G8B8_USCALED"},
            {VK_FORMAT_R8G8B8_SSCALED, "VK_FORMAT_R8G8B8_SSCALED"},
            {VK_FORMAT_R8G8B8_UINT, "VK_FORMAT_R8G8B8_UINT"},
            {VK_FORMAT_R8G8B8_SINT, "VK_FORMAT_R8G8B8_SINT"},
            {VK_FORMAT_R8G8B8_SRGB, "VK_FORMAT_R8G8B8_SRGB"},
            {VK_FORMAT_B8G8R8_UNORM, "VK_FORMAT_B8G8R8_UNORM"},
            {VK_FORMAT_B8G8R8_SNORM, "VK_FORMAT_B8G8R8_SNORM"},
            {VK_FORMAT_B8G8R8_USCALED, "VK_FORMAT_B8G8R8_USCALED"},
            {VK_FORMAT_B8G8R8_SSCALED, "VK_FORMAT_B8G8R8_SSCALED"},
            {VK_FORMAT_B8G8R8_UINT, "VK_FORMAT_B8G8R8_UINT"},
            {VK_FORMAT_B8G8R8_SINT, "VK_FORMAT_B8G8R8_SINT"},
            {VK_FORMAT_B8G8R8_SRGB, "VK_FORMAT_B8G8R8_SRGB"},
            {VK_FORMAT_R8G8B8A8_UNORM, "VK_FORMAT_R8G8B8A8_UNORM"},
            {VK_FORMAT_R8G8B8A8_SNORM, "VK_FORMAT_R8G8B8A8_SNORM"},
            {VK_FORMAT_R8G8B8A8_USCALED, "VK_FORMAT_R8G8B8A8_USCALED"},
            {VK_FORMAT_R8G8B8A8_SSCALED, "VK_FORMAT_R8G8B8A8_SSCALED"},
            {VK_FORMAT_R8G8B8A8_UINT, "VK_FORMAT_R8G8B8A8_UINT"},
            {VK_FORMAT_R8G8B8A8_SINT, "VK_FORMAT_R8G8B8A8_SINT"},
            {VK_FORMAT_R8G8B8A8_SRGB, "VK_FORMAT_R8G8B8A8_SRGB"},
            {VK_FORMAT_B8G8R8A8_UNORM, "VK_FORMAT_B8G8R8A8_UNORM"},
            {VK_FORMAT_B8G8R8A8_SNORM, "VK_FORMAT_B8G8R8A8_SNORM"},
            {VK_FORMAT_B8G8R8A8_USCALED, "VK_FORMAT_B8G8R8A8_USCALED"},
            {VK_FORMAT_B8G8R8A8_SSCALED, "VK_FORMAT_B8G8R8A8_SSCALED"},
            {VK_FORMAT_B8G8R8A8_UINT, "VK_FORMAT_B8G8R8A8_UINT"},
            {VK_FORMAT_B8G8R8A8_SINT, "VK_FORMAT_B8G8R8A8_SINT"},
            {VK_FORMAT_B8G8R8A8_SRGB, "VK_FORMAT_B8G8R8A8_SRGB"},
        };
    }

    String VulkanMappings::getFormatDescription(VkFormat format) const {
        auto it = formatMap_.find(format);
        if (it != formatMap_.end()) {
            return it->second;
        }
        return "Unknown Format";
    }
}
