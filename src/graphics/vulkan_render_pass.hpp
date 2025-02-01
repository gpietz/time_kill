#pragma once

#include "vulkan_resources.hpp"
#include <vulkan/vulkan.h>

namespace time_kill::graphics {
    //! The render pass in Vulkan defines how rendering is performed on the frame buffer. It determines which
    //! memory areas are used for rendering, how they are initialised and saved and which dependencies exist
    //! between the various rendering operations.
    class VulkanRenderPass {
    public:
          explicit VulkanRenderPass(VulkanResources& vulkanResources);
          ~VulkanRenderPass();

          void createRenderPass() const;
          void destroyRenderPass() const;

    private:
          VulkanResources& resources_;
    };
}
