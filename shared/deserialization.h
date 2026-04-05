//
// Created by anton on 2/23/26.
//

#ifndef CYCLONITE_DESERIALIZATION_H
#define CYCLONITE_DESERIALIZATION_H

#include "shaderModuleBinary.h"
#include <fstream>
#include <metrix/containers.h>
#include <type_traits>

namespace cyclonite::shared {
// TODO:: move to metrix
template<typename T, typename = void>
struct is_resizeable : std::false_type
{};

template<typename T>
struct is_resizeable<T, std::void_t<decltype(std::declval<T>().resize(int{}))>> : std::true_type
{};

template<typename T, typename std::enable_if_t<std::is_integral_v<std::decay_t<T>>, int> = 0>
static void readStream(T& dst, std::istream& stream)
{
    stream.read(reinterpret_cast<char*>(&dst), sizeof(T));
}

template<typename T,
         typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderModuleBlockHeader>, int> = 0>
static void readStream(T& dst, std::istream& stream);

template<typename T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderInfoBlock>, int> = 0>
static void readStream(T& dst, std::istream& stream);

template<typename T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderReflectionData>, int> = 0>
static void readStream(T& dst, std::istream& stream);

template<typename T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::BoundResource>, int> = 0>
static void readStream(T& dst, std::istream& stream);

template<typename T,
         typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ConstantBufferReflection>, int> = 0>
static void readStream(T& dst, std::istream& stream);

template<typename T, typename std::enable_if_t<metrix::is_iterable_v<std::decay_t<T>>, int> = 0>
static void readStream(T& dst, std::istream& stream)
{
    auto count = uint32_t{ 0 };
    readStream(count, stream);

    if constexpr (is_resizeable<std::decay_t<T>>::value) {
        dst.resize(count);
    } else {
        assert(std::size(dst) == count);
    }

    for (auto& item : dst) {
        readStream(item, stream);
    }
}

template<typename T,
         typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderModuleBlockHeader>, int> /* =0*/>
static void readStream(T& dst, std::istream& stream)
{
    auto blockId = uint32_t{ 0 };
    auto blockOffset = uint64_t{ 0 };
    auto baseOffset = uint64_t{ 0 };
    auto blockSize = uint64_t{ 0 };

    readStream(blockId, stream);
    readStream(blockOffset, stream);
    readStream(baseOffset, stream);
    readStream(blockSize, stream);

    dst.id = blockId;
    dst.blockOffset = blockOffset;
    dst.size = blockSize;
    dst.baseOffset = baseOffset;
}

template<typename T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderInfoBlock>, int> /* = 0*/>
static void readStream(T& dst, std::istream& stream)
{
    auto entryPoint = std::string{};
    readStream(entryPoint, stream);

    auto profile = uint32_t{ 0 };
    readStream(profile, stream);

    auto name = std::string{};
    readStream(name, stream);

    auto uuid = std::string{};
    readStream(uuid, stream);

    dst.name = name;
    dst.uuid = uuid;
    dst.targetProfile = profile;
    dst.entryPoint = entryPoint;
}

template<typename T,
         typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderReflectionData>, int> /* = 0*/>
static void readStream(T& dst, std::istream& stream)
{
    readStream(dst.version, stream);
    readStream(dst.generatorName, stream);
    readStream(dst.boundResources, stream);
    readStream(dst.constantBuffers, stream);
}

template<typename T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::BoundResource>, int> /* = 0*/>
static void readStream(T& dst, std::istream& stream)
{
    readStream(dst.name, stream);

    auto type = uint8_t{ 0 };
    readStream(type, stream);
    switch (type) {
        case uint8_t{ 0 }:
            dst.type = ShaderResourceType::Undefined;
            break;
        case uint8_t{ 1 }:
            dst.type = ShaderResourceType::CBuffer;
            break;
        case uint8_t{ 2 }:
            dst.type = ShaderResourceType::TBuffer;
            break;
        case uint8_t{ 3 }:
            dst.type = ShaderResourceType::Texture;
            break;
        case uint8_t{ 4 }:
            dst.type = ShaderResourceType::Sampler;
            break;
        case uint8_t{ 5 }:
            dst.type = ShaderResourceType::RwTyped;
            break;
        case uint8_t{ 6 }:
            dst.type = ShaderResourceType::StructuredBuffer;
            break;
        case uint8_t{ 7 }:
            dst.type = ShaderResourceType::RwStructuredBuffer;
            break;
        case uint8_t{ 8 }:
            dst.type = ShaderResourceType::ByteAddress;
            break;
        case uint8_t{ 9 }:
            dst.type = ShaderResourceType::RwByteAddress;
            break;
        case uint8_t{ 10 }:
            dst.type = ShaderResourceType::AppendStructuredBuffer;
            break;
        case uint8_t{ 11 }:
            dst.type = ShaderResourceType::ConsumeStructuredBuffer;
            break;
        case uint8_t{ 12 }:
            dst.type = ShaderResourceType::RwStructuredWithCounter;
            break;
        case uint8_t{ 13 }:
            dst.type = ShaderResourceType::RtAccelerationStructure;
            break;
        case uint8_t{ 14 }:
            dst.type = ShaderResourceType::FeedbackTexture;
            break;
        default:
            throw std::runtime_error{ "shader reflection contains unexpected resource type" };
    };

    readStream(dst.space, stream);
    readStream(dst.bindPoint, stream);
    readStream(dst.bindCount, stream);

    auto texComponent = uint8_t{ 0 };
    readStream(texComponent, stream);
    switch (texComponent) {
        case uint8_t{ 0 }:
            dst.textureComponetType = TextureResourceComponetType::Undefined;
            break;
        case uint8_t{ 1 }:
            dst.textureComponetType = TextureResourceComponetType::UNORM;
            break;
        case uint8_t{ 2 }:
            dst.textureComponetType = TextureResourceComponetType::SNORM;
            break;
        case uint8_t{ 3 }:
            dst.textureComponetType = TextureResourceComponetType::SINT;
            break;
        case uint8_t{ 4 }:
            dst.textureComponetType = TextureResourceComponetType::UINT;
            break;
        case uint8_t{ 5 }:
            dst.textureComponetType = TextureResourceComponetType::FLOAT;
            break;
        case uint8_t{ 6 }:
            dst.textureComponetType = TextureResourceComponetType::MIXED;
            break;
        case uint8_t{ 7 }:
            dst.textureComponetType = TextureResourceComponetType::DOUBLE;
            break;
        case uint8_t{ 8 }:
            dst.textureComponetType = TextureResourceComponetType::CONTINUED;
            break;
        default:
            throw std::runtime_error{ "shader reflection contains unexpected texture component type" };
    }

    readStream(dst.sampleCount, stream);

    auto srvDimension = uint32_t{ 0 };
    switch (srvDimension) {
        case uint32_t{ 0 }:
            dst.dimension = ResourceViewDimension::Undefined;
            break;
        case uint32_t{ 1 }:
            dst.dimension = ResourceViewDimension::Buffer;
            break;
        case uint32_t{ 2 }:
            dst.dimension = ResourceViewDimension::Texture1D;
            break;
        case uint32_t{ 3 }:
            dst.dimension = ResourceViewDimension::Texture1DArray;
            break;
        case uint32_t{ 4 }:
            dst.dimension = ResourceViewDimension::Texture2D;
            break;
        case uint32_t{ 5 }:
            dst.dimension = ResourceViewDimension::Texture2DArray;
            break;
        case uint32_t{ 6 }:
            dst.dimension = ResourceViewDimension::Texture2DMS;
            break;
        case uint32_t{ 7 }:
            dst.dimension = ResourceViewDimension::Texture2DMSArray;
            break;
        case uint32_t{ 8 }:
            dst.dimension = ResourceViewDimension::Texture3D;
            break;
        case uint32_t{ 9 }:
            dst.dimension = ResourceViewDimension::TextureCube;
            break;
        case uint32_t{ 10 }:
            dst.dimension = ResourceViewDimension::TextureCubeArray;
            break;
        case uint32_t{ 11 }:
            dst.dimension = ResourceViewDimension::BufferEx;
            break;
        default:
            throw std::runtime_error{ "shader reflection contains unexpected SRV dimension" };
    }
}

template<typename T,
         typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ConstantBufferReflection>, int> /* = 0*/>
static void readStream(T& dst, std::istream& stream)
{
    readStream(dst.name, stream);

    auto bufferType = uint8_t{ 0 };
    readStream(bufferType, stream);
    switch (bufferType) {
        case uint8_t{ 0 }:
            dst.type = ConstantBufferType::Undefined;
            break;
        case uint8_t{ 1 }:
            dst.type = ConstantBufferType::CBuffer;
            break;
        case uint8_t{ 2 }:
            dst.type = ConstantBufferType::TBuffer;
            break;
        case uint8_t{ 3 }:
            dst.type = ConstantBufferType::InterfacePointer;
            break;
        case uint8_t{ 4 }:
            dst.type = ConstantBufferType::ResourceBindInfo;
            break;
        default:
            throw std::runtime_error{ "shader reflection contains unexpected constant buffer type " };
    }

    readStream(dst.size, stream);
}
}

#endif // CYCLONITE_DESERIALIZATION_H