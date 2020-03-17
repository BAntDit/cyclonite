//
// Created by bantdit on 12/28/19.
//

#ifndef CYCLONITE_MINIMAL_H
#define CYCLONITE_MINIMAL_H

#include "cyclonite.h"

namespace examples
{
class Minimal final: public cyclonite::BaseApp<Minimal>, public cyclonite::EventReceivable
{
public:
    Minimal();

    auto init(cyclonite::Options const& options) -> Minimal&;

    auto run() -> Minimal&;

    void done();

    void onKeyDown(SDL_Keycode keyCode, uint16_t mod);

private:
    bool shutdown_;
    std::unique_ptr<cyclonite::Root> root_;
    ecs_config_t::entity_manager_t entities_;
    ecs_config_t::system_manager_t systems_;
    enttx::Entity cameraEntity_;
};
}

#endif //CYCLONITE_MINIMAL_H
