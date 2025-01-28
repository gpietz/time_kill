#include "vulkan_resources.hpp"

namespace time_kill::graphics {
    VulkanResources::VulkanResources()
        : instance(nullptr), surface(nullptr), physicalDevice(nullptr), logicalDevice(nullptr),
          graphicsQueue(nullptr), presentQueue(nullptr) {}
}
