//
// Created by bantdit on 11/8/20.
//

#ifndef CYCLONITE_CONTROLLER_H
#define CYCLONITE_CONTROLLER_H

#include "viewer.h"

namespace examples::viewer {
class Model;

class Controller final : public cyclonite::EventReceivable
{
private:
    using ecs_config_t = Viewer::ecs_config_t;

public:
    Controller() noexcept;

    void init(cyclonite::Input& input, ecs_config_t::system_manager_t& systems);

    void onQuit();

    void onKeyDown(SDL_Keycode keyCode, uint16_t mod);

    void onMouseButtonDown(uint8_t button, uint8_t clicks, int32_t x, int32_t y);

    void onMouseButtonUp(uint8_t button, int32_t x, int32_t y);

    void onMouseMotion(int32_t x, int32_t y);

    void onMouseWheel(int32_t y);

    [[nodiscard]] auto alive() const -> bool { return !shutdown_; }

    void update(Model& model, cyclonite::real dt);

private:
    cyclonite::Input* input_;
    ecs_config_t::system_manager_t* systems_;
    bool shutdown_;
    bool isInRotation_;
    cyclonite::vec2 rotationStart_;
    cyclonite::vec2 rotate_;
    cyclonite::real scroll_;
    cyclonite::real distance_;
    cyclonite::real minDistance_;
    cyclonite::real maxDistance_;
    cyclonite::real dumping_;
    cyclonite::real azimuth_;
    cyclonite::real polar_;
    cyclonite::vec3 target_;
};
}

#endif // CYCLONITE_CONTROLLER_H
