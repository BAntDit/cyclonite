//
// Created by bantdit on 8/2/20.
//

#include "viewer.h"
#include "controller.h"
#include "model.h"
#include "view.h"
#include <chrono>

using namespace cyclonite;
using namespace easy_mp;

namespace examples::viewer {
Viewer::Viewer()
  : root_{ std::make_unique<cyclonite::Root>() }
  , entities_{}
  , systems_{ &entities_ }
  , model_{ nullptr }
  , view_{ nullptr }
  , controller_{ nullptr }
{}

auto Viewer::init(cyclonite::Options options) -> Viewer&
{
    options.parse([](auto&& layout) -> void {
        layout("help, h",
               "show help")("device-name", boost::program_options::value<std::string>(), "specifies device name");
    });

    auto deviceId =
      options.has("device-name") ? root_->getDeviceId(options.get<std::string>("device-name")) : root_->getDeviceId();

    root_->init(deviceId);

    view_ = std::make_unique<View>();
    view_->init(root_->device(), root_->taskManager(), systems_);

    model_ = std::make_unique<Model>();
    model_->init(root_->device(), entities_, systems_, "./scene.gltf");

    controller_ = std::make_unique<Controller>();
    controller_->init(root_->input(), systems_);

    return *this;
}

auto Viewer::run() -> Viewer&
{
    auto start = std::chrono::high_resolution_clock::now();

    while (controller_->alive()) {
        auto end = std::chrono::high_resolution_clock::now();

        auto dt = std::chrono::duration<real, std::ratio<1>>{ end - start }.count();

        start = end;

        controller_->update(*model_, dt);
        view_->draw(*model_);
    }

    return *this;
}

void Viewer::done()
{
    systems_.get<systems::RenderSystem>().finish();
}
}

CYCLONITE_APP(examples::viewer::Viewer)
