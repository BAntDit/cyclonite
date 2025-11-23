//
// Created by anton on 7/27/25.
//

#ifndef CYCLONITE_VKTEXTURE_H
#define CYCLONITE_VKTEXTURE_H

#include "core/refFromThisMixin.h"
#include "core/spinLock.h"
#include "gfx/common.h"
#include "handle.h"
#include "vmaUsage.h"
#include <unordered_map>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class Texture
  : public core::ResourceBase
  , public core::EnableRefFromThis
{
public:
    Texture(core::ResourceManagerBase* resourceManager,
            core::ResourceId resourceId,
            core::ResourceSharedRef deviceRef,
            core::ResourceSharedRef samplerRef,
            GpuMemoryAllocationFlagBits allocationFlags,
            TextureCreationFlagBits imageCreateFlags,
            TextureType textureType,
            Format format,
            uint32_t width,
            uint32_t height,
            uint32_t depth,
            uint32_t mipCount,
            uint32_t arrayLayerCount,
            TextureTiling tiling,
            TextureUsageFlagBits usageFlags);

    ~Texture();

    [[nodiscard]] auto currentState() const -> TextureState { return state_; }

    [[nodiscard]] auto format() const -> Format { return format_; }

    [[nodiscard]] auto type() const -> TextureType { return type_; }

    [[nodiscard]] auto width() const -> uint32_t { return width_; }

    [[nodiscard]] auto height() const -> uint32_t { return height_; }

    [[nodiscard]] auto depth() const -> uint32_t { return depth_; }

    [[nodiscard]] auto mipCount() const -> uint32_t { return mipCount_; }

    [[nodiscard]] auto device() const -> core::ResourceSharedRef { return deviceRef_; }

    [[nodiscard]] auto handle() const -> VkImage { return vkImage_; }

    [[nodiscard]] auto sampler() const -> core::ResourceSharedRef { return samplerRef_; }

    [[nodiscard]] auto getRTV(uint16_t mipLevel) -> core::ResourceWeakRef;

    // [[nodiscard]] auto getSRV(uint16_t mipLevel) -> core::ResourceWeakRef;

private:
    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef samplerRef_;
    VmaAllocation allocation_;
    VkImage vkImage_;
    TextureState state_;
    Format format_;
    uint32_t width_;
    uint32_t height_;
    uint32_t depth_;
    uint32_t mipCount_;
    TextureType type_;
    std::unordered_map<uint32_t, core::ResourceSharedRef> rtvs_;
    core::SpinLock rtvsGuard_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VKTEXTURE_H
