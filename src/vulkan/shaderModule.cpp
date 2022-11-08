//
// Created by bantdit on 11/8/22.
//

#include "shaderModule.h"

namespace cyclonite::vulkan
{
ShaderModule::ShaderModule(Device const& device,
                           std::vector<uint32_t> const& code,
                           VkShaderStageFlags stageFlags,
                           std::string const& entryPointName,
                           uint32_t constantOffset,
                           uint32_t constantSize)
  : vkShaderModule_{ device.handle(), vkDestroyShaderModule }
  , stageFlags_{ stageFlags }
  , constantSize_{ constantSize }
  , constantOffset_{ constantOffset }
  , vkDescriptorSetLayout_{ device.handle(), vkDestroyDescriptorSetLayout }
  , entryPointName_{ entryPointName }
{
    constexpr uint32_t offsetAlign = 4;

    assert(0 == (constantOffset % offsetAlign));

    (void)offsetAlign;

    if (constantOffset + constantSize > device.capabilities().maxPushConstantsSize) {
        throw std::runtime_error("can not create GPU program module. The module requires " +
                                 std::to_string(constantOffset + constantSize) + " bytes for constants, device " +
                                 device.name() + ", can provide only " +
                                 std::to_string(device.capabilities().maxPushConstantsSize) + " bytes");
    }

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};

    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size() * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = code.data();

    if (auto result = vkCreateShaderModule(device.handle(), &shaderModuleCreateInfo, nullptr, &vkShaderModule_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("can not create GPU program module on device: " + device.name() +
                                     ", not enough memory on host.");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("can not create GPU program module on device: " + device.name() +
                                     ", not enough memory on device.");
        }

        assert(false);
    }
}
}
