//
// Created by anton on 8/25/26.
//

#ifndef CYCLONITE_SHARED_ASSET_MODULE_BINARY_H
#define CYCLONITE_SHARED_ASSET_MODULE_BINARY_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>
#include <string>
#include <boost/cstdfloat.hpp>
#include <fvf.h>

namespace cyclonite::shared
{
inline constexpr uint32_t ASSET_MODULE_MAGIC_NUMBER = 0x41534301;     // Cyclonite Asset Module v01

enum class AssetAccessorDataType : uint8_t
{
    Undefined = 0,
    Scalar = 1,
    Vector2 = 2,
    Vector3 = 3,
    Vector4 = 4,
    Matrix2x2 = 5,
    Matrix3x3 = 6,
    Matrix2x3 = 7,
    Matrix3x2 = 8,
    Matrix4x4 = 9,
    Matrix4x2 = 10,
    Matrix2x4 = 11,
    Matrix4x3 = 12,
    Matrix3x4 = 13,
    Count = 14
};

enum class AssetAccessorComponentType : uint8_t
{
    Undefined = 0,
    Byte = 1,
    Boolean = 2,
    Float64_t = 3,
    Float32_t = 4,
    Float16_t = 5,
    Uint64_t = 6,
    Uint32_t = 7,
    Uint16_t = 8,
    Uint8_t = 9,
    Int64_t = 10,
    Int32_t = 11,
    Int16_t = 12,
    Int8_t = 13,
    Count = 14
};

enum class AssetPrimitiveTopology : uint8_t
{
    UNDEFINED = 0,
    POINT_LIST = 1,
    LINE_LIST = 2,
    LINE_STRIP = 3,
    TRIANGLE_LIST = 4,
    TRIANGLE_STRIP = 5,
    TRIANGLE_FAN = 6,
    LINE_LIST_WITH_ADJACENCY = 7,
    LINE_STRIP_WITH_ADJACENCY = 8,
    TRIANGLE_LIST_WITH_ADJACENCY = 9,
    TRIANGLE_STRIP_WITH_ADJACENCY = 10,
    PATCH_LIST = 11,
    COUNT = 12
};

enum class AssetTransformType: uint8_t
{
    Undefined = 0,
    Components = 1,
    Matrix = 2
};

namespace internal
{
inline auto toAssetAccessorDataType(uint8_t v) -> AssetAccessorDataType
{
    auto r = AssetAccessorDataType::Undefined;
    if (v < metrix::value_cast(AssetAccessorDataType::Count)) {
        r = static_cast<AssetAccessorDataType>(v);
    }
    return r;
}

inline auto toAssetAccessorComponentType(uint8_t v) -> AssetAccessorComponentType
{
    auto r = AssetAccessorComponentType::Undefined;
    if (v < metrix::value_cast(AssetAccessorComponentType::Count)) {
        r = static_cast<AssetAccessorComponentType>(v);
    }
    return r;
}

inline auto toAssetPrimitiveTopology(uint8_t v) -> AssetPrimitiveTopology
{
    auto r = AssetPrimitiveTopology::UNDEFINED;
    if (v < metrix::value_cast(AssetPrimitiveTopology::COUNT)) {
        r = static_cast<AssetPrimitiveTopology>(v);
    }
    return r;
}
}

struct AssetBlockHeader
{
    void getBlockHeaderData(uint64_t& baseOffsetOut, uint64_t& blockOffsetOut, uint64_t& sizeOut, uint32_t& idOut) const
    {
        baseOffsetOut = baseOffset;
        blockOffsetOut = blockOffset;
        sizeOut = size;
        idOut = id;
    }

    void setBlockHeaderData(uint64_t baseOffsetIn, uint64_t blockOffsetIn, uint64_t sizeIn, uint32_t idIn)
    {
        baseOffset = baseOffsetIn;
        blockOffset = blockOffsetIn;
        size = sizeIn;
        id = idIn;
    }

    uint64_t baseOffset;
    uint64_t blockOffset;
    uint64_t size;
    uint32_t id;
};

struct AssetBufferView
{
    void get(uint32_t& bufferIndexOut, uint32_t& offsetOut, uint32_t sizeOut, uint8_t& strideOut) 
    {
        bufferIndexOut = bufferIndex;
        offsetOut = offset;
        sizeOut = size;
        strideOut = stride;
    }

    void set(uint32_t bufferIndexIn, uint32_t offsetIn, uint32_t sizeIn, uint8_t strideIn)
    {
        bufferIndex = bufferIndexIn;
        offset = offsetIn;
        size = sizeIn;
        stride = strideIn;
    }

    uint32_t bufferIndex;
    uint32_t offset;
    uint32_t size;
    // places here to avoid copying in accessors of the same view
    uint8_t stride;
};

struct AssetDataAccessor
{
    void get(
        uint32_t& bufferViewOut, 
        uint32_t& elementCountOut, 
        uint8_t& byteOffsetOut, 
        uint8_t& typeOut, 
        uint8_t& componentTypeOut)
    {
        bufferViewOut = bufferViewIndex;
        elementCountOut = elementCount;
        byteOffsetOut = byteOffset;
        typeOut = metrix::value_cast(type);
        componentTypeOut = metrix::value_cast(componentType);
    }

    void set(uint32_t bufferViewIn, 
        uint32_t elementCountIn, 
        uint8_t byteOffsetIn, 
        uint8_t typeIn, 
        uint8_t componentTypeIn)
    {
        bufferViewIndex = bufferViewIn;
        elementCount = elementCountIn;
        byteOffset = byteOffsetIn;
        type = internal::toAssetAccessorDataType(typeIn);
        componentType = internal::toAssetAccessorComponentType(componentTypeIn);
    }

    uint32_t bufferViewIndex;
    uint32_t elementCount;
    uint8_t byteOffset;
    AssetAccessorDataType type;
    AssetAccessorComponentType componentType;
};

struct AssetSubMesh
{
    void get(
        std::vector<uint32_t>& attributesOut, 
        std::vector<uint32_t>& morphTargetOut, 
        uint32_t& indicesOut, 
        uint32_t& materialOut, 
        uint8_t& topologyOut)
    {
        attributesOut = attributes;
        morphTargetOut = morphTargetAccessors;
        indicesOut = indices;
        materialOut = material;
        topologyOut = metrix::value_cast(primitiveTopology);
    }

    void set(std::vector<uint32_t> const& attributesIn, 
        std::vector<uint32_t> const& morphTargetIn, 
        uint32_t indicesIn, 
        uint32_t materialIn, 
        uint8_t topologyIn) 
    {
        attributes = attributesIn;
        morphTargetAccessors = morphTargetIn;
        indices = indicesIn;
        material = materialIn;
        primitiveTopology = internal::toAssetPrimitiveTopology(topologyIn);
    }

    std::vector<uint32_t> attributes;
    std::vector<uint32_t> morphTargetAccessors;
    uint32_t indices;
    uint32_t material;
    AssetPrimitiveTopology primitiveTopology;
};

struct AssetMesh
{
    void get(std::string& nameOut, std::vector<uint32_t>& subMeshesOut) 
    {
        nameOut = name;
        subMeshesOut = subMeshes;
    }

    void set(std::string const& nameIn, std::vector<uint32_t> const& subMeshesIn)
    {
        name = nameIn;
        subMeshes = subMeshesIn;
    }

    std::string name;
    std::vector<uint32_t> subMeshes;
};

struct AssetVertexAttribute
{
    void get(uint64_t& semanticOut, uint32_t accessorOut)
    {
        semanticOut = metrix::value_cast(semantic);
        accessorOut = attributeAccessor;
    }

    void set(uint64_t semanticIn, uint32_t accessorIn)
    {
        // todo:: ... 
    }

    VertexFormatFlags semantic;
    uint32_t attributeAccessor;
};

struct AssetMaterial
{
    std::string name;
};

struct AssetNode
{
    std::string name;
    std::vector<uint32_t> children;
    std::array<boost::float32_t, 12> transform;
    uint32_t mesh;
    AssetTransformType transformType;
};

struct AssetModuleBinary
{
    std::vector<AssetBlockHeader> blockHeaders;
    std::vector<std::vector<std::byte>> buffers;
    std::vector<AssetBufferView> bufferViews;
    std::vector<AssetDataAccessor> dataAccessors;
    std::vector<AssetVertexAttribute> dataAccessors;
    std::vector<AssetSubMesh> subMeshes;
    std::vector<AssetMaterial> materials;
    std::vector<AssetMesh> meshes;
    std::vector<AssetNode> nodes;
    std::vector<uint32_t> rootNodes;
};
}

#endif // CYCLONITE_SHARED_ASSET_MODULE_BINARY_H