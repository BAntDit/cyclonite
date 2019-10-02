//
// Created by bantdit on 10/2/19.
//

#ifndef CYCLONITE_ARENA_H
#define CYCLONITE_ARENA_H

#include <cstddef>

namespace cyclonite {
template<typename MemoryPage>
class Arena
{
public:
    class AllocatedMemory
    {
    public:
        AllocatedMemory(MemoryPage& memoryPage, size_t offset, size_t size);

        AllocatedMemory(AllocatedMemory const&) = delete;

        AllocatedMemory(AllocatedMemory&& allocatedMemory) noexcept;

        ~AllocatedMemory();

        auto operator=(AllocatedMemory const&) -> AllocatedMemory& = delete;

        auto operator=(AllocatedMemory&& rhs) noexcept -> AllocatedMemory&;

        explicit operator std::byte*() { return reinterpret_cast<std::byte*>(ptr_); }

        [[nodiscard]] auto ptr() const -> void* { return ptr_; }

        [[nodiscard]] auto offset() const -> size_t { return offset_; }

        [[nodiscard]] auto size() const -> size_t { return size_; }

    private:
        MemoryPage* memoryPage_;
        void* ptr_;
        size_t offset_;
        size_t size_;
    };

public:
    [[nodiscard]] auto size() const -> size_t;

    [[nodiscard]] auto ptr() const -> void*;

    [[nodiscard]] auto maxAvailableRange() const -> size_t;

    [[nodiscard]] auto alloc(size_t size) -> AllocatedMemory;

    void free(AllocatedMemory const& allocatedMemory);
};

template<typename MemoryPage>
auto Arena<MemoryPage>::size() const -> size_t
{
    return static_cast<MemoryPage const*>(this)->size();
}

template<typename MemoryPage>
auto Arena<MemoryPage>::ptr() const -> void*
{
    return static_cast<MemoryPage const*>(this)->ptr();
}

template<typename MemoryPage>
auto Arena<MemoryPage>::maxAvailableRange() const -> size_t
{
    return static_cast<MemoryPage const*>(this)->maxAvailableRange();
}

template<typename MemoryPage>
auto Arena<MemoryPage>::alloc(size_t size) -> Arena<MemoryPage>::AllocatedMemory
{
    return static_cast<MemoryPage*>(this)->alloc(size);
}

template<typename MemoryPage>
void Arena<MemoryPage>::free(Arena<MemoryPage>::AllocatedMemory const& allocatedMemory)
{
    static_cast<MemoryPage*>(this)->free(allocatedMemory);
}

template<typename MemoryPage>
Arena<MemoryPage>::AllocatedMemory::AllocatedMemory(MemoryPage& memoryPage, size_t offset, size_t size)
  : memoryPage_{ &memoryPage }
  , ptr_{ memoryPage_->ptr() == nullptr ? nullptr : reinterpret_cast<std::byte*>(memoryPage_->ptr()) + offset }
  , offset_{ offset }
  , size_{ size }
{}

template<typename MemoryPage>
Arena<MemoryPage>::AllocatedMemory::AllocatedMemory(Arena<MemoryPage>::AllocatedMemory&& allocatedMemory) noexcept
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

template<typename MemoryPage>
Arena<MemoryPage>::AllocatedMemory::~AllocatedMemory()
{
    if (memoryPage_ != nullptr) {
        memoryPage_->free(*this);
    }
}

template<typename MemoryPage>
auto Arena<MemoryPage>::AllocatedMemory::operator=(Arena<MemoryPage>::AllocatedMemory&& rhs) noexcept
  -> Arena<MemoryPage>::AllocatedMemory&
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

#endif // CYCLONITE_ARENA_H
