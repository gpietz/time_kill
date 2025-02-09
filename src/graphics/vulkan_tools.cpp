#include "vulkan_tools.hpp"
#include "core/logger.hpp"
#include <filesystem>
#include <spirv_reflect.h>
#include <unordered_set>

namespace time_kill::graphics {
    bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    return true;
                }
            }
        }

        return false;
    }

    Vector<const char*> getRequiredExtensions(const bool enableValidationLayers) {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        Vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    QueueFamilyIndices VulkanTools::findQueueFamilies(
        VkInstance_T* instance,
        VkSurfaceKHR_T* surface,
        VkPhysicalDevice_T* device
    ) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = std::make_optional(i);
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (glfwGetPhysicalDevicePresentationSupport(instance, device, i) == GLFW_TRUE) {
                indices.presentFamily = std::make_optional(i);
            }

            if (indices.isComplete()) {
                break;
            }
        }

        return indices;
    }

    String VulkanTools::getDeviceName(VkPhysicalDevice_T* device) {
        VkPhysicalDeviceProperties deviceProperties = {};
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        return deviceProperties.deviceName;
    }

    void VulkanTools::queueWaitIdle(VkQueue_T* queue) {
        if (queue != VK_NULL_HANDLE) {
            vkQueueWaitIdle(queue);
        }
    }

    Vector<String> VulkanTools::getSpirvFiles(const VulkanConfiguration& configuration, const bool recursive) {
        Vector<String> spirvFiles;
        namespace fs = std::filesystem;

        // Determine base directory
        String rootDir = configuration.getRootDirectory();
        if (!rootDir.empty() && rootDir.back() != '/') {
            rootDir += '/';
        }

        // Retrieve shader directories or set default
        Vector<String> shaderDirs = configuration.getShaderDirectories();
        if (shaderDirs.empty()) {
            shaderDirs.emplace_back("assets/shaders/");
        }

        // Iterate through all shader directories
        for (const auto& dir : shaderDirs) {
            fs::path shaderPath = rootDir.empty() ? fs::path(dir) : fs::path(rootDir) / dir;

            if (!fs::exists(shaderPath) || !fs::is_directory(shaderPath)) {
                log_warn(std::format("Shader directory '{}' does not exist!", shaderPath.string()));
                continue;
            }

            // Recursive or non-recursive search
            if (recursive) {
                for (const auto& entry : fs::recursive_directory_iterator(shaderPath)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".spv") {
                        spirvFiles.push_back(entry.path().string());
                    }
                }
            } else {
                for (const auto& entry : fs::directory_iterator(shaderPath)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".spv") {
                        spirvFiles.push_back(entry.path().string());
                    }
                }
            }
        }

        return spirvFiles;
    }

    VkShaderStageFlagBits VulkanTools::getShaderStage(const String& filename) {
        if (filename.ends_with(".vert.spv")) return VK_SHADER_STAGE_VERTEX_BIT;
        if (filename.ends_with(".frag.spv")) return VK_SHADER_STAGE_FRAGMENT_BIT;
        if (filename.ends_with(".geom.spv")) return VK_SHADER_STAGE_GEOMETRY_BIT;
        if (filename.ends_with(".tesc.spv")) return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        if (filename.ends_with(".tese.spv")) return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        if (filename.ends_with(".comp.spv")) return VK_SHADER_STAGE_COMPUTE_BIT;

        // Optional: Ray-Tracing Shader-Types
        if (filename.ends_with(".rgen.spv")) return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        if (filename.ends_with(".rahit.spv")) return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        if (filename.ends_with(".rchit.spv")) return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        if (filename.ends_with(".rmiss.spv")) return VK_SHADER_STAGE_MISS_BIT_KHR;
        if (filename.ends_with(".rint.spv")) return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        if (filename.ends_with(".rcall.spv")) return VK_SHADER_STAGE_CALLABLE_BIT_KHR;

        throw std::runtime_error("Unknown shader type: " + filename);
    }

    template <typename T, typename F>
    Vector<T*> spvReflectEnumerateVariables(SpvReflectShaderModule& module, F enumerateFunc) {
        u32 count = 0;
        enumerateFunc(&module, &count, nullptr);

        Vector<T*> variables;
        variables.reserve(count);
        enumerateFunc(&module, &count, variables.data());

        return variables;
    }

    void throwDuplicateLocationError(const String& filename, const u32 location, const bool input) {
        std::ostringstream oss;
        oss << "SPIRV-Reflect: Duplicate " << (input ? "input" : "output")
            << " attribute location detected!\n"
            << "File: " << filename << "\n"
            << "Location: " << location << "\n";
        throw std::runtime_error(oss.str());
    }

    Vector<VkVertexInputAttributeDescription> VulkanTools::parseVertexInputAttributes(
        const Vector<char>& spirvCode,
        const String& filename
    ) {
        SpvReflectShaderModule module;
        if (const SpvReflectResult result =
            spvReflectCreateShaderModule(spirvCode.size(), spirvCode.data(), &module);
            result != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("Failed to reflect SPIR-V vertex shader: " + filename);
        }

        // Get input and output variables
        const auto inputVars =
            spvReflectEnumerateVariables<SpvReflectInterfaceVariable>(module, spvReflectEnumerateInputVariables);
        const auto outputVars =
            spvReflectEnumerateVariables<SpvReflectInterfaceVariable>(module, spvReflectEnumerateOutputVariables);

        Vector<VkVertexInputAttributeDescription> attributes;
        std::unordered_set<u32> inputLocations;
        std::unordered_set<u32> outputLocations;

        auto checkDuplicate = [&](const auto& vars, auto& locationSet, const bool isInput) {
            for (const auto& var : vars) {
                if (var->built_in != -1) continue;
                if (!locationSet.insert(var->location).second) {
                    spvReflectDestroyShaderModule(&module);
                    throwDuplicateLocationError(filename, var->location, isInput);
                }

                if (isInput) {
                    VkVertexInputAttributeDescription attribute = {};
                    attribute.location = var->location;
                    attribute.binding = 0;
                    attribute.format = static_cast<VkFormat>(var->format);
                    attribute.offset = 0;
                    attributes.push_back(attribute);
                }
            }
        };

        checkDuplicate(inputVars, inputLocations, true);
        checkDuplicate(outputVars, outputLocations, false);

        spvReflectDestroyShaderModule(&module);
        return attributes;
    }
}
