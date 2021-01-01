//
// Created by anton on 12/20/20.
//

#include "node.h"

namespace cyclonite::compositor {
Node::Builder::Builder(vulkan::Device& device, uint32_t width, uint32_t height)
  : device_{ &device }
  , width_{ width }
  , height_{ height }
  , inputLinks_{}
  , outputLinks_{}
  , depthFormat_{ VK_FORMAT_UNDEFINED }
  , depthTiling_{ VK_IMAGE_TILING_OPTIMAL }
  , colorOutputs_{}
  , surfaceProps_{}
  , renderPasses_{}
{}

void Node::Builder::_swapChainCreationErrorHandling(VkResult result)
{
    if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
        throw std::runtime_error("not enough RAM memory to create swap chain");
    }

    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
        throw std::runtime_error("not enough GPU memory to create swap chain");
    }

    if (result == VK_ERROR_DEVICE_LOST) {
        throw std::runtime_error("device lost on attempt to create swap chain");
    }

    if (result == VK_ERROR_SURFACE_LOST_KHR) {
        throw std::runtime_error("surface lost");
    }

    if (result == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) {
        throw std::runtime_error("could not create swap chain - native window is in use");
    }

    if (result == VK_ERROR_INITIALIZATION_FAILED) {
        throw std::runtime_error("swap chain initialization failed");
    }

    assert(false);
}

auto Node::Builder::_getDepthStencilLayoutByFormat(VkFormat format) -> VkImageLayout
{
    auto layout = VkImageLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            break;
        case VK_FORMAT_S8_UINT:
            layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        default:
            layout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    return layout;
}

void Node::Builder::build()
{
    auto renderPassCreateInfo = VkRenderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    auto colorAttachmentCount = std::visit(
      [](auto&& attachments) -> size_t {
          if constexpr (!std::is_same_v<std::decay_t<decltype(attachments)>, std::monostate>) {
              return attachments.size();
          }

          return 0;
      },
      colorOutputs_);

    auto subPassDescriptions = std::vector<VkSubpassDescription>{};

    subPassDescriptions.reserve(renderPasses_.size());

    for (auto&& renderPass : renderPasses_) {
        auto&& [passType, subPassDependency, inputAttachmentIndices, colorAttachmentIndices, writeDepth] = renderPass;
        auto&& subPassDesc = VkSubpassDescription{};

        subPassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        std::visit(
          [&](auto& inputs) -> std::vector<VkAttachmentReference> {
              if constexpr (!std::is_same_v<std::decay_t<decltype(inputs)>, std::monostate>) {
                  subPassDesc.inputAttachmentCount = inputs.size();
                  subPassDesc.pInputAttachments = inputs.data();
              }
          },
          inputAttachmentIndices);

        std::visit(
          [&](auto& outputs) -> std::vector<VkAttachmentReference> {
              if constexpr (!std::is_same_v<std::decay_t<decltype(outputs)>, std::monostate>) {
                  subPassDesc.colorAttachmentCount = outputs.size();
                  subPassDesc.pColorAttachments = outputs.data();
              }
          },
          colorAttachmentIndices);

        if (writeDepth && depthFormat_ != VK_FORMAT_UNDEFINED) {
            auto depthRef = VkAttachmentReference{};
            depthRef.attachment = colorAttachmentCount; // depth always goes just after color
            depthRef.layout = _getDepthStencilLayoutByFormat(depthFormat_);

            subPassDesc.pDepthStencilAttachment = &depthRef;
        }

        subPassDescriptions.emplace_back(subPassDesc);
    } // render passes end

    auto attachmentCount = (depthFormat_ != VK_FORMAT_UNDEFINED) ? colorAttachmentCount + 1 : colorAttachmentCount;

    assert(attachmentCount > 0);

    auto attachments = std::vector<VkAttachmentDescription>{};
    attachments.reserve(attachmentCount);

    auto isSurfacePass = surfaceProps_.has_value();

    // just for now - no multisampling, no stencil ops
    std::visit(
      [&](auto&& outputs) -> void {
          if constexpr (!std::is_same_v<std::decay_t<decltype(outputs)>, std::monostate>) {
              for (auto&& [format, tiling, semantic] : outputs) {
                  (void)semantic;
                  (void)tiling;

                  auto attachment = VkAttachmentDescription{};

                  attachment.format = format;
                  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                  attachment.finalLayout =
                    isSurfacePass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                  attachments.emplace_back(attachment);
              }
          }
      },
      colorOutputs_);

    if (depthFormat_ != VK_FORMAT_UNDEFINED) {
        auto attachment = VkAttachmentDescription{};

        attachment.format = depthFormat_;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = _getDepthStencilLayoutByFormat(depthFormat_);

        attachments.emplace_back(attachment);
    }

    // TODO:: prepare subpass dependencies
}
}