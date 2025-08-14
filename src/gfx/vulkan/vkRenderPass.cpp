//
// Created by anton on 7/26/25.
//

#include "vkRenderPass.h"
#include "gfx/resourceManager.h"
#include "gfx/texture.h"
#include "internal/utils.h"
#include "vkException.h"
#include <utility>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
namespace {
auto getAttachmentDescription(core::ResourceRef textureRef) -> VkAttachmentDescription
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
  std::array<core::ResourceRef, compile_time_config_t::max_color_attachment_count_v> const& attachmentRef,
  std::array<VkAttachmentDescription, max_attachment_count_v>& descriptions,
  std::index_sequence<I...>) -> uint32_t
{
    auto count = uint32_t{ 0 };
    ((attachmentRef[I].valid() && (descriptions[I] = getAttachmentDescription(attachmentRef[I]), ++count)) && ...);

    return count;
}

template<size_t... I>
auto writeAttachmentReferences(
  std::array<core::ResourceRef, compile_time_config_t::max_color_attachment_count_v> const& attachmentRef,
  std::array<VkAttachmentReference, compile_time_config_t::max_color_attachment_count_v>& vkAttachmentRefs,
  std::index_sequence<I...>) -> uint32_t
{
    auto count = uint32_t{ 0 };
    ((attachmentRef[I].valid() &&
      (vkAttachmentRefs[I] = VkAttachmentReference{ I, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, ++count)) &&
     ...);
    return count;
}
}

RenderPass::RenderPass(
  core::ResourceManagerBase* resourceManager,
  core::ResourceId resourceId,
  core::ResourceRef deviceRef,
  core::ResourceRef depthStencilRef,
  std::array<core::ResourceRef, compile_time_config_t::max_color_attachment_count_v> colorAttachmentRefs)
  : core::ResourceBase{ resourceManager, resourceId, true }
  , renderPass_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroyRenderPass }
{
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

    if (auto vkResult = vkCreateRenderPass(device.handle(), &renderPassCreateInfo, nullptr, &renderPass_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateRenderPass" };
    }

    // TODO:: create frame buffer
}
}
#endif
