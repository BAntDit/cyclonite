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

void RenderPass::_createDummyDescriptorPool(vulkan::Device const& device, size_t maxSets)
{
    std::array<VkDescriptorPoolSize, 2> poolSizes = { VkDescriptorPoolSize{}, VkDescriptorPoolSize{} };
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = maxSets;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = maxSets;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(maxSets);
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();

    if (auto result = vkCreateDescriptorPool(device.handle(), &descriptorPoolCreateInfo, nullptr, &vkDescriptorPool_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("not enough memory on host to create descriptors pool");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("not enough memory on device to create descriptors pool");
        }
        assert(false);
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
      [](auto&& rt) -> bool {
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

              vkWaitForFences(device.handle(), 1, &frameFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

              auto [commandsIndex, wait] = rt.acquireBackBufferIndex(device, frameIndex_);

              if (rtFences_[commandsIndex] != VK_NULL_HANDLE) {
                  vkWaitForFences(
                    device.handle(), 1, &rtFences_[commandsIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
              }

              vkResetFences(device.handle(), 1, &frameFence);

              rtFences_[commandsIndex] = frameFence;

              commandsIndex_ = commandsIndex;

              frameCommands_[frameIndex_].setFrameSemaphore(wait);
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
          if constexpr (std::is_same_v<std::decay_t<decltype(rt)>, SurfaceRenderTarget> ||
                        std::is_same_v<std::decay_t<decltype(rt)>, FrameBufferRenderTarget>) {
              frameIndex_ = rt.swapBuffers(device, frameIndex_);

              return;
          }

          throw std::runtime_error("empty render target");
      },
      renderTarget_);
}
}
