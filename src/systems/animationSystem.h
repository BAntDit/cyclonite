//
// Created by anton on 5/21/22.
//

#ifndef CYCLONITE_ANIMATIONSYSTEM_H
#define CYCLONITE_ANIMATIONSYSTEM_H

#include "animations/animation.h"
#include "components/animator.h"
#include "multithreading/taskManager.h"
#include "resources/resourceManager.h"
#include "updateStages.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>

namespace cyclonite::systems {
using namespace easy_mp;

class AnimationSystem : public enttx::BaseSystem<AnimationSystem>
{
public:
    AnimationSystem() = default;

    AnimationSystem(AnimationSystem const&) = delete;

    AnimationSystem(AnimationSystem&&) = default;

    ~AnimationSystem() = default;

    auto operator=(AnimationSystem const&) -> AnimationSystem& = delete;

    auto operator=(AnimationSystem &&) -> AnimationSystem& = default;

    void init(resources::ResourceManager& resourceManager, multithreading::TaskManager& taskManager);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

private:
    resources::ResourceManager* resourceManager_;
    multithreading::TaskManager* taskManager_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void AnimationSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    if constexpr (STAGE == value_cast(UpdateStage::EARLY_UPDATE)) {
        auto end = std::chrono::high_resolution_clock::now();
        auto dt = std::min(std::chrono::duration<real, std::ratio<1>>{ end - start_ }.count(), 0.1f);

        start_ = end;

        for (auto& animation : resourceManager_->template resourceList<animations::Animation>()) {
            // TODO:: make possible to be sure animation is updated once a frame
            animation.beginUpdate(dt);
        }

        {
            auto view = entityManager.template getView<components::Animator>();

            for (auto&& [entity, animator] : view) {
                for (auto idx = size_t{ 0 }, count = animator.getChannelCount(); idx < count; idx++) {
                    auto const& channel = animator.getChannel(idx);
                    auto& animation = resourceManager_->get(resources::Resource::Id{ channel.animationId })
                                        .template as<animations::Animation>();

                    auto const* sample = animation.sample(channel.samplerIndex);

                    channel.update_func(&entityManager, entity, sample);
                }
            }
        }

        for (auto& animation : resourceManager_->template resourceList<animations::Animation>()) {
            animation.endUpdate();
        }
    }

    if constexpr (STAGE == value_cast(UpdateStage::LATE_UPDATE)) {
        // TODO:: skinning (update GPU bones)
    }

    (void)systemManager;

    (void)entityManager;

    ((void)args, ...);
}
}

#endif // CYCLONITE_ANIMATIONSYSTEM_H
