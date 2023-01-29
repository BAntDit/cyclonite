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
    FRAGMENT_STAGE = 3,
    COMPUTE_STAGE = 4,
    MESHLETS_PIPELINE_TASK_STAGE = 5,
    MESHLETS_PIPELINE_MESH_STAGE = 6,
    RAY_TRACING_RAY_GENERATION_STAGE = 7,
    RAY_TRACING_INTERSECTION_STAGE = 8,
    RAY_TRACING_ANY_HIT_STAGE = 9,
    RAY_TRACING_CLOSEST_HIT_STAGE = 10,
    RAY_TRACING_MISS_STAGE = 11,
    MIN_VALUE = VERTEX_STAGE,
    MAX_VALUE = RAY_TRACING_MISS_STAGE,
    COUNT = MAX_VALUE + 1
};

class ShaderModule : public resources::Resource
{
public:
    ShaderModule(Device const& device, std::vector<uint32_t> const& spirVCode, std::string_view entryPointName);

    ShaderModule(Device const& device,
                 std::vector<uint32_t> const& spirVCode,
                 ShaderStage stage,
                 std::string_view entryPointName = "");

    /*ShaderModule(Device const& device,
                 std::vector<uint32_t> const& code,
                 VkShaderStageFlags stageFlags,
                 std::string const& entryPointName = "main",
                 uint32_t constantOffset = 0,
                 uint32_t constantSize = 0);

    [[nodiscard]] auto constantSize() const -> uint32_t { return constantSize_; }

    [[nodiscard]] auto constantOffset() const -> uint32_t { return constantOffset_; }

    [[nodiscard]] auto stageFlags() const -> VkShaderStageFlags { return stageFlags_; }

    [[nodiscard]] auto handle() const -> VkShaderModule { return static_cast<VkShaderModule>(vkShaderModule_); }

    [[nodiscard]] auto descriptorSetLayout() const -> VkDescriptorSetLayout
    {
        return static_cast<VkDescriptorSetLayout>(vkDescriptorSetLayout_);
    }

    [[nodiscard]] auto entryPointName() const -> std::string const& { return entryPointName_; }*/
private:
    void parseSpirV([[maybe_unused]] Device const& device,
                    [[maybe_unused]] std::vector<uint32_t> const& spirVCode,
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
}

#endif // CYCLONITE_SHADERMODULE_H
