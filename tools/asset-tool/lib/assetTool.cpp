//
// Created by anton on 8/25/26.
//
#include "assetTool.h"
#include <cassert>
#include <tiny_gltf.h>
#include <string>
#include <iostream>

namespace cyclonite::tools {
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
