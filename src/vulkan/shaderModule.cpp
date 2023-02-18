//
// Created by bantdit on 11/8/22.
//

#include "shaderModule.h"

namespace cyclonite::vulkan {
resources::Resource::ResourceTag ShaderModule::tag{};

ShaderModule::ShaderModule(Device const& device, std::vector<uint32_t> const& code)
  : vkShaderModule_{ device.handle(), vkDestroyShaderModule }
{
    auto shaderModuleCreateInfo = VkShaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size() * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = code.data();

    if (auto result = vkCreateShaderModule(device.handle(), &shaderModuleCreateInfo, nullptr, &vkShaderModule_);
        result != VK_SUCCESS) {
        throw std::runtime_error("could not create shader module");
    }
}
}
