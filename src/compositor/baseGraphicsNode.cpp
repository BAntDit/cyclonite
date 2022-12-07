//
// Created by bantdit on 12/1/22.
//

#include "baseGraphicsNode.h"

namespace cyclonite::compositor {
auto BaseGraphicsNode::getRenderTargetBase() const -> BaseRenderTarget const&
{
    return std::visit(
      [](auto&& rt) -> BaseRenderTarget const& {
          if constexpr (std::is_base_of_v<BaseRenderTarget, std::decay_t<decltype(rt)>>) {
              return rt;
          }

          std::terminate();
      },
      renderTarget_);
}

auto BaseGraphicsNode::getRenderTargetBase() -> BaseRenderTarget&
{
    return const_cast<BaseRenderTarget&>(std::as_const(*this).getRenderTargetBase());
}

auto BaseGraphicsNode::passFinishedSemaphore() const -> VkSemaphore
{
    return std::visit(
      [](auto&& rt) -> VkSemaphore {
          if constexpr (std::is_same_v<FrameBufferRenderTarget, std::decay_t<decltype(rt)>>) {
              return rt.signal();
          }

          if constexpr (std::is_same_v<SurfaceRenderTarget, std::decay_t<decltype(rt)>>) {
              return rt.signal();
          }

          std::terminate();
      },
      renderTarget_);
}

auto BaseGraphicsNode::passFinishedSemaphorePtr() const -> VkSemaphore const*
{
    return std::visit(
      [](auto&& rt) -> VkSemaphore const* {
          if constexpr (std::is_same_v<FrameBufferRenderTarget, std::decay_t<decltype(rt)>>) {
              return rt.signalPtr();
          }

          if constexpr (std::is_same_v<SurfaceRenderTarget, std::decay_t<decltype(rt)>>) {
              return rt.signalPtr();
          }

          std::terminate();
      },
      renderTarget_);
}

void BaseGraphicsNode::swapBuffers(vulkan::Device& device)
{
    std::visit(
      [&device](auto&& rt) -> void {
          if constexpr (std::is_same_v<FrameBufferRenderTarget, std::decay_t<decltype(rt)>> ||
                        std::is_same_v<SurfaceRenderTarget, std::decay_t<decltype(rt)>>) {
              rt.swapBuffers(device);
          }
      },
      renderTarget_);
}

auto BaseGraphicsNode::frameCommands() const -> std::pair<uint32_t, VkCommandBuffer const*>
{
    return std::make_pair(static_cast<uint32_t>(frameCommands_[bufferIndex_].graphicsCommands_->commandBufferCount()),
                          frameCommands_[bufferIndex_].graphicsCommands_->pCommandBuffers());
}
}
