#pragma once

#include "vulkan_resources.hpp"
#include <vulkan/vulkan.h>

namespace time_kill::graphics {
    class VulkanRenderPass {
    public:
          explicit VulkanRenderPass(VulkanResources& vulkanResources);
          ~VulkanRenderPass();

          void createRenderPass();
          void destroyRenderPass();

    private:
          VulkanResources& resources_;
    };
}
