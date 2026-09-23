//
// Created by anton on 8/25/26.
//

#include <boost/program_options.hpp>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include "common.h"
#include "assetTool.h"
#include "serialization.h"
#include "binaryStreamWriter.h"

int main(int argc, char* argv[])
{
    namespace po = boost::program_options;

    auto desc = po::options_description{ "asset-tool options", 160 };

    desc.add_options()                                                              // options:
        ("help", "Produce help message")                                            // --help
        ("command", po::value<std::string>(), "command to execute")                 // --command <gltf-to-asset, glb-to-asset>
        ("gltf-src", po::value<std::string>(), "gltf source file")                  // --gltf-src <path>
        ("output", po::value<std::string>(), "output file");                        // --output <path>

    auto vm = po::variables_map{};
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    auto command = cyclonite::tools::AssetToolCommand{};

    auto filepath = std::string{};

    if (vm.contains("gltf-src")) {
        filepath = vm["gltf-src"].as<std::string>();
    }

    if (vm.contains("command")) 
    {
        auto cmd = vm["command"].as<std::string>();
        if (cmd == "gltf-to-asset") {
            if (filepath.empty()) {
                std::cout << "gltf-to-asse command requires path to gltf file! - read help for correct usage" << "\n";
                std::cout << "-----------------------" << "\n";
                std::cout << desc << "\n";
                return 0;
            }

            auto coversionInput = cyclonite::tools::ConversionFromFile{};
            coversionInput.path = filepath;

            command.type = cyclonite::tools::CommandType::GLTF_TO_ASSET;
            command.input = coversionInput;
            command.output = cyclonite::shared::AssetMainBlock{};

        } else if (cmd == "glb-to-asset") {
            if (filepath.empty()) {
                std::cout << "glb-to-asse command requires path to glb file! - read help for correct usage" << "\n";
                std::cout << "-----------------------" << "\n";
                std::cout << desc << "\n";
                return 0;
            }

            auto coversionInput = cyclonite::tools::ConversionFromFile{};
            coversionInput.path = filepath;

            command.type = cyclonite::tools::CommandType::GLTF_TO_ASSET;
            command.input = coversionInput;
            command.output = cyclonite::shared::AssetMainBlock{};

        }
        else {
            std::cout << "uknown command provided! - read help for correct usage" << "\n";
            std::cout << "-----------------------" << "\n";
            std::cout << desc << "\n";
            return 0;
        }
    }
    else {
        std::cout << "no any command provided! - read help for correct usage" << "\n";
        std::cout << "-----------------------" << "\n";
        std::cout << desc << "\n";
        return 0;
    }

    cyclonite::tools::AssetTool::doCommand(command);

    if (command.type == cyclonite::tools::CommandType::GLTF_TO_ASSET ||
        command.type == cyclonite::tools::CommandType::GLTF_TO_ASSET) {
        if (!vm.contains("output")) {
            std::cout << "output path is missed! - read help for correct usage" << "\n";
            std::cout << "-----------------------" << "\n";
            std::cout << desc << "\n";
            return 0;
        }
        auto outputPath = std::filesystem::path(vm["output"].as<std::string>());

        // asset serialization:
        auto assetBinaryModule = cyclonite::shared::AssetModuleBinary{};

        auto assetBinaryModuleBlockCount = uint32_t{ 1 };

        auto headerSerializer = cyclonite::shared::Serializer{
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetBlockHeader::baseOffset>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetBlockHeader::blockOffset>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetBlockHeader::size>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetBlockHeader::id>()
        };

        auto emptyHeader = cyclonite::shared::AssetBlockHeader{}; // to define size (all headers has the same size)
        auto baseOffset = headerSerializer.computeSize(emptyHeader) * assetBinaryModuleBlockCount +
                          sizeof(assetBinaryModuleBlockCount) + sizeof(cyclonite::shared::ASSET_MODULE_MAIN_BLOCK);

        auto blockOffset = uint64_t{ 0 };

        // main block serializer:
        auto mainBlockSerializer = cyclonite::shared::Serializer{
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetMainBlock::buffers>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetMainBlock::bufferViews,
                                               &cyclonite::shared::AssetBufferView::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetMainBlock::dataAccessors,
                                               &cyclonite::shared::AssetDataAccessor::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetMainBlock::subMeshAttributes,
                                               &cyclonite::shared::AssetVertexAttribute::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetMainBlock::subMeshes,
                                               &cyclonite::shared::AssetSubMesh::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetMainBlock::meshes,
                                               &cyclonite::shared::AssetMesh::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetMainBlock::materials,
                                               &cyclonite::shared::AssetMaterial::name>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetMainBlock::nodes,
                                               &cyclonite::shared::AssetNode::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetMainBlock::scenes,
                                               &cyclonite::shared::AssetScene::get>()
        };

        assetBinaryModule.mainBlock = std::move(std::get<cyclonite::shared::AssetMainBlock>(command.output));

        auto& mainBlockHeader = assetBinaryModule.blockHeaders.emplace_back();
        mainBlockHeader.id = cyclonite::shared::ASSET_MODULE_MAIN_BLOCK;
        mainBlockHeader.baseOffset = baseOffset;
        mainBlockHeader.blockOffset = blockOffset;
        mainBlockHeader.size = mainBlockSerializer.computeSize(assetBinaryModule.mainBlock);

        auto serializer = cyclonite::shared::Serializer{
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::getMagicNumber>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::blockHeaders,
                                               &cyclonite::shared::AssetBlockHeader::getBlockHeaderData>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::mainBlock, 
                                               &cyclonite::shared::AssetMainBlock::buffers>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::mainBlock,
                                               &cyclonite::shared::AssetMainBlock::bufferViews,
                                               &cyclonite::shared::AssetBufferView::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::mainBlock,
                                               &cyclonite::shared::AssetMainBlock::dataAccessors,
                                               &cyclonite::shared::AssetDataAccessor::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::mainBlock,
                                               &cyclonite::shared::AssetMainBlock::subMeshAttributes,
                                               &cyclonite::shared::AssetVertexAttribute::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::mainBlock,
                                               &cyclonite::shared::AssetMainBlock::subMeshes,
                                               &cyclonite::shared::AssetSubMesh::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::mainBlock,
                                               &cyclonite::shared::AssetMainBlock::meshes,
                                               &cyclonite::shared::AssetMesh::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::mainBlock,
                                               &cyclonite::shared::AssetMainBlock::materials,
                                               &cyclonite::shared::AssetMaterial::name>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::mainBlock,
                                               &cyclonite::shared::AssetMainBlock::nodes,
                                               &cyclonite::shared::AssetNode::get>(),
            cyclonite::shared::makeAccessChain<&cyclonite::shared::AssetModuleBinary::mainBlock,
                                               &cyclonite::shared::AssetMainBlock::scenes,
                                               &cyclonite::shared::AssetScene::get>()
        }; 
        
        auto binaryWriter = cyclonite::shared::BinaryStreamWriter{ outputPath, cyclonite::shared::Endian::Little };
        serializer.serialize(assetBinaryModule, binaryWriter);
    }

    return 0;
}