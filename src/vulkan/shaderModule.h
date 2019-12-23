//
// Created by bantdit on 11/25/19.
//

#ifndef CYCLONITE_SHADERMODULE_H
#define CYCLONITE_SHADERMODULE_H

#include "device.h"

namespace cyclonite::vulkan {
class ShaderModule
{
public:
    ShaderModule(Device const& device,
                 std::vector<uint32_t> const& code,
                 VkShaderStageFlags stageFlags,
                 std::string const& entryPointName = u8"main",
                 uint32_t constantOffset = 0,
                 uint32_t constantSize = 0);

    template<size_t bindingCount>
    ShaderModule(Device const& device,
                 std::vector<uint32_t> const& code,
                 VkShaderStageFlags stageFlags,
                 std::array<VkDescriptorType, bindingCount> const& bindings,
                 std::string const& entryPointName = u8"main",
                 uint32_t constantOffset = 0,
                 uint32_t constantSize = 0);

    ShaderModule(ShaderModule const&) = delete;

    ShaderModule(ShaderModule&&) = default;

    ~ShaderModule() = default;

    auto operator=(ShaderModule const&) -> ShaderModule& = delete;

    auto operator=(ShaderModule &&) -> ShaderModule& = default;

    [[nodiscard]] auto constantSize() const -> uint32_t { return constantSize_; }

    [[nodiscard]] auto constantOffset() const -> uint32_t { return constantOffset_; }

    [[nodiscard]] auto stageFlags() const -> VkShaderStageFlags { return stageFlags_; }

    [[nodiscard]] auto handle() const -> VkShaderModule { return static_cast<VkShaderModule>(vkShaderModule_); }

    [[nodiscard]] auto descriptorSetLayout() const -> VkDescriptorSetLayout
    {
        return static_cast<VkDescriptorSetLayout>(vkDescriptorSetLayout_);
    }

private:
    Handle<VkShaderModule> vkShaderModule_;
    VkShaderStageFlags stageFlags_;
    uint32_t constantSize_;
    uint32_t constantOffset_;
    Handle<VkDescriptorSetLayout> vkDescriptorSetLayout_;
};

template<size_t bindingCount>
ShaderModule::ShaderModule(Device const& device,
                           std::vector<uint32_t> const& code,
                           VkShaderStageFlags stageFlags,
                           std::array<VkDescriptorType, bindingCount> const& bindings,
                           std::string const& entryPointName,
                           uint32_t constantOffset,
                           uint32_t constantSize)
  : ShaderModule(device, code, stageFlags, entryPointName, constantOffset, constantSize)
{
    std::array<VkDescriptorSetLayoutBinding, bindingCount> descriptorSetLayoutBindings = {};
    std::fill(descriptorSetLayoutBindings.begin(), descriptorSetLayoutBindings.end(), VkDescriptorSetLayoutBinding{});

    for (size_t i = 0; i < bindingCount; i++) {
        descriptorSetLayoutBindings[i].binding = static_cast<uint32_t>(i);
        descriptorSetLayoutBindings[i].descriptorType = bindings[i];
        descriptorSetLayoutBindings[i].descriptorCount = 1;
        descriptorSetLayoutBindings[i].stageFlags = stageFlags_;
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};

    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
    descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

    if (auto result = vkCreateDescriptorSetLayout(
          device.handle(), &descriptorSetLayoutCreateInfo, nullptr, &vkDescriptorSetLayout_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("can not create descriptor set layout on device: " + device.name() +
                                     ", not enough memory on host.");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("can not create descriptor set layout on device: " + device.name() +
                                     ", not enough memory on device.");
        }

        assert(false);
    }
}

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

#endif // CYCLONITE_SHADERMODULE_H
