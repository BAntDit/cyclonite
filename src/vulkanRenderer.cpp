//
// Created by bantdit on 11/3/19.
//

#include "vulkanRenderer.h"

namespace cyclonite {
VulkanRenderer::VulkanRenderer(cyclonite::vulkan::Device& device)
  : device_{ &device }
{}

void VulkanRenderer::renderOneFrame(RenderPass& renderPass)
{
    auto&& [vkFrameFence] = renderPass.begin();

    vkWaitForFences(
      static_cast<VkDevice>(device_->handle()), 1, &vkFrameFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

    if (auto result = vkResetFences(device_->handle(), 1, &vkFrameFence); result != VK_SUCCESS) {
        throw std::runtime_error("error on attempt to reset frame synchronization fence");
    }

    auto const& renderQueueSubmitInfo = renderPass.renderQueueSubmitInfo();

    if (auto result = vkQueueSubmit(
          device_->graphicsQueue(), renderQueueSubmitInfo.size(), renderQueueSubmitInfo.data(), vkFrameFence);
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
}
}
