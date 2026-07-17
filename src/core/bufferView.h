//
// Created by anton on 6/22/25.
//

#ifndef BUFFERVIEW_H
#define BUFFERVIEW_H

#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>

namespace cyclonite::core {
// presents a piece of buffer as view of necessary type
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

    BufferView();

    BufferView(DataType* ptr, size_t offset, size_t count, size_t stride = sizeof(DataType));

    [[nodiscard]] auto count() const -> size_t { return count_; }

    [[nodiscard]] auto empty() const -> bool { return ptr_ == nullptr || count_ == 0; }

    // offset in bytes from the base buffer address this view allocated from
    [[nodiscard]] auto offset() const -> size_t { return offset_; }

    [[nodiscard]] auto data() const -> DataType* { return ptr_; }

    explicit operator bool() const { return !empty(); }

    // detach this view from buffer
    // it makes the view empty
    void detach();

    [[nodiscard]] auto begin() const -> Iterator { return Iterator{ *this, 0l }; }

    [[nodiscard]] auto end() const -> Iterator
    {
        return Iterator{ *this, static_cast<typename Iterator::difference_type>(count_) };
    }

private:
    DataType* ptr_;
    size_t stride_;
    size_t count_;
    size_t offset_;
};
#include "bufferView.inl"
}

#endif // BUFFERVIEW_H
