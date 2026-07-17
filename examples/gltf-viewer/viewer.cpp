//
// Created by bantdit on 8/2/20.
//

#include "viewer.h"
#include "appConfig.h"
#include "controller.h"
#include "model.h"
#include "view.h"

using namespace cyclonite;
using namespace metrix;

namespace examples::viewer {

Viewer::Viewer()
  : root_{ std::make_unique<cyclonite::Root>() }
  , model_{ nullptr }
{
}

auto Viewer::init(cyclonite::Options options) -> Viewer&
{
    options.parse([](auto&& layout) -> void {
        layout("help, h",
               "show help")("device-name, d", boost::program_options::value<std::string>(), "specifies device name");
    });

    auto deviceId = uint32_t{ 0 };

    root_->init(deviceId);

    uint32_t width = 1024;
    uint32_t height = 768;

    model_ = std::make_unique<Model>();
    // model_->init(*root_, "./BoxAnimated.gltf", workspace);

    view_ = std::make_unique<View>();
    // view_->init(root_->taskManager(), root_->device(), workspace);

    controller_ = std::make_unique<Controller>();
    controller_->init(root_->input(), width, height);

    return *this;
}

auto Viewer::run() -> Viewer&
{
    /*auto mainTask = [this]() -> void {
        auto start = std::chrono::high_resolution_clock::now();

        while (controller_->alive()) {
            auto end = std::chrono::high_resolution_clock::now();

            auto dt = std::chrono::duration<real, std::ratio<1>>{ end - start }.count();

            start = end;

            controller_->update(*model_, dt);

            view_->draw(root_->device());

            if (auto e = root_->taskManager().getLastException()) {
                std::rethrow_exception(e);
            }
        }
    };

    auto future = root_->taskManager().start(mainTask);
    future.get();*/

    return *this;
}

void Viewer::done()
{
    view_->dispose();
    model_->dispose();
    root_->reset();
}
}

CYCLONITE_APP(examples::viewer::Viewer)
