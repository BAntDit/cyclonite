//
// Created by anton on 11/8/25.
//

#ifndef CYCLONITE_VK_PIPELINE_MANAGER_H
#define CYCLONITE_VK_PIPELINE_MANAGER_H

#include "core/resourceSharedRef.h"
#include "gfx/binding.h"
#include <chrono>
#include <map>

#if defined(GFX_DRIVER_VULKAN)
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class PipelineManager
{
public:
    // TODO:: adds PushConstantRange
    auto getOrCreatePipelineLayout() -> core::ResourceSharedRef
    {

        // VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
    }

private:
    static constexpr auto max_descriptor_set_layout_count_v = 128;
    static constexpr auto max_pipeline_set_layout_count_v = 1024;

    // TODO:: find better way to cache descriptor set layouts nad pipeline layouts
    // vector contains descriptor set layout ids
    std::array<std::tuple<std::vector<uint64_t>,
                          std::pair<core::ResourceSharedRef, std::chrono::high_resolution_clock::time_point>>,
               max_pipeline_set_layout_count_v>
      pipelineLayouts_;

    std::array<std::tuple<std::vector<Binding>,
                          std::vector<std::tuple<uint32_t, uint32_t, VkShaderStageFlags>>,
                          std::pair<core::ResourceSharedRef, std::chrono::high_resolution_clock::time_point>>,
               max_descriptor_set_layout_count_v>
      descriptorSetLayouts_;

    // key is layout resource Id
    std::multimap<uint64_t, core::ResourceSharedRef> descriptorPools_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_PIPELINE_MANAGER_H