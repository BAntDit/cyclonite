//
// Created by bantdit on 11/21/19.
//

#include "renderPass.h"
#include "vulkan/shaderModule.h"

std::vector<uint32_t> const defaultVertexShaderCode = {
#include "shaders/default.vert.spv.cpp.txt"
};

std::vector<uint32_t> const defaultFragmentShaderCode = {
#include "shaders/default.frag.spv.cpp.txt"
};

namespace cyclonite {
void RenderPass::_createRenderPass(vulkan::Device const& device, VkRenderPassCreateInfo const& renderPassCreateInfo)
{
    if (auto result = vkCreateRenderPass(device.handle(), &renderPassCreateInfo, nullptr, &vkRenderPass_);
        result != VK_SUCCESS) {

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("not enough RAM memory to create render pass");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("not enough GPU memory to create render pass");
        }

        assert(false);
    }
}

auto RenderPass::begin(vulkan::Device& device) -> std::tuple<FrameCommands&, VkFence, VkSemaphore>
{
    return std::visit(
      [&, this](auto&& rt) -> std::tuple<FrameCommands&, VkFence, VkSemaphore> {
          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, SurfaceRenderTarget>) {
              auto frontBufferIndex = rt.frontBufferIndex();
              auto frameFence = frameCommands_[frontBufferIndex].fence();
              auto passFinishedSemaphore = frameCommands_[frontBufferIndex].semaphore();

              vkWaitForFences(device.handle(), 1, &frameFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

              auto backBufferIndex = rt.acquireBackBufferIndex(device);

              if (renderTargetFences_[backBufferIndex] != VK_NULL_HANDLE) {
                  vkWaitForFences(device.handle(),
                                  1,
                                  &renderTargetFences_[backBufferIndex],
                                  VK_TRUE,
                                  std::numeric_limits<uint64_t>::max());
              }

              vkResetFences(device.handle(), 1, &frameFence);

              auto& frame = frameCommands_[backBufferIndex];
              auto framebuffer = rt.frameBuffers()[backBufferIndex].handle();
              auto bufferAvailableSemaphore = rt.frameBufferAvailableSemaphore();

              renderTargetFences_[backBufferIndex] = frameFence;

              frame.update(device,
                           static_cast<VkRenderPass>(vkRenderPass_),
                           framebuffer,
                           std::array<uint32_t, 4>{ 0, 0, rt.width(), rt.height() },
                           bufferAvailableSemaphore,
                           passFinishedSemaphore,
                           rt.getClearValues(),
                           rt.hasDepthStencil(),
                           frameUpdate_);

              return std::forward_as_tuple(frame, frameFence, passFinishedSemaphore);
          }

          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, FrameBufferRenderTarget>) {
              throw std::runtime_error("not implemented yet");
          }

          throw std::runtime_error("empty render target");
      },
      renderTarget_);
}

void RenderPass::end(vulkan::Device const& device, VkSemaphore passFinishedSemaphore)
{
    std::visit(
      [&](auto&& rt) -> void {
          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, SurfaceRenderTarget> ||
                        std::is_same_v<std::decay_t<decltype(rt)>, FrameBufferRenderTarget>) {
              rt.swapBuffers(device, static_cast<VkSemaphore>(passFinishedSemaphore));

              return;
          }

          throw std::runtime_error("empty render target");
      },
      renderTarget_);
}
}
