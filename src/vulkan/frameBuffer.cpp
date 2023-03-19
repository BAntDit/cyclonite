//
// Created by anton on 12/27/21.
//

#include "frameBuffer.h"

namespace cyclonite::vulkan {
FrameBuffer::FrameBuffer(vulkan::Device const& device,
                         VkRenderPass vkRenderPass,
                         uint32_t width,
                         uint32_t height,
                         vulkan::ImageView&& depthStencilAttachment)
  : FrameBuffer(device,
                vkRenderPass,
                width,
                height,
                std::optional<vulkan::ImageView>{ std::move(depthStencilAttachment) },
                std::array<vulkan::ImageView, 0>{})
{
}
}