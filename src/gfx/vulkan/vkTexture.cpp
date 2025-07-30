//
// Created by anton on 7/27/25.
//

#include "vkTexture.h"
#include "internal/utils.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
Texture::Texture(TextureCreationFlagBits imageCreateFlags,
                 TextureType textureType,
                 Format format,
                 uint32_t width,
                 uint32_t height,
                 uint32_t depth,
                 uint32_t mipCount,
                 uint32_t arrayLayerCount,
                 TextureTiling tiling,
                 TextureUsageFlagBits usageFlags)
  : allocation_{ VK_NULL_HANDLE }
  , vkImage_{ VK_NULL_HANDLE }
{
    auto imageCreateInfo = VkImageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.flags = imageCreateFlags.cast_to<VkImageCreateFlags>();
    imageCreateInfo.imageType = internal::getImageType(textureType);
    imageCreateInfo.format = internal::getFormat(format);
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = depth;
    imageCreateInfo.mipLevels = mipCount;
    imageCreateInfo.arrayLayers = arrayLayerCount;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = internal::getTiling(tiling);
    imageCreateInfo.usage = usageFlags.cast_to<VkImageUsageFlags>();
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // vmaCreateImage()
}
}
#endif