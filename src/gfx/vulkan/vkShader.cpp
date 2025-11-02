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
namespace {
std::atomic<uint32_t> lastShaderId_ = 1;
}

Shader::Shader(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               core::ResourceSharedRef deviceRef,
               size_t codeWordCount,
               uint32_t const* codeWords)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , id_{ lastShaderId_.fetch_add(1, std::memory_order_acq_rel) }
  , vkShaderModule_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(),
                     vkDestroyShaderModule }
{
    auto shaderModuleCreateInfo = VkShaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = codeWordCount;
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
}
#endif
