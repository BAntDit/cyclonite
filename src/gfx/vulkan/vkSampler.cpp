//
// Created by anton on 11/22/25.
//

#include "vkSampler.h"
#include "gfx/device.h"
#include "internal/utils.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
Sampler::Sampler(core::ResourceManagerBase* resourceManager,
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
                 bool unnormalizedCoords)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , magFilter_{ magFilter }
  , minFilter_{ minFilter }
  , mipFilter_{ mipFilter }
  , addressModeU_{ addressModeU }
  , addressModeV_{ addressModeV }
  , addressModeW_{ addressModeW }
  , mipLodBias_{ mipLodBias }
  , maxLod_{ maxLod }
  , minLod_{ minLod }
  , maxAnisatropy_{ maxAnisotropy }
  , vkSampler_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroySampler }
{
    assert(deviceRef.valid());
    auto& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();

    auto createInfo = VkSamplerCreateInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = magFilter == TextureFilter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    createInfo.minFilter = minFilter == TextureFilter::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
    createInfo.mipmapMode =
      mipFilter == TextureFilter::LINEAR ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
    createInfo.addressModeU = internal::getAddressMode(addressModeU);
    createInfo.addressModeV = internal::getAddressMode(addressModeV);
    createInfo.addressModeW = internal::getAddressMode(addressModeW);
    createInfo.mipLodBias = mipLodBias;
    createInfo.anisotropyEnable = (maxAnisotropy > 1.f);
    createInfo.maxAnisotropy = maxAnisotropy;
    createInfo.compareEnable = compareOp != CompareOp::ALWAYS;
    createInfo.compareOp = internal::getCompareOp(compareOp);
    createInfo.minLod = minLod;
    createInfo.maxLod = maxLod;
    createInfo.borderColor = internal::getBorderColor(borderColor);
    createInfo.unnormalizedCoordinates = unnormalizedCoords;

    if (auto vkResult = vkCreateSampler(device.handle(), &createInfo, nullptr, &vkSampler_); vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateSampler" };
    }
}
}
#endif
