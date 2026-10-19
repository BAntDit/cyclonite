
#include "geometry.h"
#include <boost/unordered/unordered_flat_map.hpp>
#include <gfx/buffer.h>
#ifdef uuid
#undef uuid
#endif
#include "geometryManager.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <gfx/device.h>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace cyclonite {
namespace {
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

auto Geometry::bytesPerComponentType(shared::AssetAccessorComponentType componentType) -> size_t
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

auto Geometry::componentCountByType(shared::AssetAccessorDataType type) -> uint8_t
{
    auto count = uint8_t{ 0 };

    if (type == shared::AssetAccessorDataType::Scalar) {
        count = 1;
    } else if (type == shared::AssetAccessorDataType::Vector2) {
        count = 2;
    } else if (type == shared::AssetAccessorDataType::Vector3) {
        count = 3;
    } else if (type == shared::AssetAccessorDataType::Vector4 || type == shared::AssetAccessorDataType::Matrix2x2) {
        count = 4;
    } else if (type == shared::AssetAccessorDataType::Matrix2x3 || type == shared::AssetAccessorDataType::Matrix3x2) {
        count = 6;
    } else if (type == shared::AssetAccessorDataType::Matrix3x3) {
        count = 9;
    } else if (type == shared::AssetAccessorDataType::Matrix4x2 || type == shared::AssetAccessorDataType::Matrix2x4) {
        count = 8;
    } else if (type == shared::AssetAccessorDataType::Matrix4x3 || type == shared::AssetAccessorDataType::Matrix3x4) {
        count = 12;
    } else if (type == shared::AssetAccessorDataType::Matrix4x4) {
        count = 16;
    }

    assert(count > 0);
    return count;
}

Geometry::Geometry(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   resources::ResourceGroupBase* resourceGroup,
                   std::string_view name,
                   boost::uuids::uuid const& uuid,
                   GeometryManager& geometryManager)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Geometry>{ resourceGroup, name, uuid }
  , rawData_{}
  , geometryManager_{ &geometryManager }
  , indices_{}
  , buffers_{}
  , bufferStrides_{}
  , attributes_{}
  , vertexCount_{ std::numeric_limits<uint32_t>::max() }
  , primitiveTopology_{ gfx::PrimitiveTopology::TRIANGLE_LIST }
{
}

void Geometry::loadImpl(std::shared_ptr<shared::AssetMainBlock> const& asset)
{
    auto uuidStr = boost::uuids::to_string(uuid());

    auto subMeshIt = std::find_if(asset->subMeshes.begin(), asset->subMeshes.end(), [uuidStr](auto&& subMesh) -> bool {
        return subMesh.uuid == uuidStr;
    });

    if (subMeshIt == asset->subMeshes.end()) {
        throw std::runtime_error(std::format("could not load geometry {}, {} from asset", name(), uuidStr));
    }

    rawData_ = std::make_unique<raw_data_t>();

    auto& subMesh = *subMeshIt;

    primitiveTopology_ = primitiveTopologyFromAsset(subMesh.primitiveTopology);

    // indices:
    if (subMesh.indices != std::numeric_limits<uint32_t>::max()) {
        assert(subMesh.indices < asset->dataAccessors.size());

        auto [bufferViewIdx, elementCount, byteOffset, type, componentType] = asset->dataAccessors[subMesh.indices];

        assert(bufferViewIdx < asset->bufferViews.size());
        auto [bufferIdx, offset, size, stride] = asset->bufferViews[bufferViewIdx];

        assert(bufferIdx < asset->buffers.size());
        auto& buffer = asset->buffers[bufferIdx];

        assert(type == shared::AssetAccessorDataType::Scalar);

        auto bytesPerElement = bytesPerComponentType(componentType);

        auto indexType =
          (bytesPerElement < sizeof(uint32_t)) ? gfx::IndexType::TYPE_UINT16 : gfx::IndexType::TYPE_UINT32;

        auto bytesPerIndex = (indexType == gfx::IndexType::TYPE_UINT16) ? sizeof(uint16_t) : sizeof(uint32_t);

        rawData_->indexType = indexType;
        rawData_->indexCount = elementCount;
        rawData_->indexData.resize(rawData_->indexCount * bytesPerIndex);

        auto* dstPtr = rawData_->indexData.data();
        auto* srcPtr = buffer.data() + offset;

        for (auto i = uint32_t{ 0 }, count = elementCount; i < count; i++) {
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
    } else {
        rawData_->indexCount = 0;
    }

    if (geometryManager_->options().flags.test(GeometryOptionFlags::KeepsOriginalAssetLayout)) {
        throw std::runtime_error("not implemented");
    }

    // vertices & vertex layout:
    auto vertexCount = std::numeric_limits<uint32_t>::max();

    auto bufferViewToFvfs =
      core::StaticHashTable<shared::VertexFormatFlagBits, max_vertex_buffer_count_v * 2, uint32_t>{};

    auto semanticToAssetAttrIndex = core::StaticHashTable<size_t, max_vertex_buffer_count_v * 2, uint64_t>{};

    // buffer index is a key
    const auto mainBufferKeyIndex = static_cast<uint32_t>(max_vertex_buffer_count_v);
    const auto skinningBufferKeyIndex = static_cast<uint32_t>(max_vertex_buffer_count_v + 1);

    auto attributes = std::vector<GeometryAttribute>{};
    attributes.reserve(subMesh.attributes.size());

    for (auto attributeIndex : subMesh.attributes) {
        auto [semantic, accessorIdx] = asset->subMeshAttributes[attributeIndex];

        assert(accessorIdx < asset->dataAccessors.size());
        auto [bufferViewIdx, elementCount, byteOffset, type, componentType] = asset->dataAccessors[accessorIdx];

        assert(bufferViewIdx < asset->bufferViews.size());
        auto [bufferIdx, offset, size, stride] = asset->bufferViews[bufferViewIdx];

        assert(bufferIdx < asset->buffers.size());
        auto& buffer = asset->buffers[bufferIdx];

        if (vertexCount == std::numeric_limits<uint32_t>::max()) {
            vertexCount = elementCount;
        }

        auto geometryAttribute = GeometryAttribute{};

        if (semantic == shared::VertexFormatFlags::POSITION || semantic == shared::VertexFormatFlags::NORMAL) {
            auto fvfIt = bufferViewToFvfs.find(mainBufferKeyIndex);
            if (fvfIt == bufferViewToFvfs.end()) {
                auto [newIt, success] = bufferViewToFvfs.add(shared::VertexFormatFlagBits{}, mainBufferKeyIndex);
                assert(success);
                fvfIt = newIt;
            }

            auto& [_, fvf] = *fvfIt;

            geometryAttribute.semantic = semantic;
            geometryAttribute.componentType = shared::AssetAccessorComponentType::Float32_t;
            geometryAttribute.type = shared::AssetAccessorDataType::Vector3;

            fvf.set(semantic);

            attributes.emplace_back(geometryAttribute);
        } else if (semantic == shared::VertexFormatFlags::TEX_COORD_01) {
            auto fvfIt = bufferViewToFvfs.find(mainBufferKeyIndex);
            if (fvfIt == bufferViewToFvfs.end()) {
                auto [newIt, success] = bufferViewToFvfs.add(shared::VertexFormatFlagBits{}, mainBufferKeyIndex);
                assert(success);
                fvfIt = newIt;
            }

            auto& [_, fvf] = *fvfIt;

            geometryAttribute.semantic = semantic;
            geometryAttribute.componentType = shared::AssetAccessorComponentType::Float32_t;
            geometryAttribute.type = shared::AssetAccessorDataType::Vector2;

            fvf.set(shared::VertexFormatFlags::TEX_COORD_COUNT_1,
                    shared::VertexFormatFlags::TEX_COORD_01,
                    shared::VertexFormatFlags::TEX_COORD_02);

            attributes.emplace_back(geometryAttribute);
        } else if (!geometryManager_->options().flags.test(GeometryOptionFlags::ComputeInFlightTBN) &&
                   (semantic == shared::VertexFormatFlags::TANGENT ||
                    semantic == shared::VertexFormatFlags::BINORMAL)) {
            auto fvfIt = bufferViewToFvfs.find(mainBufferKeyIndex);
            if (fvfIt == bufferViewToFvfs.end()) {
                auto [newIt, success] = bufferViewToFvfs.add(shared::VertexFormatFlagBits{}, mainBufferKeyIndex);
                assert(success);
                fvfIt = newIt;
            }

            auto& [_, fvf] = *fvfIt;

            fvf.set(semantic);

            geometryAttribute.semantic = semantic;
            geometryAttribute.componentType = shared::AssetAccessorComponentType::Float32_t;
            geometryAttribute.type = shared::AssetAccessorDataType::Vector3;

            attributes.emplace_back(geometryAttribute);
        } else if (semantic == shared::VertexFormatFlags::BONE_WEIGHTS ||
                   semantic == shared::VertexFormatFlags::BONE_INDICES) {
            if (geometryManager_->options().flags.test(GeometryOptionFlags::CompressSkinningAttributes)) {
                auto fvfIt = bufferViewToFvfs.find(skinningBufferKeyIndex);
                if (fvfIt == bufferViewToFvfs.end()) {
                    auto [newIt, success] = bufferViewToFvfs.add(
                      shared::VertexFormatFlagBits{ shared::VertexFormatFlags::COMPRESSED_SKINNING_DATA },
                      skinningBufferKeyIndex);
                    assert(success);

                    geometryAttribute.semantic = shared::VertexFormatFlags::COMPRESSED_SKINNING_DATA;
                    geometryAttribute.componentType = shared::AssetAccessorComponentType::Uint32_t;
                    geometryAttribute.type = shared::AssetAccessorDataType::Vector4;

                    attributes.emplace_back(geometryAttribute);
                }
            } else {
                auto fvfIt = bufferViewToFvfs.find(skinningBufferKeyIndex);
                if (fvfIt == bufferViewToFvfs.end()) {
                    auto [newIt, success] =
                      bufferViewToFvfs.add(shared::VertexFormatFlagBits{}, skinningBufferKeyIndex);
                    assert(success);

                    fvfIt = newIt;
                }

                auto& [_, fvf] = *fvfIt;

                fvf.set(semantic);

                geometryAttribute.semantic = semantic;
                geometryAttribute.componentType = (semantic == shared::VertexFormatFlags::BONE_WEIGHTS)
                                                    ? shared::AssetAccessorComponentType::Float32_t
                                                    : shared::AssetAccessorComponentType::Uint32_t;
                geometryAttribute.type = shared::AssetAccessorDataType::Vector4;

                attributes.emplace_back(geometryAttribute);
            }
        } else {
            auto fvfIt = bufferViewToFvfs.find(bufferViewIdx);
            if (fvfIt == bufferViewToFvfs.end()) {
                auto [newIt, success] = bufferViewToFvfs.add(shared::VertexFormatFlagBits{}, bufferViewIdx);
                assert(success);
                fvfIt = newIt;
            }

            auto& [_, fvf] = *fvfIt;

            fvf.set(semantic);

            geometryAttribute.semantic = semantic;
            geometryAttribute.componentType = componentType;
            geometryAttribute.type = type;

            attributes.emplace_back(geometryAttribute);
        }

        {
            auto [_, success] = semanticToAssetAttrIndex.add(attributeIndex, static_cast<uint64_t>(semantic));
            assert(success);
        }
    } // asset attributes (fvf pass)

    for (auto [_, fvf] : bufferViewToFvfs) {
        auto attrIt =
          std::find_if(attributes.begin(), attributes.end(), [fvf](auto&& a) -> bool { return fvf.test(a.semantic); });

        (*attrIt).bufferFvfMask = fvf.value;
    }

    // fill byte offsets
    for (auto& attr : attributes) {
        auto fvf = shared::VertexFormatFlagBits{};
        fvf.value = attr.bufferFvfMask;

        if (attr.semantic == shared::VertexFormatFlags::POSITION) {
            attr.byteOffset = 0;
        } else if (attr.semantic == shared::VertexFormatFlags::NORMAL) {
            attr.byteOffset = bytesPerComponentType(shared::AssetAccessorComponentType::Float32_t) *
                              componentCountByType(shared::AssetAccessorDataType::Vector3);
        } else if (attr.semantic == shared::VertexFormatFlags::TEX_COORD_01) {
            attr.byteOffset = 2 * bytesPerComponentType(shared::AssetAccessorComponentType::Float32_t) *
                              componentCountByType(shared::AssetAccessorDataType::Vector3);
        } else if (attr.semantic == shared::VertexFormatFlags::TANGENT) {
            assert(fvf.test(shared::VertexFormatFlags::TEX_COORD_01));
            attr.byteOffset = (2 * bytesPerComponentType(shared::AssetAccessorComponentType::Float32_t) *
                               componentCountByType(shared::AssetAccessorDataType::Vector3)) +
                              (bytesPerComponentType(shared::AssetAccessorComponentType::Float32_t) *
                               componentCountByType(shared::AssetAccessorDataType::Vector2));
        } else if (attr.semantic == shared::VertexFormatFlags::BINORMAL) {
            assert(fvf.test(shared::VertexFormatFlags::TEX_COORD_01));
            assert(fvf.test(shared::VertexFormatFlags::TANGENT));

            attr.byteOffset = (2 * bytesPerComponentType(shared::AssetAccessorComponentType::Float32_t) *
                               componentCountByType(shared::AssetAccessorDataType::Vector3)) +
                              (bytesPerComponentType(shared::AssetAccessorComponentType::Float32_t) *
                               componentCountByType(shared::AssetAccessorDataType::Vector2)) +
                              (bytesPerComponentType(shared::AssetAccessorComponentType::Float32_t) *
                               componentCountByType(shared::AssetAccessorDataType::Vector3));
        } else if (attr.semantic == shared::VertexFormatFlags::COMPRESSED_SKINNING_DATA) {
            attr.byteOffset = 0;
        } else if (attr.semantic == shared::VertexFormatFlags::BONE_WEIGHTS) {
            attr.byteOffset = 0;
        } else if (attr.semantic == shared::VertexFormatFlags::BONE_INDICES) {
            assert(fvf.test(shared::VertexFormatFlags::BONE_WEIGHTS));
            attr.byteOffset = bytesPerComponentType(shared::AssetAccessorComponentType::Float32_t) *
                              componentCountByType(shared::AssetAccessorDataType::Vector4);
        } else {
            auto* attrIndex = semanticToAssetAttrIndex.at(static_cast<uint64_t>(attr.semantic));
            assert(attrIndex != nullptr);

            auto [semantic, accessorIdx] = asset->subMeshAttributes[*attrIndex];

            assert(accessorIdx < asset->dataAccessors.size());
            auto [bufferViewIdx, elementCount, byteOffset, type, componentType] = asset->dataAccessors[accessorIdx];

            attr.byteOffset = byteOffset;
        }
    } // byte offset pass (end)

    setupBufferLayout(attributes);

    // fill vertex data
    // prepare raw buffers:
    for (auto [k, stride] : bufferStrides_) {
        auto [fvf] = k;

        auto [it, success] = rawData_->vertexData.add(std::vector<std::byte>{}, fvf);
        assert(success);

        auto& [_, v] = *it;

        auto size = static_cast<size_t>(vertexCount_ * stride);

        v.resize(size, std::byte{ 0 });
    }

    // fill vertex attributes data:
    for (auto& [k, attr] : attributes_) {
        auto [semanticKey] = k;

        auto* vertexData = rawData_->vertexData.at(attr.bufferFvfMask);
        assert(vertexData != nullptr);

        auto* vertexDataDst = vertexData->data();

        if (attr.semantic == shared::VertexFormatFlags::COMPRESSED_SKINNING_DATA) {
            auto* boneWeightAttrIdx =
                          semanticToAssetAttrIndex.at(static_cast<uint64_t>(shared::VertexFormatFlags::BONE_WEIGHTS));
            assert(boneWeightAttrIdx != nullptr);

            auto* boneIndicesAttrIdx =
              semanticToAssetAttrIndex.at(static_cast<uint64_t>(shared::VertexFormatFlags::BONE_INDICES));
            assert(boneIndicesAttrIdx != nullptr);

            auto [_0, weightsAccessorIdx] = asset->subMeshAttributes[*boneWeightAttrIdx];
            auto [_1, indicesAccessorIdx] = asset->subMeshAttributes[*boneIndicesAttrIdx];

            assert(weightsAccessorIdx < asset->dataAccessors.size());
            auto [weightsViewIdx, weightsElementCount, weightsByteOffset, weightsType, weightsComponentType] =
              asset->dataAccessors[weightsAccessorIdx];

            assert(weightsType == shared::AssetAccessorDataType::Vector4);
            assert(weightsComponentType == shared::AssetAccessorComponentType::Float32_t);

            assert(indicesAccessorIdx < asset->dataAccessors.size());
            auto [indicesViewIdx, indicesElementCount, indicesByteOffset, indicesType, indicesComponentType] =
              asset->dataAccessors[indicesAccessorIdx];

            assert(indicesType == shared::AssetAccessorDataType::Vector4);
            assert(indicesComponentType == shared::AssetAccessorComponentType::Float32_t);

            assert(weightsViewIdx < asset->bufferViews.size());
            auto [weightsBufferIdx, weightsOffset, weightsSize, weightsStride] = asset->bufferViews[weightsViewIdx];

            assert(weightsBufferIdx < asset->buffers.size());
            auto& weightsBuffer = asset->buffers[weightsBufferIdx];

            assert(indicesViewIdx < asset->bufferViews.size());
            auto [indicesBufferIdx, indicesOffset, indicesSize, indicesStride] = asset->bufferViews[indicesViewIdx];

            assert(indicesBufferIdx < asset->buffers.size());
            auto& indicesBuffer = asset->buffers[indicesBufferIdx];

            auto* srcWeightsPtr = weightsBuffer.data() + weightsOffset;
            auto* srcIndicesPtr = indicesBuffer.data() + indicesOffset;

            auto dstStride = sizeof(uint32_t) * 4;

            for (auto attrIdx = uint32_t{ 0 }; attrIdx < vertexCount_; attrIdx++) {
                auto weights = vec4{};
                auto indices = vec4{};

                memcpy(glm::value_ptr(weights),
                       srcWeightsPtr + weightsStride * attrIdx + weightsByteOffset,
                       sizeof(real) * 4);

                memcpy(glm::value_ptr(indices),
                       srcIndicesPtr + indicesStride * attrIdx + indicesByteOffset,
                       sizeof(real) * 4);

                auto packedSkinningData = glm::uvec4{ glm::packHalf2x16(vec2{ weights.x, weights.y }),
                                                      glm::packHalf2x16(vec2{ weights.z, weights.w }),
                                                      glm::packHalf2x16(vec2{ indices.x, indices.y }),
                                                      glm::packHalf2x16(vec2{ indices.z, indices.w }) };

                memcpy(vertexDataDst + attrIdx * dstStride, glm::value_ptr(packedSkinningData), dstStride);
            }
        } else {
            auto* originalAttrIdx = semanticToAssetAttrIndex.at(semanticKey);
            assert(originalAttrIdx != nullptr);

            auto [semantic, accessorIdx] = asset->subMeshAttributes[*originalAttrIdx];

            assert(accessorIdx < asset->dataAccessors.size());
            auto [bufferViewIdx, elementCount, byteOffset, type, componentType] = asset->dataAccessors[accessorIdx];

            assert(bufferViewIdx < asset->bufferViews.size());
            auto [bufferIdx, offset, size, stride] = asset->bufferViews[bufferViewIdx];

            assert(bufferIdx < asset->buffers.size());
            auto& buffer = asset->buffers[bufferIdx];

            auto* srcPtr = buffer.data() + offset;

            auto srcByteOffset = byteOffset;
            auto dstByteOffset = attr.byteOffset;

            auto srcStride = stride;
            auto* dstStride = bufferStrides_.at(attr.bufferFvfMask);
            assert(dstStride != nullptr);

            auto srcByteCount = componentCountByType(type) * bytesPerComponentType(componentType);
            auto dstByteCount = componentCountByType(attr.type) * bytesPerComponentType(attr.componentType);

            auto byteCount = std::min(srcByteCount, dstByteCount);

            assert(elementCount == vertexCount_);

            assert(elementCount == vertexCount_);
            for (auto attrIdx = uint32_t{ 0 }; attrIdx < vertexCount_; attrIdx++) {
                memcpy(vertexDataDst + (*dstStride) * attrIdx + dstByteOffset,
                       srcPtr + srcStride * attrIdx + srcByteOffset,
                       byteCount);
            }
        }
    } // end fill vertex data
}

void Geometry::prepareImpl(core::ResourceSharedRef deviceRef)
{
    auto& device = deviceRef.template as<gfx::Device>();

    assert(rawData_);

    if (rawData_->indexCount > 0) {
        auto indexAllocationFlags = gfx::GpuMemoryAllocationFlagBits{};
        indexAllocationFlags.set(gfx::GpuMemoryAllocationFlags::HOST_ACCESS_SEQUENTIAL_WRITE);

        auto usageFlags = gfx::BufferUsageFlagBits{};
        usageFlags.set(gfx::BufferUsageFlags::TRANSFER_SRC);

        auto size = rawData_->indexData.size();

        auto stagingRef = core::ResourceSharedRef{ device.createBuffer(indexAllocationFlags, usageFlags, size) };

        auto& indexStaging = stagingRef.as<gfx::Buffer>();

        auto* dstPtr = reinterpret_cast<std::byte*>(indexStaging.map());
        auto* srcPtr = rawData_->indexData.data();

        memcpy(dstPtr, srcPtr, size);

        indexStaging.unmap();

        setIndices(stagingRef, rawData_->indexType, rawData_->indexCount);
    }

    for (auto& [k, v] : rawData_->vertexData) {
        auto [fvfKey] = k;

        auto vertexAllocationFlags = gfx::GpuMemoryAllocationFlagBits{};
        vertexAllocationFlags.set(gfx::GpuMemoryAllocationFlags::HOST_ACCESS_SEQUENTIAL_WRITE);

        auto usageFlags = gfx::BufferUsageFlagBits{};
        usageFlags.set(gfx::BufferUsageFlags::TRANSFER_SRC);

        auto size = v.size();

        auto stagingRef = core::ResourceSharedRef{ device.createBuffer(vertexAllocationFlags, usageFlags, size) };

        auto& staging = stagingRef.as<gfx::Buffer>();

        auto* stagingPtr = reinterpret_cast<std::byte*>(staging.map());
        assert(stagingPtr != nullptr);

        memcpy(stagingPtr, v.data(), size);

        staging.unmap();

        auto fvf = shared::VertexFormatFlagBits{};
        fvf.value = fvfKey;

        setVertices(fvf, stagingRef);
    }

    rawData_.reset(nullptr);
}

void Geometry::setVertices(shared::VertexFormatFlagBits fvf, core::ResourceSharedRef const& stagingRef)
{
    assert(geometryManager_ != nullptr);
    assert(stagingRef.valid());

    auto* stride = bufferStrides_.at(fvf.value);
    if (stride == nullptr) {
        throw std::runtime_error("buffer does not respect geometry layout");
    }

    auto& deviceRef = geometryManager_->device();
    auto& device = deviceRef.as<gfx::Device>();

    auto bufferIt = buffers_.find(fvf.value);
    if (bufferIt == buffers_.end()) {
        auto [newIt, success] = buffers_.add(core::ResourceSharedRef{}, fvf.value);
        assert(success);
        bufferIt = newIt;
    }

    auto& [_, bufferRef] = *bufferIt;

    auto allocationFlags = gfx::GpuMemoryAllocationFlagBits{};
    allocationFlags.set(gfx::GpuMemoryAllocationFlags::DEDICATED_MEMORY);

    auto usageFlags = gfx::BufferUsageFlagBits{};
    usageFlags.set(gfx::BufferUsageFlags::STORAGE_BUFFER,
                   gfx::BufferUsageFlags::SHADER_DEVICE_ADDRESS,
                   gfx::BufferUsageFlags::TRANSFER_DST);

    auto size = *stride * vertexCount_;
    assert(size > 0);

    bufferRef = device.createBuffer(allocationFlags, usageFlags, size);

    geometryManager_->commitGpuData(stagingRef, bufferRef, 0, size);
}

void Geometry::setIndices(core::ResourceSharedRef const& stagingRef, gfx::IndexType indexType, uint32_t indexCount)
{
    assert(geometryManager_ != nullptr);
    assert(stagingRef.valid());

    if (indices_.valid()) {
        indices_.reset();
    }

    indices_ = geometryManager_->allocateIndices(indexCount, indexType);

    auto bytesPerIndex = (indexType == gfx::IndexType::TYPE_UINT16) ? sizeof(uint16_t) : sizeof(uint32_t);

    auto size = bytesPerIndex * indexCount;

    if (!indices_.valid()) {
        throw std::runtime_error("could not set indices");
    }

    geometryManager_->commitGpuData(stagingRef, indices_.indexBuffer(), indices_.firstIndex() * bytesPerIndex, size);
}
}
