//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERPASS_H
#define CYCLONITE_RENDERPASS_H

#include "options.h"
#include "renderTargetBuilder.h"
#include "surface.h"
#include "vulkan/baseCommandBufferSet.h"
#include "vulkan/buffer.h"
#include "typedefs.h"
#include <optional>

namespace cyclonite {
class RenderPass
{
private:
    using graphics_queue_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

public:
    class FrameCommands
    {
    public:
        FrameCommands() noexcept;

        explicit FrameCommands(vulkan::Device const& device);

        FrameCommands(FrameCommands const&) = delete;

        FrameCommands(FrameCommands&&) = default;

        ~FrameCommands() = default;

        auto operator=(FrameCommands const&) -> FrameCommands& = delete;

        auto operator=(FrameCommands &&) -> FrameCommands& = default;

        [[nodiscard]] auto version() const -> uint64_t { return version_; }

        [[nodiscard]] auto transferVersion() const -> uint32_t
        {
            return static_cast<uint32_t>(version_ & 0xffffffffUL);
        }

        [[nodiscard]] auto graphicsVersion() const -> uint32_t { return static_cast<uint32_t>(version_ >> 32UL); }

        template<typename UpdateCallback>
        auto updatePersistentTransfer(UpdateCallback&& callback)
          -> std::enable_if_t<std::is_invocable_v<UpdateCallback,
                                                  std::vector<std::shared_ptr<vulkan::BaseCommandBufferSet>>&,
                                                  std::vector<VkSemaphore>&,
                                                  std::vector<VkPipelineStageFlags>&>>;

        template<typename UpdateCallback>
        auto updateTransientTransfer(UpdateCallback&& callback)
          -> std::enable_if_t<std::is_invocable_v<UpdateCallback,
                                                  std::vector<std::unique_ptr<vulkan::BaseCommandBufferSet>>&,
                                                  std::vector<VkSemaphore>&,
                                                  std::vector<VkPipelineStageFlags>&>>;

        void update(vulkan::Device& device,
                    VkRenderPass renderPass,
                    VkDescriptorPool descriptorPool,
                    VkFramebuffer framebuffer,
                    std::array<uint32_t, 4>&& viewport,
                    VkSemaphore frameBufferAvailableSemaphore,
                    VkSemaphore passFinishedSemaphore,
                    std::pair<size_t, VkClearValue const*>&& clearValues,
                    bool depthStencilRequired,
                    FrameCommands& frameUpdate);

        [[nodiscard]] auto transferQueueSubmitInfo() const -> std::unique_ptr<VkSubmitInfo> const&
        {
            return transferQueueSubmitInfo_;
        }

        [[nodiscard]] auto graphicsQueueSubmitInfo() const -> VkSubmitInfo const& { return graphicsQueueSubmitInfo_; }

        [[nodiscard]] auto fence() const -> VkFence { return static_cast<VkFence>(fence_); }

        [[nodiscard]] auto semaphore() const -> VkSemaphore { return static_cast<VkSemaphore>(passFinishedSemaphore_); }

        [[nodiscard]] auto indicesBuffer() const -> std::shared_ptr<vulkan::Buffer> const& { return indicesBuffer_; }

        [[nodiscard]] auto transferBuffer() const -> std::shared_ptr<vulkan::Buffer> const& { return transformBuffer_; }

        [[nodiscard]] auto commandBuffer() const -> std::shared_ptr<vulkan::Buffer> const& { return commandBuffer_; }

        void setIndicesBuffer(std::shared_ptr<vulkan::Buffer> const& buffer)
        {
            indicesBuffer_ = buffer;
            version_ = static_cast<uint64_t>(transferVersion()) | static_cast<uint64_t>(graphicsVersion() + 1) << 32UL;
        }

        void setTransferBuffer(std::shared_ptr<vulkan::Buffer> const& buffer)
        {
            transformBuffer_ = buffer;
            version_ = static_cast<uint64_t>(transferVersion()) | static_cast<uint64_t>(graphicsVersion() + 1) << 32UL;
        }

        void setCommandBuffer(std::shared_ptr<vulkan::Buffer> const& buffer, uint32_t commandCount)
        {
            drawCommandCount_ = commandCount;
            commandBuffer_ = buffer;
            version_ = static_cast<uint64_t>(transferVersion()) | static_cast<uint64_t>(graphicsVersion() + 1) << 32UL;
        }

    private:
        void _updatePipeline(vulkan::Device& device,
                             VkRenderPass renderPass,
                             std::array<uint32_t, 4> const& viewport,
                             bool depthStencilRequired);

        void _createDescriptorSets(VkDevice vkDevice, VkDescriptorPool vkDescriptorPool);

        void _clearTransientTransfer();

    private:
        uint64_t version_;

        std::vector<std::shared_ptr<vulkan::BaseCommandBufferSet>> persistentTransfer_;
        std::vector<VkCommandBuffer> transferCommands_;
        std::vector<VkSemaphore> transferSemaphores_;

        std::vector<std::unique_ptr<vulkan::BaseCommandBufferSet>> transientTransfer_;
        std::vector<vulkan::Handle<VkSemaphore>> transientSemaphores_;
        std::vector<VkPipelineStageFlags> transientDstWaitFlags_;

        vulkan::Handle<VkFence> fence_;
        vulkan::Handle<VkSemaphore> passFinishedSemaphore_;
        VkSemaphore vkSignalSemaphore_;

        std::unique_ptr<graphics_queue_commands_t> graphicsCommands_;
        std::vector<VkSemaphore> waitSemaphores_;
        std::vector<VkPipelineStageFlags> dstWaitFlags_;

        std::shared_ptr<vulkan::Buffer> indicesBuffer_;
        std::shared_ptr<vulkan::Buffer> transformBuffer_;
        std::shared_ptr<vulkan::Buffer> commandBuffer_;

        uint32_t drawCommandCount_;

        // dummy
        vulkan::Handle<VkDescriptorSetLayout> descriptorSetLayout_;
        vulkan::Handle<VkPipelineLayout> pipelineLayout_;
        vulkan::Handle<VkPipeline> pipeline_;
        //

        VkDescriptorSet vkTransformBufferDescriptor_;

        std::unique_ptr<VkSubmitInfo> transferQueueSubmitInfo_;

        VkSubmitInfo graphicsQueueSubmitInfo_;
    };

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
      std::array<VkPresentModeKHR, presentModeCandidateCount> const&
        presentModeCandidates = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR },
      VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

    template<typename ColorOutput, size_t presentModeCandidateCount>
    RenderPass(vulkan::Device& device,
               Options::WindowProperties const& windowProperties,
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

    auto begin(vulkan::Device& device) -> std::tuple<FrameCommands&, VkFence, VkSemaphore>;

    void end(vulkan::Device const& device, VkSemaphore passFinishedSemaphore);

    auto frame() -> RenderPass::FrameCommands& { return frameUpdate_; }

private:
    void _createRenderPass(vulkan::Device const& device, VkRenderPassCreateInfo const& renderPassCreateInfo);

    void _createDummyDescriptorPool(vulkan::Device const& device, size_t maxSets);

private:
    vulkan::Handle<VkRenderPass> vkRenderPass_;
    std::variant<std::monostate, SurfaceRenderTarget, FrameBufferRenderTarget> renderTarget_;
    std::vector<VkFence> renderTargetFences_;

    // tmp:: create descriptor set pool here // dummy // for dummy pipeline // just for now
    vulkan::Handle<VkDescriptorPool> vkDescriptorPool_;

    vulkan::CommandBufferSet<vulkan::CommandPool, std::vector<VkCommandBuffer>> commandBufferSet_;

    FrameCommands frameUpdate_;
    std::vector<FrameCommands> frameCommands_;
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
  , renderTargetFences_{}
  , vkDescriptorPool_{ device.handle(), vkDestroyDescriptorPool }
  , frameUpdate_{}
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
  , renderTargetFences_{}
  , vkDescriptorPool_{ device.handle(), vkDestroyDescriptorPool }
  , frameUpdate_{}
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

    const auto frameCount = renderTarget.swapChainLength();

    renderTargetFences_.resize(frameCount, VK_NULL_HANDLE);

    _createDummyDescriptorPool(device, frameCount);

    frameCommands_.reserve(frameCount);

    for (size_t i = 0; i < frameCount; i++) {
        frameCommands_.emplace_back(device);
    }

    renderTarget_ = std::move(renderTarget);
}

template<typename UpdateCallback>
auto RenderPass::FrameCommands::updatePersistentTransfer(UpdateCallback&& callback)
  -> std::enable_if_t<std::is_invocable_v<UpdateCallback,
                                          std::vector<std::shared_ptr<vulkan::BaseCommandBufferSet>>&,
                                          std::vector<VkSemaphore>&,
                                          std::vector<VkPipelineStageFlags>&>>
{
    persistentTransfer_.clear();
    transientSemaphores_.clear();
    waitSemaphores_.clear();
    dstWaitFlags_.clear();

    callback(persistentTransfer_, transferSemaphores_, dstWaitFlags_);

    transferCommands_.clear();

    for (auto&& pt : persistentTransfer_) {
        auto count = pt->commandBufferCount();

        transferCommands_.reserve(transferCommands_.size() + count);

        for (size_t i = 0; i < count; i++)
            transferCommands_.push_back(pt->getCommandBuffer(i));
    }

    version_ = static_cast<uint64_t>(transferVersion() + 1) | static_cast<uint64_t>(graphicsVersion()) << 32UL;
}

template<typename UpdateCallback>
auto RenderPass::FrameCommands::updateTransientTransfer(UpdateCallback&& callback)
  -> std::enable_if_t<std::is_invocable_v<UpdateCallback,
                                          std::vector<std::unique_ptr<vulkan::BaseCommandBufferSet>>&,
                                          std::vector<VkSemaphore>&,
                                          std::vector<VkPipelineStageFlags>&>>
{
    transientTransfer_.clear();
    transientSemaphores_.clear();
    transientDstWaitFlags_.clear();

    callback(transientTransfer_, transientSemaphores_, transientDstWaitFlags_);

    version_ = static_cast<uint64_t>(transferVersion() + 1) | static_cast<uint64_t>(graphicsVersion()) << 32UL;
}
}

#endif // CYCLONITE_RENDERPASS_H
