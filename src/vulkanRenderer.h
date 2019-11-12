//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_VULKANRENDERER_H
#define CYCLONITE_VULKANRENDERER_H

#include "renderPass.h"
#include "vulkan/device.h"
#include <array>

namespace cyclonite {
class VulkanRenderer
{
public:
    VulkanRenderer(vulkan::Device& device);

    ~VulkanRenderer() = default;

    void renderOneFrame(RenderPass const& renderPass);

private:
    std::array<vulkan::Handle<VkFence>, 2> frameSyncFences_; // for a while dummy sync
    uint64_t frameNumber_;                                   // tmp
    vulkan::Device* device_;
};
}

#endif // CYCLONITE_VULKANRENDERER_H
