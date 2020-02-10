//
// Created by bantdit on 2/10/20.
//

#ifndef CYCLONITE_COMMANDBUFFERSET_H
#define CYCLONITE_COMMANDBUFFERSET_H

#include <cassert>
#include <easy-mp/containers.h>
#include <thread>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
class CommandPool;

template<typename Container>
class CommandBufferSet
{
    static_assert(easy_mp::is_contiguous_v<Container> && std::is_same_v<Container::value_type, VkCommandBuffer>);

public:
    friend class CommandPool;

    CommandBufferSet(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags, Container&& buffers) noexcept;

    CommandBufferSet(CommandBufferSet const&) = delete;

    CommandBufferSet(CommandBufferSet&&) = default;

    ~CommandBufferSet() = default;

    auto operator=(CommandBufferSet const&) -> CommandBufferSet& = delete;

    auto operator=(CommandBufferSet &&) -> CommandBufferSet& = default;

    [[nodiscard]] auto threadId() const -> std::thread::id { return threadId_; }

    [[nodiscard]] auto queueFamilyIndex() const -> uint32_t { return queueFamilyIndex_; }

    [[nodiscard]] auto flags() const -> VkCommandPoolCreateFlags { return flags_; }

private:
    std::thread::id threadId_;
    uint32_t queueFamilyIndex_;
    VkCommandPoolCreateFlags flags_;
    Container commandBuffers_;
};

template<typename Container>
CommandBufferSet<Container>::CommandBufferSet(uint32_t queueFamilyIndex,
                                              VkCommandPoolCreateFlags flags,
                                              Container&& buffers) noexcept
  : threadId_{ std::this_thread::get_id() }
  , queueFamilyIndex_{ queueFamilyIndex }
  , flags_{ flags }
  , commandBuffers_(std::move(buffers))
{}
}

#endif // CYCLONITE_COMMANDBUFFERSET_H
