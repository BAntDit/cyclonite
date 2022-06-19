//
// Created by bantdit on 10/19/19.
//

#include "staging.h"
#include "vulkan/device.h"

namespace cyclonite::resources {
Resource::ResourceTag Staging::tag{};

Staging::Staging(ResourceManager* resourceManager,
                 vulkan::Device& device,
                 VkBufferUsageFlags usageFlags,
                 VkDeviceSize size)
  : Resource{ resourceManager }
  , buffers::Arena<Staging, allocator_t>{ static_cast<size_t>(size),
                                          allocator_t{ resourceManager, &Staging::type_tag() } }
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

void Staging::handleDynamicBufferRealloc() /* override*/
{
    auto queue = std::move(freeRanges_);
    freeRanges_ =
      std::deque<std::pair<size_t, size_t>, allocator_t>{ allocator_t{ resourceManager(), &Staging::type_tag() } };

    for (auto& i : queue) {
        freeRanges_.push_back(i);
    }
}
}
