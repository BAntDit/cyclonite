//
// Created by anton on 5/29/26.
//

#include "bindlessResourceManager.h"
#include "gfx/device.h"
#include <bit>

#include "descriptorSet.h"

namespace cyclonite::gfx {
BindlessResourceManager::BindlessResourceManager(size_t swapChainLength, core::ResourceSharedRef deviceRef)
  : frameIndex_{ 0 }
  , updateStack_{}
  , globalDescriptorCount_{ swapChainLength + 1 }
  , globalDescriptorSetRef_{}
  , freeResourceIndices_{}
{
    auto& device = deviceRef.as<gfx::Device>();

    auto allShaderStages = gfx::ShaderStageFlagBits{ gfx::ShaderStageFlags::ALL };
    auto setLayoutFlags = gfx::DescriptorSetLayoutFlagBits{ gfx::DescriptorSetLayoutFlags::UPDATE_AFTER_BIND };
    auto bindingFlags =
      gfx::BindingFlagBits{ gfx::BindingFlags::UPDATE_AFTER_BIND, gfx::BindingFlags::PARTIALLY_BOUND };

    auto bindings = std::array{ gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              0,
                                              gfx::DescriptorType::UNIFORM_BUFFER,
                                              100000,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags },
                                gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              1,
                                              gfx::DescriptorType::STORAGE_BUFFER,
                                              100000,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags },
                                gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              2,
                                              gfx::DescriptorType::SAMPLED_IMAGE,
                                              100000,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags },
                                gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              3,
                                              gfx::DescriptorType::COMBINED_IMAGE_SAMPLER,
                                              100000,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags } };

    auto bindingSchema = device.getOrCreatePipelineBindingSchema(bindings, std::span<gfx::PushConstantRange>{});

    assert(globalDescriptorCount_ <= max_global_descriptor_set_count_v);
    for (auto i = size_t{ 0 }; i < globalDescriptorCount_; i++) {
        globalDescriptorSetRef_[i] = device.allocateDescriptorSetBySchema(
          bindingSchema, metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS), false);
    }
}

void BindlessResourceManager::startFrame(uint64_t frameIndex)
{
    auto it = updateStack_.begin();
    while (it != updateStack_.end()) {
        auto& [data, mask] = *(it);

        if (mask.test(frameIndex)) {
            auto srcIndex = static_cast<size_t>(std::countr_zero(~mask.to_ullong())); // TODO:: test
            assert(srcIndex < globalDescriptorCount_);

            auto& dst = globalDescriptorSetRef_[frameIndex];
            auto const& src = globalDescriptorSetRef_[srcIndex];
            dst.as<gfx::DescriptorSet>().copy(src, data);

            mask.reset(frameIndex);
        }

        if (mask.none()) {
            it = updateStack_.erase(it);
        } else {
            ++it;
        }
    }
}
}
