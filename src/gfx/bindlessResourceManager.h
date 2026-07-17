//
// Created by anton on 5/29/26.
//

#ifndef CYCLONITE_BINDLESS_RESOURCE_MANAGER_H
#define CYCLONITE_BINDLESS_RESOURCE_MANAGER_H

#include "core/resourceSharedRef.h"
#include "descriptorUpdateData.h"
#include "gfx/config.h"
#include <array>
#include <bitset>
#include <list>
#include <map>
#include <span>
#include <unordered_map>
#include <vector>

namespace cyclonite::gfx {
class BindlessResourceManager
{
public:
    BindlessResourceManager();

    void init(size_t swapChainLength, core::ResourceSharedRef deviceRef);

    void startFrame(uint64_t frameIndex);

    void endFrame() { frameIndex_ = std::numeric_limits<uint64_t>::max(); }

    [[nodiscard]] auto currentFrameGlobalDescriptorSet() const -> core::ResourceSharedRef const&
    {
        return globalDescriptorSetRef_[frameIndex_];
    }

    auto emplaceTexture(core::ResourceSharedRef const& resourceRef) -> uint32_t;

    auto emplaceUniformBuffer(core::ResourceSharedRef const& resourceRef, size_t offset, size_t size) -> uint32_t;

    auto emplaceStorageBuffer(core::ResourceSharedRef const& resourceRef, size_t offset, size_t size) -> uint32_t;

    auto releaseResourceIndex(uint32_t resourceIndex) -> bool;

private:
    static constexpr size_t max_global_descriptor_set_count_v = config_t::max_swapchain_length_v + 1;
    static constexpr uint32_t max_global_descriptor_count_v = 100000;

    using copy_data_t = std::vector<DescriptorCopyData>;
    using frame_mask_t = std::bitset<max_global_descriptor_set_count_v>;
    using update_stack_t = std::list<std::pair<copy_data_t, frame_mask_t>>;

private:
    [[nodiscard]] auto getElementIndex(gfx::DescriptorType descriptorType) -> uint32_t;

    void emplaceResource(core::ResourceSharedRef const& resourceRef,
                         gfx::DescriptorWriteData const& writeData,
                         uint32_t elementIndex,
                         gfx::DescriptorType descriptorType);

    void setUpdateMask(frame_mask_t& mask) const;

    uint64_t frameIndex_;
    uint32_t lastResourceIndex_;

    update_stack_t updateStack_;

    size_t globalDescriptorCount_;
    std::array<core::ResourceSharedRef, max_global_descriptor_set_count_v> globalDescriptorSetRef_;
    std::unordered_map<uint32_t, std::pair<gfx::DescriptorType, core::ResourceSharedRef>> emplacedResources_;
    std::map<gfx::DescriptorType, std::vector<uint32_t>> freeResourceIndices_;
};
}

#endif // CYCLONITE_BINDLESS_RESOURCE_MANAGER_H