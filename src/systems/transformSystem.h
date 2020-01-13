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
#include <boost/dynamic_bitset.hpp>

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

private:
    static const size_t needsUpdateTransformComponentsBit = 1;
    static const size_t needsUpdateLocalMatrixBit = 2;
    static const size_t needsUpdateWorldMatrixBit = 3;

    std::vector<mat4> worldMatrices_;
    boost::dynamic_bitset<> updateStatus_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE>
void TransformSystem::update(SystemManager& systemManager, EntityManager& entityManager)
{
    if constexpr (STAGE == easy_mp::value_cast(UpdateStage::EARLY_UPDATE)) {
        auto view = entityManager.template getView<components::Transform>();

        for (auto& [entity, transform] : view) {
            (void)entity;

            auto& [position, scale, orientation, matrix, flags, globalIndex, parentIndex] = transform;

            auto const& parentMatrix =
              parentIndex != std::numeric_limits<size_t>::max() ? worldMatrices_[parentIndex] : mat4{ 1.0f };

            assert(!(flags.test(needsUpdateTransformComponentsBit) && flags.test(needsUpdateLocalMatrixBit)));

            if (flags.test(needsUpdateLocalMatrixBit)) {
                matrix = glm::translate(position) * glm::mat4_cast(orientation) * glm::scale(scale);
            }

            if (flags.test(needsUpdateTransformComponentsBit)) {
                // TODO:: ...
            }

            if (flags.test(needsUpdateWorldMatrixBit) || updateStatus_.test(parentIndex)) {
                assert(worldMatrices_.size() < globalIndex);

                worldMatrices_[globalIndex] = parentMatrix * matrix;

                updateStatus_.set(globalIndex);
            } else {
                updateStatus_.reset(globalIndex);
            }

            flags.reset();
        }
    }
}
}

#endif // CYCLONITE_TRANSFORMSYSTEM_H
