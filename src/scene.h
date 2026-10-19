
#ifndef CYCLONITE_SCENE_H
#define CYCLONITE_SCENE_H

#include "assetModuleBinary.h"
#include "core/resourceBase.h"
#include "resources/managedResource.h"

namespace cyclonite {
template<typename Config>
class Root;

class Scene
  : public core::ResourceBase
  , public resources::ManagedResource<cyclonite::Scene>
{
public:
    template<typename Config>
    Scene(core::ResourceManagerBase* resourceManager,
          core::ResourceId resourceId,
          resources::ResourceGroupBase* resourceGroup,
          std::string_view name,
          boost::uuids::uuid const& uuid);

    template<typename Config>
    void loadImpl(std::pair<Root<Config>*, std::shared_ptr<shared::AssetMainBlock>> const& rootAndAsset);

private:
    static auto getSceneAsset(std::shared_ptr<shared::AssetMainBlock> const& asset, boost::uuids::uuid const& uuid)
      -> shared::AssetScene const&;

    static auto computeNodeCount(std::shared_ptr<shared::AssetMainBlock> const& asset,
                                 shared::AssetScene const& assetScene) -> size_t;

    static auto computeChildrenNodeCount(std::shared_ptr<shared::AssetMainBlock> const& asset,
                                         shared::AssetNode const& assetNode) -> size_t;
};
}

#ifdef CYCLONITE_SCENE_IMPLEMENTATION
#include "enttx/entityManager.h"
#include "root.h"

namespace cyclonite {
template<typename Config>
Scene::Scene(core::ResourceManagerBase* resourceManager,
             core::ResourceId resourceId,
             resources::ResourceGroupBase* resourceGroup,
             std::string_view name,
             boost::uuids::uuid const& uuid)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Scene>{ resourceGroup, name, uuid }
{
}

template<typename Config>
void Scene::loadImpl(std::pair<Root<Config>*, std::shared_ptr<shared::AssetMainBlock>> const& rootAndAsset)
{
    auto [root, asset] = rootAndAsset;

    auto& sceneAsset = getSceneAsset(asset, uuid());

    auto nodeCount = computeNodeCount(asset, sceneAsset);
 
    auto [it, success] = root.sceneEcs_.insert(
      std::pair{ static_cast<uint64_t>(resourceId), enttx::EntityManager<Config>{ nodeCount } });
    assert(success);

    auto& [_, ecs] = *it;

    // TODO:: 
}
}
#endif
#endif // CYCLONITE_SCENE_H
