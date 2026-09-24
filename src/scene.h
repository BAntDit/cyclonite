
#ifndef CYCLONITE_SCENE_H
#define CYCLONITE_SCENE_H

#include "core/resourceBase.h"
#include "resources/managedResource.h"

namespace cyclonite {
class Scene
  : public core::ResourceBase
  , public resources::ManagedResource<cyclonite::Scene>
{
public:
    Scene(core::ResourceManagerBase* resourceManager,
          core::ResourceId resourceId,
          resources::ResourceGroupBase* resourceGroup,
          std::string_view name,
          boost::uuids::uuid const& uuid);

private:
};
}

#endif // CYCLONITE_SCENE_H
