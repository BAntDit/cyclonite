//
// Created by bantdit on 11/21/19.
//

#include "renderPass.h"

namespace cyclonite {
RenderPass::RenderPass(vulkan::Device const& device,
                       Options::WindowProperties const& windowProperties,
                       VkSampleCountFlagBits sampleCount /* = VK_SAMPLE_COUNT_1_BIT*/)
  : surface_{ Surface{ device.vulkanInstance(), device, windowProperties } }
  , vkRenderPass_{ device.handle(), vkDestroyRenderPass }
  , frameSyncFences_(surface_->swapChainLength(), vulkan::Handle<VkFence>{ device.handle(), vkDestroyFence })
  , renderQueueSubmitInfo_{}
  , frameNumber_{ 0 }
{
    {
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        for (auto& fence : frameSyncFences_) {
            if (vkCreateFence(device.handle(), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
                throw std::runtime_error("could not create sync fences for frame.");
            }
        }
    }

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

    // we have no depth for now
    VkSubpassDescription subPass = {};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subPass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (auto result = vkCreateRenderPass(device.handle(), &renderPassInfo, nullptr, &vkRenderPass_);
        result != VK_SUCCESS) {

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("not enough RAM memory to create render pass");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("not enough GPU memory to create render pass");
        }

        assert(false);
    }
}

auto RenderPass::begin() -> std::tuple<VkFence>
{
    auto i = static_cast<size_t>(frameNumber_++ % surface_->swapChainLength());

    return std::make_tuple(static_cast<VkFence>(frameSyncFences_[i]));
}
}
