//
// Created by anton on 8/25/26.
//

#ifndef CYCLONITE_SHARED_ASSET_MODULE_BINARY_H
#define CYCLONITE_SHARED_ASSET_MODULE_BINARY_H

#include <cstdint>
#include <cstddef>
#include <vector>
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
    Matrix3x4 = 13
};

enum class AssetAccessorComponentType : uint8_t
{
    Byte = 0,
    Boolean = 1,
    float64_t = 2,
    float32_t = 3,
    float16_t = 4,
    uint64_t = 5,
    uint32_t = 6,
    uint16_t = 7,
    uint8_t = 8,
    int64_t = 9,
    int32_t = 10,
    int16_t = 11,
    int8_t = 12
};

enum class AssetPrimitiveTopology : uint8_t
{
    POINT_LIST = 0,
    LINE_LIST = 1,
    LINE_STRIP = 2,
    TRIANGLE_LIST = 3,
    TRIANGLE_STRIP = 4,
    TRIANGLE_FAN = 5,
    LINE_LIST_WITH_ADJACENCY = 6,
    LINE_STRIP_WITH_ADJACENCY = 7,
    TRIANGLE_LIST_WITH_ADJACENCY = 8,
    TRIANGLE_STRIP_WITH_ADJACENCY = 9,
    PATCH_LIST = 10
};

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

struct GeometryAttributeData
{
    uint32_t bufferIndex;
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
    uint32_t bufferViewIndex;
    uint32_t elementCount;
    uint8_t byteOffset;
    AssetAccessorDataType type;
    AssetAccessorComponentType componentType;
};

struct AssetGeometry
{
    std::vector<VertexFormatFlags> attributeSemantics;
    std::vector<uint32_t> attributes;
    std::vector<uint32_t> morphTargetAccessors;
    uint32_t indices;
    AssetPrimitiveTopology primitiveTopology;
};

struct AssetVertexAttribute
{
    VertexFormatFlags semantic;
    uint32_t attributeAccessor;
};

struct AssetModuleBinary
{
    std::vector<AssetBlockHeader> blockHeaders;
    std::vector<std::vector<std::byte>> buffers;
    std::vector<AssetBufferView> bufferViews;
    std::vector<AssetDataAccessor> dataAccessors;
    std::vector<AssetVertexAttribute> dataAccessors;
    std::vector<AssetGeometry> geometries;
};
}

#endif // CYCLONITE_SHARED_ASSET_MODULE_BINARY_H