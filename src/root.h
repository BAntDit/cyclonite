//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "config.h"
#include "gfx/instance.h"
#include "input.h"
#include "multithreading/taskManager.h"

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

    auto operator=(Root&&) -> Root& = delete;

    void init();

    void init(uint32_t deviceId);

    // [[nodiscard]] auto getDeviceCount() const -> size_t { return physicalDeviceList_.size(); }

    [[nodiscard]] auto capabilities() const -> Capabilities const& { return capabilities_; }

    [[nodiscard]] auto input() const -> Input const& { return input_; }

    [[nodiscard]] auto input() -> Input& { return input_; }

    void reset();

private:
    Capabilities capabilities_;
    std::unique_ptr<gfx::Instance> vulkanInstance_;
    Input input_;
};
}
#endif // CYCLONITE_ROOT_H
