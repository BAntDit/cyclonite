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
    void getBlockHeaderData(uint32_t& blockId, uint64_t& offsetBlock, uint64_t& offsetBase, uint64_t& sizeBlock) const
    {
        blockId = id;
        offsetBlock = blockOffset;
        offsetBase = baseOffset;
        sizeBlock = size;
    }

    uint64_t baseOffset;
    uint64_t blockOffset;
    uint64_t size;
    uint32_t id;
};

struct ShaderInfoBlock
{
    void getEntryPoint(std::string& ep) const { ep = entryPoint; }
    void getProfile(uint32_t& tp) { tp = targetProfile; }

    uint32_t targetProfile;
    std::string entryPoint;
};

struct ShaderModuleBinary
{
    void getMagicNumber(uint32_t& magicNumber) const { magicNumber = SHADER_MODULE_MAGIC_NUMBER; }
    void getBlockCount(uint32_t& blockCount) const { blockCount = static_cast<uint32_t>(blockHeaders.size()); }
    void getBlockHeaders(std::vector<ShaderModuleBlockHeader>& blocks) const { blocks = blockHeaders; }
    void getSpirvCode(std::vector<uint32_t>& code) const { code = spirvCode; }
    void getReflectionData(ShaderReflectionData& reflection) const { reflection = reflectionData; }
    void getInfoBlock(ShaderInfoBlock& ib) const { ib = infoBlock; }

    std::vector<ShaderModuleBlockHeader> blockHeaders;
    ShaderInfoBlock infoBlock;
    std::vector<uint32_t> spirvCode;
    ShaderReflectionData reflectionData;
};
}

#endif // CYCLONITE_SHADER_MODULE_BINARY_H