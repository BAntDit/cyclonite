//
// Created by anton on 8/25/26.
//
#include "assetTool.h"
#include <cassert>
#include <tiny_gltf.h>
#include <string>
#include <iostream>

#if defined(_WIN32) // _WIN32 / _WIN64 at once
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif

#include <limits>

namespace cyclonite::tools {
namespace internal
{
struct TwoIntKey
{
    TwoIntKey() : 
        value{ std::numeric_limits<uint64_t>::max() }
    {}

    explicit TwoIntKey(uint64_t v) : 
        value{ v }
    {}

    TwoIntKey(uint32_t a, uint32_t b) : 
        value{ static_cast<uint64_t>(a) | (static_cast<uint64_t>(b) << 32ull) }
    {}

    uint64_t value;
};
}

/*static */ void AssetTool::doCommand(AssetToolCommand& command)
{
    switch (command.type) {
        case CommandType::GLB_TO_ASSET:
            [[fallthrough]];
        case CommandType::GLTF_TO_ASSET: {
            auto model = tinygltf::Model{};
            auto const& input = std::get<ConversionFromFile>(command.input);
            auto& output = std::get<shared::AssetModuleBinary>(command.output);
            auto loader = tinygltf::TinyGLTF{};
            
            auto err = std::string{};
            auto warn = std::string{};

            if (command.type == CommandType::GLTF_TO_ASSET) {
                if (!loader.LoadASCIIFromFile(&model, &err, &warn, input.path.string())) {
                    throw std::runtime_error{ err };
                }
            } else if (command.type == CommandType::GLB_TO_ASSET) {
                if (!loader.LoadBinaryFromFile(&model, &err, &warn, input.path.string())) {
                    throw std::runtime_error{ err };
                }
            }

            if (!warn.empty()) {
                std::cout << warn << std::endl;
            }

            output.buffers.reserve(model.buffers.size());
            for (auto const& gltfBuffer : model.buffers) {
                auto& assetBuffer = output.buffers.emplace_back();
                assetBuffer.reserve(gltfBuffer.data.size());

                for (auto byte : gltfBuffer.data) {
                    assetBuffer.push_back(std::byte{ byte });
                }
            }

            output.bufferViews.reserve(model.bufferViews.size());
            for (auto const& gltfBufferView : model.bufferViews) {
                auto& assetBufferView = output.bufferViews.emplace_back();
                assetBufferView.bufferIndex = static_cast<uint32_t>(gltfBufferView.buffer);
                assetBufferView.offset = static_cast<uint32_t>(gltfBufferView.byteOffset);
                assetBufferView.size = static_cast<uint32_t>(gltfBufferView.byteLength);
                assetBufferView.stride = static_cast<uint32_t>(gltfBufferView.byteStride);
            }

            auto subMeshCount = uint32_t{0};
            for (auto const& gltfMesh : model.meshes) {
                subMeshCount += gltfMesh.primitives.size();
            }

            output.subMeshes.reserve(subMeshCount);

            /*auto accessor = tinygltf::Accessor{};
            auto mesh = tinygltf::Mesh{};
            auto primitive = tinygltf::Primitive{};

            primitive.attributes*/

            // model.
            // model.buffers
        } break;
        default:
            assert(false);
    }
}
}
