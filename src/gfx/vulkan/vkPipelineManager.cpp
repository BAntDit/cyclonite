//
// Created by anton on 11/9/25.
//

#include "vkPipelineManager.h"
#include "gfx/device.h"
#include <ranges>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
auto PipelineManager::getOrCreatePipelineBindingSchema(std::span<Binding const> bindings,
                                                       std::span<PushConstantRange const> pushConstantRanges)
  -> core::ResourceSharedRef
{
    auto bindingSchemaRef = core::ResourceSharedRef();

    auto sets = bindings | std::views::transform([](auto const& binding) -> uint32_t { return binding.set(); });

    for (auto set : sets) {
        auto sb = bindings | std::views::filter([=](auto const& binding) -> uint32_t { return binding.set() == set; });
    }

    return bindingSchemaRef;
}

auto PipelineManager::getOrCreateDescriptorSetLayout(std::span<Binding const> bindings) -> core::ResourceSharedRef
{
    auto descriptorSetLayoutRef = core::ResourceSharedRef();

    for (auto i = uint32_t{ 0 }; i < descriptorSetLayoutCount_; i++) {
        auto& [vb, p] = descriptorSetLayouts_[i];
        if (vb.size() == bindings.size() && std::equal(vb.begin(), vb.end(), bindings.begin())) {
            auto& [r, tp] = p;
            descriptorSetLayoutRef = r;
            tp = std::chrono::high_resolution_clock::now();
        }
    }

    if (!descriptorSetLayoutRef.valid()) {
        if (descriptorSetLayoutCount_ == max_descriptor_set_layout_count_v) {
            const auto countToRelease = uint32_t{ 16 };
            freeDescriptorSetLayouts(countToRelease);
        }

        assert(deviceRef_.valid());
        auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();

        auto& [vb, p] = descriptorSetLayouts_[descriptorSetLayoutCount_];
        auto& [r, tp] = p;
        vb = std::vector<Binding>(bindings.begin(), bindings.end());
        tp = std::chrono::high_resolution_clock::now();
        r = device.createDescriptorSetLayout(bindings);

        descriptorSetLayoutRef = r;
    }

    assert(descriptorSetLayoutRef.valid());
    return descriptorSetLayoutRef;
}

void PipelineManager::freeDescriptorSetLayouts(uint32_t count)
{
    std::ranges::sort(descriptorSetLayouts_, [](auto const& a, auto const& b) -> bool {
        auto const& [_1, p1] = a;
        auto const& [_2, p2] = b;
        auto const& [_3, tp1] = p1;
        auto const& [_4, tp2] = p1;

        return tp1 < tp2;
    });

    for (auto i = count; i != 0; i--) {
        auto& [vb, p] = descriptorSetLayouts_[i];
        auto& [r, tp] = p;

        vb = std::vector<Binding>{};
        r = core::ResourceSharedRef();

        descriptorSetLayoutCount_--;
    }
}
}
#endif