//
// Created by anton on 4/26/26.
//

#include "material.h"

namespace cyclonite {
Material::Material(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   resources::ResourceGroupBase* resourceGroup,
                   std::string_view name,
                   boost::uuids::uuid const& uuid)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Material>{ resourceGroup, name, uuid }
  , rawData_{}
  , pipeline_{}
{
}

void Material::prepareImpl()
{
    // TODO::
}
}
