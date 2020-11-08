//
// Created by bantdit on 11/2/20.
//

#ifndef CYCLONITE_VIEW_H
#define CYCLONITE_VIEW_H

#include "viewer.h"

namespace examples::viewer {
class Model;

class View
{
private:
    using ecs_config_t = Viewer::ecs_config_t;

public:
    View() noexcept;

    void init(cyclonite::vulkan::Device& device,
              cyclonite::multithreading::TaskManager& taskManager,
              ecs_config_t::entity_manager_t& entities,
              ecs_config_t::system_manager_t& systems);

    void draw(Model const& model);

private:
    ecs_config_t::system_manager_t* systems_;
};
}

#endif // CYCLONITE_VIEW_H
