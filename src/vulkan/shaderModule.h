//
// Created by bantdit on 11/25/19.
//

#ifndef CYCLONITE_SHADERMODULE_H
#define CYCLONITE_SHADERMODULE_H

#include "concepts.h"
#include "config.h"
#include "device.h"
#include "pipelineDescriptorSets.h"
#include "resources/resource.h"
#include "typedefs.h"

namespace cyclonite::vulkan {
class ShaderModule : public resources::Resource
{
public:
    ShaderModule(Device const& device, std::vector<uint32_t> const& code);

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    [[nodiscard]] auto handle() const -> VkShaderModule { return static_cast<VkShaderModule>(vkShaderModule_); }

private:
    Handle<VkShaderModule> vkShaderModule_;

private:
    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return ShaderModule::tag; }
    static auto type_tag() -> ResourceTag& { return ShaderModule::tag; }
};
}

#endif // CYCLONITE_SHADERMODULE_H
