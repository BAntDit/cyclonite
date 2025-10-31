//
// Created by anton on 10/31/25.
//

#include "vkBuffer.h"
#include "gfx/device.h"
#include "internal/utils.h"
#include "vkException.h"

namespace cyclonite::gfx::vulkan {
Buffer::Buffer(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               core::ResourceSharedRef deviceRef,
               size_t size,
               GpuMemoryAllocationFlagBits allocationFlags,
               BufferUsageFlagBits usageFlags)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , deviceRef_{ std::move(deviceRef) }
  , allocation_{ VK_NULL_HANDLE }
  , vkBuffer_{ VK_NULL_HANDLE }
  , usageFlags_{ usageFlags }
{
    assert(deviceRef_.valid());
    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();
    auto allocator = device.allocator();

    auto bufferCreateInfo = VkBufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = static_cast<VkDeviceSize>(size);
    bufferCreateInfo.usage = usageFlags_.cast_to<VkBufferUsageFlags>();
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto allocationCreateInfo = VmaAllocationCreateInfo{};
    allocationCreateInfo.flags = internal::getVmaAllocationFlags(allocationFlags);
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    if (auto vkResult =
          vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &vkBuffer_, &allocation_, nullptr);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateBuffer" };
    }
}

Buffer::~Buffer()
{
    if (vkBuffer_ != VK_NULL_HANDLE) {
        auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();
        auto allocator = device.allocator();
        vmaDestroyBuffer(allocator, vkBuffer_, allocation_);
    }

    vkBuffer_ = VK_NULL_HANDLE;
    allocation_ = VK_NULL_HANDLE;
}
}