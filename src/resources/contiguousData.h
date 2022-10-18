//
// Created by anton on 6/29/22.
//

#ifndef CYCLONITE_CONTIGUOUSDATA_H
#define CYCLONITE_CONTIGUOUSDATA_H

#include "resource.h"

namespace cyclonite::resources {
template<typename DataType>
class ContiguousData : public resources::Resource
{
public:
    class Iterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = DataType;
        using difference_type = std::ptrdiff_t;
        using pointer = DataType*;
        using reference = DataType&;

        Iterator(DataType* data, size_t count, difference_type index) noexcept;

        explicit operator bool() const { return index_ < count_; }

        auto operator==(Iterator const& rhs) const -> bool { return data_ == rhs.data_ && index_ == rhs.index_; }

        auto operator!=(Iterator const& rhs) const -> bool { return data_ != rhs.data_ || index_ != rhs.index_; }

        auto operator+=(difference_type diff) -> Iterator&;

        auto operator-=(difference_type diff) -> Iterator&;

        auto operator++() -> Iterator&;

        auto operator--() -> Iterator&;

        auto operator++(int) -> Iterator;

        auto operator--(int) -> Iterator;

        auto operator+(difference_type diff) -> Iterator;

        auto operator-(difference_type diff) -> Iterator;

        auto operator-(Iterator const& rhs) const -> difference_type;

        auto operator*() -> DataType&;

        auto operator*() const -> DataType const&;

        auto operator-> () const -> pointer;

        [[nodiscard]] auto ptr() const -> pointer;

        auto operator[](size_t index) const -> reference;

    private:
        DataType* data_;
        size_t count_;
        difference_type index_;
    };

public:
    explicit ContiguousData(size_t dataCount) noexcept;

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    [[nodiscard]] auto count() const -> size_t { return count_; }

    [[nodiscard]] auto data() const -> DataType* { return data_; }

    [[nodiscard]] auto operator[](size_t index) const -> DataType&;

    [[nodiscard]] auto begin() const -> Iterator;

    [[nodiscard]] auto end() const -> Iterator;

private:
    static ResourceTag tag;

protected:
    void handleDynamicDataAllocation() override;

public:
    static auto type_tag_const() -> ResourceTag const& { return ContiguousData::tag; }
    static auto type_tag() -> ResourceTag& { return ContiguousData::tag; }

private:
    DataType* data_;
    size_t count_;
};

template<typename DataType>
Resource::ResourceTag ContiguousData<DataType>::tag{};

template<typename DataType>
ContiguousData<DataType>::ContiguousData(size_t dataCount) noexcept
  : resources::Resource{ dataCount * sizeof(DataType) }
  , data_{ nullptr }
  , count_{ dataCount }
{}

template<typename DataType>
void ContiguousData<DataType>::handleDynamicDataAllocation()
{
    data_ = new (dynamicData()) DataType[count_];
}

template<typename DataType>
ContiguousData<DataType>::Iterator::Iterator(DataType* data, size_t count, difference_type index) noexcept
  : data_{ data }
  , count_{ count }
  , index_{ index }
{
    assert(index_ <= static_cast<difference_type>(count_));
}

template<typename DataType>
auto ContiguousData<DataType>::operator[](size_t index) const -> DataType&
{
    assert(index < count_);
    return *(data_ + index);
}

template<typename DataType>
auto ContiguousData<DataType>::begin() const -> Iterator
{
    return Iterator{ data_, count_, 0 };
}

template<typename DataType>
auto ContiguousData<DataType>::end() const -> Iterator
{
    return Iterator{ data_, count_, static_cast<typename Iterator::difference_type>(count_) };
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator+=(difference_type diff) -> Iterator&
{
    index_ += diff;
    assert(index_ <= static_cast<difference_type>(count_));
    return *this;
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator-=(difference_type diff) -> Iterator&
{
    assert(diff <= index_);
    index_ -= diff;
    return *this;
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator++() -> Iterator&
{
    assert(index_ <= static_cast<difference_type>(count_));
    index_++;
    return *this;
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator--() -> Iterator&
{
    assert(index_ > 0l);
    index_--;
    return *this;
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator++(int) -> Iterator
{
    assert(index_ <= static_cast<difference_type>(count_));
    auto prev = Iterator{ data_, count_, index_ };
    index_++;
    return prev;
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator--(int) -> Iterator
{
    assert(index_ > 0l);
    auto prev = Iterator{ data_, count_, index_ };
    index_--;
    return prev;
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator+(difference_type diff) -> Iterator
{
    return Iterator{ data_, count_, index_ + diff };
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator-(difference_type diff) -> Iterator
{
    assert(index_ >= diff);
    return Iterator{ data_, count_, index_ - diff };
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator-(Iterator const& rhs) const -> difference_type
{
    return rhs.index_ - index_;
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator*() -> DataType&
{
    return *(data_ + index_);
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator*() const -> DataType const&
{
    return *(data_ + index_);
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator-> () const -> DataType*
{
    return data_ + index_;
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::ptr() const -> DataType*
{
    return data_ + index_;
}

template<typename DataType>
auto ContiguousData<DataType>::Iterator::operator[](size_t index) const -> reference
{
    return *(data_ + index);
}
}

#endif // CYCLONITE_CONTIGUOUSDATA_H
