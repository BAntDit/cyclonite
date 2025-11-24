//
// Created by anton on 11/15/25.
//

#include "vkDescriptorSet.h"
#include "gfx/buffer.h"
#include "gfx/device.h"
#include "gfx/sampler.h"
#include "gfx/texture.h"
#include "gfx/shaderResourceView.h"
#include "internal/utils.h"
#include "vkDescriptorPool.h"
#include <vector>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
DescriptorSet::DescriptorSet(core::ResourceManagerBase* resourceManager,
                             core::ResourceId resourceId,
                             core::ResourceSharedRef descriptorPool,
                             uint32_t setIndex)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , descriptorPool_{ std::move(descriptorPool) }
  , vkDescriptorSet_{ VK_NULL_HANDLE }
  , index_{ setIndex }
{
    assert(descriptorPool_.valid());
    auto& pool = descriptorPool_.as<DescriptorPool>();

    vkDescriptorSet_ = pool.allocateDescriptorSet();
}

void DescriptorSet::update(std::span<DescriptorWriteData const> updateData)
{
    auto writeData = std::vector<VkWriteDescriptorSet>{};
    writeData.reserve(updateData.size());

    for (auto const& updateDesc : updateData) {
        auto& writeDesc = writeData.emplace_back();
        auto [resource, binding, element, desc] = updateDesc;

        auto descType =
          std::visit([](auto&& d) -> VkDescriptorType { return internal::getDescriptorType(d.type); }, desc);

        writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDesc.dstSet = vkDescriptorSet_;
        writeDesc.dstBinding = binding;
        writeDesc.dstArrayElement = element;
        writeDesc.descriptorCount = 1;
        writeDesc.descriptorType = descType;

        assert(resource.valid());
        if (descType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
            descType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
            descType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            auto bufferInfo = VkDescriptorBufferInfo{};

            auto const& bufferDesc = std::get<BufferResourceDescription>(desc);
            auto& buffer = resource.as<type_traits::platform_implementation_t<gfx::Buffer>>();

            bufferInfo.buffer = buffer.handle();
            bufferInfo.offset = bufferDesc.offset;
            bufferInfo.range = bufferDesc.size;

            writeDesc.pBufferInfo = &bufferInfo;
        } else if (descType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            auto imageInfo = VkDescriptorImageInfo{};

            auto& texture = resource.as<type_traits::platform_implementation_t<gfx::Texture>>();
            auto srvRef = texture.getSRV();
            auto samplerRef = texture.sampler();

            auto& srv = srvRef.as<type_traits::platform_implementation_t<gfx::ShaderResourceView>>();
            auto& sampler = samplerRef.as<type_traits::platform_implementation_t<gfx::Sampler>>();

            auto const & imageDesc = std::get<TextureResourceDescription>(desc);

            imageInfo.imageView = srv.handle();
            imageInfo.sampler = sampler.handle();
            imageInfo.imageLayout = internal::getImageLayout(imageDesc.state);

            writeDesc.pImageInfo = &imageInfo;
        } else if (descType == VK_DESCRIPTOR_TYPE_SAMPLER) {
            auto imageInfo = VkDescriptorImageInfo{};
            auto& sampler = resource.as<type_traits::platform_implementation_t<gfx::Sampler>>();

            imageInfo.imageView = VK_NULL_HANDLE;
            imageInfo.sampler = sampler.handle();
            imageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;

            writeDesc.pImageInfo = &imageInfo;
        }
    }

    auto& pool = descriptorPool_.as<DescriptorPool>();
    auto deviceRef = pool.device();
    auto& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();

    vkUpdateDescriptorSets(device.handle(), writeData.size(), writeData.data(), 0, nullptr);
}

DescriptorSet::~DescriptorSet()
{
    assert(descriptorPool_.valid());
    auto& pool = descriptorPool_.as<DescriptorPool>();

    if (vkDescriptorSet_ != VK_NULL_HANDLE) {
        pool.freeDescriptorSet(vkDescriptorSet_);
        vkDescriptorSet_ = VK_NULL_HANDLE;
    }
}
}
#endif