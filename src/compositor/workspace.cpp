//
// Created by anton on 1/10/21.
//

#include "workspace.h"

namespace cyclonite::compositor {
void Workspace::render(vulkan::Device& device)
{
    auto frameFence = static_cast<VkFence>(frameFences_[frameIndex_]);

    // wait until frame over before render this frame
    vkWaitForFences(device.handle(), 1, &frameFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

    for (auto&& node : nodes_) {
        node->render(frameIndex_, frameFence);
    }

    vkResetFences(device.handle(), 1, &frameFence);

    frameIndex_ = frameNumber_++ % swapChainLength_;
}
}
