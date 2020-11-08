//
// Created by bantdit on 8/2/20.
//

#include "viewer.h"
#include "controller.h"
#include "model.h"
#include "view.h"

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
    view_->init(root_->device(), root_->taskManager(), entities_, systems_);

    model_ = std::make_unique<Model>();
    model_->init(root_->device(), entities_, systems_, "./scene.gltf");

    controller_ = std::make_unique<Controller>();
    controller_->init(root_->input(), systems_);

    return *this;
}

auto Viewer::run() -> Viewer&
{
    while (controller_->alive()) {
        controller_->update(*model_, 0.f);
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
