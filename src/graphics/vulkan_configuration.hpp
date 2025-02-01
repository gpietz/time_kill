#pragma once

#include "prerequisites.hpp"

namespace time_kill::graphics {
    class VulkanConfiguration {
    public:
        VulkanConfiguration() = default;



        //! Enables Vulkan validation layers and debug messenger if true. Defaults to false.
        bool debugEnabled = true;
        bool enableValidationLayers = true;
        bool enableExtensions = true;
        bool enableMSAA = false;

        void setRootDirectory(const String& directory) {
            rootDirectory_ = directory;
        }

        const String& getRootDirectory() const {
            return rootDirectory_;
        }

        void addShaderDirectory(const String& directory) {
            shaderModuleDirectories_.push_back(directory);
        }

        [[nodiscard]] const Vector<String>& getShaderDirectories() const {
            return shaderModuleDirectories_;
        }

    private:
        String rootDirectory_;
        Vector<String> shaderModuleDirectories_;
    };
}
