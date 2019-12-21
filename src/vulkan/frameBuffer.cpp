//
// Created by bantdit on 12/21/19.
//

#include "frameBuffer.h"

namespace cyclonite::vulkan {
FrameBuffer::FrameBuffer(vulkan::Device const& device,
                         VkRenderPass vkRenderPass,
                         uint32_t width,
                         uint32_t height,
                         std::vector<vulkan::ImageView>&& attachments)
  : attachments_{ std::move(attachments) }
  , vkFrameBuffer_{ device.handle(), vkDestroyFramebuffer }
{
    std::vector<VkImageView> imageViews_(attachments_.size(), VK_NULL_HANDLE);

    for (size_t i = 0, count = attachments_.size(); i < count; i++) {
        imageViews_[i] = attachments_[i].handle();
    }

    VkFramebufferCreateInfo framebufferInfo = {};

    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vkRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews_.size());
    framebufferInfo.pAttachments = imageViews_.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1; // for now

    if (auto result = vkCreateFramebuffer(device.handle(), &framebufferInfo, nullptr, &vkFrameBuffer_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("failed to create framebuffer: out of host memory");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("failed to create framebuffer: out of device memory");
        }

        assert(false);
    }
}
}
