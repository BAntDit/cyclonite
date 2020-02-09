//
// Created by bantdit on 2/7/20.
//

#ifndef CYCLONITE_COMMANDPOOL_H
#define CYCLONITE_COMMANDPOOL_H

#include "../hash.h"
#include "../multithreading/taskManager.h"
#include "commandBuffer.h"
#include "handle.h"
#include <easy-mp/containers.h>
#include <thread>
#include <unordered_map>

namespace cyclonite::vulkan {
class Device;

class CommandPool
{
public:
    CommandPool(vulkan::Device const& device, multithreading::TaskManager const& taskManager);

    CommandPool(CommandPool const&) = delete;

    CommandPool(CommandPool&&) = default;

    ~CommandPool() = default;

    auto operator=(CommandPool const&) -> CommandPool& = delete;

    auto operator=(CommandPool &&) -> CommandPool& = default;

    template<typename AllocationCallback, typename Container>
    auto allocCommandBuffers(Container&& container,
                             uint32_t queueFamilyIndex,
                             VkCommandPoolCreateFlags flags,
                             AllocationCallback&& callback)
      -> std::enable_if_t<easy_mp::is_iterable_v<Container> && std::is_same_v<Container::value_type, CommandBuffer> &&
                            std::is_invocable_v<AllocationCallback, Container&&>,
                          Container&&>;

    template<typename Container>
    auto releaseCommandBuffers(Container&& commandBuffers)
      -> std::enable_if_t<easy_mp::is_iterable_v<Container> && std::is_same_v<Container::value_type, CommandBuffer>,
                          std::future<void>>;

private:
    using pool_key_t = std::tuple<std::thread::id, uint32_t, VkCommandPoolCreateFlags>;
    using command_pool_t = std::tuple<vulkan::Handle<VkCommandPool>, std::vector<VkCommandBuffer>>;

    VkDevice device_;
    multithreading::TaskManager const* taskManager_;
    std::unordered_map<pool_key_t, command_pool_t, hash> commandPools_;
};

template<typename AllocationCallback, typename Container>
auto CommandPool::allocCommandBuffers(Container&& container,
                                      uint32_t queueFamilyIndex,
                                      VkCommandPoolCreateFlags flags,
                                      AllocationCallback&& callback)
  -> std::enable_if_t<easy_mp::is_iterable_v<Container> && std::is_same_v<Container::value_type, CommandBuffer> &&
                        std::is_invocable_v<AllocationCallback, Container&&>,
                      Container&&>
{
    auto threadId = std::this_thread::get_id();

    auto it = commandPools_.find(std::make_tuple(threadId, queueFamilyIndex, flags));

    auto& [key, value] = (*it);
    auto& [pool, buffers] = value;

    (void)key;

    auto countBuffers = container.size();

    auto future = taskManager_->strand([&, &buffers = buffers]() -> std::vector<VkCommandBuffer> {
        std::vector<VkCommandBuffer> res(countBuffers, VK_NULL_HANDLE);

        auto count = std::min(buffers.size(), res.size());
        auto first = buffers.size() - count;

        std::copy(std::next(buffers.begin(), first), buffers.end(), res.begin());

        buffers.erase(std::next(buffers.begin(), first), buffers.end());

        return res;
    });

    auto&& commandBuffers = future.get();

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
                  vkAllocateCommandBuffers(device_, &commandBufferAllocateInfo, commandBuffers.data() + allocateForm);
                result != VK_SUCCESS) {
                throw std::runtime_error("could not allocate command buffers");
            }
        }

        if (needsResets && (flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) != 0) {
            if (auto result = vkResetCommandPool(buffer, 0); result != VK_SUCCESS) {
                throw std::runtime_error("could not release command buffer");
            }
        } else if (needsResets && (flags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) == 0) {
            if (auto result = vkResetCommandPool(buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
                result != VK_SUCCESS) {
                throw std::runtime_error("could not release command buffer");
            }
        }

        container[i].vkCommandBuffer_ = buffer;
        container[i].threadId_ = threadId;
        container[i].queueFamilyIndex_ = queueFamilyIndex;
        container[i].flags_ = flags;
    }

    callback(std::forward<Container>(container));

    return std::forward<Container>(container);
}

template<typename Container>
auto CommandPool::releaseCommandBuffers(Container&& commandBuffers)
  -> std::enable_if_t<easy_mp::is_iterable_v<Container> && std::is_same_v<Container::value_type, CommandBuffer>,
                      std::future<void>>
{
    assert(commandBuffers.size() > 0);

    auto& cb = commandBuffers[0];

    auto threadId = cb.threadId();
    auto queueFamilyIndex = cb.queueFamilyIndex();
    auto flags = cb.flags();

    auto it = commandPools_.find(std::make_tuple(threadId, queueFamilyIndex, flags));

    auto& [key, value] = (*it);
    auto& [pool, buffers] = value;

    (void)key;
    (void)pool;

    return taskManager_->strand([&, &buffers = buffers]() -> void {
        buffers.reserve(buffers.size() + commandBuffers.size());

        for (auto&& buffer : commandBuffers) {
            assert(buffer.vkCommandBuffer_ != VK_NULL_HANDLE);

            buffers.push_back(buffer.vkCommandBuffer_);

            buffer.vkCommandBuffer_ = VK_NULL_HANDLE;
        }
    });
}
}

#endif // CYCLONITE_COMMANDPOOL_H
