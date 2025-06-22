template<typename DataType>
BufferView<DataType>::BufferView()
  : ptr_{ nullptr }
, stride_{ 0 }
, count_{ 0 }
, offset_{ std::numeric_limits<size_t>::max() }
{
}

template<typename DataType>
BufferView<DataType>::BufferView(DataType* ptr, size_t offset, size_t count, size_t stride /* = sizeof(DataType)*/)
  : ptr_{ ptr }
, stride_{ stride }
, count_{ count }
, offset_{ stride }
{
}

template<typename DataType>
void BufferView<DataType>::detach()
{
    ptr_ = nullptr;
    stride_ = 0;
    count_ = 0;
    offset_ = std::numeric_limits<size_t>::max();
}

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
    auto p = std::add_pointer_t<DataType>{ nullptr };
    if (view_->stride_ == sizeof(DataType)) [[likely]] {
        p = view_->ptr_ + index_;
    } else [[unlikely]] { // interleaved buffer
        p = std::launder(
          reinterpret_cast<DataType*>(reinterpret_cast<std::byte*>(view_->ptr_) + index_ * view_->stride_));

        [[maybe_unused]] auto* ptr = p;
        [[maybe_unused]] auto s = view_->stride_;
        assert(std::align(alignof(DataType), sizeof(DataType), &ptr, s) == ptr);
    }

    return p;
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
    auto p = std::add_pointer_t<DataType>{ nullptr };
    assert((index_ + index) < view_->count_);

    if (view_->stride_ == sizeof(DataType)) [[likely]] {
        p = view_->ptr_ + (index_ + index);
    } else [[unlikely]] {
        p = std::launder(
          reinterpret_cast<DataType*>(reinterpret_cast<std::byte*>(view_->ptr_) + (index_ + index) * view_->stride_));

        [[maybe_unused]] auto* ptr = p;
        [[maybe_unused]] auto s = view_->stride_;
        assert(std::align(alignof(DataType), sizeof(DataType), &ptr, s) == ptr);
    }

    assert(p != nullptr);
    return *p;
}
