//
// Created by bantdit on 11/25/19.
//

#ifndef CYCLONITE_SHADERMODULE_H
#define CYCLONITE_SHADERMODULE_H

#include "concepts.h"
#include "config.h"
#include "device.h"
#include "pipelineDescriptorSets.h"
#include "resources/resource.h"

namespace cyclonite::vulkan {
enum class ShaderStage
{
    VERTEX_STAGE = 0,
    TESSELATION_CONTROL_STAGE = 1,
    TESSELATION_EVALUATION_STAGE = 2,
    GEOMETRY_STAGE = 3,
    FRAGMENT_STAGE = 4,
    COMPUTE_STAGE = 5,
    MESHLETS_PIPELINE_TASK_STAGE = 6,
    MESHLETS_PIPELINE_MESH_STAGE = 7,
    RAY_TRACING_RAY_GENERATION_STAGE = 8,
    RAY_TRACING_INTERSECTION_STAGE = 9,
    RAY_TRACING_ANY_HIT_STAGE = 10,
    RAY_TRACING_CLOSEST_HIT_STAGE = 11,
    RAY_TRACING_MISS_STAGE = 12,
    MIN_VALUE = VERTEX_STAGE,
    MAX_VALUE = RAY_TRACING_MISS_STAGE,
    COUNT = MAX_VALUE + 1,
    UNDEFINED = COUNT
};

class ShaderModule : public resources::Resource
{
public:
    ShaderModule(Device const& device, std::vector<uint32_t> const& spirVCode, std::string_view entryPointName);

    ShaderModule(Device const& device,
                 std::vector<uint32_t> const& spirVCode,
                 ShaderStage stage,
                 std::string_view entryPointName = "");

    [[nodiscard]] auto handle() const -> VkShaderModule { return static_cast<VkShaderModule>(vkShaderModule_); }

    [[nodiscard]] auto entryPointName() const -> std::string_view { return entryPointName_; }

    [[nodiscard]] auto stageFlags() const -> VkShaderStageFlags { return stageFlags_; }

    [[nodiscard]] auto getDescriptorSetLayoutCount() const -> uint32_t { return descriptorSetLayoutCount_; }

    template<VkDescriptorSetLayoutContainer Container>
    [[nodiscard]] auto getDescriptorSetLayouts(Container&& container) const -> Container;

private:
    void parseSpirV([[maybe_unused]] Device const& device,
                    [[maybe_unused]] std::vector<uint32_t> const& spirVCode,
                    [[maybe_unused]] ShaderStage stage,
                    [[maybe_unused]] std::string_view entryPointName);

private:
    std::string entryPointName_;
    VkShaderStageFlags stageFlags_;
    Handle<VkShaderModule> vkShaderModule_;
    std::array<Handle<VkDescriptorSetLayout>, maxDescriptorSetsPerPipeline> descriptorSetLayouts_;
    uint32_t descriptorSetLayoutCount_;

private:
    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return ShaderModule::tag; }
    static auto type_tag() -> ResourceTag& { return ShaderModule::tag; }
};

template<VkDescriptorSetLayoutContainer Container>
auto ShaderModule::getDescriptorSetLayouts(Container&& container) const -> Container
{
    assert(container.size() >= descriptorSetLayoutCount_);

    for (auto i = uint32_t{ 0 }, count = descriptorSetLayoutCount_; i < count; i++) {
        container[i] = static_cast<VkDescriptorSetLayout>(descriptorSetLayouts_[i]);
    }

    return container;
}
}

#endif // CYCLONITE_SHADERMODULE_H
