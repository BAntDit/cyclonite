//
// Created by anton on 7/28/25.
//

#ifndef CYCLONITE_GFX_VULKAN_UTILS_H
#define CYCLONITE_GFX_VULKAN_UTILS_H

#include "gfx/common.h"
#include <cassert>

#if defined(GFX_DRIVER_VULKAN)
#include "vmaUsage.h"
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan::internal {
inline constexpr auto getImageLayout(TextureState state) -> VkImageLayout 
{
    auto result = VK_IMAGE_LAYOUT_UNDEFINED;

    switch (state) 
    {
        case TextureState::UNDEFINED:
            result = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        case TextureState::GENERAL:
            result = VK_IMAGE_LAYOUT_GENERAL;
            break;
        case TextureState::COLOR_ATTACHMENT_OPTIMAL:
            result = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        case TextureState::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            result = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        case TextureState::DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            result = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            break;
        case TextureState::SHADER_READ_ONLY_OPTIMAL:
            result = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            break;
        case TextureState::TRANSFER_DST_OPTIMAL:
            result = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            break;
        case TextureState::TRANSFER_SRC_OPTIMAL:
            result = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            break;
        case TextureState::PREINITIALIZED:
            result = VK_IMAGE_LAYOUT_PREINITIALIZED;
            break;
        default:
            assert(false);
    }

    return result;
}

inline constexpr auto getImageViewType(TextureType type) -> VkImageViewType
{
    auto result = VK_IMAGE_VIEW_TYPE_MAX_ENUM;

    switch (type) {
        case TextureType::TEXTURE_1D:
            result = VK_IMAGE_VIEW_TYPE_1D;
            break;
        case TextureType::TEXTURE_1D_ARRAY:
            result = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
            break;
        case TextureType::TEXTURE_2D:
            result = VK_IMAGE_VIEW_TYPE_2D;
            break;
        case TextureType::TEXTURE_2D_ARRAY:
            result = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
            break;
        case TextureType::TEXTURE_CUBE:
            result = VK_IMAGE_VIEW_TYPE_CUBE;
            break;
        case TextureType::TEXTURE_CUBE_ARRAY:
            result = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            break;
        case TextureType::TEXTURE_3D:
            result = VK_IMAGE_VIEW_TYPE_3D;
            break;
        default:
            assert(false);
    }

    return result;
}

inline constexpr auto getVmaAllocationFlags(GpuMemoryAllocationFlagBits allocationFlags) -> VmaAllocationCreateFlags
{
    auto flags = VmaAllocationCreateFlags{};

    if (allocationFlags.test(GpuMemoryAllocationFlags::DEDICATED_MEMORY)) {
        flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }
    if (allocationFlags.test(GpuMemoryAllocationFlags::USE_EXISTING_BLOCK)) {
        flags |= VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT;
    }
    if (allocationFlags.test(GpuMemoryAllocationFlags::PERSISTENT_MAPPED_MEMORY)) {
        flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }
    if (allocationFlags.test(GpuMemoryAllocationFlags::HOST_ACCESS_SEQUENTIAL_WRITE)) {
        flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }
    if (allocationFlags.test(GpuMemoryAllocationFlags::HOST_ACCESS_RANDOM_ORDER_WRITE_AND_READ)) {
        flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    }
    if (allocationFlags.test(GpuMemoryAllocationFlags::MIN_MEMORY_STRATEGY)) {
        flags |= VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
    }
    if (allocationFlags.test(GpuMemoryAllocationFlags::MIN_TIME_STRATEGY)) {
        flags |= VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT;
    }
    if (allocationFlags.test(GpuMemoryAllocationFlags::MIN_OFFSET_STRATEGY)) {
        flags |= VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT;
    }
    return flags;
}

inline constexpr auto getTiling(TextureTiling tiling) -> VkImageTiling
{
    auto vkTiling = VK_IMAGE_TILING_OPTIMAL;

    if (tiling == TextureTiling::OPTIMAL) {
        vkTiling = VK_IMAGE_TILING_OPTIMAL;
    } else if (tiling == TextureTiling::LINEAR) {
        vkTiling = VK_IMAGE_TILING_LINEAR;
    } else {
        assert(false);
    }

    return vkTiling;
}

inline constexpr auto getImageType(TextureType type) -> VkImageType
{
    auto result = VkImageType{ VK_IMAGE_TYPE_MAX_ENUM };

    switch (type) {
        case TextureType::TEXTURE_1D:
            [[fallthrough]];
        case TextureType::TEXTURE_1D_ARRAY:
            result = VK_IMAGE_TYPE_1D;
            break;
        case TextureType::TEXTURE_2D:
            [[fallthrough]];
        case TextureType::TEXTURE_2D_ARRAY:
            [[fallthrough]];
        case TextureType::TEXTURE_CUBE:
            [[fallthrough]];
        case TextureType::TEXTURE_CUBE_ARRAY:
            result = VK_IMAGE_TYPE_2D;
            break;
        case TextureType::TEXTURE_3D:
            result = VK_IMAGE_TYPE_3D;
            break;
        default:
            assert(false);
    }

    return result;
}

inline constexpr auto getFormat(Format format) -> VkFormat
{
    auto vkFormat = VkFormat{ VK_FORMAT_UNDEFINED };

    switch (format) {
        case Format::A1R5G5B5_UNORM_PACK16:
            vkFormat = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
            break;
        case Format::R4G4_UNORM_PACK8:
            vkFormat = VK_FORMAT_R4G4_UNORM_PACK8;
            break;
        case Format::R4G4B4A4_UNORM_PACK16:
            vkFormat = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
            break;
        case Format::B4G4R4A4_UNORM_PACK16:
            vkFormat = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
            break;
        case Format::R5G6B5_UNORM_PACK16:
            vkFormat = VK_FORMAT_R5G6B5_UNORM_PACK16;
            break;
        case Format::B5G6R5_UNORM_PACK16:
            vkFormat = VK_FORMAT_B5G6R5_UNORM_PACK16;
            break;
        case Format::R5G5B5A1_UNORM_PACK16:
            vkFormat = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
            break;
        case Format::B5G5R5A1_UNORM_PACK16:
            vkFormat = VK_FORMAT_B5G5R5A1_UNORM_PACK16;
            break;
        case Format::R8_UNORM:
            vkFormat = VK_FORMAT_R8_UNORM;
            break;
        case Format::R8_SNORM:
            vkFormat = VK_FORMAT_R8_SNORM;
            break;
        case Format::R8_USCALED:
            vkFormat = VK_FORMAT_R8_USCALED;
            break;
        case Format::R8_SSCALED:
            vkFormat = VK_FORMAT_R8_SSCALED;
            break;
        case Format::R8_UINT:
            vkFormat = VK_FORMAT_R8_UINT;
            break;
        case Format::R8_SINT:
            vkFormat = VK_FORMAT_R8_SINT;
            break;
        case Format::R8_SRGB:
            vkFormat = VK_FORMAT_R8_SRGB;
            break;
        case Format::R8G8_UNORM:
            vkFormat = VK_FORMAT_R8G8_UNORM;
            break;
        case Format::R8G8_SNORM:
            vkFormat = VK_FORMAT_R8G8_SNORM;
            break;
        case Format::R8G8_USCALED:
            vkFormat = VK_FORMAT_R8G8_USCALED;
            break;
        case Format::R8G8_SSCALED:
            vkFormat = VK_FORMAT_R8G8_SSCALED;
            break;
        case Format::R8G8_UINT:
            vkFormat = VK_FORMAT_R8G8_UINT;
            break;
        case Format::R8G8_SINT:
            vkFormat = VK_FORMAT_R8G8_SINT;
            break;
        case Format::R8G8_SRGB:
            vkFormat = VK_FORMAT_R8G8_SRGB;
            break;
        case Format::R8G8B8_UNORM:
            vkFormat = VK_FORMAT_R8G8B8_UNORM;
            break;
        case Format::R8G8B8_SNORM:
            vkFormat = VK_FORMAT_R8G8B8_SNORM;
            break;
        case Format::R8G8B8_USCALED:
            vkFormat = VK_FORMAT_R8G8B8_USCALED;
            break;
        case Format::R8G8B8_SSCALED:
            vkFormat = VK_FORMAT_R8G8B8_SSCALED;
            break;
        case Format::R8G8B8_UINT:
            vkFormat = VK_FORMAT_R8G8B8_UINT;
            break;
        case Format::R8G8B8_SINT:
            vkFormat = VK_FORMAT_R8G8B8_SINT;
            break;
        case Format::R8G8B8_SRGB:
            vkFormat = VK_FORMAT_R8G8B8_SRGB;
            break;
        case Format::B8G8R8_UNORM:
            vkFormat = VK_FORMAT_B8G8R8_UNORM;
            break;
        case Format::B8G8R8_SNORM:
            vkFormat = VK_FORMAT_B8G8R8_SNORM;
            break;
        case Format::B8G8R8_USCALED:
            vkFormat = VK_FORMAT_B8G8R8_USCALED;
            break;
        case Format::B8G8R8_SSCALED:
            vkFormat = VK_FORMAT_B8G8R8_SSCALED;
            break;
        case Format::B8G8R8_UINT:
            vkFormat = VK_FORMAT_B8G8R8_UINT;
            break;
        case Format::B8G8R8_SINT:
            vkFormat = VK_FORMAT_B8G8R8_SINT;
            break;
        case Format::B8G8R8_SRGB:
            vkFormat = VK_FORMAT_B8G8R8_SRGB;
            break;
        case Format::R8G8B8A8_UNORM:
            vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case Format::R8G8B8A8_SNORM:
            vkFormat = VK_FORMAT_R8G8B8A8_SNORM;
            break;
        case Format::R8G8B8A8_USCALED:
            vkFormat = VK_FORMAT_R8G8B8A8_USCALED;
            break;
        case Format::R8G8B8A8_SSCALED:
            vkFormat = VK_FORMAT_R8G8B8A8_SSCALED;
            break;
        case Format::R8G8B8A8_UINT:
            vkFormat = VK_FORMAT_R8G8B8A8_UINT;
            break;
        case Format::R8G8B8A8_SINT:
            vkFormat = VK_FORMAT_R8G8B8A8_SINT;
            break;
        case Format::R8G8B8A8_SRGB:
            vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        case Format::B8G8R8A8_UNORM:
            vkFormat = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        case Format::B8G8R8A8_SNORM:
            vkFormat = VK_FORMAT_B8G8R8A8_SNORM;
            break;
        case Format::B8G8R8A8_USCALED:
            vkFormat = VK_FORMAT_B8G8R8A8_USCALED;
            break;
        case Format::B8G8R8A8_SSCALED:
            vkFormat = VK_FORMAT_B8G8R8A8_SSCALED;
            break;
        case Format::B8G8R8A8_UINT:
            vkFormat = VK_FORMAT_B8G8R8A8_UINT;
            break;
        case Format::B8G8R8A8_SINT:
            vkFormat = VK_FORMAT_B8G8R8A8_SINT;
            break;
        case Format::B8G8R8A8_SRGB:
            vkFormat = VK_FORMAT_B8G8R8A8_SRGB;
            break;
        case Format::A8B8G8R8_UNORM_PACK32:
            vkFormat = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
            break;
        case Format::A8B8G8R8_SNORM_PACK32:
            vkFormat = VK_FORMAT_A8B8G8R8_SNORM_PACK32;
            break;
        case Format::A8B8G8R8_USCALED_PACK32:
            vkFormat = VK_FORMAT_A8B8G8R8_USCALED_PACK32;
            break;
        case Format::A8B8G8R8_SSCALED_PACK32:
            vkFormat = VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
            break;
        case Format::A8B8G8R8_UINT_PACK32:
            vkFormat = VK_FORMAT_A8B8G8R8_UINT_PACK32;
            break;
        case Format::A8B8G8R8_SINT_PACK32:
            vkFormat = VK_FORMAT_A8B8G8R8_SINT_PACK32;
            break;
        case Format::A8B8G8R8_SRGB_PACK32:
            vkFormat = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
            break;
        case Format::A2R10G10B10_UNORM_PACK32:
            vkFormat = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
            break;
        case Format::A2R10G10B10_SNORM_PACK32:
            vkFormat = VK_FORMAT_A2R10G10B10_SNORM_PACK32;
            break;
        case Format::A2R10G10B10_USCALED_PACK32:
            vkFormat = VK_FORMAT_A2R10G10B10_USCALED_PACK32;
            break;
        case Format::A2R10G10B10_SSCALED_PACK32:
            vkFormat = VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
            break;
        case Format::A2R10G10B10_UINT_PACK32:
            vkFormat = VK_FORMAT_A2R10G10B10_UINT_PACK32;
            break;
        case Format::A2R10G10B10_SINT_PACK32:
            vkFormat = VK_FORMAT_A2R10G10B10_SINT_PACK32;
            break;
        case Format::A2B10G10R10_UNORM_PACK32:
            vkFormat = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
            break;
        case Format::A2B10G10R10_SNORM_PACK32:
            vkFormat = VK_FORMAT_A2B10G10R10_SNORM_PACK32;
            break;
        case Format::A2B10G10R10_USCALED_PACK32:
            vkFormat = VK_FORMAT_A2B10G10R10_USCALED_PACK32;
            break;
        case Format::A2B10G10R10_SSCALED_PACK32:
            vkFormat = VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
            break;
        case Format::A2B10G10R10_UINT_PACK32:
            vkFormat = VK_FORMAT_A2B10G10R10_UINT_PACK32;
            break;
        case Format::A2B10G10R10_SINT_PACK32:
            vkFormat = VK_FORMAT_A2B10G10R10_SINT_PACK32;
            break;
        case Format::R16_UNORM:
            vkFormat = VK_FORMAT_R16_UNORM;
            break;
        case Format::R16_SNORM:
            vkFormat = VK_FORMAT_R16_SNORM;
            break;
        case Format::R16_USCALED:
            vkFormat = VK_FORMAT_R16_USCALED;
            break;
        case Format::R16_SSCALED:
            vkFormat = VK_FORMAT_R16_SSCALED;
            break;
        case Format::R16_UINT:
            vkFormat = VK_FORMAT_R16_UINT;
            break;
        case Format::R16_SINT:
            vkFormat = VK_FORMAT_R16_SINT;
            break;
        case Format::R16_SFLOAT:
            vkFormat = VK_FORMAT_R16_SFLOAT;
            break;
        case Format::R16G16_UNORM:
            vkFormat = VK_FORMAT_R16G16_UNORM;
            break;
        case Format::R16G16_SNORM:
            vkFormat = VK_FORMAT_R16G16_SNORM;
            break;
        case Format::R16G16_USCALED:
            vkFormat = VK_FORMAT_R16G16_USCALED;
            break;
        case Format::R16G16_SSCALED:
            vkFormat = VK_FORMAT_R16G16_SSCALED;
            break;
        case Format::R16G16_UINT:
            vkFormat = VK_FORMAT_R16G16_UINT;
            break;
        case Format::R16G16_SINT:
            vkFormat = VK_FORMAT_R16G16_SINT;
            break;
        case Format::R16G16_SFLOAT:
            vkFormat = VK_FORMAT_R16G16_SFLOAT;
            break;
        case Format::R16G16B16_UNORM:
            vkFormat = VK_FORMAT_R16G16B16_UNORM;
            break;
        case Format::R16G16B16_SNORM:
            vkFormat = VK_FORMAT_R16G16B16_SNORM;
            break;
        case Format::R16G16B16_USCALED:
            vkFormat = VK_FORMAT_R16G16B16_USCALED;
            break;
        case Format::R16G16B16_SSCALED:
            vkFormat = VK_FORMAT_R16G16B16_SSCALED;
            break;
        case Format::R16G16B16_UINT:
            vkFormat = VK_FORMAT_R16G16B16_UINT;
            break;
        case Format::R16G16B16_SINT:
            vkFormat = VK_FORMAT_R16G16B16_SINT;
            break;
        case Format::R16G16B16_SFLOAT:
            vkFormat = VK_FORMAT_R16G16B16_SFLOAT;
            break;
        case Format::R16G16B16A16_UNORM:
            vkFormat = VK_FORMAT_R16G16B16A16_UNORM;
            break;
        case Format::R16G16B16A16_SNORM:
            vkFormat = VK_FORMAT_R16G16B16A16_SNORM;
            break;
        case Format::R16G16B16A16_USCALED:
            vkFormat = VK_FORMAT_R16G16B16A16_USCALED;
            break;
        case Format::R16G16B16A16_SSCALED:
            vkFormat = VK_FORMAT_R16G16B16A16_SSCALED;
            break;
        case Format::R16G16B16A16_UINT:
            vkFormat = VK_FORMAT_R16G16B16A16_UINT;
            break;
        case Format::R16G16B16A16_SINT:
            vkFormat = VK_FORMAT_R16G16B16A16_SINT;
            break;
        case Format::R16G16B16A16_SFLOAT:
            vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
            break;
        case Format::R32_UINT:
            vkFormat = VK_FORMAT_R32_UINT;
            break;
        case Format::R32_SINT:
            vkFormat = VK_FORMAT_R32_SINT;
            break;
        case Format::R32_SFLOAT:
            vkFormat = VK_FORMAT_R32_SFLOAT;
            break;
        case Format::R32G32_UINT:
            vkFormat = VK_FORMAT_R32G32_UINT;
            break;
        case Format::R32G32_SINT:
            vkFormat = VK_FORMAT_R32G32_SINT;
            break;
        case Format::R32G32_SFLOAT:
            vkFormat = VK_FORMAT_R32G32_SFLOAT;
            break;
        case Format::R32G32B32_UINT:
            vkFormat = VK_FORMAT_R32G32B32_UINT;
            break;
        case Format::R32G32B32_SINT:
            vkFormat = VK_FORMAT_R32G32B32_SINT;
            break;
        case Format::R32G32B32_SFLOAT:
            vkFormat = VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case Format::R32G32B32A32_UINT:
            vkFormat = VK_FORMAT_R32G32B32A32_UINT;
            break;
        case Format::R32G32B32A32_SINT:
            vkFormat = VK_FORMAT_R32G32B32A32_SINT;
            break;
        case Format::R32G32B32A32_SFLOAT:
            vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
        case Format::R64_UINT:
            vkFormat = VK_FORMAT_R64_UINT;
            break;
        case Format::R64_SINT:
            vkFormat = VK_FORMAT_R64_SINT;
            break;
        case Format::R64_SFLOAT:
            vkFormat = VK_FORMAT_R64_SFLOAT;
            break;
        case Format::R64G64_UINT:
            vkFormat = VK_FORMAT_R64G64_UINT;
            break;
        case Format::R64G64_SINT:
            vkFormat = VK_FORMAT_R64G64_SINT;
            break;
        case Format::R64G64_SFLOAT:
            vkFormat = VK_FORMAT_R64G64_SFLOAT;
            break;
        case Format::R64G64B64_UINT:
            vkFormat = VK_FORMAT_R64G64B64_UINT;
            break;
        case Format::R64G64B64_SINT:
            vkFormat = VK_FORMAT_R64G64B64_SINT;
            break;
        case Format::R64G64B64_SFLOAT:
            vkFormat = VK_FORMAT_R64G64B64_SFLOAT;
            break;
        case Format::R64G64B64A64_UINT:
            vkFormat = VK_FORMAT_R64G64B64A64_UINT;
            break;
        case Format::R64G64B64A64_SINT:
            vkFormat = VK_FORMAT_R64G64B64A64_SINT;
            break;
        case Format::R64G64B64A64_SFLOAT:
            vkFormat = VK_FORMAT_R64G64B64A64_SFLOAT;
            break;
        case Format::B10G11R11_UFLOAT_PACK32:
            vkFormat = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
            break;
        case Format::E5B9G9R9_UFLOAT_PACK32:
            vkFormat = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
            break;
        case Format::D16_UNORM:
            vkFormat = VK_FORMAT_D16_UNORM;
            break;
        case Format::X8_D24_UNORM_PACK32:
            vkFormat = VK_FORMAT_X8_D24_UNORM_PACK32;
            break;
        case Format::D32_SFLOAT:
            vkFormat = VK_FORMAT_D32_SFLOAT;
            break;
        case Format::S8_UINT:
            vkFormat = VK_FORMAT_S8_UINT;
            break;
        case Format::D16_UNORM_S8_UINT:
            vkFormat = VK_FORMAT_D16_UNORM_S8_UINT;
            break;
        case Format::D24_UNORM_S8_UINT:
            vkFormat = VK_FORMAT_D24_UNORM_S8_UINT;
            break;
        case Format::D32_SFLOAT_S8_UINT:
            vkFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
            break;
        case Format::BC1_RGB_UNORM_BLOCK:
            vkFormat = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            break;
        case Format::BC1_RGB_SRGB_BLOCK:
            vkFormat = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
            break;
        case Format::BC1_RGBA_UNORM_BLOCK:
            vkFormat = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            break;
        case Format::BC1_RGBA_SRGB_BLOCK:
            vkFormat = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            break;
        case Format::BC2_UNORM_BLOCK:
            vkFormat = VK_FORMAT_BC2_UNORM_BLOCK;
            break;
        case Format::BC2_SRGB_BLOCK:
            vkFormat = VK_FORMAT_BC2_SRGB_BLOCK;
            break;
        case Format::BC3_UNORM_BLOCK:
            vkFormat = VK_FORMAT_BC3_UNORM_BLOCK;
            break;
        case Format::BC3_SRGB_BLOCK:
            vkFormat = VK_FORMAT_BC3_SRGB_BLOCK;
            break;
        case Format::BC4_UNORM_BLOCK:
            vkFormat = VK_FORMAT_BC4_UNORM_BLOCK;
            break;
        case Format::BC4_SNORM_BLOCK:
            vkFormat = VK_FORMAT_BC4_SNORM_BLOCK;
            break;
        case Format::BC5_UNORM_BLOCK:
            vkFormat = VK_FORMAT_BC5_UNORM_BLOCK;
            break;
        case Format::BC5_SNORM_BLOCK:
            vkFormat = VK_FORMAT_BC5_SNORM_BLOCK;
            break;
        case Format::BC6H_UFLOAT_BLOCK:
            vkFormat = VK_FORMAT_BC6H_UFLOAT_BLOCK;
            break;
        case Format::BC6H_SFLOAT_BLOCK:
            vkFormat = VK_FORMAT_BC6H_SFLOAT_BLOCK;
            break;
        case Format::BC7_UNORM_BLOCK:
            vkFormat = VK_FORMAT_BC7_UNORM_BLOCK;
            break;
        case Format::BC7_SRGB_BLOCK:
            vkFormat = VK_FORMAT_BC7_SRGB_BLOCK;
            break;
        case Format::ETC2_R8G8B8_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            break;
        case Format::ETC2_R8G8B8_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
            break;
        case Format::ETC2_R8G8B8A1_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
            break;
        case Format::ETC2_R8G8B8A1_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
            break;
        case Format::ETC2_R8G8B8A8_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
            break;
        case Format::ETC2_R8G8B8A8_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
            break;
        case Format::EAC_R11_UNORM_BLOCK:
            vkFormat = VK_FORMAT_EAC_R11_UNORM_BLOCK;
            break;
        case Format::EAC_R11_SNORM_BLOCK:
            vkFormat = VK_FORMAT_EAC_R11_SNORM_BLOCK;
            break;
        case Format::EAC_R11G11_UNORM_BLOCK:
            vkFormat = VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
            break;
        case Format::EAC_R11G11_SNORM_BLOCK:
            vkFormat = VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
            break;
        case Format::ASTC_4x4_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
            break;
        case Format::ASTC_4x4_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
            break;
        case Format::ASTC_5x4_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
            break;
        case Format::ASTC_5x4_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
            break;
        case Format::ASTC_5x5_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
            break;
        case Format::ASTC_5x5_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
            break;
        case Format::ASTC_6x5_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
            break;
        case Format::ASTC_6x5_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
            break;
        case Format::ASTC_6x6_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
            break;
        case Format::ASTC_6x6_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
            break;
        case Format::ASTC_8x5_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
            break;
        case Format::ASTC_8x5_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
            break;
        case Format::ASTC_8x6_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
            break;
        case Format::ASTC_8x6_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
            break;
        case Format::ASTC_8x8_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
            break;
        case Format::ASTC_8x8_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
            break;
        case Format::ASTC_10x5_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
            break;
        case Format::ASTC_10x5_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
            break;
        case Format::ASTC_10x6_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
            break;
        case Format::ASTC_10x6_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
            break;
        case Format::ASTC_10x8_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
            break;
        case Format::ASTC_10x8_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
            break;
        case Format::ASTC_10x10_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
            break;
        case Format::ASTC_10x10_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
            break;
        case Format::ASTC_12x10_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
            break;
        case Format::ASTC_12x10_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
            break;
        case Format::ASTC_12x12_UNORM_BLOCK:
            vkFormat = VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
            break;
        case Format::ASTC_12x12_SRGB_BLOCK:
            vkFormat = VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
            break;
        case Format::UNDEFINED:
            vkFormat = VK_FORMAT_UNDEFINED;
            break;
        default:
            assert(false);
    }

    return vkFormat;
}
}

#endif
#endif // CYCLONITE_GFX_VULKAN_UTILS_H
