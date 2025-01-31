//! @file vulkan_context.hpp
//! @brief Defines the VulkanContext class, which manages the Vulkan instance, device, and related resources.
#pragma once

#include "core/window.hpp"
#include "vulkan_resources.hpp"
#include "vulkan_swapchain.hpp"
#include "vulkan_render_pass.hpp"
#include <vulkan/vulkan.h>

namespace time_kill::graphics {
    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct VulkanFeatures {
        bool enableRaytracing = false;
        bool enableTesselation = false;
        bool enableAnisotropy = false;
    };

    //! @class VulkanContext
    //! @brief Manages the Vulkan context, including instance, physical device selection,
    //! logical device creation, surface creation, and debug utilities.
    class VulkanContext  {
    public:
        //! @brief Constructs a VulkanContext.
        //! @param window The window to create a surface for.
        //! @param debugEnabled Enables Vulkan validation layers and debug messenger if true. Defaults to false.
        explicit VulkanContext(const core::Window& window, bool debugEnabled = false);

        //! @brief Destroys the VulkanContext and releases all allocated resources.
        ~VulkanContext();

        //! @brief Waits for all operations on the graphics and present queues to complete.
        void queuesWaitIdle(bool waitForDevice = false) const;

    private:
        //=== Debug methods

        /// @brief Callback function used by the Vulkan validation layers to report debug messages.
        /// @param messageSeverity Severity of the message.
        /// @param messageType Type of the message.
        /// @param pCallbackData Pointer to the callback data.
        /// @param pUserData User data (unused).
        /// @return VK_TRUE if the validation layer should continue, VK_FALSE otherwise.
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );

        //! Called after the Vulkan instance is created to set up the debug messenger for validation layer messages.
        void createDebugMessenger(const core::Window& window);

        //! Called during destruction to clean up the debug messenger.
        void cleanupDebugMessenger();

        //=== Instance and surface creation

        //! Initializes the Vulkan instance. This is the first major Vulkan object to create.
        void createInstance(const core::Window& window);

        //! Creates a surface for rendering (e.g., using GLFW). This is typically done after the instance is created.
        void createSurface(const core::Window& window);

        //=== Physical device selection

        //! Enumerates and selects a physical device (GPU) that supports the required features.
        void pickPhysicalDevice(const core::Window& window);

        void createLogicalDevice(const core::Window& window);

        //! A helper function used by pickPhysicalDevice to score and select the best physical device.
        int rateDeviceSuitability(VkPhysicalDevice device, const core::Window& window) const;

        //=== Helper methods

        //! A utility function to log GLFW extensions (optional, used during instance creation).
        static void logGlfwVulkanExtensions(uint32_t extensionCount, const char** glfwExtensions);
        static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, const core::Window& window) const;

        //=== Member variables
        bool debugEnabled_ = false;                 ///< Enables debug features if true.
        VkDebugUtilsMessengerEXT debugMessenger_;   ///< Debug messenger for validation layers.
        VulkanResources resources_;
        VulkanSwapchain swapchain_;
        VulkanRenderPass renderPass_;
    };
}
