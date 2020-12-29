//
// Created by bantdit on 1/7/20.
//

#include "frameBufferRenderTarget.h"

namespace cyclonite::render {
auto FrameBufferRenderTarget::swapBuffers(vulkan::Device const& device, uint32_t currentFrameImageIndex) -> uint32_t
{
    (void)device;
    (void)currentFrameImageIndex;

    throw std::runtime_error("not implemented yet");
}
}
