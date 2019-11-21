//
// Created by bantdit on 11/21/19.
//

#include "renderPass.h"

namespace cyclonite {
RenderPass::RenderPass(VkInstance vkInstance,
                       cyclonite::vulkan::Device const& device,
                       cyclonite::Options::WindowProperties const& windowProperties,
                       VkSampleCountFlagBits sampleCount/* = VK_SAMPLE_COUNT_1_BIT*/)
  : surface_{ Surface{ vkInstance, device, windowProperties } }
  , renderQueueSubmitInfo_{}
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = surface_->format();
    colorAttachment.samples = sampleCount;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}
}
