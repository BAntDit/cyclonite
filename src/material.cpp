//
// Created by anton on 4/26/26.
//

#include "material.h"
#include "gfx/device.h"
#include "gfx/shader.h"
#include "shader.h"

namespace cyclonite {
Material::Material(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   resources::ResourceGroupBase* resourceGroup,
                   std::string_view name,
                   boost::uuids::uuid const& uuid)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Material>{ resourceGroup, name, uuid }
  , rawData_{}
  , pipeline_{}
{
}

void Material::prepareImpl()
{
    auto& g = group();
    auto ref = g.deviceRef();
    auto& device = ref.as<gfx::Device>();

    assert(rawData_);
    auto bindings = std::vector<gfx::Binding>{};
    auto bindingCount = size_t{ 0 };

    auto actualShaders = std::array<core::ResourceSharedRef, metrix::value_cast(gfx::ShaderStageFlags::STAGE_COUNT)>{};
    auto shaderCount = size_t{ 0 };

    auto& shaderSet = rawData_->shaderSet;
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
    auto passRef = rawData_->passRef;
    assert(passRef.valid());

    auto primitiveTopology = rawData_->primitiveTopology;
    auto primitiveRestart = rawData_->primitiveRestart;
    auto const& rasterizationState = rawData_->rasterizationState;

    // TODO:: make primitive topology as dynamic parameters

    pipeline_ = device.getOrCreatePrimitiveRasterizationPipeline(creationFlags,
                                                                 bindings,
                                                                 std::span<gfx::PushConstantRange>{},
                                                                 primitiveTopology,
                                                                 primitiveRestart,
                                                                 passRef,
                                                                 rasterizationState,
                                                                 actualShaders);
}

auto Material::manualSetup(core::ResourceSharedRef passRef,
                           shader_set_t const& shaderSet,
                           gfx::RasterizationState const& rasterizationState,
                           gfx::PrimitiveTopology primitiveTopology /* = gfx::PrimitiveTopology::TRIANGLE_LIST*/,
                           bool primitiveRestart /* = false*/) -> std::shared_future<void>
{
    [[maybe_unused]] auto expectedState = resources::is_loadable<Material> ? resources::ManagedResourceState::Loaded
                                                                           : resources::ManagedResourceState::Initial;
    assert(state() == expectedState);

    rawData_.reset();
    rawData_ = std::make_unique<raw_data_t>();
    rawData_->passRef = std::move(passRef);
    rawData_->shaderSet = shader_set_t{ shaderSet };
    rawData_->rasterizationState = rasterizationState;
    rawData_->primitiveTopology = primitiveTopology;
    rawData_->primitiveRestart = primitiveRestart;

    return prepare();
}
}
