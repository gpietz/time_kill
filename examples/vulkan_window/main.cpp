#include "core/window.hpp"
#include "graphics/vulkan_context.hpp"
#include <iostream>
#include <sstream>

#include "core/logger.hpp"

constexpr auto WINDOW_TITLE = "Vulkan Window Example";

int main() {
    try {
        // Initialize file logging
        time_kill::core::Logger::getInstance().init("vulkan_window.log", true);
        time_kill::core::Logger::getInstance().setDateSeparator(time_kill::core::DateSeparator::Period);

        // Create a window instance with vulkan context
        time_kill::core::Window window(800, 600, WINDOW_TITLE, true);
        time_kill::graphics::VulkanContext vulkanContext(window, true);

        // Center the window on screen
        window.centerOnScreen();
        
        // Show the window
        window.setVisible(true);
        
        // Main loop
        int windowPosition[2] = {0, 0};
        int windowDimension[2] = {0, 0};
        while (!window.shouldClose()) {
            glfwPollEvents();
            
            // Test window methods
            if (window.isVisible()) {
                int x, y, width, height;
                window.getPosition(x, y);
                window.getWidthAndHeight(width, height);
                if (windowPosition[0] != x || windowPosition[1] != y ||
                    windowDimension[0] != width || windowDimension[1] != height) {
                    windowPosition[0] = x;
                    windowPosition[1] = y;
                    windowDimension[0] = width;
                    windowDimension[1] = height;
                    std::ostringstream oss;
                    oss << WINDOW_TITLE << " (Pos: " << windowPosition[0] << ", " << windowPosition[1]
                        << "; Size: " << windowDimension[0] << "x" << windowDimension[1] << ")";
                    if (!window.isVulkanSupported()) {
                        oss << "  *** NO VULKAN SUPPORT ***";
                    }
                    window.setTitle(oss.str());
                }
            }
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
} 