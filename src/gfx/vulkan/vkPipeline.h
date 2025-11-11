//
// Created by anton on 11/2/25.
//

#ifndef CYCLONITE_GFX_VK_PIPELINE_H
#define CYCLONITE_GFX_VK_PIPELINE_H

#include "core/hashTable.h"
#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "gfx/renderStates.h"
#include "handle.h"
#include <span>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class Pipeline : public core::ResourceBase
{
public:
    Pipeline(core::ResourceManagerBase* resourceManager,
             core::ResourceId resourceId,
             core::ResourceSharedRef deviceRef,
             PipelineType type,
             PipelineCreationFlagBits creationFlags,
             core::ResourceSharedRef bindingSchemaRef,
             PrimitiveTopology primitiveTopology,
             bool primitiveRestartEnable,
             core::ResourceSharedRef renderPassRef,
             RasterizationState const& rasterizationState,
             std::span<core::ResourceSharedRef const> shaders);

    [[nodiscard]] auto type() const -> PipelineType { return type_; }

private:
    void initPrimitiveRasterizationPipeline(
      core::ResourceSharedRef const& deviceRef,
      PipelineCreationFlagBits creationFlags,
      PrimitiveTopology primitiveTopology,
      bool primitiveRestartEnable,
      RasterizationState const& rasterizationState,
      std::span<core::ResourceSharedRef const> shaders);

private:
    core::StaticHashTable<core::ResourceSharedRef, metrix::value_cast(ShaderStageFlags::STAGE_COUNT), ShaderStageFlags>
      shaders_;
    core::ResourceSharedRef bindingSchemaRef_;
    core::ResourceSharedRef renderPassRef_;
    Handle<VkPipeline> vkPipeline_;
    PipelineType type_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_GFX_VK_PIPELINE_H