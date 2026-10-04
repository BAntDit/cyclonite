
#ifndef CYCLONITE_SYSTEM_MANAGER_H
#define CYCLONITE_SYSTEM_MANAGER_H

#include "core/resourceSharedRef.h"
#include "multithreading/utility.h"
#include "rootConfigTraits.h"
#include "stages.h"
#include <array>
#include <future>
#include <metrix/type_list.h>
#include <tuple>

namespace cyclonite::systems {
template<typename Config>
class Root;

namespace internal {
template<typename SystemList>
struct system_tuple_t;

template<typename... System>
struct system_tuple_t<metrix::type_list<System...>>
{
    std::tuple<System...> systems;
};
}

template<typename Config>
class SystemManager
{
public:
    using config_t = cyclonite::ConfigTraits<Config>;
    using system_list_t = typename config_t::system_list_t;
    using system_update_stage_enum_t = typename config_t::system_update_stage_enum_t;
    static_assert(SystemStageListConcept<system_update_stage_enum_t>);

    static constexpr size_t system_update_stage_count_v = system_update_stage_enum_t::LAST_STAGE;

    template<typename S, typename R = void>
    using enable_if_system = std::enable_if_t<system_list_t ::template has_type<S>::value, R>;

public:
    explicit SystemManager(Root<Config>& root);

    template<typename System>
    [[nodiscard]] auto getSystem() const -> enable_if_system<System, System const&>;

    template<typename System>
    [[nodiscard]] auto getSystem() -> enable_if_system<System, System&>;

    template<typename... Args>
    auto runSystems(core::ResourceSharedRef const& scene, Args&&... args) -> std::shared_future<void>;

private:
    template<size_t... Stages, typename... Args>
    auto runSystemStages(std::index_sequence<Stages...>,
                         core::ResourceSharedRef const& scene,
                         Args&&... args) -> std::shared_future<void>;

    template<size_t Stage, size_t... SystemIndex, typename... Args>
    auto runSystemStage(std::index_sequence<SystemIndex...>,
                        std::shared_future<void>& prevStageFuture,
                        Root<Config>& root,
                        core::ResourceSharedRef const& scene,
                        Args&&... args) -> std::future<void>;

    Root<Config>* root_;
    internal::system_tuple_t<system_list_t> systems_;
};

template<typename Config>
SystemManager<Config>::SystemManager(Root<Config>& root)
  : root_{ root }
  , systems_{}
{
}

template<typename Config>
template<typename System>
auto SystemManager<Config>::getSystem() const -> enable_if_system<System, System const&>
{
    return std::get<system_list_t::template get_type_index<System>::value>(systems_.systems);
}

template<typename Config>
template<typename System>
auto SystemManager<Config>::getSystem() -> enable_if_system<System, System&>
{
    return std::get<system_list_t::template get_type_index<System>::value>(systems_.systems);
}

template<typename Config>
template<typename... Args>
auto SystemManager<Config>::runSystems(core::ResourceSharedRef const& scene, Args&&... args) -> std::shared_future<void>
{
    return runSystemStages(std::make_index_sequence<system_update_stage_count_v>{}, scene, std::forward<Args>(args)...);
}

template<typename Config>
template<size_t... Stages, typename... Args>
auto SystemManager<Config>::runSystemStages(std::index_sequence<Stages...>,
                                            core::ResourceSharedRef const& scene,
                                            Args&&... args) -> std::shared_future<void>
{
    auto lastStageFuture = std::shared_future<void>{};

    ((lastStageFuture = runSystemStage<Stages>(
        std::make_index_sequence<system_list_t::size>{}, lastStageFuture, *root_, scene, std::forward<Args>(args)...)),
     ...);

    return lastStageFuture;
}

template<typename Config>
template<size_t Stage, size_t... SystemIndex, typename... Args>
auto SystemManager<Config>::runSystemStage(std::index_sequence<SystemIndex...>,
                                           std::shared_future<void>& prevStageFuture,
                                           Root<Config>& root,
                                           core::ResourceSharedRef const& scene,
                                           Args&&... args) -> std::future<void>
{
    return multithreading::when_all(
      std::get<SystemIndex>(systems_.systems)
        .template run<Stage>(root, prevStageFuture, scene, std::forward<Args>(args)...)...);
}
}

#endif // CYCLONITE_SYSTEM_MANAGER_H
