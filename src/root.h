//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "gfx/instance.h"
#include "input.h"
#include "multithreading/taskManager.h"
#include "resources/resourceGroupManager.h"

#include "material.h"
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
    using resource_type_list_t =
      metrix::distinct<typename metrix::concat<typename config_t::custom_resource_type_list_t,
                                               metrix::type_list<Shader, Material>>::type>::type;

    Root() = default;

    void initResourceManager(core::ResourceSharedRef const& deviceRef);

private:
    template<typename... TypeList>
    struct resource_manager_wrap_t;

    template<typename... Resources>
    struct resource_manager_wrap_t<metrix::type_list<Resources...>>
    {
        explicit resource_manager_wrap_t(core::ResourceSharedRef const& deviceRef)
          : manager_{ deviceRef }
        {
        }

        resources::ResourceGroupManager<Resources...> manager_;
    };

    std::unique_ptr<resource_manager_wrap_t<resource_type_list_t>> resourceManager_;

public:
    [[nodiscard]] auto resourceManager() const -> decltype(auto) { return (resourceManager_.manager_); }
    [[nodiscard]] auto resourceManager() -> decltype(auto) { return (resourceManager_.manager_); }
};

template<typename Config>
void Root<Config>::initResourceManager(core::ResourceSharedRef const& deviceRef)
{
    resourceManager_ = std::make_unique<resource_manager_wrap_t<resource_type_list_t>>(deviceRef);
}
}
#endif // CYCLONITE_ROOT_H
