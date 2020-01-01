//
// Created by bantdit on 12/31/19.
//

#include "minimal.h"

namespace examples
{
Minimal::Minimal()
  : shutdown_{ false }
  , root_{ std::make_unique<cyclonite::Root<config_t>>() }
  , renderPass_{ nullptr }
{}

auto Minimal::init(cyclonite::Options const& options) -> Minimal& {
    auto const& windows = options.windows();

    auto const& mainWindow = windows[0];

    root_->init(options);

    root_->input().keyDown += cyclonite::Event<SDL_KeyboardEvent>::EventHandler(this, &Minimal::onKeyDown);

    renderPass_ = std::make_unique<cyclonite::RenderPass>(
      root_->device(),
      mainWindow,
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

    // TODO:: ...

    return *this;
}

auto Minimal::run() -> Minimal& {
    while(!shutdown_) {
        root_->input().pollEvent();

        // TODO:: ...
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

