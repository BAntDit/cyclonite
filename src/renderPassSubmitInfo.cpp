//
// Created by bantdit on 2/15/20.
//

#include "renderPass.h"

namespace cyclonite {
RenderPass::FrameCommands::FrameCommands()
  : drawCommandsTransferCommandsIndex_{ std::numeric_limits<size_t>::max() }
  , graphicsCommands_{ nullptr }
  , transferCommands_{}
  , transferSemaphores_{}
  , dstWaitFlags_{}
  , transientCommandBuffers_{}
  , transientSemaphores_{}
  , transientDstWaitFlags_{}
  , transferQueueSubmitInfo_{ nullptr }
  , graphicsQueueSubmitInfo_{ nullptr }
{}

auto RenderPass::FrameCommands::hasDrawCommandsTransferCommands() const -> bool
{
    return drawCommandsTransferCommandsIndex_ != std::numeric_limits<size_t>::max();
}

void RenderPass::FrameCommands::addTransientTransferCommands(std::unique_ptr<vulkan::BaseCommandBufferSet>&& bufferSet,
                                                             vulkan::Handle<VkSemaphore>&& semaphore,
                                                             VkPipelineStageFlags dstWaitFlag)
{
    transferQueueSubmitInfo_.reset();
    graphicsQueueSubmitInfo_.reset();

    transientCommandBuffers_.emplace_back(std::move(bufferSet));
    transientSemaphores_.emplace_back(std::move(semaphore));
    transientDstWaitFlags_.emplace_back(dstWaitFlag);
}
}