//
// Created by bantdit on 1/15/23.
//

#ifndef CYCLONITE_DESCRIPTORTYPE_H
#define CYCLONITE_DESCRIPTORTYPE_H

#include <cassert>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
enum class DescriptorType
{
    SAMPLER = 0,
    COMBINED_IMAGE_SAMPLER = 1,
    SAMPLED_IMAGE = 2,
    STORAGE_IMAGE = 3,
    UNIFORM_TEXEL_BUFFER = 4,
    STORAGE_TEXEL_BUFFER = 5,
    UNIFORM_BUFFER = 6,
    STORAGE_BUFFER = 7,
    UNIFORM_BUFFER_DYNAMIC = 8,
    STORAGE_BUFFER_DYNAMIC = 9,
    INPUT_ATTACHMENT = 10,
    INLINE_UNIFORM_BLOCK = 11,
    ACCELERATION_STRUCTURE = 12,
    MUTABLE_VALUE = 13,
    MIN_VALUE = SAMPLER,
    MAX_VALUE = MUTABLE_VALUE,
    COUNT = MAX_VALUE + 1
};
}

#endif // CYCLONITE_DESCRIPTORTYPE_H