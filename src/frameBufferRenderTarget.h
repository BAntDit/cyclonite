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
    template<size_t count>
    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t width,
                            uint32_t height,
                            std::array<framebuffer_attachment_t, count> const& images,
                            std::array<RenderTargetOutputSemantic, count> const& semantics);

    template<size_t count>
    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t width,
                            uint32_t height,
                            framebuffer_attachment_t const& depthStencil,
                            std::array<framebuffer_attachment_t, count> const& images,
                            std::array<RenderTargetOutputSemantic, count> const& semantics);

    FrameBufferRenderTarget(vulkan::Device& device,
                            VkRenderPass vkRenderPass,
                            uint32_t width,
                            uint32_t height,
                            size_t bufferCount,
                            framebuffer_attachment_t const& depthStencil);

    FrameBufferRenderTarget(FrameBufferRenderTarget const&) = delete;

    FrameBufferRenderTarget(FrameBufferRenderTarget&&) = default;

    ~FrameBufferRenderTarget() = default;

    auto operator=(FrameBufferRenderTarget const&) -> FrameBufferRenderTarget& = delete;

    auto operator=(FrameBufferRenderTarget &&) -> FrameBufferRenderTarget& = default;

    auto swapBuffers(vulkan::Device const& device, uint32_t currentFrameImageIndex) -> uint32_t;

private:
    std::vector<vulkan::Handle<VkSemaphore>> bufferAvailableSemaphores_;
};

inline auto getImageView(vulkan::Device& device,
                         [[maybe_unused]] uint32_t width,
                         [[maybe_unused]] uint32_t height,
                         FrameBufferRenderTarget::framebuffer_attachment_t const& image,
                         VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) -> vulkan::ImageView
{
    return std::visit(
      [&](auto&& out) -> vulkan::ImageView {
          if constexpr (std::is_same_v<std::decay_t<decltype(out)>, vulkan::ImagePtr>) {
              return vulkan::ImageView{ device, out };
          }

          if constexpr (std::is_same_v<std::decay_t<decltype(out)>, VkFormat>) {
              return vulkan::ImageView{ device,
                                        std::make_shared<vulkan::Image>(
                                          device, width, height, 1, 1, 1, out, VK_IMAGE_TILING_OPTIMAL, usageFlags) };
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
    return std::array{ getImageView(device,
                                    width,
                                    height,
                                    outputs[offset + idx],
                                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)... };
}

template<size_t count>
FrameBufferRenderTarget::FrameBufferRenderTarget(vulkan::Device& device,
                                                 VkRenderPass vkRenderPass,
                                                 uint32_t width,
                                                 uint32_t height,
                                                 std::array<framebuffer_attachment_t, count> const& images,
                                                 std::array<RenderTargetOutputSemantic, count> const& semantics)
  : BaseRenderTarget(width, height, count)
{
    static_assert(count > 0);

    assert(images.size() % count == 0);

    auto bufferCount = images.size() / count;

    for (auto i = size_t{ 0 }; i < count; i++) {
        auto&& semantic = semantics[i];
        outputSemantics_[semantic] = i;
    }

    frameBuffers_.reserve(bufferCount);

    for (auto i = size_t{ 0 }; i < bufferCount; i++) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          width,
          height,
          getColorAttachments(std::make_index_sequence<count>{}, device, width, height, count * i, images));
    }
}

template<size_t count>
FrameBufferRenderTarget::FrameBufferRenderTarget(vulkan::Device& device,
                                                 VkRenderPass vkRenderPass,
                                                 uint32_t width,
                                                 uint32_t height,
                                                 framebuffer_attachment_t const& depthStencil,
                                                 std::array<framebuffer_attachment_t, count> const& images,
                                                 std::array<RenderTargetOutputSemantic, count> const& semantics)
  : BaseRenderTarget(width, height, count, true)
{
    static_assert(count > 0);

    assert(images.size() % count == 0);

    auto bufferCount = images.size() / count;

    for (auto i = size_t{ 0 }; i < count; i++) {
        auto&& semantic = semantics[i];
        outputSemantics_[semantic] = i;
    }

    frameBuffers_.reserve(bufferCount);

    for (auto i = size_t{ 0 }; i < bufferCount; i++) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          width,
          height,
          getImageView(device,
                       width,
                       height,
                       depthStencil,
                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT),
          getColorAttachments(std::make_index_sequence<count>{}, device, width, height, count * i, images));
    }
}
}

#endif // CYCLONITE_FRAMEBUFFERRENDERTARGET_H
