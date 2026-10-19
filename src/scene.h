
#ifndef CYCLONITE_SCENE_H
#define CYCLONITE_SCENE_H

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
          boost::uuids::uuid const& uuid,
          size_t initialCapacity,
          Root<Config>& root);
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
             boost::uuids::uuid const& uuid,
             size_t initialCapacity,
             Root<Config>& root)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Scene>{ resourceGroup, name, uuid }
{
    auto [_, success] = root.sceneEcs_.insert(
      std::pair{ static_cast<uint64_t>(resourceId), enttx::EntityManager<Config>{ initialCapacity } });
    assert(success);
}
}
#endif
#endif // CYCLONITE_SCENE_H
