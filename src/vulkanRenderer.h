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

    void renderOneFrame(RenderPass& renderPass);

private:
    vulkan::Device* device_;
};
}

#endif // CYCLONITE_VULKANRENDERER_H
