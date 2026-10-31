//
// Created by anton on 10/31/26.
//

#ifndef CYCLONITE_LOADERS_DEFAULT_LOADER_H
#define CYCLONITE_LOADERS_DEFAULT_LOADER_H

#include "assetModuleBinary.h"
#include "binaryStreamReader.h"
#include "geometry.h"
#include "root.h"
#include "scene.h"
#include "serialization.h"
#include "shader.h"
#include "shaderModuleBinary.h"
#include <boost/uuid/string_generator.hpp>
#include <filesystem>
#include <fstream>

namespace cyclonite::loaders {
namespace internal {
struct magic_number_t
{
    uint32_t value;
};
}

template<typename Config>
static auto loadResources(Root<Config>& root,
                          std::string_view destinationGroupName,
                          std::filesystem::path const& sourceLocation) -> std::future<void>
{
    if (!std::filesystem::exists(sourceLocation)) {
        throw std::runtime_error("location does not exists");
    }

    auto& resourceManager = root.resourceManager();

    auto entryCount = std::distance(std::filesystem::recursive_directory_iterator(sourceLocation),
                                    std::filesystem::recursive_directory_iterator{});

    auto futures = std::vector<std::shared_future<void>>{};
    futures.reserve(entryCount);

    for (auto const& entry : std::filesystem::recursive_directory_iterator(sourceLocation)) {
        if (!entry.path().has_extension())
            continue;

        if (entry.path().extension().string() == ".bin") {
            auto magicNumber = internal::magic_number_t{};

            auto magicDeserializer =
              shared::Deserializer{ shared::makeAccessChain<&internal::magic_number_t::value>() };

            auto file = std::ifstream{};

            file.exceptions(std::ios::failbit);
            file.open(entry.path().string(), std::ios::binary | std::ios::in);
            file.exceptions(std::ios::badbit);

            auto magicReader = shared::BinaryStreamReader{ file, shared::Endian::Little };

            magicDeserializer.deserialize(magicNumber, magicReader);

            switch (magicNumber.value) {
                case shared::ASSET_MODULE_MAGIC_NUMBER: {
                    auto assetModuleBinary = shared::AssetModuleBinary{};

                    auto headersDeserializer =
                      shared::Deserializer{ shared::makeAccessChain<&shared::AssetModuleBinary::blockHeaders,
                                                                    &shared::AssetBlockHeader::setBlockHeaderData>() };

                    auto headersReader = shared::BinaryStreamReader{ file, shared::Endian::Little };
                    headersReader.setStreamOffset(sizeof(magicNumber.value));

                    headersDeserializer.deserialize(assetModuleBinary, headersReader);

                    if (!assetModuleBinary.blockHeaders.empty()) {
                        if (auto& header = assetModuleBinary.blockHeaders.back();
                            header.id == shared::ASSET_MODULE_MAIN_BLOCK) {
                            auto [baseOffset, blockOffset, _0, _1] = header;

                            auto mainBlockReader = shared::BinaryStreamReader{ file, shared::Endian::Little };
                            mainBlockReader.setStreamOffset(baseOffset + blockOffset);

                            // main block deserializer:
                            auto mainBlockDeserializer = shared::Deserializer{
                                shared::makeAccessChain<&shared::AssetMainBlock::buffers>(),
                                shared::makeAccessChain<&shared::AssetMainBlock::bufferViews,
                                                        &shared::AssetBufferView::set>(),
                                shared::makeAccessChain<&shared::AssetMainBlock::dataAccessors,
                                                        &shared::AssetDataAccessor::set>(),
                                shared::makeAccessChain<&shared::AssetMainBlock::subMeshAttributes,
                                                        &shared::AssetVertexAttribute::set>(),
                                shared::makeAccessChain<&shared::AssetMainBlock::subMeshes,
                                                        &shared::AssetSubMesh::set>(),
                                shared::makeAccessChain<&shared::AssetMainBlock::meshes, &shared::AssetMesh::set>(),
                                shared::makeAccessChain<&shared::AssetMainBlock::materials,
                                                        &shared::AssetMaterial::set>(),
                                shared::makeAccessChain<&shared::AssetMainBlock::nodes, &shared::AssetNode::set>(),
                                shared::makeAccessChain<&shared::AssetMainBlock::scenes, &shared::AssetScene::set>()
                            };

                            auto mainBlock = std::make_shared<shared::AssetMainBlock>();
                            mainBlockDeserializer.deserialize(*mainBlock, mainBlockReader);

                            auto generator = boost::uuids::string_generator{};

                            if (resourceManager.template groupHasResourceType<Geometry>()) {
                                for (auto& assetSubMesh : mainBlock->subMeshes) {
                                    auto uuid = generator(assetSubMesh.uuid);

                                    auto resourceRef = resourceManager.getResource(destinationGroupName, uuid);
                                    if (!resourceRef.valid()) {
                                        resourceRef = resourceManager.template addResource<Geometry>(
                                          destinationGroupName, uuid, assetSubMesh.name);
                                    }

                                    auto& geometry = resourceRef.template as<Geometry>();
                                    futures.emplace_back(geometry.loadFrom(mainBlock));
                                }
                            } // geometry (end)

                            if (resourceManager.template groupHasResourceType<Scene>()) {
                                for (auto& assetScene : mainBlock->scenes) {
                                    auto uuid = generator(assetScene.uuid);

                                    auto resourceRef = resourceManager.getResource(destinationGroupName, uuid);
                                    if (!resourceRef.valid()) {
                                        resourceRef = resourceManager.template addResource<Scene>(
                                          destinationGroupName, uuid, assetScene.name);
                                    }

                                    auto& scene = resourceRef.template as<Scene>();
                                    auto future = scene.loadFrom(std::pair{ &root, mainBlock });

                                    futures.push_back(std::move(future));
                                }
                            } // scenes (end)
                        } // if main block (end)
                    }
                } break;
                case shared::SHADER_MODULE_MAGIC_NUMBER: {
                    if (resourceManager.template groupHasResourceType<Shader>()) {
                        auto shaderModuleBinary = shared::ShaderModuleBinary{};

                        auto headersDeserializer = shared::Deserializer{
                            shared::makeAccessChain<&shared::ShaderModuleBinary::blockHeaders,
                                                    &shared::ShaderModuleBlockHeader::setBlockHeaderData>()
                        };

                        auto headersReader = shared::BinaryStreamReader{ file, shared::Endian::Little };
                        headersReader.setStreamOffset(sizeof(magicNumber.value));

                        headersDeserializer.deserialize(shaderModuleBinary, headersReader);

                        auto infoBlockIt =
                          std::find_if(shaderModuleBinary.blockHeaders.begin(),
                                       shaderModuleBinary.blockHeaders.end(),
                                       [](auto&& h) -> bool { return (h.id == shared::SHADER_MODULE_INFO_BLOCK); });

                        if (infoBlockIt != shaderModuleBinary.blockHeaders.end()) {
                            auto [baseOffset, blockOffset, _0, _1] = *infoBlockIt;

                            auto infoBlockDeserializer =
                              shared::Deserializer{ shared::makeAccessChain<&shared::ShaderModuleBinary::infoBlock,
                                                                            &shared::ShaderInfoBlock::entryPoint>(),
                                                    shared::makeAccessChain<&shared::ShaderModuleBinary::infoBlock,
                                                                            &shared::ShaderInfoBlock::targetProfile>(),
                                                    shared::makeAccessChain<&shared::ShaderModuleBinary::infoBlock,
                                                                            &shared::ShaderInfoBlock::name>(),
                                                    shared::makeAccessChain<&shared::ShaderModuleBinary::infoBlock,
                                                                            &shared::ShaderInfoBlock::uuid>() };

                            auto infoBlockReader = shared::BinaryStreamReader{ file, shared::Endian::Little };
                            infoBlockReader.setStreamOffset(baseOffset + blockOffset);

                            infoBlockDeserializer.deserialize(shaderModuleBinary, infoBlockReader);

                            auto generator = boost::uuids::string_generator{};
                            auto uuid = generator(shaderModuleBinary.infoBlock.uuid);

                            auto resourceRef = resourceManager.getResource(destinationGroupName, uuid);
                            if (!resourceRef.valid()) {
                                resourceRef = resourceManager.template addResource<Shader>(
                                  destinationGroupName, uuid, shaderModuleBinary.infoBlock.name);
                            }
                        } // if info block found
                    } // sm case
                } break;
                default:
                    [[fallthrough]];
            }

            file.close();
        }
    }

    return multithreading::when_all(futures);
}
}

#endif // CYCLONITE_LOADERS_DEFAULT_LOADER_H
