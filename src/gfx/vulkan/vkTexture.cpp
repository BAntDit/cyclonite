//
// Created by anton on 7/27/25.
//

#include "vkTexture.h"
#include "gfx/device.h"
#include "internal/utils.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
Texture::Texture(core::ResourceRef deviceRef,
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
                 TextureUsageFlagBits usageFlags)
  : deviceRef_{ deviceRef }
  , allocation_{ VK_NULL_HANDLE }
  , vkImage_{ VK_NULL_HANDLE }
{
    assert(deviceRef_.valid());
    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();
    auto allocator = device.allocator();

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

    auto allocationCreateInfo = VmaAllocationCreateInfo{};
    allocationCreateInfo.flags = internal::getVmaAllocationFlags(allocationFlags);
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    if (auto vkResult =
          vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &vkImage_, &allocation_, nullptr);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vmaCreateImage" };
    }

    deviceRef_.retain();
}

Texture::~Texture()
{
    assert(deviceRef_.valid());
    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();
    auto allocator = device.allocator();

    vmaDestroyImage(allocator, vkImage_, allocation_);

    deviceRef_.release();
}
}
#endif