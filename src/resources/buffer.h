//
// Created by anton on 5/28/22.
//

#ifndef CYCLONITE_BUFFER_H
#define CYCLONITE_BUFFER_H

#include "resource.h"
#include <array>

namespace cyclonite::resources {
class Buffer : public Resource
{
public:
    template<typename DataType>
    class View
    {
    public:
        class Iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using value_type = DataType;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            Iterator(View<DataType> const& view, difference_type index);

            explicit operator bool() const { return index_ < view_.count_; }

            auto operator==(Iterator const& rhs) const -> bool { return &view_ == &rhs.view_ && index_ == rhs.index_; }

            auto operator!=(Iterator const& rhs) const -> bool { return &view_ != &rhs.view_ || index_ != rhs.index_; }

            auto operator+=(difference_type diff) -> Iterator&;

            auto operator-=(difference_type diff) -> Iterator&;

            auto operator++() -> Iterator&;

            auto operator--() -> Iterator&;

            auto operator++(int) -> Iterator;

            auto operator--(int) -> Iterator;

            auto operator+(difference_type diff) -> Iterator;

            auto operator-(difference_type diff) -> Iterator;

            auto operator-(Iterator const& rhs) -> difference_type;

            auto operator*() -> DataType&;

            auto operator*() const -> DataType const&;

            auto operator->() const -> DataType*;

            auto ptr() const -> DataType*;

        private:
            difference_type index_;
            View<DataType> const& view_;
        };

    public:
        View(void* dataPtr, size_t offset, size_t count, size_t stride = sizeof(DataType));

        [[nodiscard]] auto count() const -> size_t { return count_; }

        [[nodiscard]] auto stride() const -> size_t { return stride_; }

    private:
        void* ptr_;
        size_t stride_;
        size_t count_;
    };

public:
    explicit Buffer(size_t size) noexcept
      : Resource(size)
    {}

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    [[nodiscard]] auto size() const -> size_t { return dynamicDataSize(); }

    auto data() -> std::byte* { return dynamicData(); }

    template<typename DataType>
    auto view(size_t offset, size_t count, size_t stride = sizeof(DataType)) -> View<DataType>;

public:
    static ResourceTag tag;
    static auto type_tag_const() -> ResourceTag const& { return tag; }
    static auto type_tag() -> ResourceTag& { return tag; }
};

template<typename DataType>
auto Buffer::view(size_t offset, size_t count, size_t stride /* = sizeof(DataType)*/) -> View<DataType>
{
    return View<DataType>{ data(), offset, count, stride };
}

template<typename DataType>
Buffer::View<DataType>::View(void* dataPtr, size_t offset, size_t count, size_t stride)
  : ptr_{ reinterpret_cast<std::byte*>(dataPtr) + offset }
  , stride_{ stride }
  , count_{ count }
{}

template<typename DataType>
Buffer::View<DataType>::Iterator::Iterator(View<DataType> const& view, difference_type index)
  : index_{ index }
  , view_{ view }
{
    assert(index_ <= static_cast<difference_type>(view_.count_));
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator+=(difference_type diff) -> Iterator&
{
    index_ += diff;
    assert(index_ <= static_cast<difference_type>(view_.count_));
    return *this;
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator-=(difference_type diff) -> Iterator&
{
    assert(diff <= index_);
    index_ -= diff;
    return *this;
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator++() -> Iterator&
{
    assert(index_ <= static_cast<difference_type>(view_.count_));
    index_++;
    return *this;
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator--() -> Iterator&
{
    assert(index_ > 0l);
    index_--;
    return *this;
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator++(int) -> Iterator
{
    assert(index_ <= static_cast<difference_type>(view_.count_));
    auto prev = Iterator{ *this, index_ };
    index_++;
    return prev;
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator--(int) -> Iterator
{
    assert(index_ > 0l);
    auto prev = Iterator{ *this, index_ };
    index_--;
    return prev;
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator+(difference_type diff) -> Iterator
{
    return Iterator{ *this, index_ + diff };
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator-(difference_type diff) -> Iterator
{
    assert(index_ >= diff);
    return Iterator{ *this, index_ - diff };
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator-(Iterator const& rhs) -> difference_type
{
    return rhs.index_ - index_;
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator->() const -> DataType*
{
    void* p = reinterpret_cast<std::byte*>(view_.ptr_) + view_.stride_ * index_;
    return reinterpret_cast<DataType*>(p);
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator*() const -> DataType const&
{
    return *(this->operator->());
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::operator*() -> DataType&
{
    return *(this->operator->());
}

template<typename DataType>
auto Buffer::View<DataType>::Iterator::ptr() const -> DataType*
{
    return this->operator->();
}
}

#endif // CYCLONITE_BUFFER_H
