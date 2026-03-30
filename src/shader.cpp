//
// Created by anton on 3/30/26.
//

#include "shader.h"

namespace cyclonite {
Shader::Shader(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               std::string_view name,
               boost::uuids::uuid uuid)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Shader>{ name, uuid }
{
}
}
