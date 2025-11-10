//
// Created by anton on 11/9/25.
//

#include "vkPipelineManager.h"
#include "vkDevice.h"
#include <algorithm>
#include <ranges>
#include <unordered_set>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
auto PipelineManager::getOrCreatePipelineBindingSchema(std::span<Binding const> bindings,
                                                       std::span<PushConstantRange const> pushConstantRanges)
  -> core::ResourceSharedRef
{
    auto bindingSchemaRef = core::ResourceSharedRef();

    auto sets = std::unordered_set<uint32_t>{};
    std::transform(bindings.begin(), bindings.end(), std::inserter(sets, sets.end()), [](auto const& b) -> uint32_t {
        return b.set();
    });

    auto descrSets = std::vector<core::ResourceSharedRef>{};
    descrSets.reserve(sets.size());

    for (auto set : sets) {
        auto sb = bindings | std::views::filter([=](auto const& binding) -> uint32_t { return binding.set() == set; });
        descrSets.emplace_back(getOrCreateDescriptorSetLayout(std::vector<Binding>(sb.begin(), sb.end())));
    }

    for (auto i = uint32_t{ 0 }; i < pipelineLayoutCount_; i++) {
        auto& [vs, vr, p] = pipelineLayouts_[i];

        if (vs.size() == descrSets.size() && vr.size() == pushConstantRanges.size()) {
            if (!std::equal(vs.begin(), vs.end(), descrSets.begin(), [](auto a, auto const& b) -> bool {
                    return a == static_cast<uint64_t>(b.id());
                }))
                continue;

            if (!std::equal(vr.begin(), vr.end(), pushConstantRanges.begin(), [](auto const& a, auto const& b) -> bool {
                    auto [size1, offset1, state1] = a;
                    auto [size2, offset2, state2] = b;

                    return size1 == size2 && offset1 == offset2 && state1 == state2;
                }))
                continue;

            auto& [r, tp] = p;

            tp = std::chrono::high_resolution_clock::now();
            bindingSchemaRef = r;

            break;
        }
    }

    if (!bindingSchemaRef.valid()) {
        // TODO:: 
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

            break;
        }
    }

    if (!descriptorSetLayoutRef.valid()) {
        if (descriptorSetLayoutCount_ == max_descriptor_set_layout_count_v) {
            const auto countToRelease = uint32_t{ 16 };
            freeDescriptorSetLayouts(countToRelease);
        }

        assert(device_ != nullptr);

        auto& [vb, p] = descriptorSetLayouts_[descriptorSetLayoutCount_];
        auto& [r, tp] = p;
        vb = std::vector<Binding>(bindings.begin(), bindings.end());
        tp = std::chrono::high_resolution_clock::now();
        r = device_->createDescriptorSetLayout(bindings);

        descriptorSetLayoutCount_++;

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

        return tp1 > tp2; // old one first
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