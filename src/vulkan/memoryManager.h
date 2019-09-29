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
    using page_key_t = std::pair<uint32_t, VkDeviceSize>;

    multithreading::TaskManager const* taskManager_;
    std::unordered_map<page_key_t, std::vector<MemoryPage>, boost::hash<page_key_t>> pages_;
};
}

#endif // CYCLONITE_MEMORYMANAGER_H
