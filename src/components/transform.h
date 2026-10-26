//
// Created by anton on 6/7/26.
//

#ifndef CYCLONITE_COMPONENTS_TRANSFORM_H
#define CYCLONITE_COMPONENTS_TRANSFORM_H
#include "common.h"
#include "enttx/entity.h"
#include <cstdint>

namespace cyclonite::components {
class TransformStorage;

class Transform
{
public:
    friend class TransformStorage;

    enum class State : uint8_t
    {
        UPDATE_NOTHING = 0,
        UPDATE_LOCAL = 1,
        UPDATE_COMPONENTS = 2,
        UPDATE_WORLD = 3,
        UPDATE_CHILDREN = 4,
        MIN_VALUE = UPDATE_NOTHING,
        MAX_VALUE = UPDATE_CHILDREN,
        COUNT = 5
    };

    Transform();

    Transform(enttx::Entity parent, uint32_t level);

    Transform(enttx::Entity parent, uint32_t level, vec3 const& pos, vec3 const& sc, quat const& rot);

    explicit Transform(enttx::Entity parent, uint32_t level, mat4 mat);

    [[nodiscard]] auto parent() const -> enttx::Entity { return parent_; }

    [[nodiscard]] auto position() const -> vec3 const& { return position_; }

    [[nodiscard]] auto scale() const -> vec3 const& { return scale_; }

    [[nodiscard]] auto orientation() const -> quat const& { return orientation_; }

    [[nodiscard]] auto level() const -> uint32_t { return level_; }

    [[nodiscard]] auto state() const -> State { return state_; }

    [[nodiscard]] auto state() -> State& { return state_; }

    [[nodiscard]] auto matrix() const -> mat4 const& { return matrix_; }

    [[nodiscard]] auto worldMatrix() const -> mat4 const& { return worldMatrix_; }

    void setWorldMatrix(mat4 const& worldMatrix);

    void setPosition(vec3 const& pos);

    void setScale(vec3 const& sc);

    void setOrientation(quat const& rotation);

    void setMatrix(mat4 const& mat);

private:
    enttx::Entity parent_;

    glm::vec3 position_;
    glm::vec3 scale_;
    quat orientation_;

    mat4 matrix_;
    mat4 worldMatrix_;

    uint32_t level_;

    State state_;
};
}

#endif // CYCLONITE_COMPONENTS_TRANSFORM_H