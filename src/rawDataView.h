//
// Created by bantdit on 9/12/20.
//

#ifndef CYCLONITE_DATAVIEW_H
#define CYCLONITE_DATAVIEW_H

#include <cassert>
#include <cstddef>
#include <iterator>
#include <utility>

namespace cyclonite {
template<typename DataType>
class RawDataView
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

        Iterator(RawDataView<DataType> const& view, difference_type index);

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

        auto operator-> () const -> DataType*;

        auto ptr() const -> DataType*;

    private:
        difference_type index_;
        RawDataView<DataType> const& view_;
    };

    RawDataView(void* dataPtr, size_t offset, size_t count, size_t stride = sizeof(DataType));

    auto begin() const -> Iterator { return Iterator{ *this, 0 }; }

    auto end() const -> Iterator { return Iterator{ *this, count_ }; }

    [[nodiscard]] auto count() const -> size_t { return count_; }

    [[nodiscard]] auto stride() const -> size_t { return stride_; }

private:
    size_t stride_;
    size_t count_;
    void* ptr_;
};

template<typename DataType>
RawDataView<DataType>::RawDataView(void* dataPtr, size_t offset, size_t count, size_t stride)
  : stride_{ stride }
  , count_{ count }
  , ptr_{ reinterpret_cast<std::byte*>(dataPtr) + offset }
{}

template<typename DataType>
RawDataView<DataType>::Iterator::Iterator(RawDataView<DataType> const& view, difference_type index)
  : index_{ index }
  , view_{ view }
{
    assert(index_ < view_.count_);
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator+=(difference_type diff) -> Iterator&
{
    index_ += diff;
    assert(index_ <= view_.count_);
    return *this;
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator-=(difference_type diff) -> Iterator&
{
    assert(diff <= index_);
    index_ -= diff;
    return *this;
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator++() -> Iterator&
{
    assert(index_ < view_.count_);
    index_++;
    return *this;
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator--() -> Iterator&
{
    assert(index_ > 0);
    index_--;
    return *this;
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator++(int) -> Iterator
{
    assert(index_ < view_.count_);
    auto prev = Iterator{ *this, index_ };
    index_++;
    return prev;
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator--(int) -> Iterator
{
    assert(index_ > 0);
    auto prev = Iterator{ *this, index_ };
    index_--;
    return prev;
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator+(difference_type diff) -> Iterator
{
    return Iterator{ *this, index_ + diff };
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator-(difference_type diff) -> Iterator
{
    assert(index_ >= diff);
    return Iterator{ *this, index_ - diff };
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator-(Iterator const& rhs) -> difference_type
{
    return rhs.index_ - index_;
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator-> () const -> DataType*
{
    void* p = reinterpret_cast<std::byte*>(view_.ptr_) + view_.stride_ * index_;
    return reinterpret_cast<DataType*>(p);
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator*() const -> DataType const&
{
    return *(this->operator->());
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::operator*() -> DataType&
{
    return *(this->operator->());
}

template<typename DataType>
auto RawDataView<DataType>::Iterator::ptr() const -> DataType*
{
    return this->operator->();
}
}

#endif // CYCLONITE_DATAVIEW_H
