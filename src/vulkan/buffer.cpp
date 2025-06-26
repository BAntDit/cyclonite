//
// Created by bantdit on 10/5/19.
//

#include "buffer.h"
#include "gfx/device.h"
#include "internal/fillBufferCreationInfo.h"
#include "memoryManager.h"

namespace cyclonite::vulkan {
Buffer::Buffer(Device& device,
               VkMemoryPropertyFlags memoryPropertyFlags,
               VkBufferUsageFlags usageFlags,
               VkDeviceSize size,
               owner_queue_family_indices_t ownerQueueFamilyIndices)
  : allocatedMemory_{}
  , vkBuffer_{ VK_NULL_HANDLE /*device.handle()*/, vkDestroyBuffer }
{
    VkBufferCreateInfo bufferCreateInfo = {};

    internal::fillBufferCreationInfo(bufferCreateInfo, usageFlags, size, ownerQueueFamilyIndices);

    if (auto result = vkCreateBuffer(VK_NULL_HANDLE /*device.handle()*/, &bufferCreateInfo, nullptr, &vkBuffer_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
            throw std::runtime_error("not enough RAM to create buffer");

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
            throw std::runtime_error("not enough GPU Memory to create buffer");

        assert(false);
    }

    {
        VkMemoryRequirements memoryRequirements = {};
        vkGetBufferMemoryRequirements(VK_NULL_HANDLE/*device.handle()*/, static_cast<VkBuffer>(vkBuffer_), &memoryRequirements);

        // allocatedMemory_ = device.memoryManager().alloc(memoryRequirements, memoryPropertyFlags);
    }

    if (auto result = vkBindBufferMemory(VK_NULL_HANDLE, // device.handle(),
                                         static_cast<VkBuffer>(vkBuffer_),
                                         allocatedMemory_.memoryPage().handle(),
                                         allocatedMemory_.offset());
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
            throw std::runtime_error("not enough RAM to bind memory to buffer");

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
            throw std::runtime_error("not enough GPU Memory to bind memory to buffer");

        assert(false);
    }
}
}
