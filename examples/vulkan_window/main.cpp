#include "core/window.hpp"
#include "core/logger.hpp"
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_configuration.hpp"
#include <iostream>
#include <sstream>

constexpr auto WINDOW_TITLE = "Vulkan Window Example";

int main() {
    try {
        using namespace time_kill;
        using namespace time_kill::core;
        using namespace time_kill::graphics;

        // Initialize file logging
        log_init("vulkan_window.log", true);
        log_enable_trace(true);
        log_set_date_separator(time_kill::core::DateSeparator::Period);

        // Create a Vulkan configuration with debug messages enabled
        VulkanConfiguration vulkanConfig = {};
        vulkanConfig.debugEnabled = true;
        vulkanConfig.setRootDirectory("../../../");

        // Create a window instance with vulkan context
        Window window(800, 600, WINDOW_TITLE, true);
        VulkanContext vulkanContext(window, vulkanConfig);

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