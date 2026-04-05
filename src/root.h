//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "gfx/instance.h"
#include "input.h"
#include "multithreading/taskManager.h"
#include "resources/resourceGroupManager.h"

#include <memory>
#include <string_view>

#include "rootConfigTraits.h"

// TODO:: add root config

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

    void init(std::string_view appName);

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

template<typename Config = ConfigTraits<internal::DefaultConfig>>
class Root : public RootBase
{
public:
    using config_t = Config;
    using resource_type_list_t = metrix::distinct<
      metrix::concat<typename config_t::custom_resource_type_list_t, metrix::type_list<Shader>>>::type;

    Root() = default;

private:
    template<typename... TypeList>
    struct resource_manager_wrap_t;

    template<typename... Resources>
    struct resource_manager_wrap_t<metrix::type_list<Resources...>> {
        resources::ResourceGroupManager<Resources...> manager_;
    };

    resource_manager_wrap_t<resource_type_list_t> resourceManager_;

public:
    [[nodiscard]] auto resourceManager() const -> decltype(auto) { return (resourceManager_.manager_); }
    [[nodiscard]] auto resourceManager() -> decltype(auto) { return (resourceManager_.manager_); }
};
}
#endif // CYCLONITE_ROOT_H
