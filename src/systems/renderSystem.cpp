//
// Created by bantdit on 3/22/20.
//

#include "renderSystem.h"

/*
std::vector<uint32_t> const defaultVertexShaderCode = {
#include "shaders/default.vert.spv.cpp.txt"
};

std::vector<uint32_t> const defaultFragmentShaderCode = {
#include "shaders/default.frag.spv.cpp.txt"
};
*/

namespace cyclonite::systems {
void RenderSystem::init(multithreading::TaskManager& taskManager, vulkan::Device& device)
{
    taskManager_ = &taskManager;
    device_ = &device;
}

void RenderSystem::finish()
{
    assert(taskManager_ != nullptr);
    assert(device_ != nullptr);

    auto gfxStop = taskManager_->submitRenderTask([queue = device_->graphicsQueue()]() -> void {
        if (auto result = vkQueueWaitIdle(queue); result != VK_SUCCESS) {
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                throw std::runtime_error("could not wait until graphics queue gets idle, out of host memory");
            }

            if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                throw std::runtime_error("could not wait until graphics queue gets idle, out of device memory");
            }

            if (result == VK_ERROR_DEVICE_LOST) {
                throw std::runtime_error("could not wait until graphics queue gets idle, device lost");
            }
        }
    });

    auto transferStop = taskManager_->submitRenderTask([queue = device_->hostTransferQueue()]() -> void {
        if (auto result = vkQueueWaitIdle(queue); result != VK_SUCCESS) {
            if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                throw std::runtime_error("could not wait until transfer queue gets idle, out of host memory");
            }

            if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                throw std::runtime_error("could not wait until transfer queue gets idle, out of device memory");
            }

            if (result == VK_ERROR_DEVICE_LOST) {
                throw std::runtime_error("could not wait until transfer queue gets idle, device lost");
            }
        }
    });

    transferStop.get();
    gfxStop.get();
}
}