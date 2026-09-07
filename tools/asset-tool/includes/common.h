//
// Created by anton on 8/26/26.
//

#ifndef ASSET_TOOL_COMMON_H
#define ASSET_TOOL_COMMON_H

#include <cstdint>
#include <filesystem>
#include <variant>
#include <optional>
#include <array>
#include "fvf.h"
#include "assetModuleBinary.h"

namespace cyclonite::tools {
enum class CommandType: uint16_t
{
    GLTF_TO_ASSET = 0,
    GLB_TO_ASSET = 1
};

struct ConversionFromFile
{
    std::filesystem::path path;
};

struct AssetToolCommand
{
    CommandType type;
    std::variant<std::monostate, ConversionFromFile> input;
    std::variant<std::monostate, shared::AssetModuleBinary> output;
};
}

#endif // ASSET_TOOL_COMMON_H