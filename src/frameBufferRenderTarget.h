//
// Created by bantdit on 1/7/20.
//

#ifndef CYCLONITE_FRAMEBUFFERRENDERTARGET_H
#define CYCLONITE_FRAMEBUFFERRENDERTARGET_H

#include "baseRenderTarget.h"

namespace cyclonite::render {
class FrameBufferRenderTarget : public BaseRenderTarget
{
public:
    template<template<typename> class container, size_t count>
    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t width,
                            uint32_t height,
                            container<std::variant<vulkan::ImagePtr, VkFormat>> const& images,
                            std::array<RenderTargetOutputSemantic, count> const& semantics,
                            std::array<VkClearColorValue, count> const& clearColorValues);

    template<template<typename> class container, size_t count>
    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t width,
                            uint32_t height,
                            container<std::variant<VkFormat, vulkan::ImagePtr>> depthStencil,
                            VkClearDepthStencilValue clearDepthStencilValue,
                            container<std::variant<vulkan::ImagePtr, VkFormat>> const& images,
                            std::array<RenderTargetOutputSemantic, count> const& semantics,
                            std::array<VkClearColorValue, count> const& clearColorValues);

    FrameBufferRenderTarget(FrameBufferRenderTarget const&) = delete;

    FrameBufferRenderTarget(FrameBufferRenderTarget&&) = default;

    ~FrameBufferRenderTarget() = default;

    auto operator=(FrameBufferRenderTarget const&) -> FrameBufferRenderTarget& = delete;

    auto operator=(FrameBufferRenderTarget &&) -> FrameBufferRenderTarget& = default;

    auto swapBuffers(vulkan::Device const& device, uint32_t currentFrameImageIndex) -> uint32_t;

private:
    std::vector<vulkan::Handle<VkSemaphore>> bufferAvailableSemaphores_;
};

inline auto getImage(vulkan::Device& device,
                     [[maybe_unused]] uint32_t width,
                     [[maybe_unused]] uint32_t height,
                     std::variant<VkFormat, vulkan::ImagePtr> const& image)
{
    return std::visit(
      [&](auto&& out) -> vulkan::ImageView {
          if constexpr (std::is_same_v<std::decay_t<decltype(out)>, vulkan::ImagePtr>) {
              return vulkan::ImageView{ device, out };
          }

          if constexpr (std::is_same_v<std::decay_t<decltype(out)>, VkFormat>) {
              return vulkan::ImageView{
                  device,
                  std::make_shared<vulkan::Image>(
                    device, width, height, 1, 1, 1, out, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
              };
          }

          std::terminate();
      },
      image);
}

template<template<typename> class container, size_t... idx>
inline auto getColorAttachments(std::index_sequence<idx...>,
                                vulkan::Device& device,
                                [[maybe_unused]] uint32_t width,
                                [[maybe_unused]] uint32_t height,
                                size_t offset,
                                container<vulkan::ImagePtr> const& outputs)
  -> std::array<vulkan::ImageView, sizeof...(idx)>
{
    return std::array{ (getImage(device, width, height, outputs[offset + idx]), ...) };
}

template<template<typename> class container, size_t count>
FrameBufferRenderTarget::FrameBufferRenderTarget(vulkan::Device& device,
                                                 VkRenderPass vkRenderPass,
                                                 uint32_t width,
                                                 uint32_t height,
                                                 container<std::variant<vulkan::ImagePtr, VkFormat>> const& images,
                                                 std::array<RenderTargetOutputSemantic, count> const& semantics,
                                                 std::array<VkClearColorValue, count> const& clearColorValues)
  : BaseRenderTarget(width, height, clearColorValues)
{
    assert(images.size() % count == 0);

    auto bufferCount = images.size() / count;

    for (auto i = size_t{ 0 }; i < count; i++) {
        auto&& semantic = semantics[i];
        outputSemantics_[semantic] = i;
    }

    for (auto i = size_t{ 0 }; i < bufferCount; i++) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          width,
          height,
          getColorAttachments(std::make_index_sequence<count>{}, device, width, height, count * i, images));
    }
}

template<template<typename> class container, size_t count>
FrameBufferRenderTarget::FrameBufferRenderTarget(vulkan::Device& device,
                                                 VkRenderPass vkRenderPass,
                                                 uint32_t width,
                                                 uint32_t height,
                                                 container<std::variant<VkFormat, vulkan::ImagePtr>> depthStencil,
                                                 VkClearDepthStencilValue clearDepthStencilValue,
                                                 container<std::variant<vulkan::ImagePtr, VkFormat>> const& images,
                                                 std::array<RenderTargetOutputSemantic, count> const& semantics,
                                                 std::array<VkClearColorValue, count> const& clearColorValues)
  : BaseRenderTarget(width, height, clearDepthStencilValue, clearColorValues)
{
    assert(images.size() % count == 0);

    auto bufferCount = images.size() / count;

    for (auto i = size_t{ 0 }; i < count; i++) {
        auto&& semantic = semantics[i];
        outputSemantics_[semantic] = i;
    }

    for (auto i = size_t{ 0 }; i < bufferCount; i++) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          width,
          height,
          vulkan::ImageView{ device, getImage(device, width, height, depthStencil) },
          getColorAttachments(std::make_index_sequence<count>{}, device, width, height, count * i, images));
    }
}
}

#endif // CYCLONITE_FRAMEBUFFERRENDERTARGET_H
