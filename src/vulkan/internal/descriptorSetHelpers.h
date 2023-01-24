//
// Created by anton on 1/18/23.
//

#ifndef CYCLONITE_DESCRIPTORSETHELPERS_H
#define CYCLONITE_DESCRIPTORSETHELPERS_H

#include "vulkan/pipelineDescriptorSets.h"
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan::internal {
inline auto descriptorTypeToVulkanDescriptorType(DescriptorType type) -> VkDescriptorType
{
    auto vkType = VkDescriptorType{ VK_DESCRIPTOR_TYPE_MAX_ENUM };

    switch (type) {
        case DescriptorType::SAMPLER:
            vkType = VK_DESCRIPTOR_TYPE_SAMPLER;
            break;
        case DescriptorType::COMBINED_IMAGE_SAMPLER:
            vkType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        case DescriptorType::SAMPLED_IMAGE:
            vkType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            break;
        case DescriptorType::STORAGE_IMAGE:
            vkType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            break;
        case DescriptorType::UNIFORM_TEXEL_BUFFER:
            vkType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            break;
        case DescriptorType::STORAGE_TEXEL_BUFFER:
            vkType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            break;
        case DescriptorType::UNIFORM_BUFFER:
            vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
        case DescriptorType::STORAGE_BUFFER:
            vkType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            break;
        case DescriptorType::UNIFORM_BUFFER_DYNAMIC:
            vkType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            break;
        case DescriptorType::STORAGE_BUFFER_DYNAMIC:
            vkType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            break;
        case DescriptorType::INPUT_ATTACHMENT:
            vkType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            break;
        case DescriptorType::INLINE_UNIFORM_BLOCK:
            vkType = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
            break;
        case DescriptorType::ACCELERATION_STRUCTURE:
            vkType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
            break;
        default:
            vkType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    assert(vkType != VK_DESCRIPTOR_TYPE_MAX_ENUM);
    return vkType;
}
}

#endif // CYCLONITE_DESCRIPTORSETHELPERS_H
