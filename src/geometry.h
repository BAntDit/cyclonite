
#ifndef CYCLONITE_GEOMETRY_H
#define CYCLONITE_GEOMETRY_H

#include <metrix/containers.h>
#include "assetModuleBinary.h"
#include "core/hashTable.h"
#include "core/resourceSharedRef.h"
#include "fvf.h"
#include "gfx/common.h"
#include "gfx/geometryIndicesAllocation.h"
#include "resources/managedResource.h"

namespace cyclonite {
class GeometryManager;

struct GeometryAttribute
{
    uint64_t bufferFvfMask;
    shared::VertexFormatFlags semantic;
    shared::AssetAccessorDataType type;
    shared::AssetAccessorComponentType componentType;
    uint8_t byteOffset; // offset per vertex
};

template<typename T>
concept AttributeContainerConcept = requires(T t)
{
    { T::value_type };

    { T::iterator };

    { t.begin() };

    { t.end() };

    requires std::is_same_v<typename T::value_type, GeometryAttribute>;
};

class Geometry
  : public core::ResourceBase
  , public resources::ManagedResource<cyclonite::Geometry>
{
public:
    Geometry(core::ResourceManagerBase* resourceManager,
             core::ResourceId resourceId,
             resources::ResourceGroupBase* resourceGroup,
             std::string_view name,
             boost::uuids::uuid const& uuid,
             GeometryManager& geometryManager);

    Geometry(core::ResourceManagerBase* resourceManager,
             core::ResourceId resourceId,
             resources::ResourceGroupBase* resourceGroup,
             std::string_view name,
             boost::uuids::uuid const& uuid,
             GeometryManager& geometryManager,
             std::shared_ptr<shared::AssetMainBlock> const& asset);

    using core::ResourceBase::resourceBase;

    void loadImpl(std::istream& stream);

    void prepareImpl(core::ResourceSharedRef deviceRef);

    void setIndices(core::ResourceSharedRef const& stagingRef, gfx::IndexType indexType, uint32_t indexCount);

    template<AttributeContainerConcept AttributeContainer>
    void setupBufferLayout(AttributeContainer&& attributes);
    // void setAttribute(shared::VertexFormatFlags semantic)

    // void unlock();
    // TODO::
    // void commit();

private:
    static auto bytesPerComponentType(shared::AssetAccessorComponentType componentType) -> size_t;

    static auto componentCountByType(shared::AssetAccessorDataType type) -> uint8_t;

    constexpr static size_t max_attribute_count_v = 64;
    constexpr static size_t max_vertex_buffer_count_v = 8;

    GeometryManager* geometryManager_;

    std::shared_ptr<shared::AssetMainBlock> asset_;

    gfx::GeometryIndicesAllocation indices_;

    core::StaticHashTable<core::ResourceSharedRef, max_vertex_buffer_count_v, uint64_t> buffers_;
    core::StaticHashTable<uint8_t, max_vertex_buffer_count_v, uint64_t> bufferStrides_;
    core::StaticHashTable<GeometryAttribute, max_attribute_count_v, uint64_t> attributes_;

    uint32_t vertexCount_;

    gfx::PrimitiveTopology primitiveTopology_;
};

template<AttributeContainerConcept AttributeContainer>
void Geometry::setupBufferLayout(AttributeContainer&& attributes)
{
    buffers_.clear();
    bufferStrides_.clear();
    attributes_.clear();

    for (auto&& attr : attributes) {
        auto [fvf, semantic, type, componentType, byteOffset] = attr;

        auto strideIt = bufferStrides_.find(fvf);
        if (strideIt == bufferStrides_.end()) {
            auto [newIt, success] = bufferStrides_.add(uint8_t{ 0 }, fvf);
            assert(success);
            strideIt = newIt;
        }

        auto bytesPerComponent = bytesPerComponentType(componentType);
        auto componentCount = componentCountByType(type);

        auto& [_, stride] = (*strideIt);

        stride += bytesPerComponent * static_cast<uint8_t>(bytesPerComponent);

        auto buffIt = buffers_.find(fvf);
        if (buffIt == buffers_.end()) {
            auto [newIt, success] = buffers_.add(core::ResourceSharedRef{}, fvf);
            assert(success);
        }

        {
            auto [attrIt, success] = attributes_.add(attr, static_cast<uint64_t>(semantic));
            assert(success);
        }
    }
}
}

#endif // CYCLONITE_GEOMETRY_H
