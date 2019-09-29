//
// Created by bantdit on 9/26/19.
//

#include "memoryPage.h"

namespace cyclonite::vulkan {
// memory manager always creates new memory page in strand
// todo:: make constructor private
MemoryPage::MemoryPage(multithreading::TaskManager const& taskManager,
                       Device const& device,
                       VkDeviceSize pageSize,
                       uint32_t memoryTypeIndex,
                       bool hostVisible)
  : taskManager_{ &taskManager }
  , vkDevice_{ device.handle() }
  , hostVisible_{ hostVisible }
  , vkDeviceMemory_{ device.handle(), vkFreeMemory }
  , pageSize_{ pageSize }
  , freeRanges_{ { 0, pageSize } }
  , ptr_{ nullptr }
{
    VkMemoryAllocateInfo memoryAllocateInfo = {};

    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = pageSize_;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;

    if (auto result = vkAllocateMemory(device.handle(), &memoryAllocateInfo, nullptr, &vkDeviceMemory_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
            throw OutOfMemory(pageSize_);

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY)
            throw OutOfMemory(pageSize_);

        if (result == VK_ERROR_TOO_MANY_OBJECTS)
            throw std::runtime_error("could not create memory page, too many allocations at once");

        assert(false);
    }

    assert(!hostVisible || 0 == pageSize % device.capabilities().minMemoryMapAlignment);

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

MemoryPage::MemoryPage(cyclonite::vulkan::MemoryPage&& memoryPage) noexcept
  : taskManager_{ memoryPage.taskManager_ }
  , vkDevice_{ memoryPage.vkDevice_ }
  , hostVisible_{ memoryPage.hostVisible_ }
  , vkDeviceMemory_{ std::move(memoryPage.vkDeviceMemory_) }
  , pageSize_{ memoryPage.pageSize_ }
  , freeRanges_{ std::move(memoryPage.freeRanges_) }
  , ptr_{ memoryPage.ptr_ }
{
    memoryPage.vkDevice_ = VK_NULL_HANDLE;
    memoryPage.hostVisible_ = false;
    memoryPage.pageSize_ = 0;
    memoryPage.ptr_ = nullptr;
}

MemoryPage::~MemoryPage()
{
    auto future = taskManager_->strand([this]() -> void {
        if (vkDevice_ != VK_NULL_HANDLE && ptr_ != nullptr) {
            vkUnmapMemory(vkDevice_, handle());
        }
    });

    future.get();
}

auto MemoryPage::operator=(MemoryPage&& rhs) noexcept -> MemoryPage&
{
    taskManager_ = rhs.taskManager_;
    vkDevice_ = rhs.vkDevice_;
    hostVisible_ = rhs.hostVisible_;
    vkDeviceMemory_ = std::move(rhs.vkDeviceMemory_);
    pageSize_ = rhs.pageSize_;
    freeRanges_ = std::move(rhs.freeRanges_);
    ptr_ = rhs.ptr_;

    rhs.vkDevice_ = VK_NULL_HANDLE;
    rhs.hostVisible_ = false;
    rhs.pageSize_ = 0;
    rhs.ptr_ = nullptr;

    return *this;
}

auto MemoryPage::maxAvailableRange() const -> VkDeviceSize
{
    return freeRanges_.empty() ? 0 : freeRanges_.back().second;
}

MemoryPage::AllocatedMemory::AllocatedMemory(MemoryPage& memoryPage, VkDeviceSize offset, VkDeviceSize size)
  : memoryPage_{ &memoryPage }
  , ptr_{ memoryPage_->ptr() == nullptr ? nullptr : reinterpret_cast<std::byte*>(memoryPage_->ptr()) + offset }
  , offset_{ offset }
  , size_{ size }
{}

MemoryPage::AllocatedMemory::AllocatedMemory(MemoryPage::AllocatedMemory&& allocatedMemory) noexcept
  : memoryPage_{ allocatedMemory.memoryPage_ }
  , ptr_{ allocatedMemory.ptr_ }
  , offset_{ allocatedMemory.offset_ }
  , size_{ allocatedMemory.size_ }
{
    allocatedMemory.memoryPage_ = nullptr;
    allocatedMemory.ptr_ = nullptr;
    allocatedMemory.offset_ = 0;
    allocatedMemory.size_ = 0;
}

MemoryPage::AllocatedMemory::~AllocatedMemory()
{
    if (memoryPage_ != nullptr) {
        memoryPage_->_free(*this);
    }
}

auto MemoryPage::AllocatedMemory::operator=(MemoryPage::AllocatedMemory&& rhs) noexcept -> MemoryPage::AllocatedMemory&
{
    memoryPage_ = rhs.memoryPage_;
    ptr_ = rhs.ptr_;
    offset_ = rhs.offset_;
    size_ = rhs.size_;

    rhs.memoryPage_ = nullptr;
    rhs.ptr_ = nullptr;
    rhs.offset_ = 0;
    rhs.size_ = 0;

    return *this;
}
}