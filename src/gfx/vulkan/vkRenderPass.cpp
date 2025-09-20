//
// Created by anton on 7/26/25.
//

#include "vkRenderPass.h"
#include "gfx/resourceManager.h"
#include "internal/utils.h"
#include "vkException.h"
#include "vkRenderTargetView.h"
#include <utility>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
namespace {
auto getAttachmentDescription(core::ResourceSharedRef textureRef) -> VkAttachmentDescription
{
    auto attachmentDesc = VkAttachmentDescription{};
    if (textureRef.valid()) {
        auto& t = textureRef.as<type_traits::platform_implementation_t<gfx::Texture>>();

        auto containsStencil = isStencilFormat(t.format());

        attachmentDesc.format = internal::getFormat(t.format());
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.stencilStoreOp =
          containsStencil ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc.initialLayout = internal::getImageLayout(t.currentState());
        attachmentDesc.finalLayout = isColorFormat(t.format()) ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                                               : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    return attachmentDesc;
}

constexpr auto max_attachment_count_v = compile_time_config_t::max_color_attachment_count_v + 1; // +1 depth

template<size_t... I>
auto writeAttachmentDescriptions(
  std::array<core::ResourceSharedRef, compile_time_config_t::max_color_attachment_count_v> const& attachmentRef,
  std::array<VkAttachmentDescription, max_attachment_count_v>& descriptions,
  std::index_sequence<I...>) -> uint32_t
{
    auto count = uint32_t{ 0 };
    ((attachmentRef[I].valid() && (descriptions[I] = getAttachmentDescription(attachmentRef[I]), ++count)) && ...);

    return count;
}

template<size_t... I>
auto writeAttachmentReferences(
  std::array<core::ResourceSharedRef, compile_time_config_t::max_color_attachment_count_v> const& attachmentRef,
  std::array<VkAttachmentReference, compile_time_config_t::max_color_attachment_count_v>& vkAttachmentRefs,
  std::index_sequence<I...>) -> uint32_t
{
    auto count = uint32_t{ 0 };
    ((attachmentRef[I].valid() &&
      (vkAttachmentRefs[I] = VkAttachmentReference{ I, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, ++count)) &&
     ...);
    return count;
}

auto getRTV(core::ResourceSharedRef& attachmentRef, uint16_t mipLevel) -> VkImageView
{
    assert(attachmentRef.valid());

    auto& texture = attachmentRef.as<gfx::Texture>();
    auto& rtv = texture.getRTV(mipLevel).as<gfx::vulkan::RenderTargetView>();

    return rtv.handle();
}

template<size_t... I>
auto writeRTVs(
  std::array<VkImageView, max_attachment_count_v>& attachments,
  std::array<core::ResourceSharedRef, compile_time_config_t::max_color_attachment_count_v>& colorAttachmentRefs,
  std::array<std::pair<uint16_t, uint16_t>, compile_time_config_t::max_color_attachment_count_v> const&
    colorAttachmentSubresDescs,
  std::index_sequence<I...>) -> uint32_t
{
    auto count = uint32_t{ 0 };
    ((colorAttachmentRefs[I].valid() &&
      (attachments[I] = getRTV(colorAttachmentRefs[I], colorAttachmentSubresDescs[I].first), ++count)) &&
     ...);
    return count;
}
}

RenderPass::RenderPass(
  core::ResourceManagerBase* resourceManager,
  core::ResourceId resourceId,
  core::ResourceSharedRef deviceRef,
  core::ResourceSharedRef depthStencilRef,
  std::array<core::ResourceSharedRef, compile_time_config_t::max_color_attachment_count_v> colorAttachmentRefs,
  std::array<std::pair<uint16_t, uint16_t>, compile_time_config_t::max_color_attachment_count_v>
    colorAttachmentSubresDescs,
  uint32_t width,
  uint32_t height)
  : core::ResourceBase{ resourceManager, resourceId, true }
  , deviceRef_{ deviceRef }
  , renderTargets_{ std::make_pair(depthStencilRef, colorAttachmentRefs) }
  , vkRenderPass_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroyRenderPass }
  , vkFrameBuffers_{}
  , bufferCount_{ 1 }
  , currentBufferIndex_{ 0 }
{
    assert(deviceRef_.valid());

    vkFrameBuffers_[0] =
      Handle<VkFramebuffer>{ deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>().handle(),
                             vkDestroyFramebuffer };

    auto attachmentDescriptions = std::array<VkAttachmentDescription, max_attachment_count_v>{}; // +1 for depth
    auto colorAttachmentReferences =
      std::array<VkAttachmentReference, compile_time_config_t::max_color_attachment_count_v>{};

    auto attachmentDescCount =
      writeAttachmentDescriptions(colorAttachmentRefs,
                                  attachmentDescriptions,
                                  std::make_index_sequence<compile_time_config_t::max_color_attachment_count_v>{});

    auto colorAttachmentCount =
      writeAttachmentReferences(colorAttachmentRefs,
                                colorAttachmentReferences,
                                std::make_index_sequence<compile_time_config_t::max_color_attachment_count_v>{});
    assert(attachmentDescCount == colorAttachmentCount);

    auto depthAttachmentIndex = std::numeric_limits<uint32_t>::max();
    auto depthAttachmentReference = VkAttachmentReference{};
    if (depthStencilRef.valid()) {
        attachmentDescriptions[depthAttachmentIndex = attachmentDescCount++] =
          getAttachmentDescription(depthStencilRef);

        depthAttachmentReference.attachment = depthAttachmentIndex;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    auto subpassDescription = VkSubpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = colorAttachmentCount;
    subpassDescription.pColorAttachments = colorAttachmentReferences.data();
    if (depthAttachmentIndex != std::numeric_limits<uint32_t>::max()) {
        subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
    }

    auto renderPassCreateInfo = VkRenderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = attachmentDescCount;
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;

    auto& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();

    if (auto vkResult = vkCreateRenderPass(device.handle(), &renderPassCreateInfo, nullptr, &vkRenderPass_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateRenderPass" };
    }

    auto rtvCount = uint32_t{ 0 };
    auto attachments = std::array<VkImageView, max_attachment_count_v>{};

    rtvCount = writeRTVs(attachments,
                         colorAttachmentRefs,
                         colorAttachmentSubresDescs,
                         std::make_index_sequence<compile_time_config_t::max_color_attachment_count_v>{});

    if (depthStencilRef.valid()) {
        auto& dsTex = depthStencilRef.as<gfx::Texture>();
        auto& dsv = dsTex.getRTV(0).as<gfx::vulkan::RenderTargetView>();

        attachments[rtvCount++] = dsv.handle();
    }

    vkFrameBuffers_[0] = Handle<VkFramebuffer>{ device.handle(), vkDestroyFramebuffer };

    auto frameBufferCreateInfo = VkFramebufferCreateInfo{};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.renderPass = static_cast<VkRenderPass>(vkRenderPass_);
    frameBufferCreateInfo.attachmentCount = rtvCount;
    frameBufferCreateInfo.pAttachments = attachments.data();
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    if (auto vkResult = vkCreateFramebuffer(device.handle(), &frameBufferCreateInfo, nullptr, &vkFrameBuffers_[0]);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateFramebuffer" };
    }
}

RenderPass::RenderPass(core::ResourceManagerBase* resourceManager,
                       core::ResourceId resourceId,
                       core::ResourceSharedRef deviceRef,
                       core::ResourceSharedRef renderWindowRef)
  : core::ResourceBase{ resourceManager, resourceId, true }
  , deviceRef_{ deviceRef }
  , renderTargets_{ renderWindowRef }
  , vkRenderPass_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroyRenderPass }
  , vkFrameBuffers_{}
  , bufferCount_{ renderWindowRef.as<type_traits::platform_implementation_t<gfx::RenderWindow>>().swapchainLength() }
  , currentBufferIndex_{ 0 }
{
    assert(deviceRef_.valid());

    auto& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();
    auto& renderWindow = renderWindowRef.as<type_traits::platform_implementation_t<gfx::RenderWindow>>();

    auto attachmentDescCount = uint32_t{ 1 };
    auto attachmentDescs = std::array{ VkAttachmentDescription{}, VkAttachmentDescription{} };

    attachmentDescs[0].format = vulkan::internal::getFormat(renderWindow.colorOutputFormat());
    attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    if (renderWindow.hasDepth()) {
        attachmentDescCount++;

        attachmentDescs[1].format = vulkan::internal::getFormat(renderWindow.depthStencilFormat());
        attachmentDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescs[1].stencilStoreOp = isStencilFormat(renderWindow.depthStencilFormat())
                                              ? VK_ATTACHMENT_STORE_OP_STORE
                                              : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescs[1].finalLayout =
          !isStencilFormat(renderWindow.depthStencilFormat())      ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
          : isStencilOnlyFormat(renderWindow.depthStencilFormat()) ? VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
                                                                   : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    auto colorAttachmentCount = uint32_t{ 1 };
    auto colorAttachmentReference = VkAttachmentReference{};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto subpassDescription = VkSubpassDescription{};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = colorAttachmentCount;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    auto depthStencilAttachmentReference = VkAttachmentReference{};
    if (renderWindow.hasDepth()) {
        depthStencilAttachmentReference.attachment = 1;
        depthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subpassDescription.pDepthStencilAttachment = &depthStencilAttachmentReference;
    }

    auto renderPassCreateInfo = VkRenderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = attachmentDescCount;
    renderPassCreateInfo.pAttachments = attachmentDescs.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;

    if (auto vkResult = vkCreateRenderPass(device.handle(), &renderPassCreateInfo, nullptr, &vkRenderPass_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateRenderPass" };
    }

    for (auto i = uint32_t{ 0 }, count = renderWindow.swapchainLength(); i < count; i++) {
        auto attachmentCount = renderWindow.hasDepth() ? uint32_t{ 2 } : uint32_t{ 1 };
        auto attachments = std::array{ renderWindow.getImageView(i), renderWindow.getDSV(i) };

        vkFrameBuffers_[i] = Handle<VkFramebuffer>{ device.handle(), vkDestroyFramebuffer };

        auto frameBufferCreateInfo = VkFramebufferCreateInfo{};
        frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfo.renderPass = static_cast<VkRenderPass>(vkRenderPass_);
        frameBufferCreateInfo.attachmentCount = attachmentCount;
        frameBufferCreateInfo.pAttachments = attachments.data();
        frameBufferCreateInfo.width = renderWindow.width();
        frameBufferCreateInfo.height = renderWindow.height();
        frameBufferCreateInfo.layers = 1;

        if (auto vkResult = vkCreateFramebuffer(device.handle(), &frameBufferCreateInfo, nullptr, &vkFrameBuffers_[i]);
            vkResult != VK_SUCCESS) {
            throw Exception{ vkResult, "vkCreateFramebuffer" };
        }
    }
}
}
#endif
