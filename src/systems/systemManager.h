
#ifndef CYCLONITE_SYSTEM_MANAGER_H
#define CYCLONITE_SYSTEM_MANAGER_H

#include <tuple>
#include <array>
#include <future>
#include <metrix/type_list.h>
#include "core/resourceSharedRef.h"
#include "multithreading/utility.h"

namespace cyclonite::systems 
{
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
	using config_t = Config;
	using system_list_t = typename config_t::enttx_config_t;
	using system_update_stage_enum_t = typename config_t::system_update_stage_enum_t;
    static_assert(SystemUpdateStageListConcept<system_update_stage_enum_t>);

	static constexpr size_t system_update_stage_count_v = system_update_stage_enum_t::LAST_STAGE;

	template<typename S, typename R = void>
    using enable_if_system = std::enable_if_t<system_list_t ::template has_type<S>::value, R>;

public:
    SystemManager(Root<Config>& root) = default;

	template<typename System>
    [[nodiscard]] auto getSystem() const -> enable_if_system<System, System const&>;

    template<typename System>
    [[nodiscard]] auto getSystem() -> enable_if_system<System, System&>;

    template<typename... Args>
    void runSystems(core::ResourceSharedRef const& scene, Args&&... args);

private:
    template<size_t... Stages, typename... Args>
    void runSystemStages(std::index_sequence<Stages...>, core::ResourceSharedRef const& scene, Args&&... args);

    template<size_t... SystemIndex, typename... Args>
    auto runSystemStage(std::index_sequence<SystemIndex...>,
                        system_update_stage_enum_t stage,
                        core::ResourceSharedRef const& scene,
                        Args&&... args) -> std::shared_future<void>;

    Root<Config>* root_;
    internal::system_tuple_t systems_;
};

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
void SystemManager<Config>::runSystems(core::ResourceSharedRef const& scene, Args&&... args)
{
    runSystemStages(std::make_index_sequence<system_update_stage_count_v>{}, scene, std::forward<Args>(args)...);
}

template<typename Config>
template<size_t... Stages, typename... Args>
void SystemManager<Config>::runSystemStages(std::index_sequence<Stages...>,
                                            core::ResourceSharedRef const& scene,
                                            Args&&... args)
{
    (runSystemStage(
       std::make_index_sequence<system_list_t::size>{},
       static_cast<system_update_stage_enum_t>(static_cast<std::underlying_type_t<system_update_stage_enum_t>>(Stages)),
       scene,
       std::forward<Args>(args)...),
     ...);
}

template<typename Config>
template<size_t... SystemIndex, typename... Args>
auto SystemManager<Config>::runSystemStage(std::index_sequence<SystemIndex...>,
                                            system_update_stage_enum_t stage,
                                            core::ResourceSharedRef const& scene,
                                            Args&&... args) -> std::shared_future<void>
{
    return std::shared_future{ multithreading::when_all(
      std::get<SystemIndex>(systems_.systems).run(stage, scene, std::forward<Args>(args)...)...) }; // TODO:: move to template
}
}

#endif // CYCLONITE_SYSTEM_MANAGER_H
