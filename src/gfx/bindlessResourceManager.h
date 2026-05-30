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
#include <vector>

namespace cyclonite::gfx {
class BindlessResourceManager
{
public:
    BindlessResourceManager(size_t swapChainLength, core::ResourceSharedRef deviceRef);

    void emplaceSeparatedTexture(std::span<core::ResourceSharedRef> textureRef);

    void startFrame(uint64_t frameIndex);

    void endFrame() {}

    [[nodiscard]] auto currentFrameGlobalDescriptorSet() const -> core::ResourceSharedRef const&
    {
        return globalDescriptorSetRef_[frameIndex_];
    }

private:
    static constexpr size_t max_global_descriptor_set_count_v = config_t::max_swapchain_length_v + 1;

    using copy_data_t = std::vector<DescriptorCopyData>;
    using frame_mask_t = std::bitset<max_global_descriptor_set_count_v>;
    using update_stack_t = std::list<std::pair<copy_data_t, frame_mask_t>>;

    uint64_t frameIndex_;

    update_stack_t updateStack_;
    std::array<core::ResourceSharedRef, max_global_descriptor_set_count_v> globalDescriptorSetRef_;
    std::map<gfx::DescriptorType, std::vector<size_t>> freeResourceIndices_;
};
}

#endif // CYCLONITE_BINDLESS_RESOURCE_MANAGER_H