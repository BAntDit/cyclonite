//
// Created by anton on 5/21/22.
//

#include <bit>
#include "animationStorage.h"

namespace cyclonite::components {
AnimationStorage::AnimationStorage()
  : store_(4096, AnimationChannel{}) // TODO:: make possible to set initial capacity
  , animations_{}
  , freeRanges_{}
  , freeIndices_{}
  , indices_{}
{
    animations_.reserve(1024);
    freeRanges_.insert(std::make_pair(size_t{ 0 }, size_t{ 4096 }));
}

auto AnimationStorage::get(uint32_t index) const -> Animation const&
{
    assert(index < indices_.size());
    auto idx = indices_[index];

    assert(idx < animations_.size());
    return animations_[idx];
}

auto AnimationStorage::get(uint32_t index) -> Animation&
{
    assert(index < indices_.size());
    auto idx = indices_[index];

    assert(idx < animations_.size());
    return animations_[idx];
}

auto AnimationStorage::allocChannelRange(uint16_t channelCount) -> std::pair<size_t, size_t>
{
    auto r = std::make_pair(size_t{ 0 }, size_t{ 0 });

    auto it =
      std::lower_bound(freeRanges_.cbegin(), freeRanges_.cend(), channelCount, [](auto range, auto value) -> bool {
          auto [offset, count] = range;
          (void)offset;

          return count < value;
      });

    if (it != freeRanges_.cend()) {
        auto [rangeOffset, rangeSize] = (*it);

        std::get<0>(r) = rangeOffset;
        std::get<1>(r) = rangeSize;

        freeRanges_.erase(it);
    } else {
        assert(!freeRanges_.empty());

        auto last = freeRanges_.crbegin();
        auto [rangeOffset, rangeSize] = (*last);

        std::get<0>(r) = rangeOffset;
        std::get<1>(r) = rangeSize;

        freeRanges_.erase(std::next(last, 1).base());
    }

    return r;
}

auto AnimationStorage::create(uint32_t index, uint16_t channelCount) -> Animation&
{
    auto rangeOffset = size_t{ 0 };
    auto rangeSize = size_t{ 0 };

    {
        auto [offset, size] = allocChannelRange(channelCount);

        if (size < channelCount) {
            auto diff = channelCount - size;
            auto old = store_.size();
            auto add = diff > old ? diff : old;

            // store offsets into pointers
            for (auto idx : indices_) {
                if (idx == std::numeric_limits<uint32_t>::max())
                    continue;

                auto& anim = animations_[idx];

                auto ofs = anim.baseChannel_ - store_.data();
                assert(ofs >= 0);

                static_assert(sizeof(anim.baseChannel_) == sizeof(decltype(ofs)));
                new (&anim.baseChannel_) decltype(ofs) { ofs };
            }

            store_.resize(old + add, AnimationChannel{});

            // make pointers valid again
            for (auto idx : indices_) {
                if (idx == std::numeric_limits<uint32_t>::max())
                    continue;

                auto& anim = animations_[idx];

                static_assert(sizeof(intptr_t) == sizeof(AnimationChannel*));
                auto ofs = std::bit_cast<intptr_t>(anim.baseChannel_);

                new (&anim.baseChannel_) AnimationChannel*{ store_.data() + ofs };
            }

            if ((offset + size) == old) {
                freeRanges_.insert(std::make_pair(offset, size + add));
            } else {
                freeRanges_.insert(std::make_pair(old, add));
            }
        } else {
            rangeOffset = offset;
            rangeSize = size;
        }
    }

    if (rangeSize == 0) {
        auto [offset, size] = allocChannelRange(channelCount);

        if (size < channelCount) {
            freeRanges_.insert(std::make_pair(offset, size));
            throw std::runtime_error("animation storage can not allocate memory to place new animation");
        } else {
            rangeOffset = offset;
            rangeSize = size;
        }
    }

    assert(rangeSize >= channelCount);

    if (rangeSize > channelCount) {
        freeRanges_.insert(std::make_pair(rangeOffset + channelCount, rangeSize - channelCount));
    }

    auto animationIndex = std::numeric_limits<uint32_t>::max();

    if (freeIndices_.empty()) {
        animationIndex = animations_.size();
        animations_.emplace_back();
    } else {
        animationIndex = freeIndices_.back();
        freeIndices_.pop_back();
    }

    if (index >= indices_.size()) {
        auto old = indices_.size();
        auto diff = index - old;
        auto add = diff > old ? diff : old;

        indices_.resize(old + add, std::numeric_limits<uint32_t>::max());
    }

    assert(indices_[index] == std::numeric_limits<uint32_t>::max());

    animations_[animationIndex] = Animation{ *(store_.data() + rangeOffset), channelCount };
    indices_[index] = animationIndex;

    return animations_[animationIndex];
}

void AnimationStorage::destroy(uint32_t index)
{
    assert(indices_[index] != std::numeric_limits<uint32_t>::max());

    auto animationIndex = indices_[index];
    auto& animation = animations_[animationIndex];

    auto ptr = animation.baseChannel_;
    auto count = animation.channelCount_;

    auto offset = static_cast<size_t>(ptr - store_.data());

    auto freeOffset = offset;
    auto freeCount = count;

    // merge prev if possible
    {
        auto prevIt = std::find_if(freeRanges_.cbegin(), freeRanges_.cend(), [=](auto range) -> bool {
            auto [rangeOffset, rangeCount] = range;
            return (static_cast<size_t>(offset) == (rangeOffset + rangeCount));
        });

        if (prevIt != freeRanges_.cend()) {
            auto [rangeOffset, rangeCount] = (*prevIt);
            freeOffset = rangeOffset;
            freeCount = freeCount + rangeCount;

            freeRanges_.erase(prevIt);
        }
    }

    // merge next if possible
    {
        auto nextIt =
          std::find_if(freeRanges_.cbegin(), freeRanges_.cend(), [offset, count = count](auto range) -> bool {
              auto [rangeOffset, rangeCount] = range;
              (void)rangeCount;

              return (rangeOffset == (offset + count));
          });

        if (nextIt != freeRanges_.cend()) {
            auto [rangeOffset, rangeCount] = (*nextIt);
            freeCount = freeCount + rangeCount;

            freeRanges_.erase(nextIt);
        }
    }

    freeRanges_.insert(std::make_pair(freeOffset, freeCount));
    freeIndices_.push_back(animationIndex);

    indices_[index] = std::numeric_limits<uint32_t>::max();
}
}