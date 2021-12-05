//
// Created by anton on 1/4/21.
//

#include "baseNode.h"

namespace cyclonite::compositor {
BaseNode::BaseNode() noexcept
  : uuid_{ getNewUniqueUUID() }
  , commandsIndex_{ 0 }
  , camera_{}
  , inputs_{}
  , publicSemanticBits_{}
  , vkRenderPass_{}
  , renderTarget_{}
  , signalSemaphores_{}
  , frameCommands_{}
{}

auto BaseNode::passFinishedSemaphore() const -> vulkan::Handle<VkSemaphore> const&
{
    assert(commandsIndex_ < signalSemaphores_.size());
    return signalSemaphores_[commandsIndex_];
}

auto BaseNode::getRenderTargetBase() -> BaseRenderTarget&
{
    return const_cast<BaseRenderTarget&>(std::as_const(*this).getRenderTargetBase());
}

auto BaseNode::getRenderTargetBase() const -> BaseRenderTarget const&
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
}
