//
// Created by bantdit on 8/31/19.
//

#ifndef CYCLONITE_MEMORYPAGE_H
#define CYCLONITE_MEMORYPAGE_H

#include "../error.h"
#include "device.h"
#include "handle.h"
#include <deque>

namespace cyclonite::vulkan {
struct OutOfMemory : Error
{
    using Error::Error;

    explicit OutOfMemory(VkDeviceSize size)
      : Error("device have no enough memory to allocate: " + std::to_string(size) + " bytes")
    {}
};

class MemoryPage
{
public:
    class AllocatedMemory
    {
    public:
        AllocatedMemory(MemoryPage& memoryPage, VkDeviceSize size);

        AllocatedMemory(AllocatedMemory const&) = delete;

        AllocatedMemory(AllocatedMemory&& allocatedMemory) noexcept;

        ~AllocatedMemory();

        auto operator=(AllocatedMemory const&) -> AllocatedMemory& = delete;

        auto operator=(AllocatedMemory&& rhs) noexcept -> AllocatedMemory&;

    private:
        MemoryPage* memoryPage_;
        VkDeviceSize offset;
        VkDeviceSize size;
    };

public:
    MemoryPage(Device const& device, VkDeviceSize pageSize, uint32_t memoryTypeIndex);

    MemoryPage(MemoryPage const&) = delete;

    MemoryPage(MemoryPage&&) = default;

    ~MemoryPage() = default;

    auto operator=(MemoryPage const&) -> MemoryPage& = delete;

    auto operator=(MemoryPage &&) -> MemoryPage& = default;

    [[nodiscard]] auto handle() const -> VkDeviceMemory { return static_cast<VkDeviceMemory>(vkDeviceMemory_); }

    [[nodiscard]] auto size() const -> VkDeviceSize { return pageSize_; }

private:
    Handle<VkDeviceMemory> vkDeviceMemory_;
    VkDeviceSize pageSize_;
    std::deque<std::pair<VkDeviceSize, VkDeviceSize>> freeRanges_;
};
}

#endif // CYCLONITE_MEMORYPAGE_H
