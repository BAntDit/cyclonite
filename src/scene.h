
#ifndef CYCLONITE_SCENE_H
#define CYCLONITE_SCENE_H

#include "assetModuleBinary.h"
#include "common.h"
#include "components/transform.h"
#include "core/resourceBase.h"
#include "enttx/entityManager.h"
#include "resources/managedResource.h"
#include "root.h"
#include <glm/gtc/type_ptr.hpp>

namespace cyclonite {
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
    template<typename EcsType>
    void addNode(EcsType& ecs,
                 enttx::Entity parent,
                 std::shared_ptr<shared::AssetMainBlock> const& asset,
                 shared::AssetNode const& assetNode);

    static auto getSceneAsset(std::shared_ptr<shared::AssetMainBlock> const& asset, boost::uuids::uuid const& uuid)
      -> shared::AssetScene const&;

    static auto computeNodeCount(std::shared_ptr<shared::AssetMainBlock> const& asset,
                                 shared::AssetScene const& assetScene) -> size_t;

    static auto computeChildrenNodeCount(std::shared_ptr<shared::AssetMainBlock> const& asset,
                                         shared::AssetNode const& assetNode) -> size_t;
};

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

template<typename EcsType>
void Scene::addNode(EcsType& ecs,
                    enttx::Entity parent,
                    std::shared_ptr<shared::AssetMainBlock> const& asset,
                    shared::AssetNode const& assetNode)
{
    auto entity = ecs.create();

    if (assetNode.transformType == shared::AssetTransformType::Matrix) {
        auto matrix = glm::mat4{ glm::transpose(glm::make_mat3x4(assetNode.transform.data())) };

        ecs.template assign<components::Transform>(entity, parent, matrix);
    } else if (assetNode.transformType == shared::AssetTransformType::Components) {
        auto position = vec3{ assetNode.transform[0], assetNode.transform[1], assetNode.transform[2] };
        auto rotation =
          quat{ assetNode.transform[3], assetNode.transform[4], assetNode.transform[5], assetNode.transform[6] };
        auto scale = vec3{ assetNode.transform[7], assetNode.transform[8], assetNode.transform[9] };

        ecs.template assign<components::Transform>(entity, parent, position, scale, rotation);
    }

    if (assetNode.mesh != std::numeric_limits<uint32_t>::max()) {
        assert(assetNode.mesh < asset->meshes.size());
        auto const& assetMesh = asset->meshes[assetNode.mesh];

        // TODO::
    }

    // TODO:: add meta data component

    for (auto child : assetNode.children) {
        assert(child < asset->nodes.size());
        auto const& childAssetNode = asset->nodes[child];

        addNode(ecs, entity, asset, childAssetNode);
    }
}

template<typename Config>
void Scene::loadImpl(std::pair<Root<Config>*, std::shared_ptr<shared::AssetMainBlock>> const& rootAndAsset)
{
    using enttx_config_t = Root<Config>::enttx_config_t;

    auto [root, asset] = rootAndAsset;

    auto& sceneAsset = getSceneAsset(asset, uuid());

    auto nodeCount = computeNodeCount(asset, sceneAsset);

    auto [it, success] = root.sceneEcs_.insert(
      std::pair{ static_cast<uint64_t>(resourceId()), enttx::EntityManager<enttx_config_t>{ nodeCount } });
    assert(success);

    auto& [_, ecs] = *it;

    auto rootEntity = ecs.create();
    ecs.template assign<components::Transform>(rootEntity);

    for (auto rootNode : sceneAsset.rootNodes) {
        auto& assetNode = asset->nodes[rootNode];

        addNode(ecs, rootEntity, asset, assetNode);
    }
}
}

#endif // CYCLONITE_SCENE_H
