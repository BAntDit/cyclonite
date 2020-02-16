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
    auto&& [frame, fence] = renderPass.begin(*device_);

    if (frame.transferQueueSubmitInfo()) { // TODO:: should make transfer from another thread, m'kay
        if (auto result =
              vkQueueSubmit(device_->hostTransferQueue(), 1, frame.transferQueueSubmitInfo().get(), VK_NULL_HANDLE);
            result != VK_SUCCESS) {
            _handleSubmitError(result);
        }
    }

    if (frame.graphicsQueueSubmitInfo()) {
        if (auto result = vkQueueSubmit(device_->graphicsQueue(), 1, frame.graphicsQueueSubmitInfo().get(), fence);
            result != VK_SUCCESS) {
            _handleSubmitError(result);
        }
    }

    renderPass.end(*device_);
}

void _handleSubmitError(VkResult result)
{
    if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
        throw std::runtime_error("can not submit command buffers, out of host memory");
    }

    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
        throw std::runtime_error("can not submit command buffers, out of device memory");
    }

    if (result == VK_ERROR_DEVICE_LOST) {
        throw std::runtime_error("can not submit command buffers, device lost");
    }

    // shouldn't have come here (the case out of spec)
    assert(false);
}
}
