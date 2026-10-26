//
// Created by anton on 10/26/26.
//

#include "transformSystem.h"

namespace cyclonite::systems {
/*static */ void TransformSystem::_updateTransform(components::Transform const& parent,
                                                   components::Transform& transform)
{
    if (transform.state() == components::Transform::State::UPDATE_LOCAL) {
        transform.setMatrix(glm::translate(transform.position()) * glm::mat4_cast(transform.orientation()) *
                            glm::scale(transform.scale()));
        transform.state() = components::Transform::State::UPDATE_WORLD;
    }

    if (transform.state() == components::Transform::State::UPDATE_COMPONENTS) {
        [[maybe_unused]] auto skew = vec3{};
        [[maybe_unused]] auto perspective = vec4{};

        auto scale = vec3{};
        auto position = vec3{};
        auto rotation = quat{};

        [[maybe_unused]] bool success =
          glm::decompose(transform.matrix(), scale, rotation, position, skew, perspective);
        assert(success);

        transform.setScale(scale);
        transform.setPosition(position);
        transform.setOrientation(rotation);

        transform.state() = components::Transform::State::UPDATE_WORLD;
    }

    if (transform.state() == components::Transform::State::UPDATE_WORLD ||
        parent.state() == components::Transform::State::UPDATE_CHILDREN) {
        transform.setWorldMatrix(parent.worldMatrix() * transform.matrix());
    }
}
}
