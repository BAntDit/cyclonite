//
// Created by anton on 6/22/22.
//

#ifndef CYCLONITE_ANIMTAIONS_ANIMATION_H
#define CYCLONITE_ANIMATIONS_ANIMATION_H

#include "resources/contiguousData.h"
#include "sampler.h"
#include <bitset>
#include <easy-mp/enum.h>
#include <future>
#include <glm/gtc/type_ptr.hpp>

using namespace easy_mp;

namespace cyclonite::multithreading {
class TaskManager;
}

namespace cyclonite::animations {
using SamplerArray = resources::ContiguousData<Sampler>;

class AnimationInterpolationTaskArray : public resources::ContiguousData<std::future<void>>
{
public:
    AnimationInterpolationTaskArray(uint16_t taskCount, uint16_t itemsPerTask);

    [[nodiscard]] auto itemsPerTask() const -> uint16_t { return itemsPerTask_; }

    void resolve();

private:
    uint16_t itemsPerTask_;
};

struct AnimationChannel
{
    using target_updater_t = void (*)(void* target, Sampler const& sampler);

    size_t samplerIndex;
    target_updater_t updater;
};

class Animation : public resources::Resource
{
    enum class AnimationBits
    {
        LOOPED_BIT = 0,
        ACTIVE_BIT = 1,
        LAST_FRAME_BIT = 2,
        MIN_VALUE = LOOPED_BIT,
        MAX_VALUE = LAST_FRAME_BIT,
        COUNT = MAX_VALUE + 1
    };

public:
    Animation(multithreading::TaskManager& taskManager, uint32_t sampleCount, bool autoplay = false) noexcept;

    Animation(multithreading::TaskManager& taskManager,
              uint32_t sampleCount,
              real duration,
              bool autoplay = false) noexcept;

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    [[nodiscard]] auto playtime() const -> real { return playtime_; }

    [[nodiscard]] auto duration() const -> real { return duration_; }

    [[nodiscard]] auto timescale() const -> real { return timescale_; }

    [[nodiscard]] auto looped() const -> bool { return flags_.test(value_cast(AnimationBits::LOOPED_BIT)); }

    [[nodiscard]] auto active() const -> bool { return flags_.test(value_cast(AnimationBits::ACTIVE_BIT)); }

    auto timescale() -> real& { return timescale_; }

    void beginUpdate(real dt);

    bool endUpdate(); // returns true, if animation is over

    void play();

    void pause();

    void stop();

    template<typename T>
    auto sample(size_t i, T&& t) const -> T;

    void setupSampler(size_t samplerIndex,
                      resources::Resource::Id inBufferId,
                      resources::Resource::Id outBufferId,
                      size_t inOffset,
                      size_t inStride,
                      size_t outOffset,
                      size_t outStride,
                      size_t elementCount,
                      size_t componentCount,
                      InterpolationType interpolationType,
                      InterpolationElementType interpolationElementType);

protected:
    void handlePostAllocation() override;

private:
    void _update();

    [[nodiscard]] auto _samplers() const -> SamplerArray const&;

    auto _samplers() -> SamplerArray&;

    multithreading::TaskManager* taskManager_;
    resources::Resource::Id interpolationTaskArrayId_;
    resources::Resource::Id samplerArrayId_;

    uint32_t sampleCount_;
    real playtime_;
    real duration_;
    real timescale_;
    std::bitset<value_cast(AnimationBits::COUNT)> flags_;

private:
    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return Animation::tag; }
    static auto type_tag() -> ResourceTag& { return Animation::tag; }
};

template<typename T>
auto Animation::sample(size_t i, T&& t) const -> T
{
    using make_func_t = T (*)(real const*);
    constexpr make_func_t makeFunc = nullptr;

    auto& samplers = _samplers();

    if constexpr (std::is_same_v<std::decay_t<T>, real>) {
        makeFunc = [](real const* src) -> real { return *src; };
    } else if constexpr (std::is_same_v<std::decay_t<T>, vec2>) {
        makeFunc = glm::make_vec2;
    } else if constexpr (std::is_same_v<std::decay_t<T>, vec3>) {
        makeFunc = glm::make_vec3;
    } else if constexpr (std::is_same_v<std::decay_t<T>, vec4>) {
        makeFunc = glm::make_vec4;
    } else if constexpr (std::is_same_v<std::decay_t<T>, quat>) {
        makeFunc = glm::make_quat;
    } else if constexpr (std::is_same_v<std::decay_t<T>, glm::tmat2x2<boost::float32_t, glm::highp>>) {
        makeFunc = glm::make_mat2x2;
    } else if constexpr (std::is_same_v<std::decay_t<T>, mat3>) {
        makeFunc = glm::make_mat3;
    } else if constexpr (std::is_same_v<std::decay_t<T>, mat3x4>) {
        makeFunc = glm::make_mat3x4;
    } else if constexpr (std::is_same_v<std::decay_t<T>, glm::tmat4x3<boost::float32_t, glm::highp>>) {
        makeFunc = glm::make_mat4x3;
    } else if constexpr (std::is_same_v<std::decay_t<T>, mat4>) {
        makeFunc = glm::make_mat4;
    } else if constexpr (std::is_constructible_v<T, real const*>) {
        makeFunc = [](real const* src) -> T { return T(src); };
    }

    static_assert(makeFunc != nullptr);

    t = samplers[i].template value(makeFunc);

    return t;
}
}

#endif // CYCLONITE_ANIMATIONS_ANIMATION_H
