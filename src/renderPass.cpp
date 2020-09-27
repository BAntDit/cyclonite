//
// Created by bantdit on 11/21/19.
//

#include "renderPass.h"
#include <iostream>

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

void RenderPass::_createSemaphores(vulkan::Device const& device, size_t count)
{
    signalSemaphores_.reserve(count);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < count; i++) {
        if (auto result = vkCreateSemaphore(device.handle(),
                                            &semaphoreCreateInfo,
                                            nullptr,
                                            &signalSemaphores_.emplace_back(device.handle(), vkDestroySemaphore));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create frame signal semaphore.");
        }
    }
}

auto RenderPass::viewport() const -> std::array<uint32_t, 4>
{
    return std::visit(
      [](auto&& rt) -> std::array<uint32_t, 4> {
          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, SurfaceRenderTarget> ||
                        std::is_same_v<std::decay_t<decltype(rt)>, FrameBufferRenderTarget>) {
              return std::array<uint32_t, 4>{ 0, 0, rt.width(), rt.height() };
          }

          std::terminate();
      },
      renderTarget_);
}

auto RenderPass::hasDepthStencil() const -> bool
{
    return std::visit(
      [](auto&& rt) -> bool {
          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, SurfaceRenderTarget> ||
                        std::is_same_v<std::decay_t<decltype(rt)>, FrameBufferRenderTarget>) {
              return rt.hasDepthStencil();
          }

          std::terminate();
      },
      renderTarget_);
}

auto RenderPass::getSwapChainLength() const -> size_t
{
    return std::visit(
      [](auto&& rt) -> size_t {
          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, SurfaceRenderTarget> ||
                        std::is_same_v<std::decay_t<decltype(rt)>, FrameBufferRenderTarget>) {
              return rt.swapChainLength();
          }

          std::terminate();
      },
      renderTarget_);
}

void RenderPass::begin(vulkan::Device& device)
{
    std::visit(
      [&, this](auto&& rt) -> void {
          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, SurfaceRenderTarget>) {
              auto frameFence = static_cast<VkFence>(frameFences_[frameIndex_]);

              // wait until frame over before render this frame
              vkWaitForFences(device.handle(), 1, &frameFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

              // it acquires image index for the frame
              // image index matches with commands for this image
              // (so, fame image index == commands index we going to render)
              // returns: commands index (image index), semaphore to be sure image is available
              auto [commandsIndex, wait] = rt.acquireBackBufferIndex(device, frameIndex_);

              if (rtFences_[commandsIndex] != VK_NULL_HANDLE) {
                  // wait until command buffer for acquired image get free
                  vkWaitForFences(
                    device.handle(), 1, &rtFences_[commandsIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
              }

              vkResetFences(device.handle(), 1, &frameFence);

              rtFences_[commandsIndex] = frameFence;

              commandsIndex_ = commandsIndex;

              frameCommands_[commandsIndex_].resetWaitSemaphores();

              // frameCommands_[frameIndex_].frameSemaphore() = wait; // ??? - is it right??? check frames sync
              // this frame renderer submits frameCommands[commands index] this frame
              // so, exactly frameCommands[commands index] must wait for semaphore to be sure frame image available
              frameCommands_[commandsIndex_].frameSemaphore() = wait;

              return;
          }

          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, FrameBufferRenderTarget>) {
              throw std::runtime_error("not implemented yet");
          }

          throw std::runtime_error("empty render target");
      },
      renderTarget_);
}

void RenderPass::update(vulkan::Device& device,
                        VkDescriptorPool descriptorPool,
                        VkDescriptorSetLayout descriptorSetLayout,
                        VkPipelineLayout pipelineLayout,
                        VkPipeline pipeline)
{
    // TODO:: review please
    std::visit(
      [&, this](auto&& rt) -> void {
          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, SurfaceRenderTarget> ||
                        std::is_same_v<std::decay_t<decltype(rt)>, FrameBufferRenderTarget>) {
              auto framebuffer = rt.frameBuffers()[commandsIndex_].handle();

              frameCommands_[commandsIndex_].update(device,
                                                    handle(),
                                                    framebuffer,
                                                    std::array<uint32_t, 4>{ 0, 0, rt.width(), rt.height() },
                                                    rt.getClearValues(),
                                                    descriptorPool,
                                                    descriptorSetLayout,
                                                    pipelineLayout,
                                                    pipeline);

              return;
          }

          throw std::runtime_error("empty render target");
      },
      renderTarget_);
}

void RenderPass::end(vulkan::Device const& device)
{
    std::visit(
      [&, this](auto&& rt) -> void {
          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, SurfaceRenderTarget>) {
              rt.swapBuffers(device, passFinishedSemaphore(), commandsIndex_);

              frameIndex_ = (frameIndex_ + 1) % rt.swapChainLength();

              return;
          }

          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, FrameBufferRenderTarget>) {
              throw std::runtime_error("not implemented yet");
          }

          throw std::runtime_error("empty render target");
      },
      renderTarget_);
}
}
