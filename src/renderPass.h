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
    template<typename DepthStencilOutput, typename ColorOutput, size_t presentModeCandidateCount>
    RenderPass(
      vulkan::Device& device,
      Options::WindowProperties const& windowProperties,
      DepthStencilOutput&& = render_target_output<type_list<render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>{},
      ColorOutput&& = render_target_output<
        type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
        RenderTargetOutputSemantic::DEFAULT>{},
      VkClearDepthStencilValue const& clearDepthStencilValue = { 1.0f, 0 },
      VkClearColorValue const& clearColorValue = { { 0.0f, 0.0f, 0.0f, 1.0f } },
      std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates =
        { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR },
      VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

    template<typename ColorOutput, size_t presentModeCandidateCount>
    RenderPass(vulkan::Device& device,
               Options::WindowProperties const& windowProperties,
               ColorOutput&& colorOutput = render_target_output<
                 type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
                 RenderTargetOutputSemantic::DEFAULT>{},
               VkClearColorValue const& clearColorValue = { { 0.0f, 0.0f, 0.0f, 1.0f } },
               std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates =
                 { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR },
               VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

    template<typename DepthStencilOutput, typename... ColorOutputs>
    RenderPass(
      vulkan::Device& device,
      uint32_t outputWidth,
      uint32_t outputHeight,
      bool doubleBuffering = false,
      DepthStencilOutput&& = render_target_output<type_list<render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>{},
      std::tuple<ColorOutputs...>&& = std::tuple{ render_target_output<
        type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
        RenderTargetOutputSemantic::DEFAULT>{} },
      VkClearDepthStencilValue const& clearDepthStencilValue = { 1.0f, 0 },
      std::array<VkClearColorValue, sizeof...(ColorOutputs)> const& clearColorValues = {
        VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } } });

    // TODO::
    // RenderPass(vulkan::Device& device);

    RenderPass(RenderPass const&) = delete;

    RenderPass(RenderPass&&) = default;

    ~RenderPass() = default;

    auto operator=(RenderPass const&) -> RenderPass& = delete;

    auto operator=(RenderPass &&) -> RenderPass& = default;

    auto renderQueueSubmitInfo() -> VkSubmitInfo const&;

    auto begin(vulkan::Device const& device) -> VkFence;

    void end(vulkan::Device const& device);

private:
    void _createRenderPass(vulkan::Device const& device, VkRenderPassCreateInfo const& renderPassCreateInfo);

    void _createSyncObjects(vulkan::Device const& device, size_t swapChainLength);

    void _createDummyPipeline(vulkan::Device const& device,
                              uint32_t renderTargetWidth,
                              uint32_t renderTargetHeight,
                              bool hasDepthStencil,
                              VkPipelineColorBlendStateCreateInfo const& colorBlendState);

    void _createDummyCommandPool(vulkan::Device const& device);

    void _createDummyCommandBuffers(vulkan::Device const& device, size_t swapChainLength);

private:
    vulkan::Handle<VkRenderPass> vkRenderPass_;
    std::variant<std::monostate, SurfaceRenderTarget, FrameBufferRenderTarget> renderTarget_;
    std::vector<vulkan::Handle<VkSemaphore>> passFinishedSemaphores_;
    std::vector<vulkan::Handle<VkFence>> frameFences_;
    std::vector<VkFence> renderTargetFences_;
    VkPipelineStageFlags waitStage_;
    VkSemaphore vkWaitSemaphore_;
    VkSemaphore vkSignalSemaphore_;
    VkSubmitInfo renderQueueSubmitInfo_;

    // tmp...
    // TODO:: combine them together into vulkan::Pipeline type
    vulkan::Handle<VkPipelineLayout> vkDummyPipelineLayout_;
    vulkan::Handle<VkPipeline> vkDummyPipeline_;

    // tmp:: let it be here just for now
    vulkan::Handle<VkCommandPool> vkCommandPool_;

    // tmp::
    std::vector<VkCommandBuffer> commandBuffers_;
};

template<typename DepthStencilOutput, typename... ColorOutputs>
RenderPass::RenderPass(vulkan::Device& device,
                       uint32_t outputWidth,
                       uint32_t outputHeight,
                       bool doubleBuffering,
                       DepthStencilOutput&&,
                       std::tuple<ColorOutputs...>&&,
                       VkClearDepthStencilValue const& clearDepthStencilValue,
                       std::array<VkClearColorValue, sizeof...(ColorOutputs)> const& clearColorValues)
  : vkRenderPass_{ device.handle(), vkDestroyRenderPass }
  , renderTarget_{}
  , passFinishedSemaphores_{}
  , frameFences_{}
  , renderTargetFences_{}
  , waitStage_{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }
  , vkWaitSemaphore_{ VK_NULL_HANDLE }
  , vkSignalSemaphore_{ VK_NULL_HANDLE }
  , renderQueueSubmitInfo_{}
  , vkDummyPipelineLayout_{ device.handle(), vkDestroyPipelineLayout }
  , vkDummyPipeline_{ device.handle(), vkDestroyPipeline }
  , vkCommandPool_{ device.handle(), vkDestroyCommandPool }
  , commandBuffers_{}
{
    using render_target_builder_t = FrameBufferRenderTargetBuilder<DepthStencilOutput, ColorOutputs...>;

    constexpr size_t color_attachment_count_v = render_target_builder_t::color_attachment_count_v;

    constexpr size_t depth_attachment_idx_v = render_target_builder_t::depth_attachment_idx_v;

    constexpr bool has_depth_stencil_attachment_v = !DepthStencilOutput::is_empty_v;

    uint32_t swapChainLength = doubleBuffering ? 2 : 1;

    render_target_builder_t renderTargetBuilder{ device,       swapChainLength,  outputWidth,
                                                 outputHeight, clearColorValues, clearDepthStencilValue };

    auto&& [attachments, references] = renderTargetBuilder.getAttachments();

    VkSubpassDescription subPassDescription = {};

    subPassDescription.colorAttachmentCount = color_attachment_count_v;
    subPassDescription.pColorAttachments = references.data();

    if constexpr (has_depth_stencil_attachment_v) {
        subPassDescription.pDepthStencilAttachment = &references[depth_attachment_idx_v];
    }

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
    renderPassInfo.pSubpasses = &subPassDescription;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    _createRenderPass(device, renderPassInfo);

    auto&& renderTarget = renderTargetBuilder.buildRenderPassTarget(static_cast<VkRenderPass>(vkRenderPass_));

    // TODO:: complete this constructor
    // ... sync object creation
    // ... pipeline creation
    // ... command buffers creation

    renderTarget_ = std::move(renderTarget);
}

template<typename ColorOutput, size_t presentModeCandidateCount>
RenderPass::RenderPass(vulkan::Device& device,
                       Options::WindowProperties const& windowProperties,
                       ColorOutput&& colorOutput,
                       VkClearColorValue const& clearColorValue,
                       std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates,
                       VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags)
  : RenderPass{ device,
                windowProperties,
                render_target_output<type_list<>>{},
                std::move(colorOutput),
                VkClearDepthStencilValue{ 1.0f, 0 },
                clearColorValue,
                presentModeCandidates,
                vkCompositeAlphaFlags }
{}

template<typename DepthStencilOutput, typename ColorOutput, size_t presentModeCandidateCount>
RenderPass::RenderPass(vulkan::Device& device,
                       Options::WindowProperties const& windowProperties,
                       DepthStencilOutput&&,
                       ColorOutput&&,
                       VkClearDepthStencilValue const& clearDepthStencilValue,
                       VkClearColorValue const& clearColorValue,
                       std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates,
                       VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags)
  : vkRenderPass_{ device.handle(), vkDestroyRenderPass }
  , renderTarget_{}
  , passFinishedSemaphores_{}
  , frameFences_{}
  , renderTargetFences_{}
  , waitStage_{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }
  , vkWaitSemaphore_{ VK_NULL_HANDLE }
  , vkSignalSemaphore_{ VK_NULL_HANDLE }
  , renderQueueSubmitInfo_{}
  , vkDummyPipelineLayout_{ device.handle(), vkDestroyPipelineLayout }
  , vkDummyPipeline_{ device.handle(), vkDestroyPipeline }
  , vkCommandPool_{ device.handle(), vkDestroyCommandPool }
  , commandBuffers_{}
{
    using render_target_builder_t = SurfaceRenderTargetBuilder<DepthStencilOutput, ColorOutput>;

    constexpr size_t color_attachment_count_v = render_target_builder_t::color_attachment_count_v;

    constexpr size_t depth_attachment_idx_v = render_target_builder_t::depth_attachment_idx_v;

    constexpr bool has_depth_stencil_attachment_v = !DepthStencilOutput::is_empty_v;

    render_target_builder_t renderTargetBuilder{ device,
                                                 Surface{ device, windowProperties },
                                                 clearColorValue,
                                                 clearDepthStencilValue,
                                                 presentModeCandidates,
                                                 vkCompositeAlphaFlags };

    auto&& [attachments, references] = renderTargetBuilder.getAttachments();

    VkSubpassDescription subPassDescription = {};

    subPassDescription.colorAttachmentCount = color_attachment_count_v;
    subPassDescription.pColorAttachments = references.data();

    if constexpr (has_depth_stencil_attachment_v) {
        subPassDescription.pDepthStencilAttachment = &references[depth_attachment_idx_v];
    }

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
    renderPassInfo.pSubpasses = &subPassDescription;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    _createRenderPass(device, renderPassInfo);

    auto&& renderTarget = renderTargetBuilder.buildRenderPassTarget(static_cast<VkRenderPass>(vkRenderPass_));

    _createSyncObjects(device, renderTarget.swapChainLength());

    // tmp for dummy pipeline
    VkPipelineColorBlendStateCreateInfo colorBlendState = {};

    std::array<VkPipelineColorBlendAttachmentState, color_attachment_count_v> colorBlendAttachmentStates = {};

    for (size_t i = 0; i < color_attachment_count_v; i++) {
        colorBlendAttachmentStates[i] =
          VkPipelineColorBlendAttachmentState{ VK_FALSE,
                                               VK_BLEND_FACTOR_ONE,
                                               VK_BLEND_FACTOR_ZERO,
                                               VK_BLEND_OP_ADD,
                                               VK_BLEND_FACTOR_ONE,
                                               VK_BLEND_FACTOR_ZERO,
                                               VK_BLEND_OP_ADD,
                                               VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
    }

    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = colorBlendAttachmentStates.size();
    colorBlendState.pAttachments = colorBlendAttachmentStates.data();
    colorBlendState.blendConstants[0] = 0.0f;
    colorBlendState.blendConstants[1] = 0.0f;
    colorBlendState.blendConstants[2] = 0.0f;
    colorBlendState.blendConstants[3] = 0.0f;

    _createDummyPipeline(
      device, renderTarget.width(), renderTarget.height(), has_depth_stencil_attachment_v, colorBlendState);

    _createDummyCommandPool(device);

    _createDummyCommandBuffers(device, renderTarget.swapChainLength());

    auto clearValues = renderTargetBuilder.getClearValues();

    auto&& frameBuffers = renderTarget.frameBuffers();

    // TODO:: create frame patch
    for (size_t i = 0, count = commandBuffers_.size(); i < count; i++) {
        auto commandBuffer = commandBuffers_[i];

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) {
            throw std::runtime_error("could not begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = static_cast<VkRenderPass>(vkRenderPass_);
        renderPassBeginInfo.framebuffer = frameBuffers[i].handle();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = renderTarget.width();
        renderPassBeginInfo.renderArea.extent.height = renderTarget.height();
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,static_cast<VkPipeline>(vkDummyPipeline_));

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
            throw std::runtime_error("could not record command buffer!");
        }
    }

    renderTarget_ = std::move(renderTarget);
}
}

#endif // CYCLONITE_RENDERPASS_H
