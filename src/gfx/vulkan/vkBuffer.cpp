//
// Created by anton on 10/31/25.
//

#include "vkBuffer.h"
#include "gfx/device.h"
#include "internal/utils.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)
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
  , deviceAddress_{}
  , usageFlags_{ usageFlags }
  , allocationFlags_{ allocationFlags }
  , owningQueueFamilyIndex_{ std::numeric_limits<uint32_t>::max() }
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

    if (usageFlags_.test(BufferUsageFlags::SHADER_DEVICE_ADDRESS)) {
        auto deviceAddressInfo = VkBufferDeviceAddressInfo{};
        deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        deviceAddressInfo.buffer = static_cast<VkBuffer>(vkBuffer_);

        deviceAddress_ = vkGetBufferDeviceAddress(device.handle(), &deviceAddressInfo);
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

    deviceRef_ = core::ResourceSharedRef{};
}

auto Buffer::map() -> void*
{
    assert(deviceRef_.valid());
    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();
    auto allocator = device.allocator();

    auto allocationInfo = VmaAllocationInfo{};
    vmaGetAllocationInfo(allocator, allocation_, &allocationInfo);

    if (allocationInfo.pMappedData != nullptr) {
        return allocationInfo.pMappedData;
    }

    auto memoryPropertyFlags = VkMemoryPropertyFlags{};
    vmaGetAllocationMemoryProperties(allocator, allocation_, &memoryPropertyFlags);

    assert((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0);

    auto* pData = std::add_pointer_t<void>{ nullptr };
    if (auto vkResult = vmaMapMemory(allocator, allocation_, &pData); vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vmaMapMemory" };
    }

    return pData;
}

void Buffer::unmap()
{
    assert(deviceRef_.valid());
    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();
    auto allocator = device.allocator();

    auto allocationInfo = VmaAllocationInfo{};
    vmaGetAllocationInfo(allocator, allocation_, &allocationInfo);

    auto memoryPropertyFlags = VkMemoryPropertyFlags{};
    vmaGetAllocationMemoryProperties(allocator, allocation_, &memoryPropertyFlags);

    auto isHostCoherent = memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (allocationFlags_.test(gfx::GpuMemoryAllocationFlags::PERSISTENT_MAPPED_MEMORY)) {
        if (!isHostCoherent) {
            if (auto vkResult = vmaFlushAllocation(allocator, allocation_, 0, VK_WHOLE_SIZE); vkResult != VK_SUCCESS) {
                throw Exception{ vkResult, "vmaFlushAllocation" };
            }
        }
    } else {
        vmaUnmapMemory(allocator, allocation_);
        if (!isHostCoherent) {
            if (auto vkResult = vmaFlushAllocation(allocator, allocation_, 0, VK_WHOLE_SIZE); vkResult != VK_SUCCESS) {
                throw Exception{ vkResult, "vmaFlushAllocation" };
            }
        }
    }
}
}
#endif
