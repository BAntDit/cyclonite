//
// Created by bantdit on 11/8/20.
//

#include "controller.h"
#include "model.h"
#include <boost/math/constants/constants.hpp>

namespace examples::viewer {
using namespace cyclonite;

constexpr static real pi = boost::math::constants::pi<real>();

Controller::Controller() noexcept
  : input_{ nullptr }
  , systems_{ nullptr }
  , shutdown_{ false }
  , isInRotation_{ false }
  , rotationStart_{}
  , rotate_{}
  , scroll_{ 0.f }
  , distance_{ 0.f }
  , minDistance_{ .5f }
  , maxDistance_{ 10.f }
  , dumping_{ .9f }
  , azimuth_{ 0.f }
  , polar_{}
  , target_{ 0.f }
{}

void Controller::init(cyclonite::Input& input, ecs_config_t::system_manager_t& systems)
{
    input_ = &input;
    systems_ = &systems;

    input.quit += Event<>::EventHandler(this, &Controller::onQuit);
    input.keyDown += Event<SDL_Keycode, uint16_t>::EventHandler(this, &Controller::onKeyDown);
    input.mouseButtonDown +=
      Event<uint8_t, uint8_t, int32_t, int32_t>::EventHandler(this, &Controller::onMouseButtonDown);
    input.mouseButtonUp += Event<uint8_t, int32_t, int32_t>::EventHandler(this, &Controller::onMouseButtonUp);
    input.mouseMotion += Event<int32_t, int32_t>::EventHandler(this, &Controller::onMouseMotion);
    input.mouseWheel += Event<int32_t>::EventHandler(this, &Controller::onMouseWheel);
}

void Controller::onQuit()
{
    shutdown_ = true;
}

void Controller::onKeyDown(SDL_Keycode keyCode, uint16_t mod)
{
    (void)mod;

    if (keyCode == SDLK_ESCAPE)
        shutdown_ = true;
}

void Controller::onMouseButtonDown(uint8_t button, uint8_t clicks, int32_t x, int32_t y)
{
    if (button == SDL_BUTTON_LEFT && clicks == 1) {
        isInRotation_ = true;
        rotationStart_.x = x;
        rotationStart_.y = y;
    }
}

void Controller::onMouseButtonUp(uint8_t button, int32_t x, int32_t y)
{
    (void)button;
    (void)x;
    (void)y;

    isInRotation_ = false;
}

void Controller::onMouseMotion(int32_t x, int32_t y)
{
    if (isInRotation_) {
        auto& renderSystem = systems_->get<systems::RenderSystem>();
        auto& renderPass = renderSystem.renderPass();
        auto [left, top, width, height] = renderPass.viewport();
        auto speed = 30.f;

        (void)left;
        (void)top;

        rotate_.x = pi * 2.f * speed * (static_cast<real>(x) - rotationStart_.x) / static_cast<real>(width);
        rotate_.y = pi * 2.f * speed * (static_cast<real>(y) - rotationStart_.y) / static_cast<real>(height);

        rotationStart_.x = x;
        rotationStart_.y = y;
    }
}

void Controller::onMouseWheel(int32_t y)
{
    if (y > 0)
        scroll_ -= 0.05f;
    else if (y < 0)
        scroll_ += 0.05f;
}

void Controller::update(Model& model, real dt)
{
    input_->pollEvent();

    distance_ += scroll_;
    distance_ = std::max(minDistance_, std::min(maxDistance_, distance_));

    scroll_ -= scroll_ * (1.f - dumping_);
    scroll_ = abs(scroll_) < 0.1e-3f ? 0.0f : scroll_;

    azimuth_ += rotate_.x * dt;
    polar_ += rotate_.y * dt;

    rotate_.x -= rotate_.x * (1.f - dumping_);
    rotate_.y -= rotate_.y * (1.f - dumping_);

    polar_ = std::max((pi / 2.f), std::min(pi - 0.1f, polar_));

    auto pos = vec3{ distance_ * sinf(polar_) * cosf(azimuth_),
                     distance_ * cosf(polar_),
                     distance_ * sinf(azimuth_) * sinf(polar_) };

    auto up = vec3{ .0f, 1.f, 0.f };
    auto fw = glm::normalize(-pos);
    auto lf = glm::normalize(glm::cross(up, fw));

    up = glm::normalize(glm::cross(fw, lf));

    model.setCameraTransform(mat4{ glm::vec4{ lf.x, lf.y, lf.z, 0.0f },
                                   glm::vec4{ up.x, up.y, up.z, 0.0f },
                                   glm::vec4{ fw.x, fw.y, fw.z, 0.0f },
                                   glm::vec4{ target_.x - pos.x, target_.y - pos.y, target_.z - pos.z, 1.0f } });
}
}
