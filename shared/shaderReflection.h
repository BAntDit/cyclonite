//
// Created by anton on 12/24/25.
//

#ifndef CYCLONITE_SHADER_REFLECTION_H
#define CYCLONITE_SHADER_REFLECTION_H

#include <cstdint>
#include <string>
#include <vector>

namespace cyclonite::shared
{
enum class ShaderResourceType: uint8_t
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

enum class TextureResourceComponetType: uint8_t
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

enum class ResourceViewDimension: uint8_t
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

struct BoundResource
{
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
    std::string name;
};

struct ShaderReflectionData
{
    uint32_t version;
    std::string generatorName;

    std::vector<BoundResource> boundResources;
    std::vector<ConstantBufferReflection> constantBuffers;
};
}

#endif //CYCLONITE_SHADER_REFLECTION_H