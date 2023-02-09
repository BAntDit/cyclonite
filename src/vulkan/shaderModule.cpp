//
// Created by bantdit on 11/8/22.
//

#include "shaderModule.h"
#ifdef ENABLE_SHADER_MODULE_FROM_SPIR_V
#include <spirv_cross/spirv_cross.hpp>
#endif

namespace cyclonite::vulkan {
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
resources::Resource::ResourceTag ShaderModule::tag{};

ShaderModule::ShaderModule(Device const& device,
                           std::vector<uint32_t> const& spirVCode,
                           ShaderStage stage,
                           std::string_view entryPointName /* = ""*/)
  : resources::Resource{}
  , entryPointName_{}
  , stageFlags_{ VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM }
  , vkShaderModule_{ device.handle(), vkDestroyShaderModule }
  , descriptorSetLayouts_{}
  , descriptorSetLayoutCount_{ 0 }
{
    parseSpirV(device, spirVCode, stage, entryPointName);

    auto shaderModuleCreateInfo = VkShaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = spirVCode.size() * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = spirVCode.data();

    if (auto result = vkCreateShaderModule(device.handle(), &shaderModuleCreateInfo, nullptr, &vkShaderModule_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create shader module");
    }
}

ShaderModule::ShaderModule(Device const& device,
                           std::vector<uint32_t> const& spirVCode,
                           std::string_view entryPointName)
  : ShaderModule(device, spirVCode, ShaderStage::UNDEFINED, entryPointName)
{}

void ShaderModule::parseSpirV([[maybe_unused]] Device const& device,
                              [[maybe_unused]] std::vector<uint32_t> const& spirVCode,
                              [[maybe_unused]] ShaderStage stage,
                              [[maybe_unused]] std::string_view entryPointName)
{
#ifdef ENABLE_SHADER_MODULE_FROM_SPIR_V
    auto compiler = spirv_cross::Compiler{ std::vector(spirVCode) };

    auto&& entries = compiler.get_entry_points_and_stages();

    {
        auto executionModel = spv::ExecutionModelMax;
        auto epName = std::string_view{};

        if (stage != ShaderStage::UNDEFINED) {
            executionModel = getSpirVShaderExecutionModel(stage);

            for (auto&& ep : entries) {
                if (ep.execution_model == executionModel && (entryPointName.empty() || ep.name == entryPointName)) {
                    epName = ep.name;
                    break;
                }
            }
        } else if (!entryPointName.empty()) {
            epName = entryPointName;

            for (auto&& ep : entries) {
                if (ep.name == entryPointName) {
                    executionModel = ep.execution_model;
                    break;
                }
            }
        }

        assert(epName.empty());
        assert(executionModel != spv::ExecutionModelMax);

        entryPointName_ = epName;
        stageFlags_ = getVulkanShaderStage(executionModel);
        compiler.set_entry_point(entryPointName_, executionModel);
    }

    assert(stageFlags_ != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM);

    auto&& active = compiler.get_active_interface_variables();
    auto&& resources = compiler.get_shader_resources(active);

    auto sets = std::unordered_map<uint32_t, size_t>{};
    auto bindingSets = std::vector<std::vector<VkDescriptorSetLayoutBinding>>{};
    auto descriptorSetLayoutCreateInfo = std::array<VkDescriptorSetLayoutCreateInfo, maxDescriptorSetsPerPipeline>{};

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

    // collect sets and bindings
    for (auto descriptorType : descriptorTypes) {
        auto const& resourcesOfType = getResourcesByDescriptorType(resources, descriptorType);

        for (auto& r : resourcesOfType) {
            auto set = compiler.get_decoration(r.id, spv::DecorationDescriptorSet);

            if (!sets.contains(set)) {
                auto const initialBindingCount = size_t{ 16 };
                bindingSets.emplace_back(std::vector<VkDescriptorSetLayoutBinding>{}).reserve(initialBindingCount);
                sets.emplace(set, descriptorSetLayoutCount_++);
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
                for (auto i = size_t{ 0 }, count = type.array.size(); i < count; i++) {
                    if (i == 0)
                        descriptorCount += type.array[0];
                    else
                        descriptorCount *= type.array[i];
                }
            }

            auto const& name = compiler.get_name(r.id);

            binding.binding = compiler.get_decoration(r.id, spv::DecorationBinding);
            binding.descriptorType = getVulkanDescriptorType(name, type, descriptorType);
            binding.descriptorCount = descriptorCount;
            binding.stageFlags = stageFlags_;
        }
    }

    // fill set layouts
    assert(descriptorSetLayoutCount_ <= maxDescriptorSetsPerPipeline);
    for (auto&& [set, idx] : sets) {
        assert(set < descriptorSetLayoutCreateInfo.size());

        auto& bindings = bindingSets[idx];

        descriptorSetLayoutCreateInfo[set].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo[set].bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo[set].pBindings = bindings.data();

        descriptorSetLayouts_[set] = Handle<VkDescriptorSetLayout>{ device.handle(), vkDestroyDescriptorSetLayout };

        if (auto result = vkCreateDescriptorSetLayout(
              device.handle(), &descriptorSetLayoutCreateInfo[set], nullptr, &descriptorSetLayouts_[set]);
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create descriptor set layouts");
        }
    }
#else
    throw std::runtime_error("SPIR-V reflection must be enabled");
#endif
}
}
