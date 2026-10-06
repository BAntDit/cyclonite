
#include "geometry.h"
#include <boost/unordered/unordered_flat_map.hpp>
#include <gfx/buffer.h>
#ifdef uuid
#undef uuid
#endif
#include "systems/geometryManagementSystem.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <gfx/device.h>

namespace cyclonite {
namespace {
auto bytesPerComponentType(shared::AssetAccessorComponentType componentType) -> size_t
{
    auto result = size_t{ 0 };

    switch (componentType) {
        case shared::AssetAccessorComponentType::Byte:
            [[fallthrough]];
        case shared::AssetAccessorComponentType::Boolean:
            [[fallthrough]];
        case shared::AssetAccessorComponentType::Int8_t:
            [[fallthrough]];
        case shared::AssetAccessorComponentType::Uint8_t:
            result = 1;
            break;
        case shared::AssetAccessorComponentType::Float16_t:
            [[fallthrough]];
        case shared::AssetAccessorComponentType::Int16_t:
            [[fallthrough]];
        case shared::AssetAccessorComponentType::Uint16_t:
            result = 2;
            break;
        case shared::AssetAccessorComponentType::Int32_t:
            [[fallthrough]];
        case shared::AssetAccessorComponentType::Uint32_t:
            [[fallthrough]];
        case shared::AssetAccessorComponentType::Float32_t:
            result = 4;
            break;
        case shared::AssetAccessorComponentType::Float64_t:
            [[fallthrough]];
        case shared::AssetAccessorComponentType::Int64_t:
            [[fallthrough]];
        case shared::AssetAccessorComponentType::Uint64_t:
            result = 8;
            break;
        default:
            assert(false);
    }

    return result;
}

auto primitiveTopologyFromAsset(shared::AssetPrimitiveTopology assetTopology) -> gfx::PrimitiveTopology
{
    auto result = gfx::PrimitiveTopology::TRIANGLE_LIST;

    switch (assetTopology) {
        case shared::AssetPrimitiveTopology::POINT_LIST:
            result = gfx::PrimitiveTopology::POINT_LIST;
            break;
        case shared::AssetPrimitiveTopology::LINE_LIST:
            result = gfx::PrimitiveTopology::LINE_LIST;
            break;
        case shared::AssetPrimitiveTopology::TRIANGLE_STRIP:
            result = gfx::PrimitiveTopology::TRIANGLE_STRIP;
            break;
        case shared::AssetPrimitiveTopology::TRIANGLE_FAN:
            result = gfx::PrimitiveTopology::TRIANGLE_FAN;
            break;
        case shared::AssetPrimitiveTopology::TRIANGLE_LIST:
            result = gfx::PrimitiveTopology::TRIANGLE_LIST;
            break;
        case shared::AssetPrimitiveTopology::LINE_STRIP:
            result = gfx::PrimitiveTopology::LINE_STRIP;
            break;
        case shared::AssetPrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
            result = gfx::PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY;
            break;
        case shared::AssetPrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
            result = gfx::PrimitiveTopology::LINE_LIST_WITH_ADJACENCY;
            break;
        case shared::AssetPrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
            result = gfx::PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY;
            break;
        case shared::AssetPrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
            result = gfx::PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY;
            break;
        case shared::AssetPrimitiveTopology::PATCH_LIST:
            result = gfx::PrimitiveTopology::PATCH_LIST;
            break;
        default:
            assert(false);
    }

    return result;
}
}

Geometry::Geometry(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   resources::ResourceGroupBase* resourceGroup,
                   std::string_view name,
                   boost::uuids::uuid const& uuid,
                   systems::GeometryManagementSystem& geometrySystem)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Geometry>{ resourceGroup, name, uuid }
  , geometrySystem_{ &geometrySystem }
  , asset_{}
  , buffers_{}
  , attributes_{}
  , primitiveTopology_{ gfx::PrimitiveTopology::TRIANGLE_LIST }
{
}

Geometry::Geometry(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   resources::ResourceGroupBase* resourceGroup,
                   std::string_view name,
                   boost::uuids::uuid const& uuid,
                   systems::GeometryManagementSystem& geometrySystem,
                   std::shared_ptr<shared::AssetMainBlock> const& asset)
  : Geometry{ resourceManager, resourceId, resourceGroup, name, uuid, geometrySystem }
{
    asset_ = asset;
}

void Geometry::loadImpl(std::istream& stream)
{
    if (asset_) {
        auto uuidStr = boost::uuids::to_string(uuid());

        auto subMeshIt = std::find_if(asset_->subMeshes.begin(),
                                      asset_->subMeshes.end(),
                                      [uuidStr](auto&& subMesh) -> bool { return subMesh.uuid == uuidStr; });

        if (subMeshIt == asset_->subMeshes.end()) {
            throw std::runtime_error(std::format("could not load geometry {}, {} from asset", name(), uuidStr));
        }

        auto& subMesh = *subMeshIt;

        primitiveTopology_ = primitiveTopologyFromAsset(subMesh.primitiveTopology);

        // TODO:: load indices
        // subMesh.indices

        auto customBufferToFvfMap = boost::unordered_flat_map<uint32_t, shared::VertexFormatFlagBits>{};
        for (auto&& atrIdx : subMesh.attributes) {
            auto [semantic, accessorIdx] = asset_->subMeshAttributes[atrIdx];
            auto [bufferViewIdx, elementCount, byteOffset, type, componentType] = asset_->dataAccessors[accessorIdx];

            // auto fvf = bufferToFvfMap[bufferViewIdx];
            // fvf.set(semantic);
        }

        asset_.reset();
    } else {
        (void)stream;
        throw std::runtime_error("not implemented");
    }
}

void Geometry::prepareImpl(core::ResourceSharedRef deviceRef)
{
    if (asset_) {
        auto& device = deviceRef.template as<gfx::Device>();

        auto uuidStr = boost::uuids::to_string(uuid());

        auto subMeshIt = std::find_if(asset_->subMeshes.begin(),
                                      asset_->subMeshes.end(),
                                      [uuidStr](auto&& subMesh) -> bool { return subMesh.uuid == uuidStr; });

        if (subMeshIt == asset_->subMeshes.end()) {
            throw std::runtime_error(std::format("could not load geometry {}, {} from asset", name(), uuidStr));
        }

        auto& subMesh = *subMeshIt;

        if (subMesh.indices != std::numeric_limits<uint32_t>::max()) {
            assert(subMesh.indices < asset_->dataAccessors.size());

            auto [bufferViewIdx, elementCount, byteOffset, type, componentType] =
              asset_->dataAccessors[subMesh.indices];

            assert(bufferViewIdx < asset_->bufferViews.size());
            auto [bufferIdx, offset, size, stride] = asset_->bufferViews[bufferViewIdx];

            assert(bufferIdx < asset_->buffers.size());
            auto& buffer = asset_->buffers[bufferIdx];

            assert(type == shared::AssetAccessorDataType::Scalar);

            auto bytesPerElement = bytesPerComponentType(componentType);

            auto indexType =
              (bytesPerElement < sizeof(uint32_t)) ? gfx::IndexType::TYPE_UINT16 : gfx::IndexType::TYPE_UINT32;

            auto bytesPerIndex = (indexType == gfx::IndexType::TYPE_UINT16) ? sizeof(uint16_t) : sizeof(uint32_t);

            indices_ = geometrySystem_->allocateIndices(elementCount, indexType);

            auto indexAllocationFlags = gfx::GpuMemoryAllocationFlagBits{};
            indexAllocationFlags.set(gfx::GpuMemoryAllocationFlags::HOST_ACCESS_SEQUENTIAL_WRITE);

            auto usageFlags = gfx::BufferUsageFlagBits{};
            usageFlags.set(gfx::BufferUsageFlags::TRANSFER_SRC);

            indices_.staging() =
              device.createBuffer(indexAllocationFlags, usageFlags, bytesPerIndex * indices_.indexCount());

            auto& indexStaging = indices_.staging().as<gfx::Buffer>();

            auto* dstPtr = reinterpret_cast<std::byte*>(indexStaging.map());
            auto* srcPtr = buffer.data() + offset;

            for (auto i = uint32_t{ 0 }, count = indices_.indexCount(); i < count; i++) {
                if (componentType == shared::AssetAccessorComponentType::Uint8_t) {
                    assert(indexType == gfx::IndexType::TYPE_UINT16);
                    auto index = static_cast<uint16_t>(*(srcPtr + i * sizeof(uint8_t) + byteOffset));
                    memcpy(dstPtr + i * sizeof(uint16_t), &index, sizeof(index));
                } else if (componentType == shared::AssetAccessorComponentType::Uint16_t) {
                    assert(indexType == gfx::IndexType::TYPE_UINT16);
                    memcpy(dstPtr + i * sizeof(uint16_t), srcPtr + i * sizeof(uint16_t) + byteOffset, sizeof(uint16_t));
                } else if (componentType == shared::AssetAccessorComponentType::Uint32_t) {
                    assert(indexType == gfx::IndexType::TYPE_UINT32);
                    memcpy(dstPtr + i * sizeof(uint32_t), srcPtr + i * sizeof(uint32_t) + byteOffset, sizeof(uint32_t));
                } else {
                    throw std::runtime_error("geometry unsupported index type");
                }
            }

            indexStaging.unmap();
            dstPtr = nullptr;
        }

        // TODO:: commit geometry

        asset_.reset();
    } else {
        throw std::runtime_error("not implemented");
    }

    // device.create
}
}
