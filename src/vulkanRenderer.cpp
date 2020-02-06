//
// Created by bantdit on 11/3/19.
//

#include "vulkanRenderer.h"
#include "renderPass.h"

namespace cyclonite {
VulkanRenderer::VulkanRenderer(cyclonite::vulkan::Device& device)
  : device_{ &device }
{}

void VulkanRenderer::renderOneFrame(RenderPass& renderPass)
{
    auto currentFrameFence = renderPass.begin(*device_);

    auto const& renderQueueSubmitInfo = renderPass.renderQueueSubmitInfo();

    if (auto result = vkQueueSubmit(
            device_->graphicsQueue(), 1, &renderQueueSubmitInfo, currentFrameFence);
        result != VK_SUCCESS) {

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("can not submit command buffers, out of host memory");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("can not submit command buffers, out of device memory");
        }

        if (result == VK_ERROR_DEVICE_LOST) {
            throw std::runtime_error("can not submit command buffers, device lost");
        }
    }

    renderPass.end(*device_);
}
}
