//
// Created by anton on 11/23/25.
//

#include "vkShaderResourceView.h"
#include "gfx/device.h"
#include "gfx/texture.h"
#include "internal/utils.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
namespace {
auto getSrvType(TextureType type) -> VkImageViewType
{
    auto result = VK_IMAGE_VIEW_TYPE_MAX_ENUM;

    switch (type) {
        case TextureType::TEXTURE_1D:
            result = VK_IMAGE_VIEW_TYPE_1D;
            break;
        case TextureType::TEXTURE_2D:
            result = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case TextureType::TEXTURE_3D:
            result = VK_IMAGE_VIEW_TYPE_3D;
            break;
        case TextureType::TEXTURE_CUBE:
            result = VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        case TextureType::TEXTURE_1D_ARRAY:
            result = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            break;
        case TextureType::TEXTURE_2D_ARRAY:
            result = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            break;
        case TextureType::TEXTURE_CUBE_ARRAY:
            result = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            break;
        default:
            assert(false);
    }

    return result;
}
}
ShaderResourceView::ShaderResourceView(core::ResourceManagerBase* resourceManager,
                                       core::ResourceId resourceId,
                                       core::ResourceWeakRef weakRef,
                                       uint32_t maxMipLevel,
                                       uint32_t maxArrayLayer)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , textureRef_{ weakRef }
  , vkImageView_{ weakRef.lock()
                    .as<type_traits::platform_implementation_t<gfx::Texture>>()
                    .device()
                    .as<type_traits::platform_implementation_t<gfx::Device>>()
                    .handle(),
                  vkDestroyImageView }
  , maxMipLevel_{ maxMipLevel }
  , maxArrayLayer_{ maxArrayLayer }
{
    if (auto texRef = weakRef.lock(); texRef.valid()) {
        auto& platformTex = texRef.as<type_traits::platform_implementation_t<gfx::Texture>>();
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

        auto srvCreateInfo = VkImageViewCreateInfo{};
        srvCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        srvCreateInfo.image = platformTex.handle();
        srvCreateInfo.viewType = getSrvType(platformTex.type());
        srvCreateInfo.format = internal::getFormat(platformTex.format());
        srvCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        srvCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        srvCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        srvCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        srvCreateInfo.subresourceRange.aspectMask = aspectMask;
        srvCreateInfo.subresourceRange.baseMipLevel = 0;
        srvCreateInfo.subresourceRange.levelCount = maxMipLevel;
        srvCreateInfo.subresourceRange.baseArrayLayer = 0;
        srvCreateInfo.subresourceRange.layerCount = maxArrayLayer;

        if (auto vkResult = vkCreateImageView(device.handle(), &srvCreateInfo, nullptr, &vkImageView_);
            vkResult != VK_SUCCESS) {
            throw Exception{ vkResult, "vkCreateImageView" };
        }
    } else {
        throw std::runtime_error("invalid texture ref");
    }
}
}
#endif
