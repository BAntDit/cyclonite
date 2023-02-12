//
// Created by bantdit on 1/16/23.
//

#ifndef CYCLONITE_DESCRIPTORSETKEY_H
#define CYCLONITE_DESCRIPTORSETKEY_H

#include "render/descriptorType.h"
#include <array>
#include <cstdint>

namespace cyclonite::vulkan {
// bytes:
// 0: samplerCount;
// 1: combinedImageSamplerCount
// 2: sampledImageCount
// 3: storageImageCount
// 4: uniformTexelBufferCount
// 5: storageTexelBufferCount
// 6: uniformBufferCount
// 7: storageBufferCount
// 8: uniformBufferDynamicCount
// 9: storageBufferDynamicCount
// 10: inputAttachmentCount
// 11: inlineUniformBlockCount
// 12: accelerationStructureCount;
// 13: mutableValueCount;
using packed_descriptor_set_key_t = std::array<uint64_t, 2>;

struct DescriptorPoolKey
{
    explicit DescriptorPoolKey(uint32_t samplerCount,
                               uint32_t combinedImageSamplerCount = 0,
                               uint32_t sampledImageCount = 0,
                               uint32_t storageImageCount = 0,
                               uint32_t uniformTexelBufferCount = 0,
                               uint32_t storageTexelBufferCount = 0,
                               uint32_t uniformBufferCount = 0,
                               uint32_t storageBufferCount = 0,
                               uint32_t uniformBufferDynamicCount = 0,
                               uint32_t storageBufferDynamicCount = 0,
                               uint32_t inputAttachmentCount = 0,
                               uint32_t inlineUniformBlockCount = 0,
                               uint32_t accelerationStructureCount = 0,
                               uint32_t mutableValueCount = 0) noexcept;

    explicit DescriptorPoolKey(std::array<uint64_t, 2> val) noexcept;

    explicit operator std::array<uint64_t, 2>() const { return key_; }

    auto operator=(std::array<uint64_t, 2> key) -> DescriptorPoolKey&;

    [[nodiscard]] auto get(DescriptorType descriptorType) const -> uint32_t;

private:
    packed_descriptor_set_key_t key_;
};
}

#endif // CYCLONITE_DESCRIPTORSETKEY_H
