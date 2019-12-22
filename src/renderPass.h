//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERPASS_H
#define CYCLONITE_RENDERPASS_H

#include "options.h"
#include "renderTargetBuilder.h"
#include "surface.h"
#include <optional>

namespace cyclonite {
class RenderPass
{
public:
    template<size_t presentModeCandidateCount,
             typename... DepthStencilOutputCandidates,
             typename... ColorOutputCandidates>
    RenderPass(vulkan::Device const& device,
               Options::WindowProperties const& windowProperties,
               VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
               std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates =
                 { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR },
               render_target_output<type_list<DepthStencilOutputCandidates...>> const& =
                 render_target_output<type_list<render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>{},
               render_target_output<type_list<ColorOutputCandidates...>> const& = render_target_output<
                 type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
                 RenderTargetOutputSemantic::DEFAULT>{});

    [[nodiscard]] auto renderQueueSubmitInfo() const -> std::vector<VkSubmitInfo> const&
    {
        return renderQueueSubmitInfo_;
    }

    auto begin() -> std::tuple<VkFence>;

private:
    vulkan::Handle<VkRenderPass> vkRenderPass_;
    std::vector<vulkan::Handle<VkFence>> frameSyncFences_;
    std::vector<VkSubmitInfo> renderQueueSubmitInfo_;
    uint64_t frameNumber_;
};

template<size_t presentModeCandidateCount, typename... DepthStencilOutputCandidates, typename... ColorOutputCandidates>
RenderPass::RenderPass(vulkan::Device const& device,
                       Options::WindowProperties const& windowProperties,
                       VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags,
                       std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates,
                       render_target_output<type_list<DepthStencilOutputCandidates...>> const&,
                       render_target_output<type_list<ColorOutputCandidates...>> const&)
  : vkRenderPass_{ device.handle(), vkDestroyRenderPass }
{
    RenderTargetBuilder<render_target_output<type_list<DepthStencilOutputCandidates...>>,
                        render_target_output<type_list<ColorOutputCandidates...>>>
      rtBuilder{ device, Surface{ device, windowProperties }, presentModeCandidates, vkCompositeAlphaFlags };

    auto [attachments, references] = rtBuilder.getAttachments();

    VkSubpassDescription subPass = {};
    // TODO:: ...
    // subPass.colorAttachmentCount = rtBuilder;
    subPass.pColorAttachments = references.data();

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subPass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
}
}

#endif // CYCLONITE_RENDERPASS_H
