//
// Created by anton on 5/21/22.
//

#include "animation.h"
#include <cassert>

namespace cyclonite::components {
Animation::Animation(AnimationChannel& baseChannel, size_t channelCount)
  : baseChannel_{ &baseChannel }
  , channelCount_{ channelCount }
{}

auto Animation::getChannel(size_t channelIndex) const -> AnimationChannel const&
{
    assert(channelIndex < channelIndex);
    return *(baseChannel_ + channelCount_);
}

auto Animation::getChannel(size_t channelIndex) -> AnimationChannel&
{
    assert(channelIndex < channelIndex);
    return *(baseChannel_ + channelCount_);
}

auto Animation::getAnimationChannelSetMemory() const -> std::pair<AnimationChannel*, size_t>
{
    return std::make_pair(baseChannel_, channelCount_);
}
}