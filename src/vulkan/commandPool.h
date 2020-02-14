//
// Created by bantdit on 2/7/20.
//

#ifndef CYCLONITE_COMMANDPOOL_H
#define CYCLONITE_COMMANDPOOL_H

#include "../hash.h"
#include "../multithreading/taskManager.h"
#include "commandBufferSet.h"
#include "handle.h"
#include <easy-mp/containers.h>
#include <thread>
#include <unordered_map>

namespace cyclonite::vulkan {
class Device;

class CommandPool : public std::enable_shared_from_this<CommandPool>
{
public:
    CommandPool(multithreading::TaskManager const& taskManager, vulkan::Device const& device);

    CommandPool(CommandPool const&) = delete;

    CommandPool(CommandPool&&) = default;

    ~CommandPool() = default;

    auto operator=(CommandPool const&) -> CommandPool& = delete;

    auto operator=(CommandPool &&) -> CommandPool& = default;

    template<typename AllocationCallback, template<typename, typename> typename BufferSet, typename Container>
    auto allocCommandBuffers(BufferSet<CommandPool, Container>&& commandBufferSet, AllocationCallback&& callback)
      -> std::enable_if_t<
        std::is_same_v<std::decay_t<BufferSet<CommandPool, Container>>, CommandBufferSet<CommandPool, Container>> &&
          std::is_invocable_v<AllocationCallback, Container&>,
        BufferSet<CommandPool, Container>&&>;

    template<template<typename, typename> typename BufferSet, typename Container>
    auto releaseCommandBuffers(BufferSet<CommandPool, Container>& commandBufferSet) -> std::enable_if_t<
      std::is_same_v<std::decay_t<BufferSet<CommandPool, Container>>, CommandBufferSet<CommandPool, Container>>,
      std::future<void>>;

private:
    using pool_key_t = std::tuple<std::thread::id, uint32_t, VkCommandPoolCreateFlags>;
    using command_pool_t = std::tuple<vulkan::Handle<VkCommandPool>, std::vector<VkCommandBuffer>>;

    VkDevice vkDevice_;
    multithreading::TaskManager const* taskManager_;
    std::unordered_map<pool_key_t, command_pool_t, hash> commandPools_;
};

template<typename AllocationCallback, template<typename, typename> typename BufferSet, typename Container>
auto CommandPool::allocCommandBuffers(BufferSet<CommandPool, Container>&& commandBufferSet,
                                      AllocationCallback&& callback)
  -> std::enable_if_t<
    std::is_same_v<std::decay_t<BufferSet<CommandPool, Container>>, CommandBufferSet<CommandPool, Container>> &&
      std::is_invocable_v<AllocationCallback, Container&>,
    BufferSet<CommandPool, Container>&&>
{
    auto it = commandPools_.find(
      std::make_tuple(commandBufferSet.threadId(), commandBufferSet.queueFamilyIndex(), commandBufferSet.flags()));

    assert(it != commandPools_.end());

    auto& [key, value] = (*it);
    auto& [pool, buffers] = value;

    (void)key;

    auto future = taskManager_->strand([&, &src = buffers, &dst = commandBufferSet.commandBuffers_]() -> void {
        auto count = std::min(src.size(), static_cast<size_t>(dst.size()));
        auto first = src.size() - count;

        std::copy(std::next(src.begin(), first), src.end(), dst.begin());

        src.erase(std::next(src.begin(), first), src.end());
    });

    future.get();

    auto& commandBuffers = commandBufferSet.commandBuffers_;

    bool needsResets = true;
    size_t allocateForm = 0;

    for (size_t i = 0, count = commandBuffers.size(); i < count; i++) {
        auto buffer = commandBuffers[i];

        if (needsResets && buffer != VK_NULL_HANDLE) {
            allocateForm++;
        } else if (needsResets && buffer == VK_NULL_HANDLE) {
            needsResets = false;

            // allocate
            VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = static_cast<VkCommandPool>(pool);
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(count - allocateForm);

            if (auto result =
                  vkAllocateCommandBuffers(vkDevice_, &commandBufferAllocateInfo, commandBuffers.data() + allocateForm);
                result != VK_SUCCESS) {
                throw std::runtime_error("could not allocate command buffers");
            }
        }

        if (needsResets && (commandBufferSet.flags() & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) != 0) {
            if (auto result = vkResetCommandBuffer(buffer, 0); result != VK_SUCCESS) {
                throw std::runtime_error("could not release command buffer");
            }
        } else if (needsResets && (commandBufferSet.flags() & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) == 0) {
            if (auto result = vkResetCommandBuffer(buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
                result != VK_SUCCESS) {
                throw std::runtime_error("could not release command buffer");
            }
        }
    }

    callback(commandBuffers);

    commandBufferSet.commandPoolPtr_ = shared_from_this();

    return std::forward<BufferSet<CommandPool, Container>>(commandBufferSet);
}

template<template<typename, typename> typename BufferSet, typename Container>
auto CommandPool::releaseCommandBuffers(BufferSet<CommandPool, Container>& commandBufferSet) -> std::enable_if_t<
  std::is_same_v<std::decay_t<BufferSet<CommandPool, Container>>, CommandBufferSet<CommandPool, Container>>,
  std::future<void>>
{
    auto threadId = commandBufferSet.threadId();
    auto queueFamilyIndex = commandBufferSet.queueFamilyIndex();
    auto flags = commandBufferSet.flags();

    auto it = commandPools_.find(std::make_tuple(threadId, queueFamilyIndex, flags));

    auto& [key, value] = (*it);
    auto& [pool, buffers] = value;

    (void)key;
    (void)pool;

    return taskManager_->strand([&, &dst = buffers, &src = commandBufferSet.commandBuffers_]() -> void {
        dst.reserve(dst.size() + src.size());

        for (size_t i = 0, count = src.size(); i < count; i++) {
            assert(src[i] != VK_NULL_HANDLE);

            dst.push_back(src[i]);

            src[i] = VK_NULL_HANDLE;
        }
    });
}
}

#endif // CYCLONITE_COMMANDPOOL_H
