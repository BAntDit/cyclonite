//
// Created by anton on 11/8/25.
//

#ifndef CYCLONITE_VK_PIPELINE_MANAGER_H
#define CYCLONITE_VK_PIPELINE_MANAGER_H

#include "core/hashTable.h"
#include "core/resourceSharedRef.h"
#include "gfx/binding.h"
#include "gfx/renderStates.h"
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

    [[nodiscard]] auto getOrCreatePipelineBindingSchema(std::span<Binding const> bindings,
                                                        std::span<PushConstantRange const> pushConstantRanges)
      -> core::ResourceSharedRef;

    [[nodiscard]] auto getOrCreatePrimitiveRasterizationPipeline(PipelineCreationFlagBits creationFlags,
                                                                 core::ResourceSharedRef const& bindingSchemaRef,
                                                                 PrimitiveTopology primitiveTopology,
                                                                 bool primitiveRestartEnable,
                                                                 core::ResourceSharedRef const& renderPassRef,
                                                                 RasterizationState const& rasterizationState,
                                                                 std::span<core::ResourceSharedRef const> shaders)
      -> core::ResourceSharedRef;

    [[nodiscard]] auto getOrCreateComputePipeline(PipelineCreationFlagBits creationFlags,
                                                  core::ResourceSharedRef const& bindingSchemaRef,
                                                  core::ResourceSharedRef const& shaderRef) -> core::ResourceSharedRef;

private:
    [[nodiscard]] auto getOrCreateDescriptorSetLayout(std::span<Binding const> bindings) -> core::ResourceSharedRef;

    void freeDescriptorSetLayouts(uint32_t count);

    void freePipelineLayouts(uint32_t count);

private:
    // TODO:: move to config
    static constexpr auto max_descriptor_set_layout_count_v = size_t{ 128 };
    static constexpr auto max_pipeline_set_layout_count_v = size_t{ 1024 };
    static constexpr auto primitive_rasterization_pipeline_cache_size_v = size_t{ 2048 };
    static constexpr auto compute_pipeline_cache_size_v = size_t{ 512 };
    static constexpr auto primitive_rasterization_shader_set_cache_size_v = size_t{ 1024 };

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

    // key:
    // 1. creation flags
    // 2. binding schema id
    // 3. PrimitiveTopology
    // 4. primitiveRestartEnable
    // 5. render pass id
    // 6. RasterizationStateFlagBits
    // 7. packed to uint32_t CompareOp + polygonMode + cullMode + frontFace;
    // 8. packed to uint64_t front face stencil state
    // 9. packed to uint64_t back face stencil state
    // 10-14. shader set
    // TODO:: move them to dynamic state: 14 - 17. depth parameters
    core::StaticHashTable<std::pair<core::ResourceSharedRef, std::chrono::high_resolution_clock::time_point>,
                          primitive_rasterization_pipeline_cache_size_v,
                          typename PipelineCreationFlagBits::type_t,   // 1. creation flags
                          uint64_t,                                    // 2. binding schema id
                          std::underlying_type_t<PrimitiveTopology>,   // 3. PrimitiveTopology
                          bool,                                        // 4. primitiveRestartEnable
                          uint64_t,                                    // 5. render pass id
                          typename RasterizationStateFlagBits::type_t, // 6. RasterizationStateFlagBits
                          uint32_t, // 7. packed to uint32_t CompareOp + polygonMode + cullMode + frontFace;
                          uint64_t, // 8. packed to uint64_t front face stencil state
                          uint64_t, // 9. packed to uint64_t back face stencil state
                          uint64_t, // 10. vertex shader
                          uint64_t, // 11. tess control shader
                          uint64_t, // 12. tess eval shader
                          uint64_t, // 13. geometry shader
                          uint64_t> // 14. fragment shader
      primitiveRasterizationPipelineCache_;

    core::StaticHashTable<std::pair<core::ResourceSharedRef, std::chrono::high_resolution_clock::time_point>,
                          compute_pipeline_cache_size_v,
                          typename PipelineCreationFlagBits::type_t, // 1. creation flags
                          uint64_t,                                  // 2. binding schema
                          uint64_t>                                  // 3. compute shader
      computePipelineCache_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_PIPELINE_MANAGER_H