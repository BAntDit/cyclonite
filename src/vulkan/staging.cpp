//
// Created by bantdit on 10/19/19.
//

#include "staging.h"
#include "device.h"

namespace cyclonite::vulkan {
Staging::Staging(Device& device, VkBufferUsageFlags usageFlags, VkDeviceSize size)
  : Arena<Staging>(static_cast<size_t>(size))
  , buffer_{ device,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
             usageFlags,
             size,
             std::array<uint32_t, 1>{ device.hostTransferQueueFamilyIndex() } }
{}

auto Staging::ptr() const -> void*
{
    return buffer_.allocatedMemory().ptr();
}
}
