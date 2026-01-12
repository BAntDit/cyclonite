//
// Created by anton on 1/12/26.
//

#ifndef CYCLONITE_SHADER_MODULE_BINARY_H
#define CYCLONITE_SHADER_MODULE_BINARY_H

#include <cstdint>
#include <cstddef>
#include <vector>

namespace cyclonite::shared
{
inline constexpr uint32_t SHADER_MODULE_MAGIC_NUMBER = 0x43534D01;     // Cyclonite Shader Module v01
inline constexpr uint32_t SHADER_MODULE_SPIRV_BLOCK = 0x43534D53;      // Cyclonite Shader Module Spir-V block
inline constexpr uint32_t SHADER_MODULE_REFLECTION_BLOCK = 0x43534D52; // Cyclonite Shader Module reflection block

#pragma pack(push, 1)
struct ShaderModuleBlockHeader
{
    uint64_t baseOffset;
    uint64_t blockOffset;
    uint64_t size;
    uint32_t id;
};
#pragma pack(pop)

struct ShaderModuleBinary
{
    std::vector<ShaderModuleBlockHeader> blockHeaders;
    std::vector<uint32_t> spirvCode;
    std::vector<std::byte> reflectionBinary;
};
}

#endif // CYCLONITE_SHADER_MODULE_BINARY_H