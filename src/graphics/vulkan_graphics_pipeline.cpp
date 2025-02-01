#include "vulkan_graphics_pipeline.hpp"
#include "vulkan_tools.hpp"
#include "core/window.hpp"
#include "core/logger.hpp"
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "vulkan_mappings.hpp"

namespace time_kill::graphics {
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanResources& resources) : resources_(resources) {}

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {}

    void VulkanGraphicsPipeline::createGraphicsPipeline(const core::Window& window, const VulkanConfiguration& configuration) const {
        // Retrieve all SPIR-V shader files
        Vector<String> spirvFiles = VulkanTools::getSpirvFiles(configuration, true);

        Vector<VkPipelineShaderStageCreateInfo> shaderStages;
        Vector<VkShaderModule> shaderModules;
        Vector<VkVertexInputAttributeDescription> vertexAttributes;

        // Load and create shader modules
        for (const auto& file : spirvFiles) {
            log_trace("Load shader: " + file);
            const VkShaderStageFlagBits stage = VulkanTools::getShaderStage(file);

            // Prevent duplicate shader types
            auto it = std::ranges::find_if(shaderStages,
                                           [stage] (const VkPipelineShaderStageCreateInfo& ssi) {
                                               return ssi.stage == stage;
                                           });

            if (it != shaderStages.end()) {
                VulkanMappings mappings;
                std::ostringstream oss;
                oss << "SPIRV-Reflect: Multiple shaders of the same type detected! " << "\n"
                    << "Shader: " << file << "\n"
                    << "conflicts with shader type: " << mappings.getShaderStageDescription(stage);
                throw std::runtime_error(oss.str());
            }

            auto shaderCode = readSpirvFile(file);
            auto shaderModule = createShaderModule(shaderCode, resources_.logicalDevice, file);
            shaderModules.push_back(shaderModule);

            VkPipelineShaderStageCreateInfo shaderStage = {};
            shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStage.stage = stage;
            shaderStage.module = shaderModule;
            shaderStage.pName = "main"; // Defines the shader entry point
            shaderStage.pSpecializationInfo = nullptr;
            shaderStages.push_back(shaderStage);

            // If it is a vertex shader, add attributes
            if (stage == VK_SHADER_STAGE_VERTEX_BIT) {
                // ReSharper disable once CppTooWideScopeInitStatement
                const auto attributes = VulkanTools::parseVertexInputAttributes(shaderCode, file);

                // Only add new locations (avoid duplicates)
                std::unordered_set<uint32_t> uniqueLocations;
                for (const auto& attr : attributes) {
                    if (!uniqueLocations.insert(attr.location).second) {
                        std::stringstream oss;
                        oss << "SPIRV-Reflect: Duplicate attribute location detected -> "
                            << attr.location << " in file " << file;
                        throw std::runtime_error(oss.str());
                    }
                    vertexAttributes.push_back(attr);
                }
            }
        }

        // Vertex Input
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0; // Is set later with `VkVertexInputBindingDescription`
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
        vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();

        // Input assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Draws triangles
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        const auto [framebufferWidth, framebufferHeight] = window.getFramebufferSize();

        // Viewport & Scissor
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(framebufferWidth);
        viewport.height = static_cast<float>(framebufferHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = { static_cast<uint32_t>(framebufferWidth), static_cast<uint32_t>(framebufferHeight) };

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Fill polygons
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        // Multisampmling (no MSAS for now)
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Color Blending (default: simple overwrite)
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
                                            | VK_COLOR_COMPONENT_G_BIT
                                            | VK_COLOR_COMPONENT_B_BIT
                                            | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        // Depth Stencil
        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        // Pipeline Layout
        VkPipelineLayout pipelineLayout = {};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // No descriptor for now
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        auto& res = resources_;
        if (vkCreatePipelineLayout(res.logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        // Create pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = resources_.renderPass;
        pipelineInfo.subpass = 0;

        VkPipeline graphicsPipeline = {};
        if (vkCreateGraphicsPipelines(res.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        } else {
            log_debug("Successfully created graphics pipeline!");
        }

        // Store graphics pipeline in VulkanResources
        res.graphicsPipeline = graphicsPipeline;

        // Release shader modules (after pipeline creation)
        for (auto shaderModule : shaderModules) {
            vkDestroyShaderModule(res.logicalDevice, shaderModule, nullptr);
        }
    }

    Vector<char> VulkanGraphicsPipeline::readSpirvFile(const String& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);;

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open SPIR_V file: " + filename);
        }

        const size_t fileSize = static_cast<size_t>(file.tellg());
        if (fileSize % 4 != 0) {
            throw std::runtime_error("SPIR-V file size is invalid: " + filename);
        }

        Vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        log_trace(std::format("Loaded SPIR-V file: {}, size: {} bytes", filename, fileSize));

        return buffer;
    }

    VkShaderModule VulkanGraphicsPipeline::createShaderModule(const Vector<char>& code,
                                                              const VkDevice device,
                                                              const String& filename) {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module: " + filename);
        }

        return shaderModule;
    }
}
