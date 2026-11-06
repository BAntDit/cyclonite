//
// Created by anton on 4/26/26.
//

#include "material.h"
#include "gfx/device.h"
#include "gfx/shader.h"
#include "shader.h"
#include <stdexcept>

namespace cyclonite {
Material::Technique::Technique(core::ResourceSharedRef const& deviceRef,
                               core::ResourceSharedRef passRef,
                               shader_set_t const& shaderSet,
                               gfx::PrimitiveTopology primitiveTopology,
                               bool primitiveRestart,
                               gfx::RasterizationState rasterizationState)
  : pipelineRef{}
{
    auto ref = deviceRef;
    auto& device = ref.as<gfx::Device>();

    auto bindings = std::vector<gfx::Binding>{};
    auto bindingCount = size_t{ 0 };

    auto actualShaders = std::array<core::ResourceSharedRef, metrix::value_cast(gfx::ShaderStageFlags::STAGE_COUNT)>{};
    auto shaderCount = size_t{ 0 };

    for (auto const& [_, shaderRef] : shaderSet) {
        auto const& shader = shaderRef.as<cyclonite::Shader>();
        bindingCount += shader.bindings().size();
        actualShaders[shaderCount++] = shader.gfxShader();
    }

    bindings.reserve(bindingCount);
    for (auto const& [_, shaderRef] : shaderSet) {
        auto const& shader = shaderRef.as<Shader>();

        auto const& stageBindings = shader.bindings();
        for (auto const& binding : stageBindings) {
            bindings.push_back(binding);
        }
    }

    std::sort(bindings.begin(), bindings.end(), [](auto const& a, auto const& b) -> bool {
        auto result = false;
        if (a.set() == b.set()) {
            result = (a.binding() == b.binding())
                       ? metrix::value_cast(a.descriptorType()) < metrix::value_cast(b.descriptorType())
                       : a.binding() < b.binding();
        } else {
            result = a.set() < b.set();
        }

        return result;
    });

    auto [f, l] = std::ranges::unique(bindings, [](auto const& a, auto const& b) -> bool {
        return (a.set() == b.set() && a.binding() == b.binding() && a.descriptorType() == b.descriptorType());
    });
    bindings.erase(f, l);

    auto creationFlags = gfx::PipelineCreationFlagBits{};
    assert(passRef.valid());

    // TODO:: make primitive topology as dynamic parameters

    pipelineRef = device.getOrCreatePrimitiveRasterizationPipeline(creationFlags,
                                                                   bindings,
                                                                   std::span<gfx::PushConstantRange>{},
                                                                   primitiveTopology,
                                                                   primitiveRestart,
                                                                   passRef,
                                                                   rasterizationState,
                                                                   actualShaders);
}

Material::Material(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   resources::ResourceGroupBase* resourceGroup,
                   std::string_view name,
                   boost::uuids::uuid const& uuid)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Material>{ resourceGroup, name, uuid }
  , techniques_{}
{
}

void Material::addPassTechnique(core::ResourceSharedRef const& deviceRef,
                                core::ResourceSharedRef passRef,
                                shader_set_t const& shaderSet,
                                gfx::PrimitiveTopology primitiveTopology,
                                bool primitiveRestart,
                                gfx::RasterizationState rasterizationState)
{
    if (techniques_.contains(static_cast<uint64_t>(passRef.id()))) {
        throw std::runtime_error{ "techinque already exists in material" };
    }

    auto [_, success] = techniques_.emplace(
      static_cast<uint64_t>(passRef.id()),
      Technique{ deviceRef, passRef, shaderSet, primitiveTopology, primitiveRestart, rasterizationState });

    if (!success) {
        throw std::runtime_error{ "could not add technique into material" };
    }
}
}
