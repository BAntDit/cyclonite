//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_VULKANRENDERER_H
#define CYCLONITE_VULKANRENDERER_H

#include "vulkan/device.h"
#include "multithreading/taskManager.h"
#include <array>

namespace cyclonite {
class RenderPass;

class VulkanRenderer
{
public:
    VulkanRenderer(vulkan::Device& device, multithreading::TaskManager& taskManager);

    ~VulkanRenderer() = default;

    void renderOneFrame(RenderPass& renderPass);

    void finish();

private:
    void _handleSubmitError(VkResult result);

private:
    multithreading::TaskManager* taskManager_;
    vulkan::Device* device_;
    uint64_t frameCounter_;
};
}

#endif // CYCLONITE_VULKANRENDERER_H
