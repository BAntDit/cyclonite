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
    explicit CommandPool(vulkan::Device const& device);

    CommandPool(CommandPool const&) = delete;

    CommandPool(CommandPool&&) = default;

    ~CommandPool() noexcept;

    auto operator=(CommandPool const&) -> CommandPool& = delete;

    auto operator=(CommandPool&&) -> CommandPool& = default;

    template<typename AllocationCallback, template<typename, typename> typename BufferSet, typename Container>
    auto allocCommandBuffers(BufferSet<CommandPool, Container>&& commandBufferSet, AllocationCallback&& callback)
      -> std::enable_if_t<
        std::is_same_v<std::decay_t<BufferSet<CommandPool, Container>>, CommandBufferSet<CommandPool, Container>> &&
          std::is_invocable_v<AllocationCallback, Container&>,
        BufferSet<CommandPool, Container>&&>;

    template<template<typename, typename> typename BufferSet, typename Container>
    auto releaseCommandBuffers(BufferSet<CommandPool, Container>& commandBufferSet) -> std::enable_if_t<
      std::is_same_v<std::decay_t<BufferSet<CommandPool, Container>>, CommandBufferSet<CommandPool, Container>>,
      void>;

private:
    using queue_family_index_t = uint32_t;
    using pool_key_t = std::tuple<queue_family_index_t, VkCommandPoolCreateFlags>;
    using command_pool_t = std::tuple<vulkan::Handle<VkCommandPool>, std::vector<VkCommandBuffer>>;

    VkDevice vkDevice_;
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
    assert(multithreading::Render::isInRenderThread());

    auto it = commandPools_.find(std::make_tuple(commandBufferSet.queueFamilyIndex(), commandBufferSet.flags()));

    assert(it != commandPools_.end());

    auto& [key, value] = (*it);
    auto& [pool, buffers] = value;

    (void)key;

    {
        auto& src = buffers;
        auto& dst = commandBufferSet.commandBuffers_;

        auto count = std::min(src.size(), static_cast<size_t>(dst.size()));
        auto first = src.size() - count;

        std::copy(std::next(src.begin(), first), src.end(), dst.begin());
        src.erase(std::next(src.begin(), first), src.end());
    }

    auto& commandBuffers = commandBufferSet.commandBuffers_;

    size_t allocateFrom = 0;

    for (size_t i = 0, count = commandBuffers.size(); i < count; i++) {
        auto buffer = commandBuffers[i];

        if (buffer != VK_NULL_HANDLE) {
            allocateFrom++;
        } else {
            // allocate
            VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = static_cast<VkCommandPool>(pool);
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(count - allocateFrom);

            if (auto result =
                  vkAllocateCommandBuffers(vkDevice_, &commandBufferAllocateInfo, commandBuffers.data() + allocateFrom);
                result != VK_SUCCESS) {
                throw std::runtime_error("could not allocate command buffers");
            }

            break;
        }
    }

    callback(commandBuffers);

    commandBufferSet.commandPoolPtr_ = shared_from_this();

    return std::forward<BufferSet<CommandPool, Container>>(commandBufferSet);
}

template<template<typename, typename> typename BufferSet, typename Container>
auto CommandPool::releaseCommandBuffers(BufferSet<CommandPool, Container>& commandBufferSet) -> std::enable_if_t<
  std::is_same_v<std::decay_t<BufferSet<CommandPool, Container>>, CommandBufferSet<CommandPool, Container>>,
  void>
{
    auto releaseCommandsTask = [&commandPools = this->commandPools_,
                                queueFamilyIndex = commandBufferSet.queueFamilyIndex(),
                                flags = commandBufferSet.flags(),
                                &src = commandBufferSet.commandBuffers_]() -> void {
        auto it = commandPools.find(std::make_tuple(queueFamilyIndex, flags));
        assert(it != commandPools.end());

        auto& [key, value] = (*it);
        auto& [pool, buffers] = value;

        (void)key;
        (void)pool;

        auto& dst = buffers;

        for (size_t i = 0, count = src.size(); i < count; i++) {
            assert(src[i] != VK_NULL_HANDLE);

            auto buffer = src[i];

            dst.push_back(buffer);

            if ((flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) != 0) {
                if (auto result = vkResetCommandBuffer(buffer, 0); result != VK_SUCCESS) {
                    throw std::runtime_error("could not release command buffer");
                }
            } else if ((flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) == 0) {
                if (auto result = vkResetCommandBuffer(buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
                    result != VK_SUCCESS) {
                    throw std::runtime_error("could not release command buffer");
                }
            }

            src[i] = VK_NULL_HANDLE;
        }
    };

    if (multithreading::Render::isInRenderThread()) {
        releaseCommandsTask();
    } else {
        assert(multithreading::Worker::isInWorkerThread());
        auto future = multithreading::Worker::threadWorker().taskManager().submitRenderTask(releaseCommandsTask);

        future.get();
    }
}
}

#endif // CYCLONITE_COMMANDPOOL_H
