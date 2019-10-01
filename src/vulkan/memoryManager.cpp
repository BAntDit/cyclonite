//
// Created by bantdit on 9/27/19.
//

#include "memoryManager.h"

namespace cyclonite::vulkan {
MemoryManager::MemoryManager(cyclonite::multithreading::TaskManager const& taskManager,
                             cyclonite::vulkan::Device const& device)
  : taskManager_{ &taskManager }
  , device_{ &device }
  , pages_{}
  , memoryTypes_{}
{
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(device_->physicalDevice(), &physicalDeviceMemoryProperties);

    VkDeviceSize streamingMemoryPageSize = 64 * 1024 * 1024;
    VkDeviceSize stagingMemoryPageSize = 64 * 1024 * 1024;
    VkDeviceSize deviceMemoryPageSize = 256 * 1024 * 1024;

    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
        auto const& memoryType = physicalDeviceMemoryProperties.memoryTypes[i];

        if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0 &&
            (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
            memoryTypes_.emplace(i, MemoryType{ true, streamingMemoryPageSize });
        } else if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0) {
            memoryTypes_.emplace(i, MemoryType{ false, deviceMemoryPageSize });
        } else if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
            memoryTypes_.emplace(i, MemoryType{ true, stagingMemoryPageSize });
        }
    }
}

auto MemoryManager::alloc(uint32_t memoryType, VkDeviceSize size, VkDeviceSize align) -> MemoryPage::AllocatedMemory
{
    assert(memoryTypes_.find(memoryType) != memoryTypes_.end());

    auto future = taskManager_->strand([&, this]() -> MemoryPage::AllocatedMemory {
        auto& pages = pages_[{ memoryType, align }];

        auto mod = size % align;
        auto alignedSize = mod == 0 ? size : size + align - mod;

        if (auto it =
              std::find_if(pages.begin(),
                           pages.end(),
                           [alignedSize](auto const& p) -> bool { return p.maxAvailableRange() >= alignedSize; });
            it != pages.end()) {

            return (*it).alloc(alignedSize);
        } else {
            auto [hostVisibility, pageSize] = memoryTypes_[memoryType];

            pages.emplace_back(*taskManager_, *device_, std::max(pageSize, alignedSize), memoryType, hostVisibility);
        }
    });

    return future.get();
}
}
