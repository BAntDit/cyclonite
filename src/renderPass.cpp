//
// Created by bantdit on 11/21/19.
//

#include "renderPass.h"

namespace cyclonite {
auto RenderPass::begin(vulkan::Device const& device) -> VkFence
{
    auto currentCainIndex = renderTarget_->currentChainIndex();

    vkWaitForFences(device.handle(), 1, &frameFences_[currentCainIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());

    auto nextChainIndex = renderTarget_->getNextChainIndex(device);

    if (renderTargetFences_[nextChainIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(
          device.handle(), 1, &renderTargetFences_[nextChainIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
    }

    return renderTargetFences_[nextChainIndex] = static_cast<VkFence>(frameFences_[currentCainIndex]);
}
}
