//
// Created by bantdit on 1/7/20.
//

#include "frameBufferRenderTarget.h"

namespace cyclonite {
void FrameBufferRenderTarget::swapBuffers(vulkan::Device const& device,
                                          vulkan::Handle<VkSemaphore>& passFinishedSemaphore)
{
    (void)device;
    (void)passFinishedSemaphore;

    throw std::runtime_error("not implemented yet");
}
}
