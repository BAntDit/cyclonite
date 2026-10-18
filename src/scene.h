
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
          Root<Config>& root);

private:
};
}

#endif // CYCLONITE_SCENE_H
