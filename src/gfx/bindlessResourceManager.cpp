//
// Created by anton on 5/29/26.
//

#include "bindlessResourceManager.h"
#include "gfx/device.h"
#include "gfx/resourceManager.h"
#include "multithreading/executor.h"
#include <bit>

#include "descriptorSet.h"

namespace cyclonite::gfx {
namespace {
enum class BindingPoint : uint32_t
{
    UNIFORM_BUFFER = 0,
    STORAGE_BUFFER = 1,
    SAMPLED_IMAGE = 2,
    COMBINED_IMAGE_SAMPLER = 3
};
}
BindlessResourceManager::BindlessResourceManager()
  : frameIndex_{ 0 }
  , lastResourceIndex_{ 0 }
  , updateStack_{}
  , globalDescriptorCount_{ 0 }
  , globalDescriptorSetRef_{}
  , emplacedResources_{}
  , freeResourceIndices_{}
{
}

void BindlessResourceManager::init(size_t swapChainLength, core::ResourceSharedRef deviceRef)
{
    globalDescriptorCount_ = swapChainLength + 1;

    auto& device = deviceRef.as<gfx::Device>();

    auto allShaderStages = gfx::ShaderStageFlagBits{ gfx::ShaderStageFlags::ALL };
    auto setLayoutFlags = gfx::DescriptorSetLayoutFlagBits{ gfx::DescriptorSetLayoutFlags::UPDATE_AFTER_BIND };
    auto bindingFlags =
      gfx::BindingFlagBits{ gfx::BindingFlags::UPDATE_AFTER_BIND, gfx::BindingFlags::PARTIALLY_BOUND };

    auto bindings = std::array{ gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              metrix::value_cast(BindingPoint::UNIFORM_BUFFER),
                                              gfx::DescriptorType::UNIFORM_BUFFER,
                                              max_global_descriptor_count_v,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags },
                                gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              metrix::value_cast(BindingPoint::STORAGE_BUFFER),
                                              gfx::DescriptorType::STORAGE_BUFFER,
                                              max_global_descriptor_count_v,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags },
                                gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              metrix::value_cast(BindingPoint::SAMPLED_IMAGE),
                                              gfx::DescriptorType::SAMPLED_IMAGE,
                                              max_global_descriptor_count_v,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags },
                                gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              metrix::value_cast(BindingPoint::COMBINED_IMAGE_SAMPLER),
                                              gfx::DescriptorType::COMBINED_IMAGE_SAMPLER,
                                              max_global_descriptor_count_v,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags } };

    auto bindingSchema = device.getOrCreatePipelineBindingSchema(bindings, std::span<gfx::PushConstantRange>{});

    assert(globalDescriptorCount_ <= max_global_descriptor_set_count_v);
    for (auto i = size_t{ 0 }; i < globalDescriptorCount_; i++) {
        globalDescriptorSetRef_[i] = device.allocateDescriptorSetBySchema(
          bindingSchema, metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS), false);
    }
}

void BindlessResourceManager::startFrame(uint64_t frameIndex)
{
    auto it = updateStack_.begin();
    while (it != updateStack_.end()) {
        auto& [data, mask] = *(it);

        if (!mask.test(frameIndex)) {
            auto srcIndex = static_cast<size_t>(std::countr_zero(~mask.to_ullong())); // TODO:: test
            assert(srcIndex < globalDescriptorCount_);

            auto& dst = globalDescriptorSetRef_[frameIndex];
            auto const& src = globalDescriptorSetRef_[srcIndex];
            dst.as<gfx::DescriptorSet>().copy(src, data);

            mask.reset(frameIndex);
        }

        if (mask.none()) {
            it = updateStack_.erase(it);
        } else {
            ++it;
        }
    }
}

auto BindlessResourceManager::emplaceTexture(core::ResourceSharedRef const& resourceRef) -> uint32_t
{
    assert(multithreading::Executor::isInRenderThread());
    assert(frameIndex_ < globalDescriptorCount_);

    bool isWriteDataComplete = false;
    auto elementIndex = std::numeric_limits<uint32_t>::max();
    auto writeData = gfx::DescriptorWriteData{};
    writeData.resource = resourceRef;

    assert(resourceRef.typeIndex() == resource_manager_t::resource_meta_t::type_index_v<gfx::Texture>());

    auto& texture = resourceRef.as<gfx::Texture>();
    bool hasSampler = texture.sampler().valid();

    auto descriptorType = gfx::DescriptorType{};

    if (hasSampler) {
        descriptorType = gfx::DescriptorType::COMBINED_IMAGE_SAMPLER;
        elementIndex = getElementIndex(descriptorType);

        if (elementIndex != std::numeric_limits<uint32_t>::max()) {
            writeData.binding = metrix::value_cast(BindingPoint::COMBINED_IMAGE_SAMPLER);
            writeData.element = elementIndex;

            auto textureDesc = TextureResourceDescription{};
            textureDesc.type = descriptorType;
            textureDesc.state = texture.currentState();
            writeData.desc = textureDesc;

            isWriteDataComplete = true;
        }
    } else {
        descriptorType = gfx::DescriptorType::SAMPLED_IMAGE;
        elementIndex = getElementIndex(descriptorType);

        if (elementIndex != std::numeric_limits<uint32_t>::max()) {
            writeData.binding = metrix::value_cast(BindingPoint::SAMPLED_IMAGE);
            writeData.element = elementIndex;

            auto textureDesc = TextureResourceDescription{};
            textureDesc.type = descriptorType;
            textureDesc.state = texture.currentState();
            writeData.desc = textureDesc;

            isWriteDataComplete = true;
        }
    }

    if (isWriteDataComplete) {
        emplaceResource(resourceRef, writeData, elementIndex, descriptorType);
    } else {
        throw std::runtime_error("could not emplace texture into global bindless descriptor");
    }

    return elementIndex;
}

auto BindlessResourceManager::emplaceUniformBuffer(core::ResourceSharedRef const& resourceRef,
                                                   size_t offset,
                                                   size_t size) -> uint32_t
{
    assert(multithreading::Executor::isInRenderThread());
    assert(frameIndex_ < globalDescriptorCount_);

    bool isWriteDataComplete = false;
    auto elementIndex = getElementIndex(gfx::DescriptorType::UNIFORM_BUFFER);
    auto writeData = gfx::DescriptorWriteData{};
    writeData.resource = resourceRef;

    assert(resourceRef.typeIndex() == resource_manager_t::resource_meta_t::type_index_v<gfx::Buffer>());

    if (elementIndex != std::numeric_limits<uint32_t>::max()) {
        writeData.binding = metrix::value_cast(BindingPoint::UNIFORM_BUFFER);
        writeData.element = elementIndex;

        auto bufferDesc = BufferResourceDescription{};
        bufferDesc.type = gfx::DescriptorType::UNIFORM_BUFFER;
        bufferDesc.offset = offset;
        bufferDesc.size = size;
        writeData.desc = bufferDesc;

        isWriteDataComplete = true;
    }

    if (isWriteDataComplete) {
        emplaceResource(resourceRef, writeData, elementIndex, gfx::DescriptorType::UNIFORM_BUFFER);
    } else {
        throw std::runtime_error("could not emplace uniform buffer into global bindless descriptor");
    }

    return elementIndex;
}

auto BindlessResourceManager::emplaceStorageBuffer(core::ResourceSharedRef const& resourceRef,
                                                   size_t offset,
                                                   size_t size) -> uint32_t
{
    assert(multithreading::Executor::isInRenderThread());
    assert(frameIndex_ < globalDescriptorCount_);

    bool isWriteDataComplete = false;
    auto elementIndex = getElementIndex(gfx::DescriptorType::STORAGE_BUFFER);
    auto writeData = gfx::DescriptorWriteData{};
    writeData.resource = resourceRef;

    assert(resourceRef.typeIndex() == resource_manager_t::resource_meta_t::type_index_v<gfx::Buffer>());

    if (elementIndex != std::numeric_limits<uint32_t>::max()) {
        writeData.binding = metrix::value_cast(BindingPoint::STORAGE_BUFFER);
        writeData.element = elementIndex;

        auto bufferDesc = BufferResourceDescription{};
        bufferDesc.type = gfx::DescriptorType::STORAGE_BUFFER;
        bufferDesc.offset = offset;
        bufferDesc.size = size;
        writeData.desc = bufferDesc;

        isWriteDataComplete = true;
    }

    if (isWriteDataComplete) {
        emplaceResource(resourceRef, writeData, elementIndex, gfx::DescriptorType::STORAGE_BUFFER);
    } else {
        throw std::runtime_error("could not emplace uniform buffer into global bindless descriptor");
    }

    return elementIndex;
}

auto BindlessResourceManager::releaseResourceIndex(uint32_t resourceIndex) -> bool
{
    if (auto it = emplacedResources_.find(resourceIndex); it != emplacedResources_.end()) {
        auto& [_, res] = *it;
        auto [descType, resourceRef] = res;

        auto freeIndicesIt = freeResourceIndices_.find(descType);
        if (freeIndicesIt == freeResourceIndices_.end()) {
            auto newFreeIndices = std::vector<uint32_t>{};
            auto [newIt, success] = freeResourceIndices_.emplace(descType, newFreeIndices);
            assert(success);

            freeIndicesIt = newIt;
        }

        auto& [_1, freeIndices] = *freeIndicesIt;
        freeIndices.push_back(resourceIndex);

        emplacedResources_.erase(resourceIndex);

        return true;
    }

    return false;
}

void BindlessResourceManager::emplaceResource(core::ResourceSharedRef const& resourceRef,
                                              gfx::DescriptorWriteData const& writeData,
                                              uint32_t elementIndex,
                                              gfx::DescriptorType descriptorType)
{
    auto& dst = globalDescriptorSetRef_[frameIndex_].as<gfx::DescriptorSet>();
    dst.update(std::span{ &writeData, 1 });

    auto mask = frame_mask_t{};
    setUpdateMask(mask);

    auto copyData = gfx::DescriptorCopyData{};
    copyData.srcElement = writeData.element;
    copyData.dstElement = writeData.element;
    copyData.srcBinding = writeData.binding;
    copyData.dstBinding = writeData.binding;

    auto copyVec = std::vector<gfx::DescriptorCopyData>{};
    copyVec.push_back(copyData);

    updateStack_.push_back(std::pair{ copyVec, mask });

    emplacedResources_.emplace(elementIndex, std::pair{ descriptorType, resourceRef });
}

auto BindlessResourceManager::getElementIndex(gfx::DescriptorType descriptorType) -> uint32_t
{
    auto elementIndex = std::numeric_limits<uint32_t>::max();

    auto freeIndicesIt = freeResourceIndices_.find(descriptorType);
    if (freeIndicesIt != freeResourceIndices_.end()) {
        auto& [_, freeIndices] = *freeIndicesIt;
        if (!freeIndices.empty()) {
            elementIndex = freeIndices.back();
            freeIndices.pop_back();
        }
    }
    if (elementIndex == std::numeric_limits<uint32_t>::max() && lastResourceIndex_ < max_global_descriptor_count_v) {
        elementIndex = lastResourceIndex_++;
    }

    return elementIndex;
}

void BindlessResourceManager::setUpdateMask(frame_mask_t& mask) const
{
    for (auto i = size_t{ 0 }; i < globalDescriptorCount_; i++) {
        if (i == frameIndex_)
            continue;
        mask.set(i, true);
    }
}
}
