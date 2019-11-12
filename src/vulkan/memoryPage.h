//
// Created by bantdit on 8/31/19.
//

#ifndef CYCLONITE_MEMORYPAGE_H
#define CYCLONITE_MEMORYPAGE_H

#include "../arena.h"
#include "../error.h"
#include "../multithreading/taskManager.h"
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

class Device;

class MemoryManager;

class MemoryPage : public Arena<MemoryPage>
{
private:
    struct private_tag
    {};

public:
    friend class MemoryManager;

    MemoryPage(multithreading::TaskManager const& taskManager,
               Device const& device,
               VkDeviceSize pageSize,
               uint32_t memoryTypeIndex,
               bool hostVisible,
               private_tag);

    MemoryPage(MemoryPage const&) = delete;

    MemoryPage(MemoryPage&& memoryPage) noexcept;

    ~MemoryPage();

    auto operator=(MemoryPage const&) -> MemoryPage& = delete;

    auto operator=(MemoryPage&& rhs) noexcept -> MemoryPage&;

    [[nodiscard]] auto handle() const -> VkDeviceMemory { return static_cast<VkDeviceMemory>(vkDeviceMemory_); }

    [[nodiscard]] auto ptr() const -> void* { return ptr_; }

private:
    multithreading::TaskManager const* taskManager_;
    VkDevice vkDevice_;
    bool hostVisible_;
    Handle<VkDeviceMemory> vkDeviceMemory_;
    void* ptr_;
};
}

#endif // CYCLONITE_MEMORYPAGE_H
