//
// Created by bantdit on 9/26/19.
//

#include "memoryPage.h"
#include "memoryManager.h"

namespace cyclonite::vulkan {
// memory manager always creates new memory page in strand
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

auto MemoryPage::maxAvailableRange() const -> size_t
{
    return freeRanges_.empty() ? 0 : static_cast<size_t>(freeRanges_.back().second);
}

// calls in strand always
auto MemoryPage::alloc(size_t size) -> Arena<MemoryPage>::AllocatedMemory
{
    auto sz = static_cast<VkDeviceSize>(size);

    auto it =
      std::lower_bound(freeRanges_.begin(), freeRanges_.end(), sz, [](auto const& lhs, VkDeviceSize rhs) -> bool {
          return (*lhs).second < rhs;
      });

    assert(it != freeRanges_.end());

    auto [rangeOffset, rangeSize] = (*it);

    freeRanges_.erase(it);

    if (rangeSize > sz) {
        auto newOffset = rangeOffset + sz;
        auto newSize = rangeSize - sz;

        if (auto newIt =
              std::upper_bound(freeRanges_.begin(),
                               freeRanges_.end(),
                               newSize,
                               [](VkDeviceSize lhs, auto const& rhs) -> bool { return lhs < (*rhs).second; });
            newIt != freeRanges_.end()) {
            freeRanges_.emplace(newIt, newOffset, newSize);
        } else {
            freeRanges_.emplace_back(newOffset, newSize);
        }
    }

    return AllocatedMemory(*this, rangeOffset, rangeSize);
}

void MemoryPage::free(Arena<MemoryPage>::AllocatedMemory const& allocatedMemory)
{
    auto offset = static_cast<VkDeviceSize>(allocatedMemory.offset());
    auto size = static_cast<VkDeviceSize>(allocatedMemory.size());

    auto prevIt = std::find_if(
      freeRanges_.begin(), freeRanges_.end(), [=](auto const& p) -> bool { return p.first + p.second == offset; });

    auto nextIt = std::find_if(
      freeRanges_.begin(), freeRanges_.end(), [=](auto const& p) -> bool { return offset + size = p.first; });

    auto prevIndex = std::distance(freeRanges_.begin(), prevIt);
    auto nextIndex = std::distance(freeRanges_.begin(), nextIt);

    auto newOffset = offset;
    auto newSize = size;

    if (prevIt != freeRanges_.end() && nextIt != freeRanges_.end()) {
        assert(prevIndex != nextIndex);

        newOffset = (*prevIt).first;
        newSize = (*prevIt).second + size + (*nextIt).second;

        if (nextIndex > prevIndex) {
            freeRanges_.erase(std::next(freeRanges_.begin(), nextIndex));
            freeRanges_.erase(std::next(freeRanges_.begin(), prevIndex));
        } else {
            freeRanges_.erase(std::next(freeRanges_.begin(), prevIndex));
            freeRanges_.erase(std::next(freeRanges_.begin(), nextIndex));
        }
    } else if (prevIt != freeRanges_.end()) {
        newOffset = (*prevIt).first;
        newSize = (*prevIt).second + size;

        freeRanges_.erase(prevIt);
    } else if (nextIt != freeRanges_.end()) {
        newSize = size + (*nextIt).second;

        freeRanges_.erase(nextIt);
    }

    if (auto newIt = std::upper_bound(freeRanges_.begin(),
                                      freeRanges_.end(),
                                      newSize,
                                      [](VkDeviceSize lhs, auto const& rhs) -> bool { return lhs < (*rhs).second; });
        newIt != freeRanges_.end()) {
        freeRanges_.emplace(newIt, newOffset, newSize);
    } else {
        freeRanges_.emplace_back(newOffset, newSize);
    }
}
}
