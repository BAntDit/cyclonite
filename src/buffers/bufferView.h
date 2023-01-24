//
// Created by anton on 5/30/22.
//

#ifndef CYCLONITE_BUFFERVIEW_H
#define CYCLONITE_BUFFERVIEW_H

#include <cassert>
#include <cstddef>
#include <iterator>

namespace cyclonite::buffers {
template<typename DataType>
class BufferView
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

        Iterator(BufferView<DataType> const& view, difference_type index);

        explicit operator bool() const { return index_ < view_->count_; }

        auto operator==(Iterator const& rhs) const -> bool { return view_ == rhs.view_ && index_ == rhs.index_; }

        auto operator!=(Iterator const& rhs) const -> bool { return view_ != rhs.view_ || index_ != rhs.index_; }

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

        auto operator[](int index) const -> reference;

    private:
        difference_type index_;
        BufferView<DataType> const* view_;
    };

public:
    BufferView(void* dataPtr, size_t offset, size_t count, size_t stride = sizeof(DataType));

    auto begin() const -> Iterator { return Iterator{ *this, 0l }; }

    auto end() const -> Iterator { return Iterator{ *this, static_cast<typename Iterator::difference_type>(count_) }; }

    [[nodiscard]] auto count() const -> size_t { return count_; }

    [[nodiscard]] auto stride() const -> size_t { return stride_; }

private:
    void* ptr_;
    size_t stride_;
    size_t count_;
};

template<typename DataType>
BufferView<DataType>::BufferView(void* dataPtr, size_t offset, size_t count, size_t stride)
  : ptr_{ reinterpret_cast<std::byte*>(dataPtr) + offset }
  , stride_{ stride }
  , count_{ count }
{}

template<typename DataType>
BufferView<DataType>::Iterator::Iterator(BufferView<DataType> const& view, difference_type index)
  : index_{ index }
  , view_{ &view }
{
    assert(index_ <= static_cast<difference_type>(view_->count_));
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator+=(difference_type diff) -> Iterator&
{
    index_ += diff;
    assert(index_ <= static_cast<difference_type>(view_->count_));
    return *this;
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator-=(difference_type diff) -> Iterator&
{
    assert(diff <= index_);
    index_ -= diff;
    return *this;
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator++() -> Iterator&
{
    assert(index_ <= static_cast<difference_type>(view_->count_));
    index_++;
    return *this;
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator--() -> Iterator&
{
    assert(index_ > 0l);
    index_--;
    return *this;
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator++(int) -> Iterator
{
    assert(index_ <= static_cast<difference_type>(view_->count_));
    auto prev = Iterator{ *view_, index_ };
    index_++;
    return prev;
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator--(int) -> Iterator
{
    assert(index_ > 0l);
    auto prev = Iterator{ *view_, index_ };
    index_--;
    return prev;
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator+(difference_type diff) -> Iterator
{
    return Iterator{ *view_, index_ + diff };
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator-(difference_type diff) -> Iterator
{
    assert(index_ >= diff);
    return Iterator{ *view_, index_ - diff };
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator-(Iterator const& rhs) -> difference_type
{
    return index_ - rhs.index_;
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator->() const -> DataType*
{
    void* p = reinterpret_cast<std::byte*>(view_->ptr_) + view_->stride_ * index_;
    return reinterpret_cast<DataType*>(p);
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator*() const -> DataType const&
{
    return *(this->operator->());
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator*() -> DataType&
{
    return *(this->operator->());
}

template<typename DataType>
auto BufferView<DataType>::Iterator::ptr() const -> DataType*
{
    return this->operator->();
}

template<typename DataType>
auto BufferView<DataType>::Iterator::operator[](int index) const -> reference
{
    void* p = reinterpret_cast<std::byte*>(view_->ptr_) + view_->stride_ * index;
    return *(reinterpret_cast<DataType*>(p));
}
}

#endif // CYCLONITE_BUFFERVIEW_H
