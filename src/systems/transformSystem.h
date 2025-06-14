//
// Created by bantdit on 1/12/20.
//

#ifndef CYCLONITE_TRANSFORMSYSTEM_H
#define CYCLONITE_TRANSFORMSYSTEM_H

#include "../components/transform.h"
#include "resources/staging.h"
#include "updateStages.h"
#include <enttx/enttx.h>
#include <metrix/enum.h>
#include <metrix/type_list.h>

namespace cyclonite::systems {
class TransformSystem : public enttx::BaseSystem<TransformSystem>
{
public:
    using tag_t = metrix::type_list<components::Transform>;

    TransformSystem() = default;

    ~TransformSystem() = default;

    void init();

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    template<typename EntityManager, typename... Args>
    auto create(EntityManager& entityManager, enttx::Entity parentEntity, enttx::Entity entity, Args&&... args)
      -> components::Transform&;

    template<typename EntityManager>
    void destroy(EntityManager& entityManager, enttx::Entity const& entity);

    // TODO:: get children

private:
    static void _decompose(mat4& mat, vec3& position, vec3& scale, quat& orientation);
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void TransformSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    (void)systemManager;

    auto&& [node, frameNumber, dt] = std::forward_as_tuple(std::forward<Args>(args)...);
    (void)node;
    (void)dt;

    if constexpr (STAGE == metrix::value_cast(UpdateStage::EARLY_UPDATE)) {
        auto& transforms = entityManager.template getStorage<components::Transform>();

        for (auto& transform : transforms) {
            auto& [position, scale, orientation, matrix, worldMatrix, state, parent, depth, lastFrameUpdate] =
              transform;

            auto* parentTransform = static_cast<uint64_t>(parent) != std::numeric_limits<uint64_t>::max()
                                      ? entityManager.template getComponent<components::Transform>(parent)
                                      : nullptr;

            (void)depth;

            auto const& parentMatrix = parentTransform != nullptr ? parentTransform->worldMatrix : mat4{ 1.0f };
            auto parentLastFrameUpdate =
              parentTransform != nullptr ? parentTransform->lastFrameUpdate : std::numeric_limits<uint64_t>::max();

            auto parentUpdated = (parentLastFrameUpdate == frameNumber);

            if (state != components::Transform::State::UPDATE_NOTHING || parentUpdated) {
                if (state == components::Transform::State::UPDATE_LOCAL) {
                    matrix = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
                    state = components::Transform::State::UPDATE_WORLD;
                } else if (state == components::Transform::State::UPDATE_COMPONENTS) {
                    _decompose(matrix, position, scale, orientation);
                    state = components::Transform::State::UPDATE_WORLD;
                }

                if (state == components::Transform::State::UPDATE_WORLD || parentUpdated) {
                    worldMatrix = parentMatrix * matrix;
                }

                lastFrameUpdate = frameNumber;
                state = components::Transform::State::UPDATE_NOTHING;
            } // if state to update
        }
    } // early update
}

template<typename EntityManager, typename... Args>
auto TransformSystem::create(EntityManager& entityManager,
                             enttx::Entity parentEntity,
                             enttx::Entity entity,
                             Args&&... args) -> components::Transform&
{
    auto& transforms = entityManager.template getStorage<components::Transform>();

    auto const* parentTransform =
      static_cast<uint64_t>(parentEntity) == std::numeric_limits<uint64_t>::max()
        ? nullptr
        : std::as_const(entityManager).template getComponent<components::Transform>(parentEntity);

    auto depth = parentTransform == nullptr ? 0 : parentTransform->depth + 1;

    // to be able to beginUpdate parent transforms first
    // all transforms must be in the right order
    auto it = std::upper_bound(transforms.begin(),
                               transforms.end(),
                               std::make_pair(depth, static_cast<uint64_t>(parentEntity)),
                               [](auto&& lhs, auto const& rhs) -> bool {
                                   auto&& [ldepth, parent] = lhs;
                                   return (ldepth < rhs.depth) ||
                                          (ldepth == rhs.depth && parent < static_cast<uint64_t>(rhs.parent));
                               });

    auto order = static_cast<size_t>(std::distance(transforms.begin(), it));

    auto& transform = entityManager.template assign<components::Transform>(entity, order, std::forward<Args>(args)...);

    transform.parent = parentEntity;
    transform.depth = depth;

    return transform;
}

template<typename EntityManager>
void TransformSystem::destroy(EntityManager& entityManager, enttx::Entity const& entity)
{
    entityManager.template destroy<components::Transform>(entity);
}
}

#endif // CYCLONITE_TRANSFORMSYSTEM_H
