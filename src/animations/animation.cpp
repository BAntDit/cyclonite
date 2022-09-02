//
// Created by anton on 6/22/22.
//

#include "animation.h"
#include "multithreading/taskManager.h"
#include "resources/resourceManager.h"

namespace cyclonite::animations {
resources::Resource::ResourceTag Animation::tag{};

Animation::Animation(multithreading::TaskManager& taskManager,
                     uint32_t samplerCount,
                     real duration,
                     bool autoplay) noexcept
  : resources::Resource{ samplerCount * sizeof(Sampler) }
  , taskManager_{ &taskManager }
  , interpolationTaskArrayId_{}
  , samplerArrayId_{}
  , playtime_{ 0.f }
  , duration_{ duration }
  , timescale_{ 1.f }
  , flags_{}
{
    flags_.reset();

    if (autoplay)
        flags_.set(value_cast(AnimationBits::ACTIVE_BIT), true);
}

Animation::Animation(multithreading::TaskManager& taskManager, uint32_t samplerCount, bool autoplay) noexcept
  : Animation{ taskManager, samplerCount, 0.f, autoplay }
{}

void Animation::handleDynamicDataAllocation()
{
    assert(0 == (dynamicDataSize() % sizeof(Sampler)));

    auto samplerCount = dynamicDataSize() / sizeof(Sampler);

    samplerArrayId_ = resourceManager().template create<SamplerArray>(samplerCount);

    auto [taskCount, itemsPerTask] = taskManager_->getTaskCount(samplerCount);

    assert(taskCount < std::numeric_limits<uint16_t>::max());
    assert(itemsPerTask < std::numeric_limits<uint16_t>::max());

    interpolationTaskArrayId_ = resourceManager().template create<AnimationInterpolationTaskArray>(
      static_cast<uint16_t>(taskCount), static_cast<uint16_t>(itemsPerTask));
}

void Animation::beginUpdate(real dt)
{
    if (!active())
        return;

    auto playtime = playtime_ + dt * timescale_;

    if (playtime <= duration_ && playtime >= 0.f) {
        playtime_ = playtime;
    } else if (playtime < 0.f || playtime > duration_) {
        playtime_ = playtime < 0.f ? 0.f : duration_;
        flags_.set(value_cast(AnimationBits::LAST_FRAME_BIT), true);
    }

    _update();
}

bool Animation::endUpdate()
{
    bool isOver = false;

    if (!active())
        return isOver;

    if (flags_.test(value_cast(AnimationBits::LAST_FRAME_BIT))) {
        auto looped = flags_.test(value_cast(AnimationBits::LOOPED_BIT));

        flags_.set(value_cast(AnimationBits::LAST_FRAME_BIT), false);
        flags_.set(value_cast(AnimationBits::ACTIVE_BIT), looped);

        isOver = true;
    }

    return isOver;
}

void Animation::_update()
{
    auto& interpolationTasks =
      resourceManager().get(interpolationTaskArrayId_).template as<AnimationInterpolationTaskArray>();

    auto& samplers = resourceManager().get(samplerArrayId_).template as<SamplerArray>();

    auto itemsPerTask = interpolationTasks.itemsPerTask();
    for (size_t i = 0, count = interpolationTasks.count(); i < count; i++) {
        auto from = samplers.begin() + static_cast<ptrdiff_t>(i * itemsPerTask);
        auto to = from + ((i * itemsPerTask + itemsPerTask <= samplers.count())
                            ? static_cast<ptrdiff_t>(itemsPerTask)
                            : static_cast<ptrdiff_t>(samplers.count() - i * itemsPerTask));

        interpolationTasks[i] = taskManager_->submit([from, to, playtime = playtime_]() -> void {
            for (auto it = from; it != to; it++) {
                (*it).update(playtime);
            }
        });
    }

    interpolationTasks.resolve();
}

void Animation::play()
{
    flags_.set(value_cast(AnimationBits::ACTIVE_BIT));
}

void Animation::pause()
{
    flags_.reset(value_cast(AnimationBits::ACTIVE_BIT));
}

void Animation::stop()
{
    flags_.reset(value_cast(AnimationBits::ACTIVE_BIT));
    playtime_ = 0.f;

    _update();
}

auto Animation::_samplers() const -> SamplerArray const&
{
    return resourceManager().get(samplerArrayId_).template as<SamplerArray>();
}

auto Animation::_samplers() -> SamplerArray&
{
    return resourceManager().get(samplerArrayId_).template as<SamplerArray>();
}

AnimationInterpolationTaskArray::AnimationInterpolationTaskArray(uint16_t taskCount, uint16_t itemsPerTask)
  : resources::ContiguousData<std::future<void>>{ taskCount }
  , itemsPerTask_{ itemsPerTask }
{}

void AnimationInterpolationTaskArray::resolve()
{
    for (auto&& task : *this)
        task.get();
}
}