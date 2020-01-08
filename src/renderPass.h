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
    // template<size_t presentModeCandidateCount,
    //         typename... DepthStencilOutputCandidates,
    //         typename... ColorOutputCandidates>
    RenderPass(vulkan::Device& device,
               Options::WindowProperties const& windowProperties,
               VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

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
    void _createDummyPipeline(vulkan::Device const& device);

    void _createDummyCommandPool(vulkan::Device const& device);

    [[nodiscard]] auto _getSwapChainLength() const -> size_t;

    [[nodiscard]] auto _getFrameBuffers() const -> std::vector<vulkan::FrameBuffer> const&;

    [[nodiscard]] auto _getSize() const -> std::pair<uint32_t, uint32_t>;

    [[nodiscard]] auto _getColorAttachmentCount() const -> size_t;

    [[nodiscard]] auto _hasDepthStencil() const -> bool;

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
}

#endif // CYCLONITE_RENDERPASS_H
