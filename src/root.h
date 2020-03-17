//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "config.h"
#include "input.h"
#include "multithreading/taskManager.h"
#include "options.h"
#include "platform.h"
#include "sdl/sdlSupport.h"
#include "surface.h"
#include "vulkan/device.h"
#include "vulkan/instance.h"

#include <iostream>
#include <memory>

namespace cyclonite {
class Root
{
public:
    struct Capabilities
    {
        std::vector<std::pair<uint16_t, uint16_t>> displayResolutions;
    };

public:
    Root();

    Root(Root const&) = delete;

    Root(Root&&) = delete;

    ~Root() = default;

    auto operator=(Root const&) -> Root& = delete;

    auto operator=(Root &&) -> Root& = delete;

    void init(Options const& options);

    [[nodiscard]] auto capabilities() const -> Capabilities const& { return capabilities_; }

    [[nodiscard]] auto input() const -> Input const& { return input_; }

    [[nodiscard]] auto input() -> Input& { return input_; }

    [[nodiscard]] auto device() const -> vulkan::Device const& { return *vulkanDevice_; }

    [[nodiscard]] auto device() -> vulkan::Device& { return *vulkanDevice_; }

    [[nodiscard]] auto taskManager() -> multithreading::TaskManager& { return taskManager_; }

    [[nodiscard]] auto taskManager() const -> multithreading::TaskManager const& { return taskManager_; }

private:
    Capabilities capabilities_;
    multithreading::TaskManager taskManager_;
    sdl::SDLSupport sdlSupport_;
    std::unique_ptr<vulkan::Instance> vulkanInstance_;
    std::unique_ptr<vulkan::Device> vulkanDevice_;
    Input input_;
};
}
#endif // CYCLONITE_ROOT_H
