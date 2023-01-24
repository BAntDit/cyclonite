//
// Created by bantdit on 1/16/23.
//

#include "descriptorSetKey.h"
#include <cassert>
#include <easy-mp/enum.h>

namespace cyclonite::vulkan {
using namespace easy_mp;

DescriptorPoolKey::DescriptorPoolKey(uint32_t samplerCount,
                                     uint32_t combinedImageSamplerCount,
                                     uint32_t sampledImageCount,
                                     uint32_t storageImageCount,
                                     uint32_t uniformTexelBufferCount,
                                     uint32_t storageTexelBufferCount,
                                     uint32_t uniformBufferCount,
                                     uint32_t storageBufferCount,
                                     uint32_t uniformBufferDynamicCount,
                                     uint32_t storageBufferDynamicCount,
                                     uint32_t inputAttachmentCount,
                                     uint32_t inlineUniformBlockCount,
                                     uint32_t accelerationStructureCount,
                                     uint32_t mutableValueCount) noexcept
  : key_{}
{
    auto* uint8Array = new (key_.data()) std::array<uint8_t, 16>{};

    assert(samplerCount < 255);
    uint8Array->at(value_cast(DescriptorType::SAMPLER)) = static_cast<uint8_t>(samplerCount);

    assert(combinedImageSamplerCount < 255);
    uint8Array->at(value_cast(DescriptorType::COMBINED_IMAGE_SAMPLER)) =
      static_cast<uint8_t>(combinedImageSamplerCount);

    assert(samplerCount < 255);
    uint8Array->at(value_cast(DescriptorType::SAMPLED_IMAGE)) = static_cast<uint8_t>(sampledImageCount);

    assert(storageImageCount < 255);
    uint8Array->at(value_cast(DescriptorType::STORAGE_IMAGE)) = static_cast<uint8_t>(storageImageCount);

    assert(uniformTexelBufferCount < 255);
    uint8Array->at(value_cast(DescriptorType::UNIFORM_TEXEL_BUFFER)) = static_cast<uint8_t>(uniformTexelBufferCount);

    assert(storageTexelBufferCount < 255);
    uint8Array->at(value_cast(DescriptorType::STORAGE_TEXEL_BUFFER)) = static_cast<uint8_t>(storageTexelBufferCount);

    assert(uniformBufferCount < 255);
    uint8Array->at(value_cast(DescriptorType::UNIFORM_BUFFER)) = static_cast<uint8_t>(uniformBufferCount);

    assert(storageBufferCount < 255);
    uint8Array->at(value_cast(DescriptorType::STORAGE_BUFFER)) = static_cast<uint8_t>(storageBufferCount);

    assert(uniformBufferDynamicCount < 255);
    uint8Array->at(value_cast(DescriptorType::UNIFORM_BUFFER_DYNAMIC)) =
      static_cast<uint8_t>(uniformBufferDynamicCount);

    assert(storageBufferDynamicCount < 255);
    uint8Array->at(value_cast(DescriptorType::STORAGE_BUFFER_DYNAMIC)) =
      static_cast<uint8_t>(storageBufferDynamicCount);

    assert(inputAttachmentCount < 255);
    uint8Array->at(value_cast(DescriptorType::INPUT_ATTACHMENT)) = static_cast<uint8_t>(inputAttachmentCount);

    assert(inlineUniformBlockCount < 255);
    uint8Array->at(value_cast(DescriptorType::INLINE_UNIFORM_BLOCK)) = static_cast<uint8_t>(inlineUniformBlockCount);

    assert(accelerationStructureCount < 255);
    uint8Array->at(value_cast(DescriptorType::ACCELERATION_STRUCTURE)) =
      static_cast<uint8_t>(accelerationStructureCount);

    assert(mutableValueCount < 255);
    uint8Array->at(value_cast(DescriptorType::MUTABLE_VALUE)) = static_cast<uint8_t>(mutableValueCount);
}

DescriptorPoolKey::DescriptorPoolKey(std::array<uint64_t, 2> val) noexcept
  : key_(val)
{}

auto DescriptorPoolKey::operator=(std::array<uint64_t, 2> key) -> DescriptorPoolKey&
{
    key_ = key;
    return *this;
}

auto DescriptorPoolKey::get(DescriptorType descriptorType) const -> uint32_t
{
    return static_cast<uint32_t>(*(reinterpret_cast<std::byte const*>(key_.data()) + value_cast(descriptorType)));
}
}