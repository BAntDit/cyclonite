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
    RenderPass(vulkan::Device& device,
               Options::WindowProperties const& windowProperties,
               VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
               std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates =
                 { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR },
               render_target_output<type_list<DepthStencilOutputCandidates...>> const& =
                 render_target_output<type_list<render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>{},
               render_target_output<type_list<ColorOutputCandidates...>> const& = render_target_output<
                 type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
                 RenderTargetOutputSemantic::DEFAULT>{});

    RenderPass(RenderPass const&) = delete;

    RenderPass(RenderPass&&) = default;

    ~RenderPass() = default;

    auto operator=(RenderPass const&) -> RenderPass& = delete;

    auto operator=(RenderPass &&) -> RenderPass& = default;

    auto renderQueueSubmitInfo() -> VkSubmitInfo const&;

    auto begin(vulkan::Device const& device) -> VkFence;

    void end(vulkan::Device const& device);

private:
    void _createDummyPipeline(vulkan::Device const& device);

    void _createDummyCommandPool(vulkan::Device const& device);

private:
    vulkan::Handle<VkRenderPass> vkRenderPass_;
    std::unique_ptr<RenderTarget> renderTarget_;
    std::vector<vulkan::Handle<VkFence>> frameFences_;
    std::vector<VkFence> renderTargetFences_;
    std::vector<VkSemaphore> passFinishedSemaphores_;
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

template<size_t presentModeCandidateCount, typename... DepthStencilOutputCandidates, typename... ColorOutputCandidates>
RenderPass::RenderPass(vulkan::Device& device,
                       Options::WindowProperties const& windowProperties,
                       VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags,
                       std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates,
                       render_target_output<type_list<DepthStencilOutputCandidates...>> const&,
                       render_target_output<type_list<ColorOutputCandidates...>> const&)
  : vkRenderPass_{ device.handle(), vkDestroyRenderPass }
  , renderTarget_{ nullptr }
  , frameFences_{}
  , renderTargetFences_{}
  , passFinishedSemaphores_{}
  , renderQueueSubmitInfo_{}
  , vkDummyPipelineLayout_{ device.handle(), vkDestroyPipelineLayout }
  , vkDummyPipeline_{ device.handle(), vkDestroyPipeline }
  , vkCommandPool_{ device.handle(), vkDestroyCommandPool }
  , commandBuffers_{}
{
    using rt_builder_t = RenderTargetBuilder<render_target_output<type_list<DepthStencilOutputCandidates...>>,
                                             render_target_output<type_list<ColorOutputCandidates...>>>;

    rt_builder_t rtBuilder{ device, Surface{ device, windowProperties }, presentModeCandidates, vkCompositeAlphaFlags };

    auto [attachments, references] = rtBuilder.getAttachments();

    VkSubpassDescription subPass = {};
    subPass.colorAttachmentCount = rt_builder_t::color_attachment_count_v;
    subPass.pColorAttachments = references.data();

    if constexpr (sizeof...(DepthStencilOutputCandidates) > 0) {
        subPass.pDepthStencilAttachment = &references[rt_builder_t::depth_attachment_idx_v];
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
    renderPassInfo.pSubpasses = &subPass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (auto result = vkCreateRenderPass(device.handle(), &renderPassInfo, nullptr, &vkRenderPass_);
        result != VK_SUCCESS) {

        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("not enough RAM memory to create render pass");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("not enough GPU memory to create render pass");
        }

        assert(false);
    }

    renderTarget_ =
      std::make_unique<RenderTarget>(rtBuilder.buildRenderPassTarget(static_cast<VkRenderPass>(vkRenderPass_)));

    frameFences_.reserve(renderTarget_->swapChainLength());
    passFinishedSemaphores_.reserve(renderTarget_->swapChainLength());

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0, count = renderTarget_->swapChainLength(); i < count; i++) {
        if (auto result = vkCreateFence(
              device.handle(), &fenceCreateInfo, nullptr, &frameFences_.emplace_back(device.handle(), vkDestroyFence));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create frame synchronization fence");
        }

        if (auto result = vkCreateSemaphore(device.handle(),
                                            &semaphoreCreateInfo,
                                            nullptr,
                                            &passFinishedSemaphores_.emplace_back(device.handle(), vkDestroySemaphore));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create pass end synchronization semaphore");
        }
    }

    renderTargetFences_.resize(renderTarget_->swapChainLength(), VK_NULL_HANDLE);

    _createDummyPipeline(device);

    _createDummyCommandPool(device);

    commandBuffers_.resize(renderTarget_->swapChainLength(), VK_NULL_HANDLE);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = static_cast<VkCommandPool>(vkCommandPool_);
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

    if (auto result = vkAllocateCommandBuffers(device.handle(), &commandBufferAllocateInfo, commandBuffers_.data());
        result != VK_SUCCESS) {
        std::runtime_error("could not allocate command buffers");
    }

    VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };

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
        renderPassBeginInfo.framebuffer = renderTarget_->frameBuffers()[i].handle();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = renderTarget_->width();
        renderPassBeginInfo.renderArea.extent.height = renderTarget_->height();
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<VkPipeline>(vkDummyPipeline_));

        vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS) {
            throw std::runtime_error("could not record command buffer!");
        }
    }
}
}

#endif // CYCLONITE_RENDERPASS_H
