
#include "geometry.h"

namespace cyclonite {
Geometry::Geometry(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   resources::ResourceGroupBase* resourceGroup,
                   std::string_view name,
                   boost::uuids::uuid const& uuid)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Geometry>{ resourceGroup, name, uuid }
  , indexBuffer_{}
  , buffers_{}
  , attributes_{}
  , indexCount_{ 0 }
  , vertexCount_{ 0 }
{
}

auto Geometry::setup(std::shared_ptr<shared::AssetMainBlock> const& asset) -> std::shared_future<void>;
}
