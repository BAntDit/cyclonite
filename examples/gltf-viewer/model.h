//
// Created by bantdit on 11/2/20.
//

#ifndef CYCLONITE_MODEL_H
#define CYCLONITE_MODEL_H

#include "viewer.h"
#include <filesystem>

namespace examples::viewer {
class Model
{
private:
    using ecs_config_t = Viewer::ecs_config_t;

public:
    Model() noexcept;

    void init(cyclonite::vulkan::Device& device,
              ecs_config_t::entity_manager_t& entities,
              ecs_config_t::system_manager_t& systems,
              std::filesystem::path const& path);

    [[nodiscard]] auto camera() const -> enttx::Entity { return camera_; }

    void setCameraTransform(cyclonite::mat4 const& matrix);

private:
    ecs_config_t::entity_manager_t* entities_;
    enttx::Entity camera_;
};
}

#endif // CYCLONITE_MODEL_H
