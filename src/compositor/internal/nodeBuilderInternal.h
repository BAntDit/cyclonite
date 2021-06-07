//
// Created by anton on 1/4/21.
//

#ifndef CYCLONITE_NODEBUILDERINTERNAL_H

#include "../../frameBufferRenderTarget.h"

namespace cyclonite::compositor::internal {
template<size_t... idx>
static auto _get_output_images(
  std::index_sequence<idx...>,
  std::array<std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>, sizeof...(idx)> const& properties)
  -> std::array<std::variant<vulkan::ImagePtr, VkFormat>, sizeof...(idx)>
{
    return std::array<std::variant<vulkan::ImagePtr, VkFormat>, sizeof...(idx)>{
        std::variant<vulkan::ImagePtr, VkFormat>{ std::get<0>(properties[idx]) }...
    };
}

template<size_t... idx>
static auto _get_output_semantics(
  std::index_sequence<idx...>,
  std::array<std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>, sizeof...(idx)> const& properties)
  -> std::array<RenderTargetOutputSemantic, sizeof...(idx)>
{
    return std::array<RenderTargetOutputSemantic, sizeof...(idx)>{ std::get<RenderTargetOutputSemantic>(
      properties[idx])... };
}

template<size_t count>
static auto _createFrameBufferRT(
  vulkan::Device& device,
  VkRenderPass vkRenderPass,
  uint32_t width,
  uint32_t height,
  VkFormat depthStencilFormat,
  std::array<std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>, count> const& properties)
  -> FrameBufferRenderTarget
{
    return (depthStencilFormat != VK_FORMAT_UNDEFINED)
             ? FrameBufferRenderTarget{ device,
                                        vkRenderPass,
                                        width,
                                        height,
                                        depthStencilFormat,
                                        _get_output_images(std::make_index_sequence<count>{}, properties),
                                        _get_output_semantics(std::make_index_sequence<count>{}, properties) }
             : FrameBufferRenderTarget{ device,
                                        vkRenderPass,
                                        width,
                                        height,
                                        _get_output_images(std::make_index_sequence<count>{}, properties),
                                        _get_output_semantics(std::make_index_sequence<count>{}, properties) };
}
}
#define CYCLONITE_NODEBUILDERINTERNAL_H

#endif // CYCLONITE_NODEBUILDERINTERNAL_H
