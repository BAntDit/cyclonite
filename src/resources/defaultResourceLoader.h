//
// Created by anton on 3/20/26.
//

#ifndef CYCLONITE_RESOURCES_DEFAULT_RESOURCE_LOADER_H
#define CYCLONITE_RESOURCES_DEFAULT_RESOURCE_LOADER_H

#include "binaryStreamReader.h"
#include "core/resourceSharedRef.h"
#include "multithreading/utility.h"
#include "serialization.h"
#include "shader.h"
#include "shaderModuleBinary.h"
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <cassert>
#include <filesystem>
#include <fstream>

namespace cyclonite::resources {
namespace internal {
struct magic_number_t
{
    uint32_t value;
};
}

class ResourceGroupBase;

class DefaultResourceLoader
{
public:
    explicit DefaultResourceLoader(std::filesystem::path const& location);

    template<typename ResourceGroup>
    void setResourceGroup(ResourceGroup& resourceGroup);

    auto load() -> std::future<void>
    {
        assert(loadInvoke_ != nullptr);
        assert(group_ != nullptr);

        return loadInvoke_(this);
    }

private:
    template<typename ResourceGroup>
    static auto loadImpl(DefaultResourceLoader* context) -> std::future<void>;

    using load_f = std::future<void> (*)(DefaultResourceLoader*);

    load_f loadInvoke_;
    ResourceGroupBase* group_;
    std::filesystem::path location_;
};

template<typename ResourceGroup>
void DefaultResourceLoader::setResourceGroup(ResourceGroup& resourceGroup)
{
    group_ = &resourceGroup;
    loadInvoke_ = &loadImpl<ResourceGroup>;
}

template<typename ResourceGroup>
/*static*/ auto DefaultResourceLoader::loadImpl(DefaultResourceLoader* context) -> std::future<void>
{
    auto* resourceGroup = std::launder(reinterpret_cast<ResourceGroup*>(context->group_));

    if (!std::filesystem::exists(context->location_)) {
        throw std::runtime_error("location does not exists");
    }

    auto entryCount = std::distance(std::filesystem::recursive_directory_iterator(context->location_),
                                    std::filesystem::recursive_directory_iterator{});

    auto futures = std::vector<std::shared_future<void>>{};
    futures.reserve(entryCount);

    for (auto const& entry : std::filesystem::recursive_directory_iterator(context->location_)) {
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
                case shared::SHADER_MODULE_MAGIC_NUMBER:
                    if constexpr (ResourceGroup::template is_group_resource_type<cyclonite::Shader>) {
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

                            auto infoBlockDeserializaer =
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

                            infoBlockDeserializaer.deserialize(shaderModuleBinary, infoBlockReader);

                            auto generator = boost::uuids::string_generator{};
                            auto uuid = generator(shaderModuleBinary.infoBlock.uuid);

                            auto ref = resourceGroup->getResource(uuid);
                            if (!ref.valid()) {
                                ref = core::ResourceSharedRef{ resourceGroup->template addResource<cyclonite::Shader>(
                                  shaderModuleBinary.infoBlock.name, uuid) };
                            }

                            auto future =
                              ref.template as<cyclonite::Shader>().load(entry.path(), std::ios::binary | std::ios::in);
                            futures.push_back(std::move(future));
                        } // if info block found
                    } // sm case
                    break;
                default:
                    [[fallthrough]];
            }

            file.close();
        }
    }

    return multithreading::when_all(futures);
}
}

#endif // CYCLONITE_RESOURCES_DEFAULT_RESOURCE_LOADER_H