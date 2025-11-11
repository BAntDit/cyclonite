//
// Created by anton on 11/8/25.
//

#ifndef CYCLONITE_VK_PIPELINE_MANAGER_H
#define CYCLONITE_VK_PIPELINE_MANAGER_H

#include "core/resourceSharedRef.h"
#include "core/hashTable.h"
#include "gfx/binding.h"
#include <chrono>
#include <map>
#include <span>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class Device;

class PipelineManager
{
public:
    explicit PipelineManager(Device* device);

    auto getOrCreatePipelineBindingSchema(std::span<Binding const> bindings,
                                          std::span<PushConstantRange const> pushConstantRanges)
      -> core::ResourceSharedRef;

private:
    auto getOrCreateDescriptorSetLayout(std::span<Binding const> bindings) -> core::ResourceSharedRef;

    void freeDescriptorSetLayouts(uint32_t count);

    void freePipelineLayouts(uint32_t count);

private:
    // TODO:: move to config
    static constexpr auto max_descriptor_set_layout_count_v = 128;
    static constexpr auto max_pipeline_set_layout_count_v = 1024;
    static constexpr auto primitive_rasterization_pipeline_cache_size_v = 2048;
    static constexpr auto primitive_rasterization_shader_set_cache_size_v = 1024;

    Device* device_;

    // TODO:: find better way to cache descriptor set layouts and pipeline layouts
    // vector contains descriptor set layout ids
    std::array<std::tuple<std::vector<uint64_t>,
                          std::vector<PushConstantRange>,
                          std::pair<core::ResourceSharedRef, std::chrono::high_resolution_clock::time_point>>,
               max_pipeline_set_layout_count_v>
      pipelineLayouts_;
    uint32_t pipelineLayoutCount_;

    std::array<std::tuple<std::vector<Binding>,
                          std::pair<core::ResourceSharedRef, std::chrono::high_resolution_clock::time_point>>,
               max_descriptor_set_layout_count_v>
      descriptorSetLayouts_;
    uint32_t descriptorSetLayoutCount_;

    // key is layout resource Id
    std::multimap<uint64_t, core::ResourceSharedRef> descriptorPools_;

    // TODO:: add primitive rasterization shader set as an internal resource
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_PIPELINE_MANAGER_H