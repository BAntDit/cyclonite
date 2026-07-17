//
// Created by anton on 11/23/25.
//

#ifndef CYCLONITE_VK_SHADER_RESOURCE_VIEW_H
#define CYCLONITE_VK_SHADER_RESOURCE_VIEW_H

#include "core/resourceBase.h"
#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include "handle.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class ShaderResourceView : public core::ResourceBase
{
public:
    ShaderResourceView(core::ResourceManagerBase* resourceManager,
                       core::ResourceId resourceId,
                       core::ResourceWeakRef weakRef,
                       uint32_t maxMipLevel,
                       uint32_t maxArrayLayer);

    [[nodiscard]] auto texture() const -> core::ResourceWeakRef { return textureRef_; }

    [[nodiscard]] auto handle() const -> VkImageView { return static_cast<VkImageView>(vkImageView_); }

    [[nodiscard]] auto maxMipLevel() const -> uint32_t { return maxMipLevel_; }

    [[nodiscard]] auto maxArrayLayer() const -> uint32_t { return maxArrayLayer_; }

private:
    core::ResourceWeakRef textureRef_;
    Handle<VkImageView> vkImageView_;
    uint32_t maxMipLevel_;
    uint32_t maxArrayLayer_;
};
}
#endif
#endif // CYCLONITE_VK_SHADER_RESOURCE_VIEW_H