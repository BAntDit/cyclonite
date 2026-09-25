
#ifndef CYCLONITE_GEOMETRY_H
#define CYCLONITE_GEOMETRY_H

#include "core/hashTable.h"
#include "core/resourceSharedRef.h"
#include "fvf.h"
#include "resources/managedResource.h"
#include "assetModuleBinary.h"

namespace cyclonite {
class Geometry
  : public core::ResourceBase
  , public resources::ManagedResource<cyclonite::Geometry>
{
public:
    struct Attribute
    {
        shared::VertexFormatFlags semantic;
        size_t stride;
        size_t byteOffset; // offset per vertex
        size_t baseOffset; // offset common;
    };

    Geometry(core::ResourceManagerBase* resourceManager,
             core::ResourceId resourceId,
             resources::ResourceGroupBase* resourceGroup,
             std::string_view name,
             boost::uuids::uuid const& uuid,
             std::shared_ptr<shared::AssetMainBlock> const& asset);

private:
    constexpr static size_t max_attribute_count_v = 64;
    constexpr static size_t max_vertex_buffer_count_v = 8;

    std::shared_ptr<shared::AssetMainBlock> rawData_;

    core::ResourceSharedRef indexBuffer_;
    core::StaticHashTable<core::ResourceSharedRef, max_vertex_buffer_count_v, uint64_t> buffers_;
    core::StaticHashTable<Attribute, max_attribute_count_v, uint64_t> attributes_;
    uint32_t indexCount_;
    uint32_t vertexCount_;
};
}

#endif // CYCLONITE_GEOMETRY_H
