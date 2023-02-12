//
// Created by anton on 2/5/23.
//

#include "material.h"
#include "technique.h"
#include "descriptorType.h"
#include "vulkan/device.h"
#ifdef ENABLE_SHADER_MODULE_FROM_SPIR_V
#include <spirv_cross/spirv_cross.hpp>
#endif
#include <unordered_set>

namespace cyclonite::render {
#ifdef ENABLE_SHADER_MODULE_FROM_SPIR_V
namespace {
auto getVulkanShaderStage(spv::ExecutionModel em) -> VkShaderStageFlags
{
    auto flags = VkShaderStageFlags{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM };

    switch (em) {
        case spv::ExecutionModelVertex:
            flags = VK_SHADER_STAGE_VERTEX_BIT;
        case spv::ExecutionModelFragment:
            flags = VK_SHADER_STAGE_FRAGMENT_BIT;
        case spv::ExecutionModelTessellationControl:
            flags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case spv::ExecutionModelTessellationEvaluation:
            flags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case spv::ExecutionModelGLCompute:
            flags = VK_SHADER_STAGE_COMPUTE_BIT;
        case spv::ExecutionModelMeshEXT:
        case spv::ExecutionModelMeshNV:
            flags = VK_SHADER_STAGE_MESH_BIT_NV;
        case spv::ExecutionModelTaskNV:
        case spv::ExecutionModelTaskEXT:
            flags = VK_SHADER_STAGE_TASK_BIT_NV;
        case spv::ExecutionModelGeometry:
            flags = VK_SHADER_STAGE_GEOMETRY_BIT;
        case spv::ExecutionModelRayGenerationKHR:
            flags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        case spv::ExecutionModelIntersectionKHR:
            flags = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        case spv::ExecutionModelAnyHitKHR:
            flags = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        case spv::ExecutionModelClosestHitKHR:
            flags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        case spv::ExecutionModelMissKHR:
            flags = VK_SHADER_STAGE_MISS_BIT_KHR;
        default:
            throw std::runtime_error("unexpected shader execution mode");
    }
    assert(flags != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM);

    return flags;
}

auto getSpirVShaderExecutionModel(ShaderStage stage) -> spv::ExecutionModel
{
    spv::ExecutionModel result = spv::ExecutionModelMax;

    switch (stage) {
        case ShaderStage::VERTEX_STAGE:
            result = spv::ExecutionModelVertex;
            break;
        case ShaderStage::FRAGMENT_STAGE:
            result = spv::ExecutionModelFragment;
            break;
        case ShaderStage::TESSELATION_CONTROL_STAGE:
            result = spv::ExecutionModelTessellationControl;
            break;
        case ShaderStage::TESSELATION_EVALUATION_STAGE:
            result = spv::ExecutionModelTessellationEvaluation;
            break;
        case ShaderStage::GEOMETRY_STAGE:
            result = spv::ExecutionModelGeometry;
            break;
        case ShaderStage::COMPUTE_STAGE:
            result = spv::ExecutionModelGLCompute;
            break;
        case ShaderStage::MESHLETS_PIPELINE_TASK_STAGE:
            result = spv::ExecutionModelTaskEXT;
            break;
        case ShaderStage::MESHLETS_PIPELINE_MESH_STAGE:
            result = spv::ExecutionModelMeshEXT;
            break;
        case ShaderStage::RAY_TRACING_RAY_GENERATION_STAGE:
            result = spv::ExecutionModelRayGenerationKHR;
            break;
        case ShaderStage::RAY_TRACING_ANY_HIT_STAGE:
            result = spv::ExecutionModelAnyHitKHR;
            break;
        case ShaderStage::RAY_TRACING_CLOSEST_HIT_STAGE:
            result = spv::ExecutionModelClosestHitKHR;
            break;
        case ShaderStage::RAY_TRACING_INTERSECTION_STAGE:
            result = spv::ExecutionModelIntersectionKHR;
            break;
        case ShaderStage::RAY_TRACING_MISS_STAGE:
            result = spv::ExecutionModelMissKHR;
            break;
        default:
            assert(false);
    }

    assert(result != spv::ExecutionModelMax);
    return result;
}

auto getResourcesByDescriptorType(spirv_cross::ShaderResources const& resources, DescriptorType descriptorType)
  -> spirv_cross::SmallVector<spirv_cross::Resource> const&
{
    auto const* result = std::add_pointer_t<spirv_cross::SmallVector<spirv_cross::Resource>>{ nullptr };

    // https://github.com/KhronosGroup/SPIRV-Cross/wiki/Reflection-API-user-guide
    // Mapping reflection output to Vulkan cheat sheet
    switch (descriptorType) {
        case DescriptorType::SAMPLER:
            result = &resources.separate_samplers;
            break;
        case DescriptorType::COMBINED_IMAGE_SAMPLER:
            result = &resources.sampled_images;
            break;
        case DescriptorType::UNIFORM_TEXEL_BUFFER:
        case DescriptorType::SAMPLED_IMAGE:
            result = &resources.separate_images;
            break;
        case DescriptorType::STORAGE_TEXEL_BUFFER:
        case DescriptorType::STORAGE_IMAGE:
            result = &resources.storage_images;
            break;
        case DescriptorType::UNIFORM_BUFFER:
        case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
            result = &resources.uniform_buffers;
            break;
        case DescriptorType::STORAGE_BUFFER:
        case DescriptorType::STORAGE_BUFFER_DYNAMIC:
            result = &resources.storage_buffers;
            break;
        case DescriptorType::INPUT_ATTACHMENT:
            result = &resources.subpass_inputs;
            break;
        case DescriptorType::ACCELERATION_STRUCTURE:
            result = &resources.acceleration_structures;
            break;
        default:
            assert(false);
    }

    assert(result != nullptr);
    return *result;
}

auto getVulkanDescriptorType(std::string const& name, spirv_cross::SPIRType const& type, DescriptorType descriptorType)
  -> VkDescriptorType
{
    auto result = VkDescriptorType{ VK_DESCRIPTOR_TYPE_MAX_ENUM };

    switch (descriptorType) {
        case DescriptorType::SAMPLER:
            result = VK_DESCRIPTOR_TYPE_SAMPLER;
            break;
        case DescriptorType::COMBINED_IMAGE_SAMPLER:
            result = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        case DescriptorType::UNIFORM_TEXEL_BUFFER:
        case DescriptorType::SAMPLED_IMAGE:
            result = (type.image.dim == spv::DimBuffer) ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
                                                        : VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::STORAGE_TEXEL_BUFFER:
        case DescriptorType::STORAGE_IMAGE:
            result = (type.image.dim == spv::DimBuffer) ? VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
                                                        : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case DescriptorType::UNIFORM_BUFFER:
        case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
            result = name.ends_with("_dynamic") ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                                                : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::STORAGE_BUFFER:
        case DescriptorType::STORAGE_BUFFER_DYNAMIC:
            result = name.ends_with("_dynamic") ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
                                                : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::INPUT_ATTACHMENT:
            result = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        case DescriptorType::ACCELERATION_STRUCTURE:
            result = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
            break;
        default:
            result = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    assert(result != VK_DESCRIPTOR_TYPE_MAX_ENUM);

    return result;
}
}
#endif
resources::Resource::ResourceTag Material::tag{};

void Material::addTechnique(vulkan::Device& device,
                            std::string_view techniqueName,
                            std::string_view nodeName,
                            size_t passIndex,
                            std::array<spir_v_code_t const*, rasterization_shader_stage_count_v> spirVCode,
                            std::array<std::string_view, rasterization_shader_stage_count_v> entryPoints,
                            std::bitset<rasterization_shader_stage_count_v> stageMask)
{
#ifdef ENABLE_SHADER_MODULE_FROM_SPIR_V
    auto technique = Technique{};

    auto uniqueModules = std::unordered_set<spir_v_code_t const*>{};
    auto stageToModuleIdx = std::array<decltype(uniqueModules.cbegin()), rasterization_shader_stage_count_v>{};
    std::fill_n(stageToModuleIdx.begin(), rasterization_shader_stage_count_v, uniqueModules.cend());

    auto sets = std::unordered_map<uint32_t, size_t>{};
    auto bindingSets = std::vector<std::vector<VkDescriptorSetLayoutBinding>>{};
    auto descriptorSetLayoutCreateInfo =
      std::array<VkDescriptorSetLayoutCreateInfo, vulkan::maxDescriptorSetsPerPipeline>{};
    auto descriptorSetLayoutCount = uint32_t{ 0 };

    constexpr auto stages = std::array{ ShaderStage::VERTEX_STAGE,
                                        ShaderStage::TESSELATION_CONTROL_STAGE,
                                        ShaderStage::TESSELATION_EVALUATION_STAGE,
                                        ShaderStage::GEOMETRY_STAGE,
                                        ShaderStage::FRAGMENT_STAGE };
    static_assert(stages.size() == rasterization_shader_stage_count_v);

    for (auto i = size_t{ 0 }, count = std::size(stages); i < count; i++) {
        if (!stageMask.test(i))
            continue;

        auto stage = stages[i];
        auto executionModel = getSpirVShaderExecutionModel(stage);
        auto stageFlags = getVulkanShaderStage(executionModel);
        auto entryPointName = std::string{ entryPoints[i] };
        auto const* code = spirVCode[i];
        assert(code != nullptr);

        auto insertion = uniqueModules.insert(code);
        stageToModuleIdx[i] = std::get<0>(insertion);

        auto compiler = spirv_cross::Compiler{ std::vector(*code) };

        auto&& entries = compiler.get_entry_points_and_stages();

        auto isEntryPointValid = false;
        for (auto&& ep : entries) {
            if (!entryPointName.empty() && entryPointName == ep.name && executionModel == ep.execution_model) {
                isEntryPointValid = true;
                break;
            } else if (entryPointName.empty() && executionModel == ep.execution_model) {
                isEntryPointValid = true;
                entryPointName = ep.name;
                break;
            }
        }

        if (!isEntryPointValid) {
            throw std::runtime_error("entry point is not valid");
        }

        compiler.set_entry_point(entryPointName, executionModel);

        auto&& active = compiler.get_active_interface_variables();
        auto&& resources = compiler.get_shader_resources(active);

        auto descriptorTypes = { DescriptorType::SAMPLER,
                                 DescriptorType::COMBINED_IMAGE_SAMPLER,
                                 DescriptorType::SAMPLED_IMAGE,
                                 DescriptorType::STORAGE_IMAGE,
                                 DescriptorType::UNIFORM_TEXEL_BUFFER,
                                 DescriptorType::STORAGE_TEXEL_BUFFER,
                                 DescriptorType::UNIFORM_BUFFER,
                                 DescriptorType::STORAGE_BUFFER,
                                 DescriptorType::UNIFORM_BUFFER_DYNAMIC,
                                 DescriptorType::STORAGE_BUFFER_DYNAMIC,
                                 DescriptorType::INPUT_ATTACHMENT,
                                 DescriptorType::INLINE_UNIFORM_BLOCK,
                                 DescriptorType::ACCELERATION_STRUCTURE,
                                 DescriptorType::MUTABLE_VALUE };

        for (auto descriptorType : descriptorTypes) {
            auto const& resourcesOfType = getResourcesByDescriptorType(resources, descriptorType);

            for (auto& r : resourcesOfType) {
                auto set = compiler.get_decoration(r.id, spv::DecorationDescriptorSet);

                if (!sets.contains(set)) {
                    auto const initialBindingCount = size_t{ 16 };
                    bindingSets.emplace_back(std::vector<VkDescriptorSetLayoutBinding>{}).reserve(initialBindingCount);
                    sets.emplace(set, descriptorSetLayoutCount++);
                }

                assert(sets.contains(set));
                auto setIdx = sets.at(set);

                assert(setIdx < bindingSets.size());
                auto& bindings = bindingSets[setIdx];
                auto& binding = bindings.emplace_back();

                auto descriptorCount = uint32_t{ 0 }; // one binding can bind several descriptors as array
                auto const& type = compiler.get_type(r.type_id);

                if (type.array.empty()) { // not an array
                    descriptorCount = 1;
                } else { // array and maybe multi dimension
                    for (auto j = size_t{ 0 }, size = type.array.size(); j < size; j++) {
                        if (j == 0)
                            descriptorCount += type.array[0];
                        else
                            descriptorCount *= type.array[j];
                    }
                }

                auto const& name = compiler.get_name(r.id);
                binding.binding = compiler.get_decoration(r.id, spv::DecorationBinding);
                binding.descriptorType = getVulkanDescriptorType(name, type, descriptorType);
                binding.descriptorCount = descriptorCount;
                binding.stageFlags = stageFlags;
            }
        }
        // TODO::
    }

    // technique.shaderModuleCount_ =
#else
    throw std::runtime_error("SPIR-V reflection must be enabled");
#endif
}
}
