//
// Created by bantdit on 1/15/23.
//

#include "descriptorSetManager.h"
#include "descriptorType.h"
#include <easy-mp/enum.h>
#include "device.h"
#include "multithreading/taskManager.h"

namespace cyclonite::vulkan {
using namespace easy_mp;

DescriptorSetManager::DescriptorSetManager(Device& device, multithreading::TaskManager& taskManager) :
  device_{ &device }
  , taskManager_{ &taskManager }
{}

void DescriptorSetManager::addPool(DescriptorPoolKey poolKey, uint32_t capacity)
{
    auto it = pools_.find(static_cast<packed_descriptor_set_key_t>(poolKey));
    if (it == pools_.end()) {
        auto poolCreateInfo = VkDescriptorPoolCreateInfo{};

        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolCreateInfo.maxSets = capacity;

        auto poolSizeCount = uint32_t{ 0 };
        auto poolSizes = std::array<VkDescriptorPoolSize, 16>{};

        auto future = taskManager_->submitRenderTask([poolKey, capacity]()-> void {
            auto poolSizeCount = uint32_t{ 0 };
            auto poolSizes = std::array<VkDescriptorPoolSize, 16>{};



            auto poolCreateInfo = VkDescriptorPoolCreateInfo{};

            poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            poolCreateInfo.maxSets = capacity;
            poolCreateInfo.poolSizeCount = poolSizeCount;
            poolCreateInfo.pPoolSizes = poolSizes.data();

            // TODO:: create...
        });
        future.get();
    }
}
}

