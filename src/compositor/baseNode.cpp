//
// Created by anton on 1/4/21.
//

#include "baseNode.h"

namespace cyclonite::compositor {
BaseNode::BaseNode() noexcept
  : commandsIndex_{ 0 }
  , camera_{}
  , inputs_{}
  , outputs_{}
  , vkRenderPass_{}
  , renderTarget_{}
  , signalSemaphores_{}
{}

auto BaseNode::passFinishedSemaphore() const -> vulkan::Handle<VkSemaphore> const&
{
    assert(commandsIndex_ < signalSemaphores_.size());
    return signalSemaphores_[commandsIndex_];
}

auto BaseNode::descriptorPool() const -> VkDescriptorPool
{
    return static_cast<VkDescriptorPool>(descriptorPool_);
}
}
