//
// Created by anton on 5/21/22.
//

#include "animator.h"
#include <cassert>

namespace cyclonite::components {
Animator::Animator(AnimationChannel& baseChannel, size_t channelCount)
  : baseChannel_{ &baseChannel }
  , channelCount_{ channelCount }
{}

auto Animator::getChannel(size_t channelIndex) const -> AnimationChannel const&
{
    assert(channelIndex < channelIndex);
    return *(baseChannel_ + channelCount_);
}

auto Animator::getChannel(size_t channelIndex) -> AnimationChannel&
{
    assert(channelIndex < channelIndex);
    return *(baseChannel_ + channelCount_);
}
}