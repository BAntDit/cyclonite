//
// Created by anton on 10/18/26.
//

#include "scene.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace cyclonite {
/*static */ auto Scene::getSceneAsset(std::shared_ptr<shared::AssetMainBlock> const& asset,
                                      boost::uuids::uuid const& uuid) -> shared::AssetScene const&
{
    auto uuidStr = boost::uuids::to_string(uuid);

    auto it = std::find_if(asset->scenes.cbegin(), asset->scenes.cend(), [&uuidStr](auto& assetScene) -> bool {
        return assetScene.uuid == uuidStr;
    });

    if (it == asset->scenes.cend()) {
        throw std::runtime_error(std::format("necessary scene is not presented in asset: {0}", uuidStr));
    }

    return *it;
}

/*static */ auto Scene::computeNodeCount(std::shared_ptr<shared::AssetMainBlock> const& asset,
                                         shared::AssetScene const& assetScene) -> size_t
{
    auto count = size_t{ 1 }; // main root

    count += assetScene.rootNodes.size();

    for (auto rootIdx : assetScene.rootNodes) {
        assert(rootIdx < asset->nodes.size());
        auto& rootAssetNode = asset->nodes[rootIdx];

        count += computeChildrenNodeCount(asset, rootAssetNode);
    }

    return count;
}

/*static */ auto Scene::computeChildrenNodeCount(std::shared_ptr<shared::AssetMainBlock> const& asset,
                                                 shared::AssetNode const& assetNode) -> size_t
{
    auto count = assetNode.children.size();

    for (auto childIdx : assetNode.children) {
        assert(childIdx < asset->nodes.size());
        auto& childAssetNode = asset->nodes[childIdx];

        count += computeChildrenNodeCount(asset, childAssetNode);
    }

    return count;
}
}
