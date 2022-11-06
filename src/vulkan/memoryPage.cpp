//
// Created by bantdit on 9/26/19.
//

#include "memoryPage.h"
#include "device.h"
#include "memoryManager.h"

namespace cyclonite::vulkan {
// memory manager always creates new memory page in strand
MemoryPage::MemoryPage(multithreading::TaskManager& taskManager,
                       Device const& device,
                       VkDeviceSize pageSize,
                       uint32_t memoryTypeIndex,
                       bool hostVisible,
                       private_tag)
  : Arena<MemoryPage>{ static_cast<size_t>(pageSize) }
  , taskManager_{ &taskManager }
  , vkDevice_{ device.handle() }
  , hostVisible_{ hostVisible }
  , vkDeviceMemory_{ device.handle(), vkFreeMemory }
  , ptr_{ nullptr }
{
    VkMemoryAllocateInfo memoryAllocateInfo = {};

    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = static_cast<VkDeviceSize>(size_);
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    if (auto result = vkAllocateMemory(device.handle(), &memoryAllocateInfo, nullptr, &vkDeviceMemory_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
            throw OutOfMemory(pageSize);

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
            throw OutOfMemory(pageSize);

        if (result == VK_ERROR_TOO_MANY_OBJECTS)
            throw std::runtime_error("could not create memory page, too many allocations at once");

        assert(false);
    }

    if (hostVisible_) {
        assert(0 == pageSize % device.capabilities().minMemoryMapAlignment);

        if (auto result = vkMapMemory(vkDevice_, handle(), 0, VK_WHOLE_SIZE, 0, &ptr_); result != VK_SUCCESS) {
            if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                throw std::runtime_error("there is no enough memory on device to map vulkan::MemoryPage");
            }

            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                throw std::runtime_error("there is no enough system memory to map vulkan::MemoryPage");
            }

            if (result == VK_ERROR_MEMORY_MAP_FAILED) {
                throw std::runtime_error("Mapping of a memory object has failed");
            }

            assert(false);
        }
    }
}

MemoryPage::MemoryPage(MemoryPage&& memoryPage) noexcept
  : Arena{ std::move(memoryPage) }
  , taskManager_{ memoryPage.taskManager_ }
  , vkDevice_{ memoryPage.vkDevice_ }
  , hostVisible_{ memoryPage.hostVisible_ }
  , vkDeviceMemory_{ std::move(memoryPage.vkDeviceMemory_) }
  , ptr_{ memoryPage.ptr_ }
{
    memoryPage.vkDevice_ = VK_NULL_HANDLE;
    memoryPage.hostVisible_ = false;
    memoryPage.ptr_ = nullptr;
}

MemoryPage::~MemoryPage()
{
    auto unmapMemoryTask = [this]() -> void {
        if (hostVisible_ && vkDevice_ != VK_NULL_HANDLE && ptr_ != nullptr) {
            vkUnmapMemory(vkDevice_, handle());
        }
    };

    if (multithreading::Render::isInRenderThread()) {
        unmapMemoryTask();
    } else {
        auto future = taskManager_->submitRenderTask(unmapMemoryTask);
        future.get();
    }
}

auto MemoryPage::operator=(MemoryPage&& rhs) noexcept -> MemoryPage&
{
    taskManager_ = rhs.taskManager_;
    vkDevice_ = rhs.vkDevice_;
    hostVisible_ = rhs.hostVisible_;
    vkDeviceMemory_ = std::move(rhs.vkDeviceMemory_);
    ptr_ = rhs.ptr_;

    rhs.vkDevice_ = VK_NULL_HANDLE;
    rhs.hostVisible_ = false;
    rhs.ptr_ = nullptr;

    Arena<MemoryPage>::operator=(std::move(rhs));

    return *this;
}
}
