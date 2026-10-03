//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "enttx/entityManager.h"
#include "resources/resourceGroupManager.h"
#include "rootBase.h"
#include "rootConfigTraits.h"
#include "systems/systemManager.h"
#include <boost/unordered/unordered_flat_map.hpp>
#include <stdexcept>

namespace cyclonite {
namespace internal {
struct DefaultConfig
{};
};

template<typename Config = internal::DefaultConfig>
class Root : public RootBase
{
public:
    using config_t = cyclonite::ConfigTraits<Config>;
    using enttx_config_t = enttx::internal::ConfigTraits<typename config_t::enttx_config_t>;

    Root();

    Root(Root const&) = delete;

    Root(Root&&) = delete;

    auto operator=(Root const&) -> Root& = delete;

    auto operator=(Root&&) -> Root& = delete;

    [[nodiscard]] auto systemManager() const -> systems::SystemManager<Config> const& { return systemManager_; }

    [[nodiscard]] auto systemManager() -> systems::SystemManager<Config>& { return systemManager_; }

    [[nodiscard]] auto getSceneEntities(core::ResourceSharedRef const& scene) const
      -> enttx::EntityManager<enttx_config_t> const&;

    [[nodiscard]] auto getSceneEntities(core::ResourceSharedRef const& scene) -> enttx::EntityManager<enttx_config_t>&;

private:
    systems::SystemManager<Config> systemManager_;
    boost::unordered_flat_map<uint64_t, enttx::EntityManager<enttx_config_t>> sceneEcs_;
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
  : RootBase{}
  , systemManager_{ *this }
  , sceneEcs_{}
  , resourceGroupManager_{ *this }
{
}

template<typename Config>
auto Root<Config>::getSceneEntities(core::ResourceSharedRef const& scene) const
  -> enttx::EntityManager<enttx_config_t> const&
{
    auto& res = scene.as<core::ResourceBase>();
    auto sceneId = static_cast<uint64_t>(res.resourceId());

    if (!sceneEcs_.contains(sceneId))
        throw std::invalid_argument("invalid scene");

    return sceneEcs_.at(sceneId);
}

template<typename Config>
auto Root<Config>::getSceneEntities(core::ResourceSharedRef const& scene) -> enttx::EntityManager<enttx_config_t>&
{
    return const_cast<enttx::EntityManager<Config>&>(std::as_const(*this)->getSceneEntities(scene));
}
} // cyclonite
#endif // CYCLONITE_ROOT_H
