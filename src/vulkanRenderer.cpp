//
// Created by bantdit on 11/3/19.
//

#include "vulkanRenderer.h"

namespace cyclonite {
VulkanRenderer::VulkanRenderer(cyclonite::vulkan::Device& device)
  : frameSyncFences_{ vulkan::Handle<VkFence>{ device.handle(), vkDestroyFence },
                      vulkan::Handle<VkFence>{ device.handle(), vkDestroyFence } }
  , frameNumber_{ 0 }
  , device_{ &device }
{
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    for (auto& fence : frameSyncFences_) {
        if (vkCreateFence(static_cast<VkDevice>(device.handle()), &fenceInfo, nullptr, &fence) != VK_SUCCESS) {
            throw std::runtime_error("could not create sync fences for frame.");
        }
    }
}

void VulkanRenderer::renderOneFrame(RenderPass const& renderPass)
{
    auto i = frameNumber_++ % 2;

    vkWaitForFences(
      static_cast<VkDevice>(device_->handle()), 1, &frameSyncFences_[i], VK_TRUE, std::numeric_limits<uint64_t>::max());

    auto const& renderQueueSubmitInfo = renderPass.renderQueueSubmitInfo();

    if (auto result = vkQueueSubmit(device_->graphicsQueue(),
                                    renderQueueSubmitInfo.size(),
                                    renderQueueSubmitInfo.data(),
                                    static_cast<VkFence>(frameSyncFences_[i]));
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
