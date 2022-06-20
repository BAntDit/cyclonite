//
// Created by bantdit on 10/19/19.
//

#include "staging.h"
#include "vulkan/device.h"

namespace cyclonite::resources {
Resource::ResourceTag Staging::tag{};

Staging::Staging(vulkan::Device& device, VkBufferUsageFlags usageFlags, VkDeviceSize size)
  : Resource{}
  , buffers::Arena<Staging>{ static_cast<size_t>(size) }
  , buffer_{ device,
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
             usageFlags,
             size,
             std::array<uint32_t, 1>{ device.hostTransferQueueFamilyIndex() } }
{}

auto Staging::ptr() const -> void const*
{
    return buffer_.allocatedMemory().ptr();
}

auto Staging::ptr() -> void*
{
    return buffer_.allocatedMemory().ptr();
}
}
