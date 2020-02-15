//
// Created by bantdit on 2/8/20.
//

#include "commandPool.h"
#include "device.h"

namespace cyclonite::vulkan {
CommandPool::CommandPool(multithreading::TaskManager const& taskManager, vulkan::Device const& device)
  : vkDevice_{ device.handle() }
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

    // pool for main thread:
    for (auto& queueFamilyIndex : device.queueFamilyIndices()) {
        createPool(std::this_thread::get_id(), queueFamilyIndex, 0);

        createPool(std::this_thread::get_id(),
                   queueFamilyIndex,
                   VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        createPool(std::this_thread::get_id(), queueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }

    // pool for each thread of thread pool:
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

CommandPool::~CommandPool() noexcept
{
    // if we're here, it's over and we don't care if something write into pool in some other threads

    for (auto& [k, v] : commandPools_) {
        (void)k;

        auto& [pool, buffers] = v;

        vkFreeCommandBuffers(vkDevice_, static_cast<VkCommandPool>(pool), buffers.size(), buffers.data());

        buffers.clear();
    }

    commandPools_.clear();
}
}