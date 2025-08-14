//
// Created by anton on 7/27/25.
//

#include "vkTexture.h"
#include "gfx/device.h"
#include "internal/utils.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
Texture::Texture(core::ResourceManagerBase* resourceManager,
                 core::ResourceId resourceId,
                 core::ResourceRef deviceRef,
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
  : core::ResourceBase{ resourceManager, resourceId, true }
  , deviceRef_{ deviceRef }
  , allocation_{ VK_NULL_HANDLE }
  , vkImage_{ VK_NULL_HANDLE }
  , state_{ TextureState::UNDEFINED }
  , format_{ format }
  , width_{ width }
  , height_{ height }
  , depth_{ depth }
  , mipCount_{ mipCount }
{
    assert(deviceRef_.valid());
    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();
    auto allocator = device.allocator();

    auto vkFormat = internal::getFormat(format_);

    auto imageCreateInfo = VkImageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.flags = imageCreateFlags.cast_to<VkImageCreateFlags>();
    imageCreateInfo.imageType = internal::getImageType(textureType);
    imageCreateInfo.format = vkFormat;
    imageCreateInfo.extent.width = width_;
    imageCreateInfo.extent.height = height_;
    imageCreateInfo.extent.depth = depth_;
    imageCreateInfo.mipLevels = mipCount_;
    imageCreateInfo.arrayLayers = arrayLayerCount;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = internal::getTiling(tiling);
    imageCreateInfo.usage = usageFlags.cast_to<VkImageUsageFlags>();
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = internal::getImageLayout(state_);

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