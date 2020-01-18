//
// Created by bantdit on 1/12/20.
//

#ifndef CYCLONITE_TRANSFORMSYSTEM_H
#define CYCLONITE_TRANSFORMSYSTEM_H

#include "../components/transform.h"
#include "../vulkan/staging.h"
#include "updateStages.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>

namespace cyclonite::systems {
class TransformSystem : public enttx::BaseSystem<TransformSystem>
{
public:
    using tag_t = easy_mp::type_list<components::Transform>;

    TransformSystem() = default;

    template<typename... Args>
    void init(Args&&... args);

    template<typename SystemManager, typename EntityManager, size_t STAGE>
    void update(SystemManager& systemManager, EntityManager& entityManager);

    template<typename EntityManager, typename... Args>
    auto create(EntityManager& entityManager,
                enttx::Entity const& parentEntity,
                enttx::Entity const& entity,
                Args&&... args) -> components::Transform&;

    template<typename EntityManager>
    void destroy(EntityManager& entityManager, enttx::Entity const& entity);

private:
    void _decompose(mat4& mat, vec3& position, vec3& scale, quat& orientation);

private:
    static const size_t needsUpdateTransformComponentsBit = 1;
    static const size_t needsUpdateLocalMatrixBit = 2;
    static const size_t needsUpdateWorldMatrixBit = 3;

    std::vector<mat4> worldMatrices_;
    std::vector<uint8_t> updateStatus_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE>
void TransformSystem::update(SystemManager& systemManager, EntityManager& entityManager)
{
    if constexpr (STAGE == easy_mp::value_cast(UpdateStage::EARLY_UPDATE)) {
        auto view = entityManager.template getView<components::Transform>();

        for (auto& [entity, transform] : view) {
            (void)entity;

            auto& [position, scale, orientation, matrix, flags, depth, globalIndex, parentIndex] = transform;

            (void)depth;

            auto const& parentMatrix =
              parentIndex != std::numeric_limits<size_t>::max() ? worldMatrices_[parentIndex] : mat4{ 1.0f };

            assert(!(flags.test(needsUpdateTransformComponentsBit) && flags.test(needsUpdateLocalMatrixBit)));

            if (flags.test(needsUpdateLocalMatrixBit)) {
                matrix = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
            }

            if (flags.test(needsUpdateTransformComponentsBit)) {
                _decompose(matrix, position, scale, orientation);
            }

            assert(worldMatrices_.size() < globalIndex);

            if (flags.test(needsUpdateWorldMatrixBit) || updateStatus_[parentIndex] == 1) {
                worldMatrices_[globalIndex] = parentMatrix * matrix;

                updateStatus_[globalIndex] = 1;
            } else {
                updateStatus_[globalIndex] = 0;
            }

            flags.reset();
        }
    }
}

template<typename EntityManager, typename... Args>
auto TransformSystem::create(EntityManager& entityManager,
                             enttx::Entity const& parentEntity,
                             enttx::Entity const& entity,
                             Args&&... args) -> components::Transform&
{
    auto& transforms = entityManager.template getStorage<components::Transform>();

    auto const* parentTransform =
      std::as_const(entityManager).template getComponent<components::Transform>(parentEntity);

    auto depth = parentTransform->depth + 1;

    auto it = std::upper_bound(
      transforms.begin(), transforms.end(), depth, [](size_t lhs, auto const& rhs) -> bool { return lhs < rhs.depth; });

    auto globalIndex = std::distance(transforms.begin(), it);

    auto& transform =
      entityManager.template assign<components::Transform>(entity, globalIndex, std::forward<Args>(args)...);

    transform.depth = depth;
    transform.gloabIndex = globalIndex;
    transform.parentIndex = parentTransform->globalIndex;

    for (auto cit = std::next(transforms.begin(), globalIndex + 1); cit != transforms.end(); cit++) {
        (*cit).globalIndex++;

        if ((*cit).parentIndex >= globalIndex) {
            (*cit).parentIndex++;
        }
    }

    return transform;
}

template<typename EntityManager>
void TransformSystem::destroy(EntityManager& entityManager, enttx::Entity const& entity)
{
    auto& transforms = entityManager.template getStorage<components::Transform>();

    auto const* transform =
        std::as_const(entityManager).template getComponent<components::Transform>(entity);

    auto globalIndex = transform->globalIndex;

    for (auto cit = std::next(transforms.begin(), globalIndex + 1); cit != transforms.end(); cit++) {
        (*cit).globalIndex--;

        assert((*cit).parentIndex != globalIndex); // it has no children to this moment

        if ((*cit).parentIndex > globalIndex) {
            (*cit).parentIndex--;
        }
    }

    entityManager.template destroy<components::Transform>(entity);
}
}

#endif // CYCLONITE_TRANSFORMSYSTEM_H
