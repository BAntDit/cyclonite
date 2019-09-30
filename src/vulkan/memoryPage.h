//
// Created by bantdit on 8/31/19.
//

#ifndef CYCLONITE_MEMORYPAGE_H
#define CYCLONITE_MEMORYPAGE_H

#include "../error.h"
#include "../multithreading/taskManager.h"
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
        AllocatedMemory(MemoryPage& memoryPage, VkDeviceSize offset, VkDeviceSize size);

        AllocatedMemory(AllocatedMemory const&) = delete;

        AllocatedMemory(AllocatedMemory&& allocatedMemory) noexcept;

        ~AllocatedMemory();

        auto operator=(AllocatedMemory const&) -> AllocatedMemory& = delete;

        auto operator=(AllocatedMemory&& rhs) noexcept -> AllocatedMemory&;

        explicit operator std::byte*() { return reinterpret_cast<std::byte*>(ptr_); }

        [[nodiscard]] auto ptr() const -> void* { return ptr_; }

        [[nodiscard]] auto offset() const -> VkDeviceSize { return offset_; }

        [[nodiscard]] auto size() const -> VkDeviceSize { return size_; }

    private:
        MemoryPage* memoryPage_;
        void* ptr_;
        VkDeviceSize offset_;
        VkDeviceSize size_;
    };

public:
    MemoryPage(multithreading::TaskManager const& taskManager,
               Device const& device,
               VkDeviceSize pageSize,
               uint32_t memoryTypeIndex,
               bool hostVisible);

    MemoryPage(MemoryPage const&) = delete;

    MemoryPage(MemoryPage&& memoryPage) noexcept;

    ~MemoryPage();

    auto operator=(MemoryPage const&) -> MemoryPage& = delete;

    auto operator=(MemoryPage&& rhs) noexcept -> MemoryPage&;

    [[nodiscard]] auto handle() const -> VkDeviceMemory { return static_cast<VkDeviceMemory>(vkDeviceMemory_); }

    [[nodiscard]] auto size() const -> VkDeviceSize { return pageSize_; }

    [[nodiscard]] auto maxAvailableRange() const -> VkDeviceSize;

    [[nodiscard]] auto ptr() const -> void* { return ptr_; }

    [[nodiscard]] auto alloc(VkDeviceSize size) -> AllocatedMemory;

private:
    void _free(AllocatedMemory const& allocatedMemory);

private:
    multithreading::TaskManager const* taskManager_;
    VkDevice vkDevice_;
    bool hostVisible_;
    Handle<VkDeviceMemory> vkDeviceMemory_;
    VkDeviceSize pageSize_;
    std::deque<std::pair<VkDeviceSize, VkDeviceSize>> freeRanges_;
    void* ptr_;
};
}

#endif // CYCLONITE_MEMORYPAGE_H
