//
// Created by bantdit on 2/15/20.
//

#include "renderPass.h"

namespace cyclonite {
RenderPass::FrameCommands::FrameCommands() noexcept
  : version_{ 0 }
  , fence_{}
  , drawCommandsTransferCommandsIndex_{ std::numeric_limits<size_t>::max() }
  , graphicsCommands_{ nullptr }
  , transferCommands_{}
  , transferSemaphores_{}
  , transferDstWaitFlags_{}
  , transientCommandBuffers_{}
  , transientSemaphores_{}
  , transientDstWaitFlags_{}
  , transferQueueSubmitInfo_{ nullptr }
  , graphicsQueueSubmitInfo_{ nullptr }
{}

RenderPass::FrameCommands::FrameCommands(vulkan::Device const& device)
  : version_{ 0 }
  , fence_{ device.handle(), vkDestroyFence }
  , drawCommandsTransferCommandsIndex_{ std::numeric_limits<size_t>::max() }
  , graphicsCommands_{ nullptr }
  , transferCommands_{}
  , transferSemaphores_{}
  , transferDstWaitFlags_{}
  , transientCommandBuffers_{}
  , transientSemaphores_{}
  , transientDstWaitFlags_{}
  , transferQueueSubmitInfo_{ nullptr }
  , graphicsQueueSubmitInfo_{ nullptr }
{
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (auto result = vkCreateFence(device.handle(), &fenceCreateInfo, nullptr, &fence_); result != VK_SUCCESS) {
        throw std::runtime_error("could not create frame synchronization fence");
    }
}

auto RenderPass::FrameCommands::hasDrawCommandsTransferCommands() const -> bool
{
    return drawCommandsTransferCommandsIndex_ != std::numeric_limits<size_t>::max();
}

void RenderPass::FrameCommands::addTransientTransferCommands(std::unique_ptr<vulkan::BaseCommandBufferSet>&& bufferSet,
                                                             vulkan::Handle<VkSemaphore>&& semaphore,
                                                             VkPipelineStageFlags dstWaitFlag)
{
    transientCommandBuffers_.emplace_back(std::move(bufferSet));
    transientSemaphores_.emplace_back(std::move(semaphore));
    transientDstWaitFlags_.emplace_back(dstWaitFlag);
}

void RenderPass::FrameCommands::_clearTransientTransfer()
{
    // clear out of date transient commands
    transientCommandBuffers_.clear();
    transientSemaphores_.clear();
    transientDstWaitFlags_.clear();
}

void RenderPass::FrameCommands::update(vulkan::Device& device, FrameCommands& frameUpdate)
{
    if (version() == frameUpdate.version())
        return;

    auto version = frameUpdate.version();

    if (transferSubmitVersion() != frameUpdate.transferSubmitVersion()) {
        _clearTransientTransfer();

        transferQueueSubmitInfo_.reset();
        graphicsQueueSubmitInfo_.reset(); // because of new semaphores list

        transferCommands_ = frameUpdate.transferCommands_;
        transferSemaphores_ = frameUpdate.transferSemaphores_;
        transferDstWaitFlags_ = frameUpdate.transientDstWaitFlags_;

        if (!frameUpdate.transientCommandBuffers_.empty()) {
            // transient commands stay actual one frame only
            // move transient commands ownership
            transientCommandBuffers_ = std::move(frameUpdate.transientCommandBuffers_);
            transientSemaphores_ = std::move(frameUpdate.transientSemaphores_);
            transientDstWaitFlags_ = std::move(frameUpdate.transientDstWaitFlags_);

            // just because vector has no standard-defined moved-from state
            // so, just in case:
            frameUpdate.transientCommandBuffers_.clear();
            frameUpdate.transientSemaphores_.clear();
            frameUpdate.transientDstWaitFlags_.clear();
            // ...

            transferSemaphores_.reserve(transferSemaphores_.size() + transientSemaphores_.size());
            transientDstWaitFlags_.reserve(transientDstWaitFlags_.size() + transientDstWaitFlags_.size());

            for (size_t i = 0, count = transientCommandBuffers_.size(); i < count; i++) {
                transferSemaphores_.push_back(static_cast<VkSemaphore>(transientSemaphores_[i]));
                transferDstWaitFlags_.push_back(transferDstWaitFlags_[i]);

                for (size_t k = 0, bufferCount = transientCommandBuffers_[i]->commandBufferCount(); k < bufferCount;
                     k++) {
                    transferCommands_.push_back(transientCommandBuffers_[i]->getCommandBuffer(k));
                }
            }

            // up version in advance
            // to reassembly transfer submit with no current frame transient commands next frame
            frameUpdate.version_ = uint64_t{ static_cast<uint64_t>(frameUpdate.transferSubmitVersion() + 1) |
                                             static_cast<uint64_t>(frameUpdate.graphicsSubmitVersion()) << 32UL };
        }

        if (!transferCommands_.empty()) {
            transferQueueSubmitInfo_ = std::make_unique<VkSubmitInfo>(VkSubmitInfo{});

            transferQueueSubmitInfo_->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            transferQueueSubmitInfo_->commandBufferCount = static_cast<uint32_t>(transferCommands_.size());
            transferQueueSubmitInfo_->pCommandBuffers = transferCommands_.data();
            transferQueueSubmitInfo_->signalSemaphoreCount = static_cast<uint32_t>(transferSemaphores_.size());
            transferQueueSubmitInfo_->pSignalSemaphores = transferSemaphores_.data();
        }
    }

    if (graphicsSubmitVersion() != frameUpdate.graphicsSubmitVersion()) {
        graphicsQueueSubmitInfo_.reset(); // it could be still alive here

        graphicsCommands_ = std::make_unique<graphics_queue_commands_t>(device.commandPool().allocCommandBuffers(
          vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>{
            device.graphicsQueueFamilyIndex(),
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            std::array<VkCommandBuffer, 1>{} },
          [](auto&& graphicsCommands) -> void {
              auto [commandBuffer] = graphicsCommands;

              VkCommandBufferBeginInfo commandBufferBeginInfo = {};
              commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

              if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
                  throw std::runtime_error("could not begin recording command buffer!");
              }

              // TODO:: THE MAIN TODO for the next time!!!

              if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
                  throw std::runtime_error("could not record command buffer!");
              }
          }));
    }

    if (!graphicsQueueSubmitInfo_) {
        // TODO::
    }

    version_ = version;
}
}