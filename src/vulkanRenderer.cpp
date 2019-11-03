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

void VulkanRenderer::renderOneFrame()
{
    auto i = frameNumber_++ % 2;

    vkWaitForFences(
      static_cast<VkDevice>(device_->handle()), 1, &frameSyncFences_[i], VK_TRUE, std::numeric_limits<uint64_t>::max());
}
}
