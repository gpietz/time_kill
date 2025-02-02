#pragma once

#include "graphics/vulkan_resources.hpp"
#include "graphics/vulkan_configuration.hpp"

namespace time_kill::core {
    class Window;
}

namespace time_kill::graphics {
    //! The graphics pipeline in Vulkan is responsible for processing and rendering graphics on the GPU.
    //! It consists of several stages, ranging from the processing of the input data to the final display
    //! on the screen. In contrast to OpenGL, where many things are configured at runtime, in Vulkan the
    //! entire pipeline must be created and optimised in advance.
    class VulkanGraphicsPipeline {
    public:
        explicit VulkanGraphicsPipeline(VulkanResources& resources);
        ~VulkanGraphicsPipeline();

        void createGraphicsPipeline(const core::Window& window, const VulkanConfiguration& configuration) const;
        void destroyGraphicsPipeline() const;

    private:

        static Vector<char> readSpirvFile(const String& filename);
        static VkShaderModule createShaderModule(const Vector<char>& code, VkDevice device, const String& filename);

        VulkanResources& resources_;
    };
}
