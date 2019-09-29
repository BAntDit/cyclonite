//
// Created by bantdit on 9/27/19.
//

#include "memoryManager.h"

namespace cyclonite::vulkan {
MemoryManager::MemoryManager(cyclonite::multithreading::TaskManager const& taskManager,
                             cyclonite::vulkan::Device const& device)
  : taskManager_{ &taskManager }
  , pages_{}
{
    VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(device.physicalDevice(), &vkPhysicalDeviceMemoryProperties);
}

auto MemoryManager::alloc(uint32_t memoryType, VkDeviceSize size, VkDeviceSize align) -> MemoryPage::AllocatedMemory
{
    auto future = taskManager_->strand([&, this]() -> MemoryPage::AllocatedMemory {
        auto& pages = pages_[{ memoryType, align }];

        auto mod = size % align;
        auto alignedSize = mod == 0 ? size : size + align - mod;

        if (auto it =
              std::find_if(pages.begin(),
                           pages.end(),
                           [alignedSize](auto const& p) -> bool { return p.maxAvailableRange() >= alignedSize; });
            it != pages.end()) {
            // TODO:: alloc from existed page
        } else {
            // TODO:: create new page and alloc
        }
    });

    return future.get();
}
}
