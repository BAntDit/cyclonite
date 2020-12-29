//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERPASS_H
#define CYCLONITE_RENDERPASS_H

#include "renderTargetBuilder.h"
#include "surface.h"
#include "typedefs.h"
#include "vulkan/baseCommandBufferSet.h"
#include "vulkan/buffer.h"
#include "windowProperties.h"
#include <optional>

namespace cyclonite::render {
class RenderPass
{
private:
    using graphics_queue_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

public:
    class Input
    {};

    class FrameCommands
    {
    public:
        explicit FrameCommands(vulkan::Device const& device);

        FrameCommands(FrameCommands const&) = delete;

        FrameCommands(FrameCommands&&) = default;

        ~FrameCommands() = default;

        auto operator=(FrameCommands const&) -> FrameCommands& = delete;

        auto operator=(FrameCommands &&) -> FrameCommands& = default;

        void update(vulkan::Device& device,
                    VkRenderPass renderPass,
                    VkFramebuffer framebuffer,
                    std::array<uint32_t, 4> const& viewport,
                    std::pair<size_t, VkClearValue const*> const& clearValues,
                    VkDescriptorPool descriptorPool,
                    VkDescriptorSetLayout descriptorSetLayout,
                    VkPipelineLayout pipelineLayout,
                    VkPipeline pipeline);

        void setUniformBuffer(std::shared_ptr<vulkan::Buffer> const& buffer);

        void setIndexBuffer(std::shared_ptr<vulkan::Buffer> const& buffer);

        void setVertexBuffer(std::shared_ptr<vulkan::Buffer> const& buffer);

        void setInstanceBuffer(std::shared_ptr<vulkan::Buffer> const& buffer);

        void setCommandBuffer(std::shared_ptr<vulkan::Buffer> const& buffer, uint32_t commandCount);

        [[nodiscard]] auto graphicsCommandCount() const -> size_t { return graphicsCommands_->commandBufferCount(); }

        [[nodiscard]] auto graphicsCommands() const -> VkCommandBuffer const*
        {
            return graphicsCommands_->pCommandBuffers();
        }

        [[nodiscard]] auto waitSemaphores() const -> std::vector<VkSemaphore> const& { return waitSemaphores_; }

        [[nodiscard]] auto waitFlags() const -> std::vector<VkPipelineStageFlags> const& { return waitFlags_; }

        [[nodiscard]] auto frameSemaphore() const -> VkSemaphore const& { return waitSemaphores_[0]; }

        [[nodiscard]] auto frameSemaphore() -> VkSemaphore& { return waitSemaphores_[0]; }

        [[nodiscard]] auto waitSemaphoreCount() const -> uint32_t { return static_cast<uint32_t>(waitSemaphoreCount_); }

        void resetWaitSemaphores();

        void addWaitSemaphore(VkSemaphore waitSemaphore,
                              VkPipelineStageFlags flags = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM);

    private:
        void _createDescriptorSets(VkDevice vkDevice,
                                   VkDescriptorPool vkDescriptorPool,
                                   VkDescriptorSetLayout descriptorSetLayout);

    private:
        VkDevice vkDevice_;

        std::shared_ptr<vulkan::Buffer> indices_;
        std::shared_ptr<vulkan::Buffer> vertices_;
        std::shared_ptr<vulkan::Buffer> instances_;
        std::shared_ptr<vulkan::Buffer> commands_;
        std::shared_ptr<vulkan::Buffer> uniforms_;

        uint32_t commandCount_;

        std::vector<VkSemaphore> waitSemaphores_;
        std::vector<VkPipelineStageFlags> waitFlags_;
        size_t waitSemaphoreCount_;

        std::unique_ptr<graphics_queue_commands_t> graphicsCommands_;

        bool descriptorSetExpired_;
        VkDescriptorSet descriptorSet_;
    };

public:
    template<typename DepthStencilOutput, typename ColorOutput, size_t presentModeCandidateCount>
    RenderPass(
      vulkan::Device& device,
      WindowProperties const& windowProperties,
      DepthStencilOutput&& = render_target_output<type_list<render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>{},
      ColorOutput&& = render_target_output<
        type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
        RenderTargetOutputSemantic::DEFAULT>{},
      VkClearDepthStencilValue const& clearDepthStencilValue = { 1.0f, 0 },
      VkClearColorValue const& clearColorValue = { { 0.0f, 0.0f, 0.0f, 1.0f } },
      std::array<VkPresentModeKHR, presentModeCandidateCount> const&
        presentModeCandidates = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR },
      VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

    template<typename ColorOutput, size_t presentModeCandidateCount>
    RenderPass(vulkan::Device& device,
               WindowProperties const& windowProperties,
               ColorOutput&& colorOutput = render_target_output<
                 type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
                 RenderTargetOutputSemantic::DEFAULT>{},
               VkClearColorValue const& clearColorValue = { { 0.0f, 0.0f, 0.0f, 1.0f } },
               std::array<VkPresentModeKHR, presentModeCandidateCount> const&
                 presentModeCandidates = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR },
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

    void begin(vulkan::Device& device);

    void update(vulkan::Device& device,
                VkDescriptorPool descriptorPool,
                VkDescriptorSetLayout descriptorSetLayout,
                VkPipelineLayout pipelineLayout,
                VkPipeline pipeline);

    void end(vulkan::Device const& device);

    auto frame() -> RenderPass::FrameCommands& { return frameCommands_[commandsIndex_]; }

    [[nodiscard]] auto passFinishedSemaphore() const -> vulkan::Handle<VkSemaphore> const&
    {
        return signalSemaphores_[commandsIndex_];
    }

    [[nodiscard]] auto commandsIndex() const -> size_t { return commandsIndex_; }

    [[nodiscard]] auto fence() const -> VkFence { return static_cast<VkFence>(frameFences_[frameIndex_]); }

    [[nodiscard]] auto handle() const -> VkRenderPass { return static_cast<VkRenderPass>(vkRenderPass_); }

    [[nodiscard]] auto viewport() const -> std::array<uint32_t, 4>;

    [[nodiscard]] auto hasDepthStencil() const -> bool;

    [[nodiscard]] auto getSwapChainLength() const -> size_t;

private:
    void _createRenderPass(vulkan::Device const& device, VkRenderPassCreateInfo const& renderPassCreateInfo);

    void _createSemaphores(vulkan::Device const& device, size_t count);

private:
    uint32_t frameIndex_;
    uint32_t commandsIndex_;

    vulkan::Handle<VkRenderPass> vkRenderPass_;
    std::variant<std::monostate, SurfaceRenderTarget, FrameBufferRenderTarget> renderTarget_;
    std::vector<vulkan::Handle<VkFence>> frameFences_;
    std::vector<VkFence> rtFences_;
    std::vector<vulkan::Handle<VkSemaphore>> signalSemaphores_;

    std::vector<FrameCommands> frameCommands_;
}; // RenderPass

template<typename DepthStencilOutput, typename... ColorOutputs>
RenderPass::RenderPass(vulkan::Device& device,
                       uint32_t outputWidth,
                       uint32_t outputHeight,
                       bool doubleBuffering,
                       DepthStencilOutput&&,
                       std::tuple<ColorOutputs...>&&,
                       VkClearDepthStencilValue const& clearDepthStencilValue,
                       std::array<VkClearColorValue, sizeof...(ColorOutputs)> const& clearColorValues)
  : frameIndex_{ 0 }
  , commandsIndex_{ 0 }
  , vkRenderPass_{ device.handle(), vkDestroyRenderPass }
  , renderTarget_{}
  , frameFences_{}
  , rtFences_{}
  , signalSemaphores_{}
  , frameCommands_{}
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
                       WindowProperties const& windowProperties,
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
                       WindowProperties const& windowProperties,
                       DepthStencilOutput&&,
                       ColorOutput&&,
                       VkClearDepthStencilValue const& clearDepthStencilValue,
                       VkClearColorValue const& clearColorValue,
                       std::array<VkPresentModeKHR, presentModeCandidateCount> const& presentModeCandidates,
                       VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags)
  : frameIndex_{ 0 }
  , commandsIndex_{ 0 }
  , vkRenderPass_{ device.handle(), vkDestroyRenderPass }
  , renderTarget_{}
  , frameFences_{}
  , rtFences_{}
  , signalSemaphores_{}
  , frameCommands_{}
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

    const auto frameCount = renderTarget.frameBufferCount();

    frameFences_.reserve(frameCount);

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < frameCount; i++) {
        if (auto result = vkCreateFence(
              device.handle(), &fenceCreateInfo, nullptr, &frameFences_.emplace_back(device.handle(), vkDestroyFence));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create frame synchronization fence");
        }
    }

    rtFences_.resize(frameCount, VK_NULL_HANDLE);

    _createSemaphores(device, frameCount);

    frameCommands_.reserve(frameCount);

    for (size_t i = 0; i < frameCount; i++) {
        frameCommands_.emplace_back(device);
    }

    renderTarget_ = std::move(renderTarget);
}
}

#endif // CYCLONITE_RENDERPASS_H
