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

    void init(std::shared_ptr<cyclonite::compositor::Workspace> const& workspace);

    [[nodiscard]] auto timeSinceLastUpdate() const -> cyclonite::real { return timeSinceLastUpdate_; }

    auto timeSinceLastUpdate() -> cyclonite::real& { return timeSinceLastUpdate_; }

private:
    cyclonite::real timeSinceLastUpdate_;
    std::shared_ptr<cyclonite::compositor::Workspace> workspace_;
};
}

#endif // CYCLONITE_MODEL_H
