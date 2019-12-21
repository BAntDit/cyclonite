//
// Created by bantdit on 12/17/19.
//

#ifndef CYCLONITE_FRAMEBUFFER_H
#define CYCLONITE_FRAMEBUFFER_H

#include "device.h"
#include "imageView.h"
#include <easy-mp/type_list.h>

namespace cyclonite::vulkan {
class FrameBuffer
{
private:
    template<typename Args>
    static auto create_image_view(Args&& args) -> ImageView;

    template<typename Args>
    static auto create_image_views(Args&& args) -> std::vector<ImageView>;

public:
    FrameBuffer(vulkan::Device const& device,
                VkRenderPass vkRenderPass,
                uint32_t width,
                uint32_t height,
                std::vector<vulkan::ImageView>&& attachments);

    template<size_t attachmentCount, typename... ImageViewArg>
    FrameBuffer(vulkan::Device const& device,
                VkRenderPass vkRenderPass,
                uint32_t width,
                uint32_t height,
                std::array<std::tuple<ImageViewArg...>, attachmentCount>&& attachmentArgs);

    FrameBuffer(FrameBuffer const&) = delete;

    FrameBuffer(FrameBuffer&&) = default;

    ~FrameBuffer() = default;

    auto operator=(FrameBuffer const&) -> FrameBuffer& = delete;

    auto operator=(FrameBuffer &&) -> FrameBuffer& = default;

    [[nodiscard]] auto handle() const -> VkFramebuffer { return static_cast<VkFramebuffer>(vkFrameBuffer_); }

    [[nodiscard]] auto getAttachment(size_t attachmentIndex) const -> ImageView const&
    {
        assert(attachmentIndex < attachments_.size());
        return attachments_[attachmentIndex];
    }

private:
    std::vector<vulkan::ImageView> attachments_;
    vulkan::Handle<VkFramebuffer> vkFrameBuffer_;
};

template<size_t attachmentCount, typename... ImageViewArg>
FrameBuffer::FrameBuffer(vulkan::Device const& device,
                         VkRenderPass vkRenderPass,
                         uint32_t width,
                         uint32_t height,
                         std::array<std::tuple<ImageViewArg...>, attachmentCount>&& attachmentArgs)
  : FrameBuffer{ device, vkRenderPass, width, height, create_image_views(std::move(attachmentArgs)) }
{}

template<typename Args>
auto FrameBuffer::create_image_view(Args&& args) -> ImageView
{
    return std::apply(
      [](auto&&... imageViewArgs) -> ImageView {
          return ImageView{ std::forward<decltype(imageViewArgs)>(imageViewArgs)... };
      },
      std::forward<Args>(args));
}

template<typename Args>
auto FrameBuffer::create_image_views(Args&& args) -> std::vector<ImageView>
{
    return std::apply(
      [](auto&&... tupleArgs) -> std::array<ImageView, args.template size()> {
          return std::vector{ create_image_view(std::forward<decltype(tupleArgs)>(tupleArgs))... };
      },
      std::forward<Args>(args));
}
}

#endif // CYCLONITE_FRAMEBUFFER_H
