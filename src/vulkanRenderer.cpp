//
// Created by bantdit on 11/3/19.
//

#include "vulkanRenderer.h"
#include "renderPass.h"
#include <iostream>

namespace cyclonite {
VulkanRenderer::VulkanRenderer(cyclonite::vulkan::Device& device, multithreading::TaskManager& taskManager)
  : taskManager_{ &taskManager }
  , device_{ &device }
  , frameCounter_{ 0 }
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

    if (auto result = vkQueueSubmit(device_->graphicsQueue(), 1, &frame.graphicsQueueSubmitInfo(), fence);
        result != VK_SUCCESS) {
        _handleSubmitError(result);
    }

    renderPass.end(*device_);
}

void VulkanRenderer::finish()
{
    auto gfxStop = taskManager_->submit([&, this]() -> void {
        if (auto result = vkQueueWaitIdle(device_->graphicsQueue()); result != VK_SUCCESS) {
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                throw std::runtime_error("could not wait until graphics queue gets idle, out of host memory");
            }

            if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                throw std::runtime_error("could not wait until graphics queue gets idle, out of device memory");
            }

            if (result == VK_ERROR_DEVICE_LOST) {
                throw std::runtime_error("could not wait until graphics queue gets idle, device lost");
            }
        }
    });

    auto transferStop = taskManager_->submit([&, this]() -> void {
        if (auto result = vkQueueWaitIdle(device_->hostTransferQueue()); result != VK_SUCCESS) {
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                throw std::runtime_error("could not wait until transfer queue gets idle, out of host memory");
            }

            if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                throw std::runtime_error("could not wait until transfer queue gets idle, out of device memory");
            }

            if (result == VK_ERROR_DEVICE_LOST) {
                throw std::runtime_error("could not wait until transfer queue gets idle, device lost");
            }
        }
    });

    transferStop.get();
    gfxStop.get();
}

void VulkanRenderer::_handleSubmitError(VkResult result)
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

    // shouldn't have come here (a case out of the spec)
    assert(false);
}
}
