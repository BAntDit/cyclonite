//
// Created by bantdit on 2/8/20.
//

#include "commandBuffer.h"
#include "commandPool.h"

namespace cyclonite::vulkan {
CommandBuffer::CommandBuffer(VkCommandBuffer commandBuffer,
                             std::thread::id threadId,
                             uint32_t queueFamilyIndex,
                             VkCommandPoolCreateFlags flags) noexcept
  : vkCommandBuffer_{ commandBuffer }
  , threadId_{ threadId }
  , queueFamilyIndex_{ queueFamilyIndex }
  , flags_{ flags }
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
