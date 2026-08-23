//
// Created by anton on 1/12/26.
//

#ifndef CYCLONITE_SHADER_MODULE_BINARY_H
#define CYCLONITE_SHADER_MODULE_BINARY_H

#include "shaderReflection.h"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace cyclonite::shared {
inline constexpr uint32_t SHADER_MODULE_MAGIC_NUMBER = 0x43534D01;     // Cyclonite Shader Module v01
inline constexpr uint32_t SHADER_MODULE_INFO_BLOCK = 0x43534D54;       // Cyclonite Shader Module info block
inline constexpr uint32_t SHADER_MODULE_SPIRV_BLOCK = 0x43534D53;      // Cyclonite Shader Module Spir-V block
inline constexpr uint32_t SHADER_MODULE_REFLECTION_BLOCK = 0x43534D52; // Cyclonite Shader Module reflection block

struct ShaderModuleBlockHeader
{
    void getBlockHeaderData(uint32_t& baseOffsetOut, uint32_t& blockOffsetOut, uint32_t& sizeOut, uint32_t& idOut) const
    {
        baseOffsetOut = baseOffset;
        blockOffsetOut = blockOffset;
        sizeOut = size;
        idOut = id;
    }

    void setBlockHeaderData(uint32_t baseOffsetIn, uint32_t blockOffsetIn, uint32_t sizeIn, uint32_t idIn)
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

struct ShaderInfoBlock
{
    uint32_t targetProfile;
    std::string entryPoint;
    std::string name;
    std::string uuid;
};

struct ShaderModuleBinary
{
    [[nodiscard]] auto getMagicNumber() const -> uint32_t { return SHADER_MODULE_MAGIC_NUMBER; }

    void testMagicNumber(uint32_t magicNumber)
    {
        if (magicNumber != SHADER_MODULE_MAGIC_NUMBER) {
            throw std::runtime_error("Invalid shader magic number");
        }
    }

    std::vector<ShaderModuleBlockHeader> blockHeaders;
    ShaderInfoBlock infoBlock;
    std::vector<uint32_t> spirvCode;
    ShaderReflectionData reflectionData;
};
}

#endif // CYCLONITE_SHADER_MODULE_BINARY_H