//
// Created by bantdit on 1/7/20.
//

#ifndef CYCLONITE_BASERENDERTARGET_H
#define CYCLONITE_BASERENDERTARGET_H

#include "vulkan/frameBuffer.h"

namespace cyclonite {
enum class RenderTargetOutputSemantic
{
    UNDEFINED = 0,
    DEFAULT = 1,
    LINEAR_HDR_COLOR = 2,
    MIN_VALUE = UNDEFINED,
    MAX_VALUE = LINEAR_HDR_COLOR,
    COUNT = MAX_VALUE + 1
};

class BaseRenderTarget
{
private:
    template<typename Indices>
    struct attachment_traits;

    template<size_t... idx>
    struct attachment_traits<std::index_sequence<idx...>>
    {
        using clear_values_type_t = std::variant<std::array<VkClearValue, idx>...>;

        template<typename T>
        constexpr static auto attachment_count(T&& t) -> size_t
        {
            return std::visit([](auto&& attachments) -> size_t { return attachments.size(); }, std::forward<T>(t));
        }
    };

    using attachment_list_traits = attachment_traits<std::make_index_sequence<17u>>; // +1 depth

    using clear_value_list_t = attachment_list_traits::clear_values_type_t;

public:
    template<size_t count>
    BaseRenderTarget(uint32_t width,
                     uint32_t height,
                     VkClearDepthStencilValue clearDepthStencilValue,
                     std::array<VkClearColorValue, count> const& clearColorValues);

    template<size_t count>
    BaseRenderTarget(uint32_t width, uint32_t height, std::array<VkClearColorValue, count> const& clearColorValues);

    BaseRenderTarget(BaseRenderTarget const&) = delete;

    BaseRenderTarget(BaseRenderTarget&&) = default;

    ~BaseRenderTarget() = default;

    auto operator=(BaseRenderTarget const&) -> BaseRenderTarget& = delete;

    auto operator=(BaseRenderTarget &&) -> BaseRenderTarget& = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

    [[nodiscard]] auto frontBufferIndex() const -> size_t { return frontBufferIndex_; }

    [[nodiscard]] auto backBufferIndex() const -> size_t { return backBufferIndex_; }

    [[nodiscard]] auto swapChainLength() const -> size_t { return swapChainLength_; }

    [[nodiscard]] auto getColorAttachment(size_t attachmentIndex) const -> vulkan::ImageView const&;

    [[nodiscard]] auto getColorAttachment(RenderTargetOutputSemantic semantic) const -> vulkan::ImageView const&;

    [[nodiscard]] auto getDepthStencilAttachment() const -> vulkan::ImageView const&;

    [[nodiscard]] auto hasAttachment(RenderTargetOutputSemantic semantic) const -> bool;

    [[nodiscard]] auto hasDepthStencil() const -> bool { return hasDepthStencil_; }

    [[nodiscard]] auto frameBuffers() const -> std::vector<vulkan::FrameBuffer> const& { return frameBuffers_; }

    [[nodiscard]] auto colorAttachmentCount() const -> size_t { return colorAttachmentCount_; }

    template<size_t count>
    void getClearValues(std::array<VkClearValue, count>& output) const // :( very ugly
    {
        std::visit(
          [&](auto const& clearValues) -> void {
              if constexpr (clearValues.size() == count) {
                  std::copy(clearValues.begin(), clearValues.end(), output.begin());
                  return;
              }
              std::terminate();
          },
          clearValues_);
    }

private:
    VkExtent2D extent_;
    clear_value_list_t clearValues_;
    bool hasDepthStencil_;

protected:
    size_t colorAttachmentCount_;
    size_t swapChainLength_;
    size_t frontBufferIndex_;
    size_t backBufferIndex_;
    std::vector<vulkan::FrameBuffer> frameBuffers_;
    std::unordered_map<RenderTargetOutputSemantic, size_t> outputSemantics_;
};

template<size_t count>
BaseRenderTarget::BaseRenderTarget(uint32_t width,
                                   uint32_t height,
                                   VkClearDepthStencilValue clearDepthStencilValue,
                                   std::array<VkClearColorValue, count> const& clearColorValues)
  : extent_{}
  , clearValues_{ std::array<VkClearValue, count + 1>{} }
  , hasDepthStencil_{ true }
  , colorAttachmentCount_{ count }
  , swapChainLength_{ 0 }
  , frontBufferIndex_{ 0 }
  , backBufferIndex_{ 0 }
  , frameBuffers_{}
  , outputSemantics_{}
{
    extent_.width = width;
    extent_.height = height;

    std::visit(
      [&](auto&& clearValues) -> void {
          for (size_t i = 0; i < count; i++)
              clearValues[i].color = clearColorValues[i];
          clearValues[count].depthStencil = clearDepthStencilValue;
      },
      clearValues_);
}

template<size_t count>
BaseRenderTarget::BaseRenderTarget(uint32_t width,
                                   uint32_t height,
                                   std::array<VkClearColorValue, count> const& clearColorValues)
  : extent_{}
  , clearValues_{ std::array<VkClearValue, count>{} }
  , hasDepthStencil_{ false }
  , colorAttachmentCount_{ count }
  , swapChainLength_{ 0 }
  , frontBufferIndex_{ 0 }
  , backBufferIndex_{ 0 }
  , frameBuffers_{}
  , outputSemantics_{}
{
    extent_.width = width;
    extent_.height = height;

    std::visit(
      [&](auto&& clearValues) -> void {
          for (size_t i = 0; i < count; i++)
              clearValues[i].color = clearColorValues[i];
      },
      clearValues_);
}
}

#endif // CYCLONITE_BASERENDERTARGET_H
