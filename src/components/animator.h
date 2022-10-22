//
// Created by anton on 5/21/22.
//

#ifndef CYCLONITE_ANIMATOR_H
#define CYCLONITE_ANIMATOR_H

#include "typedefs.h"
#include <enttx/entity.h>
#include <utility>

namespace cyclonite::components {
class AnimatorStorage;

struct AnimationChannel
{
    using target_updater_t = void (*)(void*, enttx::Entity, real const*);

    uint64_t animationId;
    size_t samplerIndex;
    target_updater_t update_func;
};

// Animator makes entity animated
struct Animator
{
    friend class AnimatorStorage;

    Animator() = default;

    Animator(AnimationChannel& baseChannel, size_t channelCount);

    [[nodiscard]] auto getChannelCount() const -> size_t { return channelCount_; }

    [[nodiscard]] auto getChannel(size_t channelIndex) const -> AnimationChannel const&;

    auto getChannel(size_t channelIndex) -> AnimationChannel&;

private:
    AnimationChannel* baseChannel_;
    size_t channelCount_;
};
}

#endif // CYCLONITE_ANIMATOR_H
