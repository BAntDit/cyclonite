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
                 std::string const& entryPointName = "main",
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

    [[nodiscard]] auto entryPointName() const -> std::string const& { return entryPointName_; }

private:
    Handle<VkShaderModule> vkShaderModule_;
    VkShaderStageFlags stageFlags_;
    uint32_t constantSize_;
    uint32_t constantOffset_;
    Handle<VkDescriptorSetLayout> vkDescriptorSetLayout_;
    std::string entryPointName_;
};
}

#endif // CYCLONITE_SHADERMODULE_H
