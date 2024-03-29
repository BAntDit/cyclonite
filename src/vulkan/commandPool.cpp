//
// Created by bantdit on 2/8/20.
//

#include "commandPool.h"
#include "device.h"

namespace cyclonite::vulkan {
CommandPool::CommandPool(vulkan::Device const& device)
  : vkDevice_{ device.handle() }
  , commandPools_{}
{
    auto createPool = [&, this](uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) -> void {
        auto [it, success] =
          commandPools_.emplace(std::make_tuple(queueFamilyIndex, flags),
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

    for (auto& queueFamilyIndex : device.queueFamilyIndices()) {
        createPool(queueFamilyIndex, 0);

        createPool(queueFamilyIndex,
                   VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        createPool(queueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }
}

CommandPool::~CommandPool() noexcept
{
    for (auto& [k, v] : commandPools_) {
        (void)k;

        auto& [pool, buffers] = v;

        if (buffers.empty())
            continue;

        vkFreeCommandBuffers(vkDevice_, static_cast<VkCommandPool>(pool), buffers.size(), buffers.data());

        buffers.clear();
    }

    commandPools_.clear();
}
}