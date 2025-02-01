#pragma once

#include "core/window.hpp"
#include "vulkan_configuration.hpp"
#include <vulkan/vulkan.h>

namespace time_kill::graphics {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers);

    //! Function that will return the required list of extensions based on whether
    //! validation layers are enabled or not.
    Vector<const char*> getRequiredExtensions(bool enableValidationLayers);

    class VulkanTools {
    public:
        static QueueFamilyIndices findQueueFamilies(
            VkInstance_T* instance,
            VkSurfaceKHR_T* surface,
            VkPhysicalDevice_T* device
        );

        static String getDeviceName(VkPhysicalDevice_T* device);
        static void queueWaitIdle(VkQueue_T* queue);

        //! Retrieves all SPIR-V shader files from the specified shader directories.
        //!
        //! This method scans through the shader directories defined in the given VulkanConfiguration
        //! object, including any specified root directory, to find all `.spv` files.
        //! If no shader directories are defined, the default path "assets/shaders/" is used.
        //!
        //! @param configuration Reference to the VulkanConfiguration containing shader directory settings.
        //! @param recursive Boolean flag to determine whether the search should be recursive.
        //! @return A vector of strings containing the full paths of all located `.spv` files.
        //!
        //! @note If a shader directory does not exist, a warning is logged, and it is skipped.
        //! @note Uses `std::filesystem` to traverse directories.
        static Vector<String> getSpirvFiles(const VulkanConfiguration& configuration, bool recursive);

        static VkShaderStageFlagBits getShaderStage(const String& filename);

        static Vector<VkVertexInputAttributeDescription> parseVertexInputAttributes(
            const Vector<char>& spirvCode,
            const String& filename
        );
    };
}
