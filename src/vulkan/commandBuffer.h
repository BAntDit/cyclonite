//
// Created by bantdit on 2/8/20.
//

#ifndef CYCLONITE_COMMANDBUFFER_H
#define CYCLONITE_COMMANDBUFFER_H

#include <thread>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
class CommandPool;

class CommandBuffer
{
public:
    friend class CommandPool;

    CommandBuffer(CommandBuffer const&) = delete;

    CommandBuffer(CommandBuffer&& commandBuffer) noexcept;

    ~CommandBuffer() = default;

    auto operator=(CommandBuffer const&) -> CommandBuffer& = delete;

    auto operator=(CommandBuffer&& rhs) noexcept -> CommandBuffer&;

    [[nodiscard]] auto threadId() const -> std::thread::id { return threadId_; }

    [[nodiscard]] auto flags() const -> VkCommandPoolCreateFlags { return flags_; }

    [[nodiscard]] auto queueFamilyIndex() const -> uint32_t { return queueFamilyIndex_; }

private:
    CommandBuffer(VkCommandBuffer commandBuffer,
                  std::thread::id threadId,
                  uint32_t queueFamilyIndex,
                  VkCommandPoolCreateFlags flags) noexcept;

private:
    VkCommandBuffer vkCommandBuffer_;
    std::thread::id threadId_;
    uint32_t queueFamilyIndex_;
    VkCommandPoolCreateFlags flags_;
};
}

#endif // CYCLONITE_COMMANDBUFFER_H
