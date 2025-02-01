#include "window.hpp"
#include <stdexcept>
#include <utility>

namespace time_kill::core {
    // Constructor: Initializes the window with given dimensions and title
    Window::Window(const int width, const int height, String  title, const bool resizable)
        : width_(width), height_(height), title_(std::move(title)), vulkanSupported_(false) {

        // Initialize GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Check if VULKAN API is supported
        vulkanSupported_ = glfwVulkanSupported();

        // Configure GLFW to not create an OpenGL context
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE,  resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Create the GLFW window
        GLFWwindow* rawWindow = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
        if (!rawWindow) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        // Associate window with class instance
        glfwSetWindowUserPointer(rawWindow, this);

        // Register Framebuffer-Size-Callback
        glfwSetFramebufferSizeCallback(rawWindow,  [](GLFWwindow* window, const int w, const int h) {
            const auto windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
            windowInstance->width_ = w;
            windowInstance->height_ = h;
        });

        window_.reset(rawWindow);
    }

    Window::Window(const WindowConfig &config) : Window(config.width, config.height, config.title, true) {
    }

    // Destructor: Cleans up resources
    Window::~Window() {
        glfwTerminate();
    }

    // Check if the window should close
    bool Window::shouldClose() const {
        return glfwWindowShouldClose(window_.get());
    }

    // Set the window title
    void Window::setTitle(const std::string& newTitle) {
        title_ = newTitle;
        glfwSetWindowTitle(window_.get(), title_.c_str());
    }

    // Get the window title
    std::string Window::getTitle() const {
        return title_;
    }

    // Get the window position
    void Window::getPosition(int& x, int& y) const {
        glfwGetWindowPos(window_.get(), &x, &y);
    }

    // Center the window on the screen
    void Window::centerOnScreen() const {
        GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
        if (!primaryMonitor) {
            throw std::runtime_error("Failed to get primary monitor");
        }

        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
        if (!mode) {
            throw std::runtime_error("Failed to get video mode");
        }

        const int screenWidth = mode->width;
        const int screenHeight = mode->height;

        const int newX = (screenWidth - width_) / 2;
        const int newY = (screenHeight - height_) / 2;

        glfwSetWindowPos(window_.get(), newX, newY);
    }

    // Get both the window width and height
    void Window::getWidthAndHeight(int& width, int& height) const {
        width = width_;
        height = height_;
    }

    // Set the window visibility
    void Window::setVisible(const bool visible) const {
        if (visible) {
            glfwShowWindow(window_.get());
        } else {
            glfwHideWindow(window_.get());
        }
    }

    // Check if the window is visible
    bool Window::isVisible() const {
        return glfwGetWindowAttrib(window_.get(), GLFW_VISIBLE) == GLFW_TRUE;
    }

    bool Window::isVulkanSupported() const {
        return vulkanSupported_;
    }

    void Window::getFramebufferSize(int& width, int& height) const {
        glfwGetFramebufferSize(window_.get(), &width, &height);
    }

    graphics::FramebufferSize Window::getFramebufferSize() const {
        return graphics::FramebufferSize(width_, height_);
    }
}
