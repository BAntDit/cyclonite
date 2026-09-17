//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "gfx/instance.h"
#include "input.h"
#include "multithreading/taskManager.h"
#include "resources/resourceGroupManager.h"

#include "rootConfigTraits.h"
#include <memory>
#include <string_view>

namespace cyclonite {
class RootBase
{
public:
    struct Capabilities
    {
        std::vector<std::pair<uint16_t, uint16_t>> displayResolutions;
    };

public:
    RootBase(RootBase const&) = delete;

    RootBase(RootBase&&) = delete;

    ~RootBase() = default;

    auto operator=(RootBase const&) -> RootBase& = delete;

    auto operator=(RootBase&&) -> RootBase& = delete;

    void initTaskManager(bool dedicatedTransferRequired,
                         bool dedicatedComputeRequired,
                         size_t threadPoolSize = std::max(std::thread::hardware_concurrency(), 1u));

    [[nodiscard]] auto capabilities() const -> Capabilities const& { return capabilities_; }

    [[nodiscard]] auto input() const -> Input const& { return input_; }

    [[nodiscard]] auto input() -> Input& { return input_; }

    [[nodiscard]] auto gfxInstance() const -> gfx::Instance const& { return *gfxInstance_; }

    [[nodiscard]] auto gfxInstance() -> gfx::Instance& { return *gfxInstance_; }

    [[nodiscard]] auto taskManager() const -> multithreading::TaskManager const& { return *taskManager_; }

    [[nodiscard]] auto taskManager() -> multithreading::TaskManager& { return *taskManager_; }

    void init(std::string_view appName);

    void reset();

protected:
    RootBase();

    Capabilities capabilities_;
    std::unique_ptr<multithreading::TaskManager> taskManager_;
    std::unique_ptr<gfx::Instance> gfxInstance_;
    Input input_;
};

namespace internal {
struct DefaultConfig
{};
};

template<typename Config = internal::DefaultConfig>
class Root : public RootBase
{
public:
    using config_t = cyclonite::ConfigTraits<Config>;

    Root();

    Root(Root const&) = delete;

    Root(Root&&) = delete;

    auto operator=(Root const&) -> Root& = delete;

    auto operator=(Root&&) -> Root& = delete;

private:
    resources::ResourceGroupManager<Config> resourceGroupManager_;

public:
    [[nodiscard]] auto resourceManager() const -> resources::ResourceGroupManager<Config> const&
    {
        return resourceGroupManager_;
    }

    [[nodiscard]] auto resourceManager() -> resources::ResourceGroupManager<Config>& { return resourceGroupManager_; }
};

template<typename Config>
Root<Config>::Root()
  : resourceGroupManager_{ *this }
{
}
}
#endif // CYCLONITE_ROOT_H
