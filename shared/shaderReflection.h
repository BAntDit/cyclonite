//
// Created by anton on 12/24/25.
//

#ifndef CYCLONITE_SHADER_REFLECTION_H
#define CYCLONITE_SHADER_REFLECTION_H

#include <cstdint>
#include <metrix/enum.h>
#include <string>
#include <vector>

namespace cyclonite::shared {
enum class ShaderResourceType : uint8_t
{
    Undefined = 0,
    CBuffer = 1,
    TBuffer = 2,
    Texture = 3,
    Sampler = 4,
    RwTyped = 5,
    StructuredBuffer = 6,
    RwStructuredBuffer = 7,
    ByteAddress = 8,
    RwByteAddress = 9,
    AppendStructuredBuffer = 10,
    ConsumeStructuredBuffer = 11,
    RwStructuredWithCounter = 12,
    RtAccelerationStructure = 13,
    FeedbackTexture = 14
};

enum class TextureResourceComponetType : uint8_t
{
    Undefined = 0,
    UNORM = 1,
    SNORM = 2,
    SINT = 3,
    UINT = 4,
    FLOAT = 5,
    MIXED = 6,
    DOUBLE = 7,
    CONTINUED = 8
};

enum class ResourceViewDimension : uint8_t
{
    Undefined = 0,
    Buffer = 1,
    Texture1D = 2,
    Texture1DArray = 3,
    Texture2D = 4,
    Texture2DArray = 5,
    Texture2DMS = 6,
    Texture2DMSArray = 7,
    Texture3D = 8,
    TextureCube = 9,
    TextureCubeArray = 10,
    BufferEx = 11
};

enum class ConstantBufferType : uint8_t
{
    Undefined = 0,
    CBuffer = 1,
    TBuffer = 2,
    InterfacePointer = 3,
    ResourceBindInfo = 4
};

struct BoundResource
{
    void getResourceData(std::string_view& resourceName,
                         uint8_t& resourceType,
                         uint32_t& resourceSpace,
                         uint32_t& resourceBindPoint,
                         uint32_t& resourceCount,
                         uint8_t& texComponent,
                         uint32_t& resourceSampleCount,
                         uint8_t& resourceViewDimension) const
    {
        resourceName = name;
        resourceType = metrix::value_cast(type);
        resourceSpace = space;
        resourceBindPoint = bindPoint;
        resourceCount = bindCount;
        texComponent = metrix::value_cast(textureComponetType);
        resourceSampleCount = sampleCount;
        resourceViewDimension = metrix::value_cast(dimension);
    }

    std::string name;
    ShaderResourceType type;

    uint32_t space;
    uint32_t bindPoint;
    uint32_t bindCount;

    TextureResourceComponetType textureComponetType;
    uint32_t sampleCount;

    ResourceViewDimension dimension;
};

struct ConstantBufferReflection
{
    void getBufferData(std::string_view& bufferName, uint8_t& bufferType, uint64_t& bufferSize) const
    {
        bufferName = name;
        bufferType = metrix::value_cast(type);
        bufferSize = size;
    }

    std::string name;
    ConstantBufferType type;
    uint64_t size;
};

struct ShaderReflectionData
{
    [[nodiscard]] auto getBoundResourceCount() const -> uint32_t
    {
        return static_cast<uint32_t>(boundResources.size());
    }

    [[nodiscard]] auto getConstantBufferCount() const -> uint32_t
    {
        return static_cast<uint32_t>(constantBuffers.size());
    }

    uint32_t version;
    std::string generatorName;

    std::vector<BoundResource> boundResources;
    std::vector<ConstantBufferReflection> constantBuffers;
};
}

#endif // CYCLONITE_SHADER_REFLECTION_H