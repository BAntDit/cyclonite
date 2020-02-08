//
// Created by bantdit on 2/7/20.
//

#ifndef CYCLONITE_COMMANDPOOL_H
#define CYCLONITE_COMMANDPOOL_H

#include "../hash.h"
#include "../multithreading/taskManager.h"
#include "commandBuffer.h"
#include "handle.h"
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

    template<typename AllocationCallback>
    auto allocCommandBuffer(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags, AllocationCallback&& callback)
      -> std::enable_if_t<std::is_invocable_v<AllocationCallback, VkCommandBuffer>, CommandBuffer>;

    auto releaseCommandBuffer(CommandBuffer commandBuffer) -> std::future<void>;

private:
    using pool_key_t = std::tuple<std::thread::id, uint32_t, VkCommandPoolCreateFlags>;
    using command_pool_t = std::tuple<vulkan::Handle<VkCommandPool>, std::vector<CommandBuffer>>;

    multithreading::TaskManager const* taskManager_;
    std::unordered_map<pool_key_t, command_pool_t, hash> commandPools_;
};
}

#endif // CYCLONITE_COMMANDPOOL_H
