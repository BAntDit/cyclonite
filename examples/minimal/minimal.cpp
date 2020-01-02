//
// Created by bantdit on 12/31/19.
//

#include "minimal.h"

namespace examples
{
Minimal::Minimal()
  : shutdown_{ false }
  , root_{ std::make_unique<cyclonite::Root<config_t>>() }
{}

auto Minimal::init(cyclonite::Options const& options) -> Minimal& {
    root_->init(options);

    root_->input().keyDown += cyclonite::Event<SDL_KeyboardEvent>::EventHandler(this, &Minimal::onKeyDown);

    return *this;
}

auto Minimal::run() -> Minimal& {
    cyclonite::Options::WindowProperties windowProperties{};

    windowProperties.title = "minimal.example";
    windowProperties.fullscreen = false;
    windowProperties.left = 0x1FFF;
    windowProperties.top = 0x1FFF;
    windowProperties.width = 512;
    windowProperties.height = 512;

    cyclonite::VulkanRenderer vulkanRenderer{ root_->device() };

    cyclonite::RenderPass renderPass{ root_->device(), windowProperties };

    while(!shutdown_) {
        root_->input().pollEvent();

        vulkanRenderer.renderOneFrame(renderPass);
    }

    return *this;
}

void Minimal::done() {}

void Minimal::onKeyDown(SDL_KeyboardEvent event) {
    if (event.keysym.sym == SDLK_ESCAPE)
        shutdown_ = true;
}
}

CYCLONITE_APP(examples::Minimal)

