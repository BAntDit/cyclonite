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

    ~TransformSystem() = default;

    void init(size_t initialTransformCount);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    template<typename EntityManager, typename... Args>
    auto create(EntityManager& entityManager, enttx::Entity parentEntity, enttx::Entity entity, Args&&... args)
      -> components::Transform&;

    template<typename EntityManager>
    void destroy(EntityManager& entityManager, enttx::Entity const& entity);

    // TODO:: get children

    [[nodiscard]] auto worldMatrices() const -> std::vector<mat4> const& { return worldMatrices_; }

private:
    template<typename EntityManager>
    void _reserveVectorsIfNecessary(EntityManager& entityManager, size_t globalIndex);

    void _decompose(mat4& mat, vec3& position, vec3& scale, quat& orientation);

private:
    std::vector<mat4> worldMatrices_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void TransformSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    using namespace easy_mp;

    ((void)args, ...);

    (void)systemManager;

    if constexpr (STAGE == value_cast(UpdateStage::EARLY_UPDATE)) {
        auto view = entityManager.template getView<components::Transform>();

        for (auto&& [entity, transform] : view) {
            (void)entity;

            auto& [position, scale, orientation, matrix, state, parent, depth, globalIndex] = transform;

            assert(worldMatrices_.size() > globalIndex);

            auto* parentTransform = static_cast<uint64_t>(parent) != std::numeric_limits<uint64_t>::max()
                                      ? entityManager.template getComponent<components::Transform>(parent)
                                      : nullptr;

            (void)depth;

            auto const& parentMatrix =
              parentTransform != nullptr ? worldMatrices_[parentTransform->globalIndex] : mat4{ 1.0f };

            if (state == components::Transform::State::UPDATE_LOCAL) {
                matrix = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
                state = components::Transform::State::UPDATE_WORLD;
            } else if (state == components::Transform::State::UPDATE_COMPONENTS) {
                _decompose(matrix, position, scale, orientation);
                state = components::Transform::State::UPDATE_WORLD;
            }

            state = (parentTransform != nullptr &&
                     parentTransform->state == components::Transform::State::UPDATE_WORLD)
                      ? components::Transform::State::UPDATE_WORLD
                      : state;

            if (state == components::Transform::State::UPDATE_WORLD) {
                worldMatrices_[globalIndex] = parentMatrix * matrix;
            }
        }
    }

    if constexpr (STAGE == easy_mp::value_cast(UpdateStage::LATE_UPDATE)) {
        auto view = entityManager.template getView<components::Transform>();

        for (auto&& [entity, transform] : view) {
            (void)entity;
            transform.state = components::Transform::State::UPDATE_NOTHING;
        }
    }
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

    auto it = std::upper_bound(transforms.begin(),
                               transforms.end(),
                               std::make_pair(depth, static_cast<uint64_t>(parentEntity)),
                               [](auto&& lhs, auto const& rhs) -> bool {
                                   auto&& [depth, parent] = lhs;
                                   return (depth < rhs.depth) ||
                                          (depth == rhs.depth && parent < static_cast<uint64_t>(rhs.parent));
                               });

    auto globalIndex = static_cast<size_t>(std::distance(transforms.begin(), it));

    auto& transform =
      entityManager.template assign<components::Transform>(entity, globalIndex, std::forward<Args>(args)...);

    transform.parent = parentEntity;
    transform.depth = depth;
    transform.globalIndex = globalIndex;

    for (auto cit = std::next(transforms.begin(), globalIndex + 1); cit != transforms.end(); cit++) {
        (*cit).globalIndex++;
    }

    _reserveVectorsIfNecessary(entityManager, globalIndex);

    worldMatrices_.insert(std::next(worldMatrices_.begin(), globalIndex), mat4{ 1.0 });

    return transform;
}

template<typename EntityManager>
void TransformSystem::destroy(EntityManager& entityManager, enttx::Entity const& entity)
{
    auto& transforms = entityManager.template getStorage<components::Transform>();

    auto const* transform = std::as_const(entityManager).template getComponent<components::Transform>(entity);

    auto globalIndex = transform->globalIndex;

    worldMatrices_.erase(std::next(worldMatrices_.begin(), globalIndex));

    for (auto cit = std::next(transforms.begin(), globalIndex + 1); cit != transforms.end(); cit++) {
        (*cit).globalIndex--;
    }

    entityManager.template destroy<components::Transform>(entity);
}

template<typename EntityManager>
void TransformSystem::_reserveVectorsIfNecessary(EntityManager& entityManager, size_t globalIndex)
{
    (void)entityManager;

    constexpr size_t chunkSize = EntityManager::template component_storage_t<components::Transform>::chunkSize;

    if (worldMatrices_.capacity() <= globalIndex) {
        auto capacity = globalIndex + 1;

        capacity = capacity + chunkSize - capacity % chunkSize;

        worldMatrices_.reserve(capacity);
    }
}
}

#endif // CYCLONITE_TRANSFORMSYSTEM_H
