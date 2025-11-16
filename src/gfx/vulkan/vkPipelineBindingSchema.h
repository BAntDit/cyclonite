//
// Created by anton on 11/8/25.
//

#ifndef CYCLONITE_VK_PIPELINE_BINDING_SCHEMA_H
#define CYCLONITE_VK_PIPELINE_BINDING_SCHEMA_H

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"
#include <span>
#include <vector>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class PipelineBindingSchema : public core::ResourceBase
{
public:
    PipelineBindingSchema(core::ResourceManagerBase* resourceManager,
                          core::ResourceId resourceId,
                          core::ResourceSharedRef deviceRef,
                          std::span<core::ResourceSharedRef const> descriptorSetLayouts,
                          std::span<PushConstantRange const> pushConstantRanges);

    PipelineBindingSchema(core::ResourceManagerBase* resourceManager,
                          core::ResourceId resourceId,
                          core::ResourceSharedRef deviceRef,
                          std::span<core::ResourceSharedRef const> descriptorSetLayouts);

    [[nodiscard]] auto descriptorSetLayouts() const -> std::vector<core::ResourceSharedRef> const&
    {
        return descriptorSetLayouts_;
    }

    [[nodiscard]] auto handle() const -> VkPipelineLayout { return static_cast<VkPipelineLayout>(pipelineLayout_); }

private:
    std::vector<core::ResourceSharedRef> descriptorSetLayouts_;
    std::vector<PushConstantRange> constantRanges_;
    Handle<VkPipelineLayout> pipelineLayout_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_PIPELINE_BINDING_SCHEMA_H
