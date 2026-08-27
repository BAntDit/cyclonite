//
// Created by anton on 8/25/26.
//

#ifndef CYCLONITE_SHARED_ASSET_MODULE_BINARY_H
#define CYCLONITE_SHARED_ASSET_MODULE_BINARY_H

#include <cstdint>
#include <cstddef>
#include <vector>

namespace cyclonite::shared
{
inline constexpr uint32_t ASSET_MODULE_MAGIC_NUMBER = 0x41534301;     // Cyclonite Asset Module v01

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

struct AssetModuleBinary
{
    std::vector<AssetBlockHeader> blockHeaders;
    std::vector<std::vector<std::byte>> buffers;
};
}

#endif // CYCLONITE_SHARED_ASSET_MODULE_BINARY_H