//
// Created by anton on 11/8/25.
//

#ifndef CYCLONITE_VK_BINDING_H
#define CYCLONITE_VK_BINDING_H

#include "gfx/common.h"

#if defined(GFX_DRIVER_VULKAN)
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class Binding
{
public:
    Binding(uint32_t set,
            uint32_t binding,
            DescriptorType descriptorType,
            ShaderStageFlagBits stageFlags,
            DescriptorSetFlagBits setFlags,
            BindingFlagBits bindingFlags)
      : set_{ set }
      , binding_{ binding }
      , descriptorType_{ descriptorType }
      , stageFlags_{ stageFlags }
      , setFlags_{ setFlags }
      , bindingFlags_{ bindingFlags }
    {
    }

    [[nodiscard]] auto set() const -> uint32_t { return set_; }

    [[nodiscard]] auto binding() const -> uint32_t { return binding_; }

    [[nodiscard]] auto descriptorType() const -> DescriptorType { return descriptorType_; }

    [[nodiscard]] auto stageFlags() const -> ShaderStageFlagBits { return stageFlags_; }

    [[nodiscard]] auto descriptorSetFlags() const -> DescriptorSetFlagBits { return setFlags_; }

    [[nodiscard]] auto bindingFlags() const -> BindingFlagBits { return bindingFlags_; }

private:
    uint32_t set_;
    uint32_t binding_;
    DescriptorType descriptorType_;
    ShaderStageFlagBits stageFlags_;
    DescriptorSetFlagBits setFlags_;
    BindingFlagBits bindingFlags_;
    // VkSampler* sampler_; TODO::
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_BINDING_H
