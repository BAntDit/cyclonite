//
// Created by anton on 11/2/25.
//

#ifndef CYCLONITE_GFX_VK_PIPELINE_H
#define CYCLONITE_GFX_VK_PIPELINE_H

#include "core/hashTable.h"
#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/renderStates.h"
#include "gfx/common.h"
#include "handle.h"
#include <array>

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
             PrimitiveTopology primitiveTopology,
             bool primitiveRestartEnable, 
             core::ResourceSharedRef renderPassRef,
             RasterizationState const& rasterizationState,
             std::array<core::ResourceSharedRef, metrix::value_cast(ShaderStageFlags::STAGE_COUNT)> const& shaders);

private:
    void initPrimitiveRasterizationPipeline(
      PipelineCreationFlagBits creationFlags,
      PrimitiveTopology primitiveTopology,
      bool primitiveRestartEnable,
      RasterizationState const& rasterizationState,
      std::array<core::ResourceSharedRef, metrix::value_cast(ShaderStageFlags::STAGE_COUNT)> const& shaders);

private:
    core::StaticHashTable<core::ResourceSharedRef, metrix::value_cast(ShaderStageFlags::STAGE_COUNT), ShaderStageFlags>
      shaders_;
    core::ResourceSharedRef renderPassRef_;
    Handle<VkPipeline> vkPipeline_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_GFX_VK_PIPELINE_H