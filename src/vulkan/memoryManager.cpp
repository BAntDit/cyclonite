//
// Created by bantdit on 9/27/19.
//

#include "memoryManager.h"
#include "device.h"

namespace cyclonite::vulkan {
MemoryManager::MemoryManager(multithreading::TaskManager& taskManager, vulkan::Device const& device)
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
            memoryTypes_.emplace(i, MemoryType{ memoryType.propertyFlags, streamingMemoryPageSize });
        } else if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0) {
            memoryTypes_.emplace(i, MemoryType{ memoryType.propertyFlags, deviceMemoryPageSize });
        } else if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
            memoryTypes_.emplace(i, MemoryType{ memoryType.propertyFlags, stagingMemoryPageSize });
        }
    }
}

auto MemoryManager::alloc(VkMemoryRequirements const& memoryRequirements, VkMemoryPropertyFlags memoryPropertyFlags)
  -> MemoryPage::AllocatedMemory
{
    auto align = memoryRequirements.alignment;
    auto size = memoryRequirements.size + (align - memoryRequirements.size % align);

    assert(size % align == 0);

    uint32_t memoryTypeIndex = std::numeric_limits<uint32_t>::max();

    for (auto&& [i, memoryType] : memoryTypes_) {
        auto bit = (static_cast<uint32_t>(1) << i);

        if ((memoryRequirements.memoryTypeBits & bit) != 0) {
            if ((memoryType.propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
                memoryTypeIndex = i;
                break;
            }
        }
    }

    if (memoryTypeIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("failed to get correct memory type");
    }

    auto allocationTask = [&, this]() -> MemoryPage::AllocatedMemory {
        auto& pages = pages_[{ memoryTypeIndex, align }];

        if (auto it = std::find_if(
              pages.begin(), pages.end(), [size](auto const& p) -> bool { return p.maxAvailableRange() >= size; });
            it != pages.end()) {

            return (*it).alloc(size);
        } else {
            auto const& type = memoryTypes_[memoryTypeIndex];

            return pages
              .emplace_back(*taskManager_,
                            *device_,
                            std::max(type.pageSize, size),
                            memoryTypeIndex,
                            type.isHostVisible(),
                            MemoryPage::private_tag{})
              .alloc(size);
        }
    };

    auto allocatedMemory = MemoryPage::AllocatedMemory{};

    if (multithreading::Render::isInRenderThread()) {
        allocatedMemory = allocationTask();
    } else {
        auto future = taskManager_->submitRenderTask(allocationTask);
        allocatedMemory = future.get();
    }

    return allocatedMemory;
}
}
