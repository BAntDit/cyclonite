//
// Created by anton on 5/21/22.
//

#ifndef CYCLONITE_ANIMATIONSYSTEM_H
#define CYCLONITE_ANIMATIONSYSTEM_H

#include "multithreading/taskManager.h"
#include "updateStages.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>
#include <animations/animation.h>

namespace cyclonite::systems {
using namespace easy_mp;

enum class InterpolationType
{
    STEP = 0,
    LINEAR = 1,
    SPHERICAL = 2,
    CUBIC = 3,
    CATMULL_ROM = 4,
    MIN_VALUE = STEP,
    MAX_VALUE = CATMULL_ROM,
    COUNT = MAX_VALUE
};

class AnimationSystem : public enttx::BaseSystem<AnimationSystem>
{
public:
    AnimationSystem() = default;

    AnimationSystem(AnimationSystem const&) = delete;

    AnimationSystem(AnimationSystem&&) = default;

    ~AnimationSystem() = default;

    auto operator=(AnimationSystem const&) -> AnimationSystem& = delete;

    auto operator=(AnimationSystem &&) -> AnimationSystem& = default;

    void init(multithreading::TaskManager& taskManager);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

private:
    multithreading::TaskManager* taskManager_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void AnimationSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    if constexpr (STAGE == value_cast(UpdateStage::EARLY_UPDATE)) {
        auto& animations = entityManager.template getStorage<animations::Animation>();

        for (auto&& animation : animations)
        {
        }
        // TDOO:: begin update
    }

    (void)systemManager;

    (void)entityManager;

    ((void)args, ...);
}
}

#endif // CYCLONITE_ANIMATIONSYSTEM_H
