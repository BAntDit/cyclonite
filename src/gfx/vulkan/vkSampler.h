
#ifndef CYCLONITE_VK_SAMPLER
#define CYCLONITE_VK_SAMPLER

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"
#include <tuple>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class Sampler : public core::ResourceBase
{
public:
    using texture_address_mode_uvw_t = std::tuple<TextureAddressMode, TextureAddressMode, TextureAddressMode>;

    Sampler(core::ResourceManagerBase* resourceManager,
            core::ResourceId resourceId,
            core::ResourceSharedRef deviceRef,
            TextureFilter magFilter,
            TextureFilter minFilter,
            TextureFilter mipFilter,
            TextureAddressMode addressModeU,
            TextureAddressMode addressModeV,
            TextureAddressMode addressModeW,
            real mipLodBias,
            real maxAnisotropy,
            CompareOp compareOp,
            real minLod,
            real maxLod,
            BorderColor borderColor,
            bool unnormalizedCoords);

    [[nodiscard]] auto handle() const -> VkSampler { return static_cast<VkSampler>(vkSampler_); }

    [[nodiscard]] auto magFilter() const -> TextureFilter { return magFilter_; }

    [[nodiscard]] auto minFilter() const -> TextureFilter { return minFilter_; }

    [[nodiscard]] auto mipFilter() const -> TextureFilter { return mipFilter_; }

    [[nodiscard]] auto addressModeUVW() const -> texture_address_mode_uvw_t
    {
        return std::make_tuple(addressModeU_, addressModeV_, addressModeW_);
    }

    [[nodiscard]] auto maxAnisatropy() const -> real { return maxAnisatropy_; }

    [[nodiscard]] auto mipLodBias() const -> real { return mipLodBias_; }
    [[nodiscard]] auto maxLod() const -> real { return maxLod_; }
    [[nodiscard]] auto minLod() const -> real { return minLod_; }

    using core::ResourceBase::resourceBase;

private:
    TextureFilter magFilter_;
    TextureFilter minFilter_;
    TextureFilter mipFilter_;

    TextureAddressMode addressModeU_;
    TextureAddressMode addressModeV_;
    TextureAddressMode addressModeW_;

    real mipLodBias_;
    real maxLod_;
    real minLod_;

    real maxAnisatropy_;

    Handle<VkSampler> vkSampler_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_SAMPLER
