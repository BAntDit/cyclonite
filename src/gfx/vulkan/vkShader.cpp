//
// Created by anton on 11/2/25.
//

#include "vkShader.h"
#include "gfx/common.h"
#include "gfx/device.h"
#include "vkException.h"
#include <atomic>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
Shader::Shader(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               core::ResourceSharedRef deviceRef,
               size_t codeWordCount,
               uint32_t const* codeWords,
               ShaderStageCreationFlagBits creationFlags,
               ShaderStageFlags stage,
               std::string_view entryPointName)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , vkShaderModule_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(),
                     vkDestroyShaderModule }
  , entryPointName_{ entryPointName }
  , creationFlags_{ creationFlags }
  , stage_{ stage }
  , specializationInfo_{}
{
    auto shaderModuleCreateInfo = VkShaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = codeWordCount * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = codeWords;

    if (auto vkResult =
          vkCreateShaderModule(deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(),
                               &shaderModuleCreateInfo,
                               nullptr,
                               &vkShaderModule_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateShaderModule" };
    }
}

auto Shader::vulkanStage() const -> VkShaderStageFlagBits
{
    auto vkStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    switch (stage_) {
        case ShaderStageFlags::VERTEX:
            vkStage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case ShaderStageFlags::TESSELLATION_EVALUATION:
            vkStage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            break;
        case ShaderStageFlags::TESSELLATION_CONTROL:
            vkStage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            break;
        case ShaderStageFlags::GEOMETRY:
            vkStage = VK_SHADER_STAGE_GEOMETRY_BIT;
            break;
        case ShaderStageFlags::FRAGMENT:
            vkStage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
        case ShaderStageFlags::COMPUTE:
            vkStage = VK_SHADER_STAGE_COMPUTE_BIT;
            break;
        default:
            vkStage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }
    assert(VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM);

    return vkStage;
}
}
#endif
