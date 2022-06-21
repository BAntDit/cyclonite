//
// Created by anton on 6/22/22.
//

#include "animation.h"

namespace cyclonite::animations {
Animation::Iterator::Iterator(Sampler* baseSampler, uint32_t samplerCount, difference_type index) noexcept
  : baseSampler_{ baseSampler }
  , index_{ index }
  , count_{ samplerCount }
{
    assert(index_ <= static_cast<difference_type>(count_));
}

auto Animation::Iterator::operator+=(difference_type diff) -> Iterator&
{
    index_ += diff;
    assert(index_ <= static_cast<difference_type>(count_));
    return *this;
}

auto Animation::Iterator::operator-=(difference_type diff) -> Iterator&
{
    assert(diff <= index_);
    index_ -= diff;
    return *this;
}

auto Animation::Iterator::operator++() -> Iterator&
{
    assert(index_ <= static_cast<difference_type>(count_));
    index_++;
    return *this;
}

auto Animation::Iterator::operator--() -> Iterator&
{
    assert(index_ > 0l);
    index_--;
    return *this;
}

auto Animation::Iterator::operator++(int) -> Iterator
{
    assert(index_ <= static_cast<difference_type>(count_));
    auto prev = Iterator{ baseSampler_, count_, index_ };
    index_++;
    return prev;
}

auto Animation::Iterator::operator--(int) -> Iterator
{
    assert(index_ > 0l);
    auto prev = Iterator{ baseSampler_, count_, index_ };
    index_--;
    return prev;
}

auto Animation::Iterator::operator+(difference_type diff) -> Iterator
{
    return Iterator{ baseSampler_, count_, index_ + diff };
}

auto Animation::Iterator::operator-(difference_type diff) -> Iterator
{
    assert(index_ >= diff);
    return Iterator{ baseSampler_, count_, index_ - diff };
}

auto Animation::Iterator::operator-(Iterator const& rhs) const -> difference_type
{
    return rhs.index_ - index_;
}

auto Animation::Iterator::operator*() -> Sampler&
{
    return *(baseSampler_ + index_);
}

auto Animation::Iterator::operator*() const -> Sampler const&
{
    return *(baseSampler_ + index_);
}

auto Animation::Iterator::operator->() const -> Sampler*
{
    return baseSampler_ + index_;
}

auto Animation::Iterator::ptr() const -> Sampler*
{
    return baseSampler_ + index_;
}

auto Animation::Iterator::operator[](int index) const -> reference
{
    return *(baseSampler_ + index);
}

Animation::Animation(uint32_t samplerCount) noexcept
  : resources::Resource{ samplerCount * sizeof(Sampler) }
  , baseSampler_{ nullptr }
  , samplerCount_{ samplerCount }
{}

void Animation::handleDynamicDataAllocation()
{
    baseSampler_ = new (dynamicData()) Sampler[samplerCount_]; // TODO:: allocate on load (implement loading)
}
}