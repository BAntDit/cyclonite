//
// Created by anton on 8/26/26.
//

#ifndef ASSET_TOOL_COMMON_H
#define ASSET_TOOL_COMMON_H

#include <cstdint>
#include <filesystem>

namespace cyclonite::tools {
enum class CommandType: uint16_t
{
    GLTF_TO_ASSET = 0,
    GLB_TO_ASSET = 1
};

struct AssetToolCommand
{
    CommandType type;
    std::filesystem::path gltfPath;
};
}

#endif // ASSET_TOOL_COMMON_H