//
// Created by bantdit on 10/5/19.
//

#include "buffer.h"
#include "device.h"
#include "internal/fillBufferCreationInfo.h"
#include "memoryManager.h"

namespace cyclonite::vulkan {
Buffer::Buffer(Device& device,
               VkMemoryPropertyFlags memoryPropertyFlags,
               VkBufferUsageFlags usageFlags,
               VkDeviceSize size,
               owner_queue_family_indices_t ownerQueueFamilyIndices)
  : allocatedMemory_{}
  , vkBuffer_{ device.handle(), vkDestroyBuffer }
{
    VkBufferCreateInfo bufferCreateInfo = {};

    internal::fillBufferCreationInfo(bufferCreateInfo, usageFlags, size, ownerQueueFamilyIndices);

    if (auto result = vkCreateBuffer(device.handle(), &bufferCreateInfo, nullptr, &vkBuffer_); result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
            throw std::runtime_error("not enough RAM to create buffer");

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
            throw std::runtime_error("not enough GPU Memory to create buffer");

        assert(false);
    }

    {
        VkMemoryRequirements memoryRequirements = {};
        vkGetBufferMemoryRequirements(device.handle(), static_cast<VkBuffer>(vkBuffer_), &memoryRequirements);

        allocatedMemory_ = device.memoryManager().alloc(memoryRequirements, memoryPropertyFlags, size);
    }
}
}
