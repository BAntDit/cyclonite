//
// Created by bantdit on 12/28/19.
//

#ifndef CYCLONITE_BASIC_H
#define CYCLONITE_BASIC_H

#include "cyclonite.h"

using namespace cyclonite;

namespace examples
{
class Basic final: public BaseApp<Basic>, public EventReceivable
{
public:
    Basic();

    auto init(Options const& options) -> Basic&;

    auto run() -> Basic&;

    void done();

    void onKeyDown(SDL_Keycode keyCode, uint16_t mod);

private:
    bool shutdown_;
    std::unique_ptr<Root> root_;
    std::unique_ptr<RenderPass> renderPass_;
    std::unique_ptr<VulkanRenderer> renderer_;
    ecs_config_t::entity_manager_t entities_;
    ecs_config_t::system_manager_t systems_;
    enttx::Entity cameraEntity_;
};
}

#endif // CYCLONITE_BASIC_H
