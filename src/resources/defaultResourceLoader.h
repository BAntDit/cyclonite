//
// Created by anton on 3/20/26.
//

#ifndef CYCLONITE_RESOURCES_DEFAULT_RESOURCELOADER_H
#define CYCLONITE_RESOURCES_DEFAULT_RESOURCELOADER_H

#include "core/resourceSharedRef.h"
#include "shader.h"
#include "shaderModuleBinary.h"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <future>

namespace cyclonite::resources {
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

            file.close();

            switch (magicNumber) {
                case shared::SHADER_MODULE_MAGIC_NUMBER:
                    if constexpr (ResourceGroup::template is_group_resource_type<cyclonite::Shader>) {
                        auto ref = core::ResourceSharedRef{ resourceGroup->template addResource<cyclonite::Shader>() };
                        auto future = ref.as<cyclonite::Shader>().load(entry.path(), std::ios::binary | std::ios::in);

                        // TODO:: store future
                    }
                    break;
            }
        }
    }
    // TODO::
}
}

#endif // CYCLONITE_RESOURCES_DEFAULT_RESOURCELOADER_H