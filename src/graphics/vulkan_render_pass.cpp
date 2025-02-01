#include "vulkan_render_pass.hpp"
#include "core/logger.hpp"
#include <array>

namespace time_kill::graphics {
    // ReSharper disable once CppParameterNamesMismatch
    VulkanRenderPass::VulkanRenderPass(VulkanResources& resources) : resources_(resources) {}

    VulkanRenderPass::~VulkanRenderPass() {
        destroyRenderPass();
    }

    void VulkanRenderPass::createRenderPass() const {
        auto& res = resources_;

        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = res.swapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = res.depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        const std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(res.logicalDevice, &renderPassInfo, nullptr, &res.renderPass)) {
            throw std::runtime_error("failed to create render pass!");
        }

        log_trace("Successfully created render pass.");
    }

    void VulkanRenderPass::destroyRenderPass() const {
        if (auto& res = resources_; res.renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(res.logicalDevice, res.renderPass, nullptr);
            res.renderPass = VK_NULL_HANDLE;
            log_trace("Destroyed render pass.");
        }
    }
}
