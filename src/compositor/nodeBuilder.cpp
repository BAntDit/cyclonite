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

auto Node::Builder::addPass(PassType passType,
                            input_attachment_indices_t<32> inputAttachmentIndices,
                            uint32_t srcPassIndex,
                            uint32_t dstPassIndex,
                            VkPipelineStageFlags srcStageMask,
                            VkPipelineStageFlags dstStageMask,
                            VkAccessFlags srcAccessMask,
                            VkAccessFlags dstAccessMask,
                            VkDependencyFlags dependencyFlags) -> Node::Builder&
{
    auto&& subPassDependency = VkSubpassDependency{};

    subPassDependency.srcSubpass = srcPassIndex;
    subPassDependency.dstSubpass = dstPassIndex;
    subPassDependency.dstStageMask = srcStageMask;
    subPassDependency.dstStageMask = dstStageMask;
    subPassDependency.srcAccessMask = srcAccessMask;
    subPassDependency.dstAccessMask = dstAccessMask;
    subPassDependency.dependencyFlags = dependencyFlags;

    renderPasses_.emplace_back(std::make_tuple(passType, subPassDependency, inputAttachmentIndices));

    return *this;
}

auto Node::Builder::addPass(PassType passType,
                            uint32_t srcPassIndex,
                            uint32_t dstPassIndex,
                            VkPipelineStageFlags srcStageMask,
                            VkPipelineStageFlags dstStageMask,
                            VkAccessFlags srcAccessMask,
                            VkAccessFlags dstAccessMask,
                            VkDependencyFlags dependencyFlags) -> Node::Builder&
{
    return addPass(passType,
                   std::monostate{},
                   srcPassIndex,
                   dstPassIndex,
                   srcStageMask,
                   dstStageMask,
                   srcAccessMask,
                   dstAccessMask,
                   dependencyFlags);
}

auto Node::Builder::addPass(PassType passType,
                            uint32_t dstPassIndex,
                            VkPipelineStageFlags srcStageMask,
                            VkPipelineStageFlags dstStageMask,
                            VkAccessFlags srcAccessMask,
                            VkAccessFlags dstAccessMask,
                            VkDependencyFlags dependencyFlags) -> Node::Builder&
{
    return addPass(passType,
                   std::monostate{},
                   VK_SUBPASS_EXTERNAL,
                   dstPassIndex,
                   srcStageMask,
                   dstStageMask,
                   srcAccessMask,
                   dstAccessMask,
                   dependencyFlags);
}

void Node::Builder::build()
{
    auto renderPassCreateInfo = VkRenderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    auto subPassDescriptions = std::vector<VkSubpassDescription>{};

    subPassDescriptions.reserve(renderPasses_.size());

    for (auto&& renderPass : renderPasses_) {
        auto&& [passType, subPassDependency, inputAttachmentIndices] = renderPass;
        auto&& subPassDesc = VkSubpassDescription{};

        subPassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        std::visit(
          [&](auto&& inputs) -> void {
              if constexpr (!std::is_same_v<std::decay_t<decltype(inputs)>, std::monostate>) {
                  auto refs = std::vector<VkAttachmentReference>{};
                  refs.reserve(inputs.size());

                  for (auto i : inputs) {
                      auto&& ref = VkAttachmentReference{};
                      ref.attachment = i;
                      ref.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                      refs.emplace_back(ref);
                  }

                  subPassDesc.inputAttachmentCount = refs.size();
                  subPassDesc.pInputAttachments = refs.data();
              }
          },
          inputAttachmentIndices);

        auto colorAttachmentCount = std::visit(
          [&](auto&& outputs) -> size_t {
              if constexpr (!std::is_same_v<std::decay_t<decltype(outputs)>, std::monostate>) {
                  auto refs = std::vector<VkAttachmentReference>{};
                  refs.reserve(outputs.size());

                  for (auto i = size_t{ 0 }, count = outputs.size(); i < count; i++) {
                      auto&& ref = VkAttachmentReference{};

                      ref.attachment = i;
                      ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                      refs.emplace_back(ref);
                  }

                  subPassDesc.colorAttachmentCount = refs.size();
                  subPassDesc.pColorAttachments = refs.data();

                  return refs.size();
              }

              return 0;
          },
          colorOutputs_);

        if (depthFormat_ != VK_FORMAT_UNDEFINED) {
            auto depthRef = VkAttachmentReference{};
            depthRef.attachment = colorAttachmentCount;                         // depth always goes just after color
            depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO:: layout by format

            subPassDesc.pDepthStencilAttachment = &depthRef;
        }

        subPassDescriptions.emplace_back(subPassDesc);
    }
}
}