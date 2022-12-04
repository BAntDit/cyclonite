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
}
