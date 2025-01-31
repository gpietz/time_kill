#pragma once

#include "prerequisites.hpp"
#include "window_config.hpp"

// Include Vulkan through GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace time_kill::graphics {
    class VulkanContext;
}

namespace time_kill::core {
    // Class to manage a GLFW window for Vulkan rendering
    class Window {
    public:
        // Constructor: Initializes the window with given dimensions and title
        Window(int width, int height, String  title, bool resizable = true);
        explicit Window(const WindowConfig& config);

        // Destructor: Cleans up resources
        ~Window();

        // Delete copy constructor and assignment operator to prevent copying
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        // Check if the window should close
        [[nodiscard]] bool shouldClose() const;

        // Set the window title
        void setTitle(const std::string& newTitle);

        // Get the window title
        [[nodiscard]] std::string getTitle() const;

        // Get the window width
        [[nodiscard]] int getWidth() const { return width_; }

        // Get the window height
        [[nodiscard]] int getHeight() const { return height_; }

        // Get the window position
        void getPosition(int& x, int& y) const;

        // Center the window on the screen
        void centerOnScreen() const;

        // Get both the window width and height
        void getWidthAndHeight(int& width, int& height) const;

        // Set the window visibility
        void setVisible(bool visible) const;

        // Check if the window is visible
        [[nodiscard]] bool isVisible() const;

        // Check if the window has VULKAN support
        [[nodiscard]] bool isVulkanSupported() const;

        // Retrieves the framebuffer dimensions.
        void getFramebufferSize(int& width, int& height) const;

    private:
        friend class graphics::VulkanContext;

        struct GLFWwindowDeleter {
            void operator()(GLFWwindow* ptr) const {
                if (ptr) {
                    glfwDestroyWindow(ptr);
                }
            }
        };

        std::unique_ptr<GLFWwindow, GLFWwindowDeleter> window_;
        int width_;           // Window width
        int height_;          // Window height
        std::string title_;   // Window title
        bool vulkanSupported_;
    };
}
