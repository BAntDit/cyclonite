//
// Created by bantdit on 2/8/20.
//

#include "commandBuffer.h"
#include "commandPool.h"

namespace cyclonite::vulkan {
CommandBuffer::CommandBuffer() noexcept
  : vkCommandBuffer_{ VK_NULL_HANDLE }
  , threadId_{ std::this_thread::get_id() }
  , queueFamilyIndex_{ 0 }
  , flags_{ 0 }
{}

CommandBuffer::CommandBuffer(CommandBuffer&& commandBuffer) noexcept
  : vkCommandBuffer_{ commandBuffer.vkCommandBuffer_ }
  , threadId_{ commandBuffer.threadId_ }
  , queueFamilyIndex_{ commandBuffer.queueFamilyIndex_ }
  , flags_{ commandBuffer.flags_ }
{
    commandBuffer.vkCommandBuffer_ = VK_NULL_HANDLE;
}

auto CommandBuffer::operator=(CommandBuffer&& rhs) noexcept -> CommandBuffer&
{
    vkCommandBuffer_ = rhs.vkCommandBuffer_;
    rhs.vkCommandBuffer_ = VK_NULL_HANDLE;

    threadId_ = rhs.threadId_;
    queueFamilyIndex_ = rhs.queueFamilyIndex_;
    flags_ = rhs.flags_;
}
}
