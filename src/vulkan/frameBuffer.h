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
    template<typename Indices>
    struct attachment_traits;

    template<size_t... idx>
    struct attachment_traits<std::index_sequence<idx...>>
    {
        using type_t = std::variant<std::array<vulkan::ImageView, idx>...>;

        constexpr static size_t max_available_attachments_v = sizeof...(idx);

        template<typename T>
        constexpr static auto attachment_count(T&& t) -> size_t
        {
            return std::visit([](auto&& attachments) -> size_t { return attachments.size(); }, std::forward<T>(t));
        }

        template<typename T>
        constexpr static auto get_attachment(T&& t, size_t i) -> vulkan::ImageView const&
        {
            return std::visit(
              [i](auto&& attachments) -> vulkan::ImageView const& {
                  assert(i < attachments.size());
                  return attachments[i];
              },
              std::forward<T>(t));
        }
    };

    using attachment_list_traits = attachment_traits<std::make_index_sequence<32u>>;

    using attachment_list_t = attachment_list_traits::type_t;

public:
    template<size_t colorAttachmentsCount>
    FrameBuffer(vulkan::Device const& device,
                VkRenderPass vkRenderPass,
                uint32_t width,
                uint32_t height,
                vulkan::ImageView&& depthStencilAttachment,
                std::array<vulkan::ImageView, colorAttachmentsCount>&& attachments);

    template<size_t colorAttachmentsCount>
    FrameBuffer(vulkan::Device const& device,
                VkRenderPass vkRenderPass,
                uint32_t width,
                uint32_t height,
                std::array<vulkan::ImageView, colorAttachmentsCount>&& attachments);

    FrameBuffer(vulkan::Device const& device,
                VkRenderPass vkRenderPass,
                uint32_t width,
                uint32_t height,
                vulkan::ImageView&& depthStencilAttachment);

    FrameBuffer(FrameBuffer const&) = delete;

    FrameBuffer(FrameBuffer&&) = default;

    ~FrameBuffer() = default;

    auto operator=(FrameBuffer const&) -> FrameBuffer& = delete;

    auto operator=(FrameBuffer &&) -> FrameBuffer& = default;

    [[nodiscard]] auto handle() const -> VkFramebuffer { return static_cast<VkFramebuffer>(vkFrameBuffer_); }

    [[nodiscard]] auto getColorAttachment(size_t attachmentIndex) const -> ImageView const&
    {
        return attachment_list_traits::get_attachment(colorAttachments_, attachmentIndex);
    }

    [[nodiscard]] auto getDepthStencilAttachment() const -> ImageView const&
    {
        assert(hasDepthStencilAttachment());
        return *depthStencilAttachments_;
    }

    [[nodiscard]] auto colorAttachmentCount() const -> size_t
    {
        return attachment_list_traits::attachment_count(colorAttachments_);
    }

    [[nodiscard]] auto hasDepthStencilAttachment() const -> bool { return static_cast<bool>(depthStencilAttachments_); }

private:
    template<size_t colorAttachmentsCount>
    FrameBuffer(vulkan::Device const& device,
                VkRenderPass vkRenderPass,
                uint32_t width,
                uint32_t height,
                std::optional<vulkan::ImageView>&& depthStencilAttachment,
                std::array<vulkan::ImageView, colorAttachmentsCount>&& attachments);

private:
    std::optional<vulkan::ImageView> depthStencilAttachments_;
    attachment_list_t colorAttachments_;
    vulkan::Handle<VkFramebuffer> vkFrameBuffer_;
};

template<size_t colorAttachmentsCount>
FrameBuffer::FrameBuffer(vulkan::Device const& device,
                         VkRenderPass vkRenderPass,
                         uint32_t width,
                         uint32_t height,
                         std::optional<vulkan::ImageView>&& depthStencilAttachment,
                         std::array<vulkan::ImageView, colorAttachmentsCount>&& attachments)
  : depthStencilAttachments_{ std::move(depthStencilAttachment) }
  , colorAttachments_(std::move(attachments))
  , vkFrameBuffer_{ device.handle(), vkDestroyFramebuffer }
{
    assert(colorAttachmentsCount <=
           std::min(device.capabilities().maxColorAttachments, attachment_list_traits::max_available_attachments_v));

    assert(depthStencilAttachments_ || colorAttachmentsCount > 0);

    using vk_attachments_t =
      std::variant<std::array<VkImageView, colorAttachmentsCount>, std::array<VkImageView, colorAttachmentsCount + 1>>;

    vk_attachments_t vkAttachments;

    if (depthStencilAttachments_)
        vkAttachments = std::array<VkImageView, colorAttachmentsCount + 1>{};
    else
        vkAttachments = std::array<VkImageView, colorAttachmentsCount>{};

    auto framebufferInfo = VkFramebufferCreateInfo{};

    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vkRenderPass;
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1; // for now

    std::visit(
      [this, &framebufferInfo](auto& array) -> void {
          if constexpr (colorAttachmentsCount > 0) {
              for (size_t i = 0; i < colorAttachmentsCount; i++) {
                  array[i] = attachment_list_traits::get_attachment(colorAttachments_, i).handle();
              }
          }

          if (array.size() > colorAttachmentsCount) {
              array[colorAttachmentsCount] = depthStencilAttachments_->handle();
          }

          framebufferInfo.attachmentCount = static_cast<uint32_t>(array.size());
          framebufferInfo.pAttachments = array.data();
      },
      vkAttachments);

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

template<size_t colorAttachmentsCount>
FrameBuffer::FrameBuffer(vulkan::Device const& device,
                         VkRenderPass vkRenderPass,
                         uint32_t width,
                         uint32_t height,
                         vulkan::ImageView&& depthStencilAttachment,
                         std::array<vulkan::ImageView, colorAttachmentsCount>&& attachments)
  : FrameBuffer(device,
                vkRenderPass,
                width,
                height,
                std::optional<vulkan::ImageView>{ std::move(depthStencilAttachment) },
                std::move(attachments))
{}

template<size_t colorAttachmentsCount>
FrameBuffer::FrameBuffer(vulkan::Device const& device,
                         VkRenderPass vkRenderPass,
                         uint32_t width,
                         uint32_t height,
                         std::array<vulkan::ImageView, colorAttachmentsCount>&& attachments)
  : FrameBuffer(device, vkRenderPass, width, height, std::optional<vulkan::ImageView>{}, std::move(attachments))
{}
}

#endif // CYCLONITE_FRAMEBUFFER_H
