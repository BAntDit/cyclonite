//
// Created by bantdit on 9/27/19.
//

#ifndef CYCLONITE_MEMORYMANAGER_H
#define CYCLONITE_MEMORYMANAGER_H

#include "../multithreading/taskManager.h"
#include "device.h"
#include "memoryPage.h"
#include <boost/functional/hash.hpp>
#include <unordered_map>

namespace cyclonite::vulkan {
class MemoryManager
{
public:
    MemoryManager(multithreading::TaskManager const& taskManager, Device const& device);

    auto alloc(uint32_t memoryType, VkDeviceSize size, VkDeviceSize align) -> MemoryPage::AllocatedMemory;

private:
    struct MemoryType
    {
        MemoryType(bool _hostVisible, VkDeviceSize _pageSize)
          : hostVisible{ _hostVisible }
          , pageSize{ _pageSize }
        {}

        bool hostVisible;
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
