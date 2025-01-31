#include "vulkan_render_pass.hpp"
#include "core/logger.hpp"
#include <array>

namespace time_kill::graphics {
    VulkanRenderPass::VulkanRenderPass(VulkanResources& resources) : resources_(resources) {}

    VulkanRenderPass::~VulkanRenderPass() {
        destroyRenderPass();
    }

    void VulkanRenderPass::createRenderPass() {
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

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();

        if (vkCreateRenderPass(res.logicalDevice, &renderPassInfo, nullptr, &res.renderPass)) {
            throw std::runtime_error("failed to create render pass!");
        }

        log_trace("Successfully created render pass.");
    }

    void VulkanRenderPass::destroyRenderPass() {
        auto& res = resources_;
        if (res.renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(res.logicalDevice, res.renderPass, nullptr);
            res.renderPass = VK_NULL_HANDLE;
            log_trace("Destroyed render pass.");
        }
    }
}
