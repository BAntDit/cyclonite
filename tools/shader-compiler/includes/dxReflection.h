//
// Created by anton on 12/22/25.
//

#ifndef TOOLS_SHADER_COMPILER_DXREFLECTION_H
#define TOOLS_SHADER_COMPILER_DXREFLECTION_H
#include "shaderReflection.h"
#include <metrix/type_traits.h>
#include <metrix/containers.h>
#include <metrix/enum.h>

struct ID3D12ShaderReflection;
struct ID3D12ShaderReflectionConstantBuffer;

namespace cyclonite::tools {
void collectReflection(ID3D12ShaderReflection* dxcShaderReflection, shared::ShaderReflectionData& reflectionData);

namespace internal {
template <typename T, typename std::enable_if_t<std::is_enum_v<std::decay_t<T>> || std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>, int> = 0>
auto getByteSize(T&& t) -> uint64_t
{
    return static_cast<uint64_t>(sizeof(t));
}

template <typename T, typename std::enable_if_t<metrix::is_iterable_v<std::decay_t<T>>, int> = 0>
auto getByteSize(T&& t) -> uint64_t;

template <typename T, typename std::enable_if_t<std::is_same_v<shared::BoundResource, std::decay_t<T>>, int> = 0>
auto getByteSize(T&& t) -> uint64_t
{
    auto size = uint64_t{0};

    size+= getByteSize(t.name);
    size+= getByteSize(t.type);
    size+= getByteSize(t.space);
    size+= getByteSize(t.bindPoint);
    size+= getByteSize(t.bindCount);
    size+= getByteSize(t.textureComponetType);
    size+= getByteSize(t.sampleCount);
    size+= getByteSize(t.dimension);

    return size;
}

template <typename T, typename std::enable_if_t<std::is_same_v<shared::ConstantBufferReflection, std::decay_t<T>>, int> = 0>
auto getByteSize(T&& t) -> uint64_t
{
    auto size = uint64_t{0};

    size+= getByteSize(t.name);
    size+= getByteSize(t.type);
    size+= getByteSize(t.size);

    return size;
}

template <typename T, typename std::enable_if_t<metrix::is_iterable_v<std::decay_t<T>>, int> = 0>
auto getByteSize(T&& t) -> uint64_t
{
    auto size = uint64_t{0};
    for (auto&& v : t) {
        size += getByteSize(std::forward<decltype(v)>(v));
    }
    return sizeof(uint64_t) /*count itself*/ + size;
}
}

inline auto getReflectionDataSize(shared::ShaderReflectionData const& reflectionData) -> uint64_t
{
    auto size = uint64_t{0};

    size += internal::getByteSize(reflectionData.version);
    size += internal::getByteSize(reflectionData.generatorName);
    size += internal::getByteSize(reflectionData.boundResources);
    size += internal::getByteSize(reflectionData.constantBuffers);

    return size;
}
}

#endif // TOOLS_SHADER_COMPILER_DXREFLECTION_H