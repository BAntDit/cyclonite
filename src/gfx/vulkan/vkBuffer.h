//
// Created by anton on 10/31/25.
//

#ifndef CYCLONITE_VK_BUFFER_H
#define CYCLONITE_VK_BUFFER_H

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"
#include "vmaUsage.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class Buffer : public core::ResourceBase
{
public:
    Buffer(core::ResourceManagerBase* resourceManager,
           core::ResourceId resourceId,
           core::ResourceSharedRef deviceRef,
           size_t size,
           GpuMemoryAllocationFlagBits allocationFlags,
           BufferUsageFlagBits usageFlags);

    ~Buffer();

    [[nodiscard]] auto handle() const -> VkBuffer { return vkBuffer_; }

    [[nodiscard]] auto deviceAddress() const -> uint64_t { return static_cast<uint64_t>(deviceAddress_); }

    [[nodiscard]] auto usage() const -> BufferUsageFlagBits { return usageFlags_; }

    [[nodiscard]] auto owningQueueFamilyIndex() const -> uint32_t { return owningQueueFamilyIndex_; }

    [[nodiscard]] auto owningQueueFamilyIndex() -> uint32_t& { return owningQueueFamilyIndex_; }

    auto map() -> void*;

    void unmap();

private:
    core::ResourceSharedRef deviceRef_;
    VmaAllocation allocation_;
    VkBuffer vkBuffer_;
    VkDeviceAddress deviceAddress_;
    BufferUsageFlagBits usageFlags_;
    GpuMemoryAllocationFlagBits allocationFlags_;
    uint32_t owningQueueFamilyIndex_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_BUFFER_H
