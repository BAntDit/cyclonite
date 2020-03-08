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
    static const size_t needsUpdateTransformComponentsBit = 0;
    static const size_t needsUpdateLocalMatrixBit = 1;
    static const size_t needsUpdateWorldMatrixBit = 2;

    std::vector<mat4> worldMatrices_;
    std::vector<uint8_t> updateStatus_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void TransformSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    ((void)args, ...);

    (void)systemManager;

    if constexpr (STAGE == easy_mp::value_cast(UpdateStage::EARLY_UPDATE)) {
        auto view = entityManager.template getView<components::Transform>();

        for (auto&& [entity, transform] : view) {
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

            assert(worldMatrices_.size() > globalIndex);

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
    auto parentIndex = parentTransform == nullptr ? std::numeric_limits<size_t>::max() : parentTransform->globalIndex;

    auto it = std::upper_bound(transforms.begin(),
                               transforms.end(),
                               std::make_pair(depth, parentIndex),
                               [](auto&& lhs, auto const& rhs) -> bool {
                                   auto&& [depth, parentIndex] = lhs;
                                   return (depth < rhs.depth) || (depth == rhs.depth && parentIndex < rhs.parentIndex);
                               });

    auto globalIndex = static_cast<size_t>(std::distance(transforms.begin(), it));

    auto& transform =
      entityManager.template assign<components::Transform>(entity, globalIndex, std::forward<Args>(args)...);

    transform.depth = depth;
    transform.globalIndex = globalIndex;
    transform.parentIndex =
      parentTransform == nullptr ? std::numeric_limits<size_t>::max() : parentTransform->globalIndex;

    for (auto cit = std::next(transforms.begin(), globalIndex + 1); cit != transforms.end(); cit++) {
        (*cit).globalIndex++;

        if ((*cit).parentIndex >= globalIndex) {
            (*cit).parentIndex++;
        }
    }

    _reserveVectorsIfNecessary(entityManager, globalIndex);

    worldMatrices_.insert(std::next(worldMatrices_.begin(), globalIndex), mat4{ 1.0 });
    updateStatus_.insert(std::next(updateStatus_.begin(), globalIndex), 0);

    return transform;
}

template<typename EntityManager>
void TransformSystem::destroy(EntityManager& entityManager, enttx::Entity const& entity)
{
    auto& transforms = entityManager.template getStorage<components::Transform>();

    auto const* transform = std::as_const(entityManager).template getComponent<components::Transform>(entity);

    auto globalIndex = transform->globalIndex;

    worldMatrices_.erase(std::next(worldMatrices_.begin(), globalIndex));
    updateStatus_.erase(std::next(updateStatus_.begin(), globalIndex));

    for (auto cit = std::next(transforms.begin(), globalIndex + 1); cit != transforms.end(); cit++) {
        (*cit).globalIndex--;

        assert((*cit).parentIndex != globalIndex); // it has no children to this moment

        if ((*cit).parentIndex > globalIndex) {
            (*cit).parentIndex--;
        }
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
        updateStatus_.reserve(capacity);
    }
}
}

#endif // CYCLONITE_TRANSFORMSYSTEM_H
