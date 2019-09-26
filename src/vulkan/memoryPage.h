//
// Created by bantdit on 8/31/19.
//

#ifndef CYCLONITE_MEMORYPAGE_H
#define CYCLONITE_MEMORYPAGE_H

#include "handle.h"
#include <deque>
#include "device.h"

namespace cyclonite::vulkan {
class MemoryPage
{
public:
    MemoryPage(Device const& device, VkDeviceSize pageSize);

private:
    Handle<VkDeviceMemory> deviceMemory_;
    VkDeviceSize pageSize_;
    std::deque<std::pair<VkDeviceSize , VkDeviceSize>> freeRanges_;
};
}

#endif // CYCLONITE_MEMORYPAGE_H
