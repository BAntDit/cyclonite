//
// Created by anton on 2/7/23.
//

#ifndef CYCLONITE_RENDER_TECHNIQUE_H
#define CYCLONITE_RENDER_TECHNIQUE_H

#include "resources/resource.h"
#include "typedefs.h"
#include "vulkan/config.h"
#include "vulkan/handle.h"
#include "vulkan/sharedHandle.h"
#include <array>
#include <bitset>

namespace cyclonite {
class BaseRenderTarget;
}

namespace cyclonite::compositor {
class BaseGraphicsNode;
}

namespace cyclonite::vulkan {
class Device;
}

namespace cyclonite::render {
class Material;

class Technique
{
    friend class Material;

public:
    using output_write_mask_t = std::bitset<channel_output_count_v>;

    enum class CullFace
    {
        NONE = 0,
        FRONT = 1,
        BACK = 2,
        BOTH = 3,
        MIN_VALUE = FRONT,
        MAX_VALUE = BACK,
        COUNT = MAX_VALUE + 1
    };

    enum class AlphaMode
    {
        OPAQUE = 0,
        MASK = 1,
        BLEND = 2,
        ADDITIVE_BLEND = 3,
        SUBTRACTIVE_BLEND = 4,
        MULTIPLY_BLEND = 5,
        CUSTOM_BLEND = 6,
        MIN_VALUE = OPAQUE,
        MAX_VALUE = CUSTOM_BLEND,
        COUNT = MAX_VALUE + 1
    };

    enum class BlendEquation
    {
        FUNC_ADD = 0,
        FUNC_SUBTRACT = 1,
        FUNC_REVERSE_SUBTRACT = 2,
        MIN_VALUE = FUNC_ADD,
        MAX_VALUE = FUNC_REVERSE_SUBTRACT,
        COUNT = MAX_VALUE + 1
    };

    enum class BlendFactor
    {
        ZERO = 0,
        ONE = 1,
        SRC_COLOR = 2,
        ONE_MINUS_SRC_COLOR = 3,
        DST_COLOR = 4,
        ONE_MINUS_DST_COLOR = 5,
        SRC_ALPHA = 6,
        ONE_MINUS_SRC_ALPHA = 7,
        DST_ALPHA = 8,
        ONE_MINUS_DST_ALPHA = 9,
        CONSTANT_COLOR = 10,
        ONE_MINUS_CONSTANT_COLOR = 11,
        CONSTANT_ALPHA = 12,
        ONE_MINUS_CONSTANT_ALPHA = 13,
        SRC_ALPHA_SATURATE = 14,
        SRC1_COLOR = 15,
        ONE_MINUS_SRC1_COLOR = 16,
        SRC1_ALPHA = 17,
        ONE_MINUS_SRC1_ALPHA = 18,
        MIN_VALUE = ZERO,
        MAX_VALUE = ONE_MINUS_SRC1_ALPHA,
        COUNT = MAX_VALUE + 1
    };

    enum class PolygonMode
    {
        FILL = 0,
        LINE = 1,
        POINT = 2,
        MIN_VALUE = FILL,
        MAX_VALUE = POINT,
        COUNT = MAX_VALUE + 1
    };

    enum class DepthFunc
    {
        NEVER = 0,
        LESS = 1,
        EQUAL = 2,
        LESS_OR_EQUAL = 3,
        GREATER = 4,
        NOT_EQUAL = 5,
        GREATER_OR_EQUAL = 6,
        ALWAYS = 7,
        MIN_VALUE = NEVER,
        MAX_VALUE = ALWAYS,
        COUNT = MAX_VALUE + 1
    };

    enum class Flags
    {
        DEPTH_WRITE = 0,
        DEPTH_TEST = 1,
        RASTERIZER_DISCARD_ENABLE = 2,
        DEPTH_CLAMP_ENABLE = 3,
        DEPTH_BIAS_ENABLE = 4,
        MIN_VALUE = DEPTH_WRITE,
        MAX_VALUE = DEPTH_BIAS_ENABLE,
        COUNT = 5
    };

    using flags_t = std::bitset<easy_mp::value_cast(Flags::COUNT)>;

public:
    Technique();

    Technique(Technique const&) = delete;

    Technique(Technique&&) = default;

    ~Technique() = default;

    auto operator=(Technique const&) -> Technique& = delete;

    auto operator=(Technique &&) -> Technique& = default;

    void update(vulkan::Device const& device,
                resources::ResourceManager const& resourceManager,
                compositor::BaseGraphicsNode const& gfxNode,
                size_t passIndex,
                bool forceUpdate = false);

    [[nodiscard]] auto stageMask() const -> std::bitset<rasterization_shader_stage_count_v> { return stageMask_; }

    [[nodiscard]] auto stageCount() const -> uint32_t { return stageMask_.count(); }

private:
    auto stageMask() -> std::bitset<rasterization_shader_stage_count_v>& { return stageMask_; }

private:
    struct shader_entry_point_t
    {
        std::string name;
        ShaderStage stage;
        size_t moduleIndex;
    };

    struct shader_buffer_resource_t
    {
        std::string name;
        uint32_t set;
        uint32_t binding;
        VkBuffer buffer;
        size_t offset;
        size_t size;
        void* ptr;
    };

    std::bitset<rasterization_shader_stage_count_v> stageMask_;

    uint32_t shaderModuleCount_;
    std::array<resources::Resource::Id, rasterization_shader_stage_count_v> shaderModules_;
    std::array<shader_entry_point_t, rasterization_shader_stage_count_v> entryPoints_;

    uint32_t colorOutputCount_;
    std::array<AlphaMode, render_target_output_semantic_count_v> alphaModePerOutput_;
    std::array<BlendEquation, render_target_output_semantic_count_v> colorBlendEquationPerOutput_;
    std::array<BlendEquation, render_target_output_semantic_count_v> alphaBlendEquationPerOutput_;
    std::array<BlendFactor, render_target_output_semantic_count_v> colorSrcBlendFactorPerOutput_;
    std::array<BlendFactor, render_target_output_semantic_count_v> colorDstBlendFactorPerOutput_;
    std::array<BlendFactor, render_target_output_semantic_count_v> alphaSrcBlendFactorPerOutput_;
    std::array<BlendFactor, render_target_output_semantic_count_v> alphaDstBlendFactorPerOutput_;
    std::array<output_write_mask_t, render_target_output_semantic_count_v> writeMaskPerOutput_;

    vec4 blendConstants_;
    real alphaCutoff_;

    CullFace cullFace_;
    PolygonMode polygonMode_;

    real lineWith_;
    real depthBiasClamp_;
    real depthBiasConstantFactor_;
    real depthBiasSlopeFactor_;

    flags_t flags_;

    DepthFunc depthFunc_;

    // TODO:: stencil test

    // TODO:: depth bounds

    uint32_t descriptorSetLayoutCount_;
    std::array<vulkan::Handle<VkDescriptorSetLayout>, vulkan::maxDescriptorSetsPerPipeline> descriptorSetLayouts_;
    vulkan::Handle<VkPipelineLayout> pipelineLayout_;
    vulkan::Handle<VkPipeline> pipeline_;

    std::unordered_map<std::string_view, shader_buffer_resource_t> buffers_;

    bool isExpired_;
};
}

#endif // CYCLONITE_RENDER_TECHNIQUE_H
