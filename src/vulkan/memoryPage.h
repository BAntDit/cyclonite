//
// Created by bantdit on 8/31/19.
//

#ifndef CYCLONITE_MEMORYPAGE_H
#define CYCLONITE_MEMORYPAGE_H

#include "handle.h"
#include <dque>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
class MemoryPage
{
public:
private:
    using MemoryBlockRange = std::pair<size_t, size_t>;

    Handle<VkDeviceMemory> deviceMemory_;

    VkDeviceSize pageSize_;
};
}

#endif // CYCLONITE_MEMORYPAGE_H
