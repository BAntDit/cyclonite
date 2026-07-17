//
// Created by anton on 11/9/25.
//

#include "vkPipelineBindingSchema.h"
#include "gfx/device.h"
#include "vkDescriptorSetLayout.h"
#include "vkException.h"
#include <ranges>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
PipelineBindingSchema::PipelineBindingSchema(core::ResourceManagerBase* resourceManager,
                                             core::ResourceId resourceId,
                                             core::ResourceSharedRef deviceRef,
                                             std::span<core::ResourceSharedRef const> descriptorSetLayouts,
                                             std::span<PushConstantRange const> pushConstantRanges)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , descriptorSetLayouts_{}
  , constantRanges_(pushConstantRanges.begin(), pushConstantRanges.end())
  , pipelineLayout_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(),
                     vkDestroyPipelineLayout }
{
    assert(deviceRef.valid());
    auto& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();

    assert(descriptorSetLayouts.size() <= metrix::value_cast(DescriptorSpace::DESCRIPTOR_SPACE_COUNT));
    for (auto setIdx = uint32_t{ 0 }, setCount = static_cast<uint32_t>(descriptorSetLayouts.size()); setIdx < setCount;
         setIdx++) {
        descriptorSetLayouts_[setIdx] = descriptorSetLayouts[setIdx];
    }

    auto layoutRange = descriptorSetLayouts | std::views::transform([](auto const& ref) -> VkDescriptorSetLayout {
                           assert(ref.valid());
                           return ref.template as<DescriptorSetLayout>().handle();
                       });

    auto vkDescriptorSetLayout = std::vector<VkDescriptorSetLayout>(layoutRange.begin(), layoutRange.end());

    auto constantRangesRange = pushConstantRanges | std::views::transform([](auto const& range) -> VkPushConstantRange {
                                   auto vkRange = VkPushConstantRange{};
                                   vkRange.offset = range.offset;
                                   vkRange.size = range.size;
                                   vkRange.stageFlags = range.stage.template cast_to<VkShaderStageFlags>();
                                   return vkRange;
                               });

    auto vkConstantRanges = std::vector<VkPushConstantRange>(constantRangesRange.begin(), constantRangesRange.end());

    auto createInfo = VkPipelineLayoutCreateInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayout.size());
    createInfo.pSetLayouts = vkDescriptorSetLayout.data();

    if (!pushConstantRanges.empty()) {
        createInfo.pushConstantRangeCount = static_cast<uint32_t>(vkConstantRanges.size());
        createInfo.pPushConstantRanges = vkConstantRanges.data();
    }

    if (auto vkResult = vkCreatePipelineLayout(device.handle(), &createInfo, nullptr, &pipelineLayout_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreatePipelineLayout" };
    }
}

PipelineBindingSchema::PipelineBindingSchema(core::ResourceManagerBase* resourceManager,
                                             core::ResourceId resourceId,
                                             core::ResourceSharedRef deviceRef,
                                             std::span<core::ResourceSharedRef const> descriptorSetLayouts)
  : PipelineBindingSchema(resourceManager, resourceId, std::move(deviceRef), descriptorSetLayouts, {})
{
}
}
#endif
