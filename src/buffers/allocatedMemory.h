//
// Created by anton on 3/19/23.
//

#ifndef CYCLONITE_ALLOCATEDMEMORY_H
#define CYCLONITE_ALLOCATEDMEMORY_H

#include "bufferView.h"
#include <cstddef>

namespace cyclonite::buffers {
template<typename T>
concept MemoryPageConcept = requires(T&& t) {
                                {
                                    t.ptr()
                                    } -> std::same_as<void*>;
                            };

template<MemoryPageConcept MemoryPage>
class Arena;

template<MemoryPageConcept MemoryPage>
class Ring;

template<MemoryPageConcept MemoryPage>
class AllocatedMemory
{
public:
    friend class Arena<MemoryPage>;
    friend class Ring<MemoryPage>;

    AllocatedMemory();

    AllocatedMemory(MemoryPage& memoryPage, size_t offset, size_t size);

    AllocatedMemory(AllocatedMemory const&) = delete;

    AllocatedMemory(AllocatedMemory&& allocatedMemory) noexcept;

    ~AllocatedMemory();

    auto operator=(AllocatedMemory const&) -> AllocatedMemory& = delete;

    auto operator=(AllocatedMemory&& rhs) noexcept -> AllocatedMemory&;

    explicit operator std::byte*() { return reinterpret_cast<std::byte*>(ptr_); }

    [[nodiscard]] auto ptr() const -> void const* { return ptr_; }

    [[nodiscard]] auto ptr() -> void* { return ptr_; }

    [[nodiscard]] auto size() const -> size_t { return size_; }

    [[nodiscard]] auto offset() const -> size_t { return offset_; }

    [[nodiscard]] auto memoryPage() const -> MemoryPage const& { return *memoryPage_; }

    template<typename DataType>
    auto view(size_t count, size_t offset = 0, size_t stride = sizeof(DataType)) -> buffers::BufferView<DataType>;

private:
    MemoryPage* memoryPage_;
    void* ptr_;
    size_t offset_;
    size_t size_;
};

template<MemoryPageConcept MemoryPage>
AllocatedMemory<MemoryPage>::AllocatedMemory()
  : memoryPage_{ nullptr }
  , ptr_{ nullptr }
  , offset_{ 0 }
  , size_{ 0 }
{
}

template<MemoryPageConcept MemoryPage>
AllocatedMemory<MemoryPage>::AllocatedMemory(MemoryPage& memoryPage, size_t offset, size_t size)
  : memoryPage_{ &memoryPage }
  , ptr_{ memoryPage_->ptr() == nullptr ? nullptr : reinterpret_cast<std::byte*>(memoryPage_->ptr()) + offset }
  , offset_{ offset }
  , size_{ size }
{
}

template<MemoryPageConcept MemoryPage>
AllocatedMemory<MemoryPage>::AllocatedMemory(AllocatedMemory&& allocatedMemory) noexcept
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

template<MemoryPageConcept MemoryPage>
AllocatedMemory<MemoryPage>::~AllocatedMemory()
{
    if (memoryPage_ != nullptr) {
        memoryPage_->free(*this);
    }
}

template<MemoryPageConcept MemoryPage>
auto AllocatedMemory<MemoryPage>::operator=(AllocatedMemory&& rhs) noexcept -> AllocatedMemory&
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

template<MemoryPageConcept MemoryPage>
template<typename DataType>
auto AllocatedMemory<MemoryPage>::view(size_t count, size_t offset /* = 0*/, size_t stride /* = sizeof(DataType)*/)
  -> buffers::BufferView<DataType>
{
    return buffers::BufferView<DataType>{ ptr(), offset, count, stride };
}
}

#endif // CYCLONITE_ALLOCATEDMEMORY_H
