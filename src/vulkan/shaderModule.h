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
    template<size_t bindingCount, size_t codeSize>
    ShaderModule(Device const& device,
                 std::array<VkDescriptorType, bindingCount> const& bindings,
                 std::array<uint32_t, codeSize> const& code,
                 std::string const& entryPointName = u8"main",
                 uint32_t constantOffset = 0,
                 uint32_t constantSize = 0);

    ShaderModule(ShaderModule const&) = delete;

    ShaderModule(ShaderModule&&) = default;

    ~ShaderModule() = default;

    auto operator=(ShaderModule const&) -> ShaderModule& = delete;

    auto operator=(ShaderModule &&) -> ShaderModule& = default;

private:
    Handle<VkShaderModule> vkShaderModule_;
};

template<size_t bindingCount, size_t codeSize>
ShaderModule::ShaderModule(cyclonite::vulkan::Device const& device,
                           std::array<VkDescriptorType, bindingCount> const& bindings,
                           std::array<uint32_t, codeSize> const& code,
                           std::string const& entryPointName,
                           uint32_t constantOffset,
                           uint32_t constantSize)
  : vkShaderModule_{ device.handle(), vkDestroyShaderModule }
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
}
}

#endif // CYCLONITE_SHADERMODULE_H
