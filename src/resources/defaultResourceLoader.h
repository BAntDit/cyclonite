//
// Created by anton on 3/20/26.
//

#ifndef CYCLONITE_RESOURCES_DEFAULT_RESOURCELOADER_H
#define CYCLONITE_RESOURCES_DEFAULT_RESOURCELOADER_H

#include "core/resourceSharedRef.h"
#include "multithreading/utility.h"
#include "shader.h"
#include "shaderModuleBinary.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <future>
#include <metrix/containers.h>

namespace cyclonite::resources {
class ResourceGroupBase;

class DefaultResourceLoader
{
    // TODO:: move to metrix
    template<typename T, typename = void>
    struct is_resizeable : std::false_type
    {};

    template<typename T>
    struct is_resizeable<T, std::void_t<decltype(std::declval<T>().resize(int{}))>> : std::true_type
    {};

    template<typename T, typename std::enable_if_t<std::is_integral_v<std::decay_t<T>>, int> = 0>
    static void readStream(T& dst, std::istream& stream)
    {
        stream.read(reinterpret_cast<char*>(&dst), sizeof(T));
    }

    template<typename T,
             typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderModuleBlockHeader>, int> = 0>
    static void readStream(T& dst, std::istream& stream);

    template<typename T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderInfoBlock>, int> = 0>
    static void readStream(T& dst, std::istream& stream);

    template<typename T, typename std::enable_if_t<metrix::is_iterable_v<std::decay_t<T>>, int> = 0>
    static void readStream(T& dst, std::istream& stream)
    {
        auto count = uint32_t{ 0 };
        readStream(count, stream);

        if constexpr (is_resizeable<std::decay_t<T>>::value) {
            dst.resize(count);
        } else {
            assert(std::size(dst) == count);
        }

        for (auto& item : dst) {
            readStream(item, stream);
        }
    }

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
        if (entry.path().has_extension())
            continue;

        if (entry.path().extension().string() == ".bin") {
            auto file = std::ifstream{};

            file.exceptions(std::ios::failbit);
            file.open(entry.path().string(), std::ios::binary | std::ios::in);
            file.exceptions(std::ios::badbit);

            auto magicNumber = uint32_t{ 0 };
            file.read(reinterpret_cast<char*>(&magicNumber), sizeof(uint32_t));

            switch (magicNumber) {
                case shared::SHADER_MODULE_MAGIC_NUMBER:
                    if constexpr (ResourceGroup::template is_group_resource_type<cyclonite::Shader>) {
                        auto smBlockHeaders = std::vector<shared::ShaderModuleBlockHeader>{};
                        readStream(smBlockHeaders, file);

                        auto infoBlockIt =
                          std::find_if(smBlockHeaders.begin(), smBlockHeaders.end(), [](auto&& h) -> bool {
                              return (h.id == shared::SHADER_MODULE_INFO_BLOCK);
                          });

                        if (infoBlockIt != smBlockHeaders.end()) {
                            auto [baseOffset, blockOffset, size, _] = *infoBlockIt;
                            file.seekg(baseOffset + blockOffset, std::ios::beg);

                            auto moduleInfo = shared::ShaderInfoBlock{};
                            readStream(moduleInfo, file);

                            // TODO:: test if resource exists

                            auto ref = core::ResourceSharedRef{ resourceGroup->template addResource<cyclonite::Shader>(
                              moduleInfo.name, moduleInfo.uuid) };
                            auto future =
                              ref.as<cyclonite::Shader>().load(entry.path(), std::ios::binary | std::ios::in);
                            futures.push_back(std::move(future));
                        }
                    }
                    break;
            }

            file.close();
        }
    }

    return multithreading::when_all(futures);
}

template<typename T,
         typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderModuleBlockHeader>, int> /* =0*/>
/*static */ void DefaultResourceLoader::readStream(T& dst, std::istream& stream)
{
    auto blockId = uint32_t{ 0 };
    auto blockOffset = uint64_t{ 0 };
    auto baseOffset = uint64_t{ 0 };
    auto blockSize = uint64_t{ 0 };

    readStream(blockId, stream);
    readStream(blockOffset, stream);
    readStream(baseOffset, stream);
    readStream(blockSize, stream);

    dst.id = blockId;
    dst.blockOffset = blockOffset;
    dst.size = blockSize;
    dst.baseOffset = baseOffset;
}

template<typename T, typename std::enable_if_t<std::is_same_v<std::decay_t<T>, shared::ShaderInfoBlock>, int> /* = 0*/>
/*static */ void DefaultResourceLoader::readStream(T& dst, std::istream& stream)
{
    // TODO::
}
}

#endif // CYCLONITE_RESOURCES_DEFAULT_RESOURCELOADER_H