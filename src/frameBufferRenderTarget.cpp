//
// Created by bantdit on 1/7/20.
//

#include "frameBufferRenderTarget.h"

namespace cyclonite {
auto FrameBufferRenderTarget::swapBuffers(vulkan::Device const& device, uint32_t frameIndex) -> uint32_t
{
    (void)device;
    (void)frameIndex;

    throw std::runtime_error("not implemented yet");
}
}
