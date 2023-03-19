//
// Created by bantdit on 1/7/20.
//

#ifndef CYCLONITE_FRAMEBUFFERRENDERTARGET_H
#define CYCLONITE_FRAMEBUFFERRENDERTARGET_H

#include "baseRenderTarget.h"

namespace cyclonite {
class FrameBufferRenderTarget : public BaseRenderTarget
{
public:
    using framebuffer_attachment_t = std::variant<vulkan::ImagePtr, VkFormat>;

public:
    template<size_t _colorAttachmentCount>
    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t width,
                            uint32_t height,
                            std::array<framebuffer_attachment_t, _colorAttachmentCount> const& images,
                            std::array<RenderTargetOutputSemantic, _colorAttachmentCount> const& semantics);

    template<size_t _colorAttachmentCount>
    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t width,
                            uint32_t height,
                            framebuffer_attachment_t const& depthStencil,
                            std::array<framebuffer_attachment_t, _colorAttachmentCount> const& images,
                            std::array<RenderTargetOutputSemantic, _colorAttachmentCount> const& semantics);

    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t width,
                            uint32_t height,
                            framebuffer_attachment_t const& depthStencil);

    FrameBufferRenderTarget(FrameBufferRenderTarget const&) = delete;

    FrameBufferRenderTarget(FrameBufferRenderTarget&&) = default;

    ~FrameBufferRenderTarget() = default;

    auto operator=(FrameBufferRenderTarget const&) -> FrameBufferRenderTarget& = delete;

    auto operator=(FrameBufferRenderTarget&&) -> FrameBufferRenderTarget& = default;

    [[nodiscard]] auto signal() const -> VkSemaphore;

    [[nodiscard]] auto signalPtr() const -> VkSemaphore const*;

    [[nodiscard]] auto wait() const -> VkSemaphore;

    void swapBuffers(vulkan::Device const& device);

    void _createSignal(vulkan::Device const& device);

private:
    std::array<vulkan::Handle<VkSemaphore>, 2> accessSemaphores_;
};

inline auto getImageView(vulkan::Device& device,
                         [[maybe_unused]] uint32_t width,
                         [[maybe_unused]] uint32_t height,
                         FrameBufferRenderTarget::framebuffer_attachment_t const& image,
                         VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                         VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) -> vulkan::ImageView
{
    return std::visit(
      [&](auto&& out) -> vulkan::ImageView {
          if constexpr (std::is_same_v<std::decay_t<decltype(out)>, vulkan::ImagePtr>) {
              return vulkan::ImageView{ device, out, VK_IMAGE_VIEW_TYPE_2D, aspectFlags };
          }

          if constexpr (std::is_same_v<std::decay_t<decltype(out)>, VkFormat>) {
              return vulkan::ImageView{ device,
                                        std::make_shared<vulkan::Image>(
                                          device, width, height, 1, 1, 1, out, VK_IMAGE_TILING_OPTIMAL, usageFlags),
                                        VK_IMAGE_VIEW_TYPE_2D,
                                        aspectFlags };
          }

          std::terminate();
      },
      image);
}

template<size_t... idx>
inline auto getColorAttachments(
  std::index_sequence<idx...>,
  vulkan::Device& device,
  uint32_t width,
  uint32_t height,
  size_t offset,
  std::array<FrameBufferRenderTarget::framebuffer_attachment_t, sizeof...(idx)> const& outputs)
  -> std::array<vulkan::ImageView, sizeof...(idx)>
{
    // TODO:: make possible to create FB readonly (no sample bit)
    return std::array{ getImageView(device,
                                    width,
                                    height,
                                    outputs[offset + idx],
                                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
                                      VK_IMAGE_USAGE_SAMPLED_BIT)... };
}

template<size_t _colorAttachmentCount>
FrameBufferRenderTarget::FrameBufferRenderTarget(
  vulkan::Device& device,
  VkRenderPass vkRenderPass,
  uint32_t width,
  uint32_t height,
  std::array<framebuffer_attachment_t, _colorAttachmentCount> const& images,
  std::array<RenderTargetOutputSemantic, _colorAttachmentCount> const& semantics)
  : BaseRenderTarget(width, height, _colorAttachmentCount)
  , accessSemaphores_{}
{
    for (auto i = size_t{ 0 }; i < _colorAttachmentCount; i++) {
        auto&& semantic = semantics[i];
        outputSemantics_[semantic] = i;
    }

    frameBuffers_.reserve(1);
    frameBuffers_.emplace_back(
      device,
      vkRenderPass,
      width,
      height,
      getColorAttachments(std::make_index_sequence<_colorAttachmentCount>{}, device, width, height, 0, images));

    for (auto i = size_t{ 0 }, count = accessSemaphores_.size(); i < count; i++) {
        accessSemaphores_[i] = vulkan::Handle<VkSemaphore>{ device.handle(), vkDestroySemaphore };
    }
}

template<size_t _colorAttachmentCount>
FrameBufferRenderTarget::FrameBufferRenderTarget(
  vulkan::Device& device,
  VkRenderPass vkRenderPass,
  uint32_t width,
  uint32_t height,
  framebuffer_attachment_t const& depthStencil,
  std::array<framebuffer_attachment_t, _colorAttachmentCount> const& images,
  std::array<RenderTargetOutputSemantic, _colorAttachmentCount> const& semantics)
  : BaseRenderTarget(width, height, _colorAttachmentCount, true)
  , accessSemaphores_{}
{
    for (auto i = size_t{ 0 }, count = _colorAttachmentCount; i < count; i++) {
        auto&& semantic = semantics[i];
        outputSemantics_[semantic] = i;
    }

    frameBuffers_.reserve(1);
    frameBuffers_.emplace_back(
      device,
      vkRenderPass,
      width,
      height,
      getImageView(device,
                   width,
                   height,
                   depthStencil,
                   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
                   VK_IMAGE_ASPECT_DEPTH_BIT),
      getColorAttachments(std::make_index_sequence<_colorAttachmentCount>{}, device, width, height, 0, images));

    for (auto i = size_t{ 0 }, count = accessSemaphores_.size(); i < count; i++) {
        accessSemaphores_[i] = vulkan::Handle<VkSemaphore>{ device.handle(), vkDestroySemaphore };
    }
}
}

#endif // CYCLONITE_FRAMEBUFFERRENDERTARGET_H
