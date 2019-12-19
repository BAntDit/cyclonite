//
// Created by bantdit on 12/17/19.
//

#ifndef CYCLONITE_FRAMEBUFFER_H
#define CYCLONITE_FRAMEBUFFER_H

#include "device.h"
#include "imageView.h"
#include <easy-mp/type_list.h>

namespace cyclonite::vulkan {
template<size_t attachmentCount>
class FrameBuffer
{
private:
    template<typename Args>
    static auto create_image_view(Args&& args) -> ImageView;

    template<typename Args>
    static auto create_image_views(Args&& args) -> std::array<ImageView, args.template size()>;

public:
    FrameBuffer(vulkan::Device const& device,
                VkRenderPass vkRenderPass,
                uint32_t width,
                uint32_t height,
                std::array<vulkan::ImageView, attachmentCount>&& attachments);

    template<typename... ImageViewArg>
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
        assert(attachmentIndex < attachmentCount);
        return attachments_[attachmentIndex];
    }

private:
    std::array<vulkan::ImageView, attachmentCount> attachments_;
    vulkan::Handle<VkFramebuffer> vkFrameBuffer_;
};

template<size_t attachmentCount>
FrameBuffer<attachmentCount>::FrameBuffer(vulkan::Device const& device,
                                          VkRenderPass vkRenderPass,
                                          uint32_t width,
                                          uint32_t height,
                                          std::array<vulkan::ImageView, attachmentCount>&& attachments)
  : attachments_{ std::move(attachments) }
  , vkFrameBuffer_{ device.handle(), vkDestroyFramebuffer }
{
    VkFramebufferCreateInfo framebufferInfo = {};

    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vkRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments_.size());
    framebufferInfo.pAttachments = attachments_.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1; // for now

    if (auto result = vkCreateFramebuffer(device.handle(), &framebufferInfo, nullptr, &vkFrameBuffer_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("failed to create framebuffer: out of host memory");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("failed to create framebuffer: out of device memory");
        }

        assert(false);
    }
}

template<size_t attachmentCount>
template<typename... ImageViewArg>
FrameBuffer<attachmentCount>::FrameBuffer(vulkan::Device const& device,
                                          VkRenderPass vkRenderPass,
                                          uint32_t width,
                                          uint32_t height,
                                          std::array<std::tuple<ImageViewArg...>, attachmentCount>&& attachmentArgs)
  : FrameBuffer{ device, vkRenderPass, width, height, create_image_views(std::move(attachmentArgs)) }
{}

template<size_t attachmentCount>
template<typename Args>
auto FrameBuffer<attachmentCount>::create_image_view(Args&& args) -> ImageView
{
    return std::apply(
      [](auto&&... imageViewArgs) -> ImageView {
          return ImageView{ std::forward<decltype(imageViewArgs)>(imageViewArgs)... };
      },
      std::forward<Args>(args));
}

template<size_t attachmentCount>
template<typename Args>
auto FrameBuffer<attachmentCount>::create_image_views(Args&& args) -> std::array<ImageView, args.template size()>
{
    return std::apply(
      [](auto&&... tupleArgs) -> std::array<ImageView, args.template size()> {
          return std::array{ create_image_view(std::forward<decltype(tupleArgs)>(tupleArgs))... };
      },
      std::forward<Args>(args));
}
}

#endif // CYCLONITE_FRAMEBUFFER_H
