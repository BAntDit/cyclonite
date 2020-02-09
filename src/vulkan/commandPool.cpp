//
// Created by bantdit on 2/8/20.
//

#include "commandPool.h"
#include "device.h"

namespace cyclonite::vulkan {
CommandPool::CommandPool(vulkan::Device const& device, multithreading::TaskManager const& taskManager)
  : device_{ device.handle() }
  , taskManager_{ &taskManager }
  , commandPools_{}
{
    auto createPool = [&, this](std::thread::id id, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) -> void {
        auto [it, success] =
          commandPools_.emplace(std::make_tuple(id, queueFamilyIndex, flags),
                                std::make_tuple(Handle<VkCommandPool>{ device.handle(), vkDestroyCommandPool },
                                                std::vector<VkCommandBuffer>{}));

        assert(success);

        auto& [key, value] = (*it);
        auto& [pool, buffers] = value;

        (void)key;
        (void)buffers;

        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.flags = flags;
        commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

        if (auto result = vkCreateCommandPool(device.handle(), &commandPoolCreateInfo, nullptr, &pool);
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create command pool");
        }
    };

    for (auto& thread : taskManager.pool()) {
        for (auto& queueFamilyIndex : device.queueFamilyIndices()) {
            createPool(thread.get_id(), queueFamilyIndex, 0); // for persistent buffers

            createPool(thread.get_id(),
                       queueFamilyIndex,
                       VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

            createPool(thread.get_id(), queueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        }
    }
}

auto CommandPool::releaseCommandBuffer(CommandBuffer commandBuffer) -> std::future<void>
{
    assert(commandBuffer.flags() != 0); // do not release persistent buffer

    auto it = commandPools_.find(
      std::make_tuple(commandBuffer.threadId(), commandBuffer.queueFamilyIndex(), commandBuffer.flags()));

    assert(it != commandPools_.end());

    auto& [key, value] = (*it);
    auto& [pool, buffers] = value;

    (void)key;
    (void)pool;

    return taskManager_->strand([&, &buffers = buffers]() -> void { buffers.push_back(std::move(commandBuffer)); });
}
}