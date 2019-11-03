//
// Created by bantdit on 9/27/19.
//

#ifndef CYCLONITE_MEMORYMANAGER_H
#define CYCLONITE_MEMORYMANAGER_H

#include "../multithreading/taskManager.h"
#include "memoryPage.h"
#include <boost/functional/hash.hpp>
#include <unordered_map>

namespace cyclonite::vulkan {
class MemoryManager
{
public:
    MemoryManager(multithreading::TaskManager const& taskManager, Device const& device);

    [[nodiscard]] auto alloc(VkMemoryRequirements const& memoryRequirements, VkMemoryPropertyFlags memoryPropertyFlags)
      -> Arena<MemoryPage>::AllocatedMemory;

private:
    struct MemoryType
    {
        MemoryType(VkMemoryPropertyFlags flags, VkDeviceSize _pageSize)
          : propertyFlags{ flags }
          , pageSize{ _pageSize } {}

              [[nodiscard]] auto isHostVisible() const -> bool
        {
            return (propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
        }

        [[nodiscard]] auto isLocal() const -> bool
        {
            return (propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0;
        }

        VkMemoryPropertyFlags propertyFlags;
        VkDeviceSize pageSize;
    };

    using page_key_t = std::pair<uint32_t, VkDeviceSize>; // memory type / align

    multithreading::TaskManager const* taskManager_;
    Device const* device_;
    std::unordered_map<page_key_t, std::vector<MemoryPage>, boost::hash<page_key_t>> pages_;
    std::unordered_map<uint32_t, MemoryType> memoryTypes_;
};
}

#endif // CYCLONITE_MEMORYMANAGER_H
