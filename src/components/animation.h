//
// Created by anton on 5/21/22.
//

#ifndef CYCLONITE_ANIMATION_H
#define CYCLONITE_ANIMATION_H

#include <enttx/entity.h>
#include <utility>

namespace cyclonite::components {
class AnimationStorage;

struct AnimationChannel
{
    using target_updater_t = void (*)(enttx::Entity);

    uint64_t animationId;
    size_t samplerIndex;
    enttx::Entity target;
    target_updater_t updater;
};

struct Animation
{
    friend class AnimationStorage;

    Animation() = default;

    Animation(AnimationChannel& baseChannel, size_t channelCount);

    [[nodiscard]] auto getChannelCount() const -> size_t { return channelCount_; }

    [[nodiscard]] auto getChannel(size_t channelIndex) const -> AnimationChannel const&;

    auto getChannel(size_t channelIndex) -> AnimationChannel&;

private:
    [[nodiscard]] auto getAnimationChannelSetMemory() const -> std::pair<AnimationChannel*, size_t>;

private:
    AnimationChannel* baseChannel_;
    size_t channelCount_;
};
}

#endif // CYCLONITE_ANIMATION_H
