//
// Created by bantdit on 9/26/19.
//

#include "memoryPage.h"

namespace cyclonite::vulkan {
MemoryPage::MemoryPage(Device const& device, VkDeviceSize pageSize, uint32_t memoryTypeIndex)
  : vkDeviceMemory_{ device.handle(), vkFreeMemory }
  , pageSize_{ pageSize }
  , freeRanges_{ { 0, pageSize } }
{
    VkMemoryAllocateInfo memoryAllocateInfo = {};

    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = pageSize_;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    if (auto result = vkAllocateMemory(device.handle(), &memoryAllocateInfo, nullptr, &vkDeviceMemory_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
            throw OutOfMemory(pageSize_);

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
            throw OutOfMemory(pageSize_);

        if (result == VK_ERROR_TOO_MANY_OBJECTS)
            throw std::runtime_error("could not create memory page, too many allocations at once");

        assert(false);
    }
}
}