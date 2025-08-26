//
// Created by anton on 8/25/25.
//

#include "vkRenderTargetView.h"
#include "gfx/common.h"
#include "gfx/device.h"
#include "gfx/texture.h"
#include "internal/utils.h"
#include "vkException.h"
#include <stdexcept>

namespace cyclonite::gfx::vulkan {
RenderTargetView::RenderTargetView(core::ResourceManagerBase* resourceManager,
                                   core::ResourceId resourceId,
                                   core::WeakResourceRef weakRef,
                                   uint32_t mipLevel)
  : core::ResourceBase{ resourceManager, resourceId, true }
  , texture_{ weakRef }
  , vkImageView_{ weakRef.lock()
                    .as<type_traits::platform_implementation_t<gfx::Texture>>()
                    .device()
                    .as<type_traits::platform_implementation_t<gfx::Device>>()
                    .handle(),
                  vkDestroyImageView }
{
    if (auto texRef = weakRef.lock(); texRef.valid()) {
        auto& platformTex = texRef.as<type_traits::platform_implementation_t<gfx::Texture>>();

        assert(platformTex.type() == TextureType::TEXTURE_2D || platformTex.type() == TextureType::TEXTURE_2D_ARRAY);
        assert(mipLevel < platformTex.mipCount());

        auto& device = platformTex.device().as<type_traits::platform_implementation_t<gfx::Device>>();

        auto aspectMask = VkImageAspectFlags{};
        if (isColorFormat(platformTex.format())) {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        } else {
            if (isStencilFormat(platformTex.format())) {
                aspectMask = (platformTex.format() == Format::S8_UINT)
                               ? VK_IMAGE_ASPECT_STENCIL_BIT
                               : VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;
            } else {
                aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            }
        }

        auto rtvCreateInfo = VkImageViewCreateInfo{};
        rtvCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        rtvCreateInfo.image = platformTex.handle();
        rtvCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        rtvCreateInfo.format = internal::getFormat(platformTex.format());
        rtvCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        rtvCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        rtvCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        rtvCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        rtvCreateInfo.subresourceRange.aspectMask = aspectMask;
        rtvCreateInfo.subresourceRange.baseMipLevel = mipLevel;
        rtvCreateInfo.subresourceRange.levelCount = 1;
        rtvCreateInfo.subresourceRange.baseArrayLayer = 0;
        rtvCreateInfo.subresourceRange.layerCount = 1;

        if (auto vkResult = vkCreateImageView(device.handle(), &rtvCreateInfo, nullptr, &vkImageView_);
            vkResult != VK_SUCCESS) {
            throw Exception{ vkResult, "vkCreateImageView" };
        }
    } else {
        throw std::runtime_error("invalid texture ref");
    }
}
}
