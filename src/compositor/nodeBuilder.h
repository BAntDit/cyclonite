//
// Created by anton on 3/29/21.
//

#ifndef CYCLONITE_NODEBUILDER_H
#define CYCLONITE_NODEBUILDER_H

#include "node.h"
#include <utility>

namespace cyclonite::compositor {
extern auto getDepthStencilImageLayoutByFormat(VkFormat format) -> VkImageLayout;

template<typename NodeConfig>
class BaseNode::Builder
{
public:
    Builder(vulkan::Device& device);

    auto setOutputResolution(uint16_t width, uint16_t height) -> Builder&;

    template<size_t inputLinkCount>
    auto createInputLinks() -> Builder&;

    template<RenderTargetOutputSemantic... semantic>
    auto setInputs(size_t nodeIndex) -> Builder&;

    template<VkImageTiling Tiling, VkFormat... format, bool IsPublic = false>
    auto setRenderTargetDepthProperties(render_target_output<type_list<render_target_candidate_t<format>...>,
                                                             RenderTargetOutputSemantic::UNDEFINED,
                                                             IsPublic,
                                                             Tiling> &&) -> Builder&;

    template<typename... RenderTargetColorOutput>
    auto setRenderTargetColorProperties(RenderTargetColorOutput&&...)
      -> std::enable_if_t<(sizeof...(RenderTargetColorOutput) > 0), Builder&>;

    template<VkColorSpaceKHR... colorSpaceCandidate, VkPresentModeKHR... presentModeCandidate>
    auto setSurface(WindowProperties&& windowProperties,
                    surface_parameters<type_list<color_space_candidate_t<colorSpaceCandidate>...>,
                                       type_list<present_mode_candidate_t<presentModeCandidate>...>>&&,
                    VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) -> Builder&;

    template<size_t inputRefCount, size_t outputRefCount>
    auto addPass(PassType passType,
                 std::array<uint32_t, inputRefCount> const& inputAttachmentIndices,
                 std::array<uint32_t, outputRefCount> const& colorAttachmentIndices,
                 uint32_t srcPassIndex,
                 uint32_t dstPassIndex,
                 VkPipelineStageFlags srcStageMask,
                 VkPipelineStageFlags dstStageMask,
                 VkAccessFlags srcAccessMask,
                 VkAccessFlags dstAccessMask,
                 bool writeDepth,
                 VkDependencyFlags dependencyFlags) -> Builder&;

    template<size_t outputRefCount>
    auto addPass(PassType passType,
                 std::array<uint32_t, outputRefCount> const& colorAttachmentIndices,
                 uint32_t dstPassIndex,
                 VkPipelineStageFlags srcStageMask,
                 VkPipelineStageFlags dstStageMask,
                 VkAccessFlags srcAccessMask,
                 VkAccessFlags dstAccessMask,
                 bool writeDepth,
                 VkDependencyFlags dependencyFlags) -> Builder&;

    auto build() -> Node<NodeConfig>;

private:
    template<size_t candidateCount>
    static auto _findSupportedFormat(std::array<VkFormat, candidateCount> const& candidates,
                                     VkPhysicalDevice physicalDevice,
                                     VkImageTiling requiredTiling,
                                     VkFormatFeatureFlags requiredFeatures) -> VkFormat;

    static void _swapChainCreationErrorHandling(VkResult result);

    template<size_t count>
    static auto _createFrameBufferRT(
      vulkan::Device& device,
      VkRenderPass vkRenderPass,
      uint32_t width,
      uint32_t height,
      VkFormat depthStencilFormat,
      std::array<std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>, count> const& properties)
      -> FrameBufferRenderTarget;

private:
    using surface_properties_t =
      std::optional<std::tuple<Surface, vulkan::Handle<VkSwapchainKHR>, VkPresentModeKHR, VkColorSpaceKHR>>;

    using render_pass_properties_t = std::tuple<PassType, attachment_ref_array_t<32>, attachment_ref_array_t<32>, bool>;

    vulkan::Device* device_;
    uint32_t width_;
    uint32_t height_;
    Links inputLinks_;
    VkFormat depthFormat_;
    VkImageTiling depthTiling_;
    std::bitset<value_cast(RenderTargetOutputSemantic::COUNT)> publicSemanticBits_;
    color_format_array_t<16> colorOutputs_;
    surface_properties_t surfaceProps_;
    std::vector<render_pass_properties_t> renderPasses_;
    std::vector<VkSubpassDependency> dependencies_;
    vulkan::Handle<VkRenderPass> vkRenderPass_;
    render_target_t renderTarget_;
};

template<typename NodeConfig>
BaseNode::Builder<NodeConfig>::Builder(vulkan::Device& device)
  : device_{ &device }
  , width_{ 0 }
  , height_{ 0 }
  , inputLinks_{}
  , depthFormat_{ VK_FORMAT_UNDEFINED }
  , depthTiling_{ VK_IMAGE_TILING_OPTIMAL }
  , publicSemanticBits_{}
  , colorOutputs_{}
  , surfaceProps_{}
  , renderPasses_{}
  , dependencies_{}
  , vkRenderPass_{ device.handle(), vkDestroyRenderPass }
  , renderTarget_{}
{}

template<typename NodeConfig>
auto BaseNode::Builder<NodeConfig>::setOutputResolution(uint16_t width, uint16_t height) -> Builder&
{
    width_ = width;
    height_ = height;

    return *this;
}

template<typename NodeConfig>
template<size_t inputLinkCount>
auto BaseNode::Builder<NodeConfig>::createInputLinks() -> Builder&
{
    inputLinks_ = Links::create<inputLinkCount>(*device_);
    return *this;
}

template<typename NodeConfig>
template<RenderTargetOutputSemantic... semantic>
auto BaseNode::Builder<NodeConfig>::setInputs(size_t nodeIndex) -> Builder&
{
    auto it = std::find_if(inputLinks_.begin(), inputLinks_.end(), [=](auto&& link) -> bool {
        return link.nodeIndex == std::numeric_limits<size_t>::max();
    });

    assert(it != inputLinks_.end());

    auto setter = []<size_t... idx>(Link & link,
                                    size_t nodeIndex,
                                    std::index_sequence<idx...>&&,
                                    std::array<RenderTargetOutputSemantic, sizeof...(idx)> && semantics)
                    ->void
    {
        link.nodeIndex = nodeIndex;
        ((link.semantics[idx] = semantics[idx]), ...);
    };

    setter(*it, nodeIndex, std::make_index_sequence<sizeof...(semantic)>{}, std::array{ semantic... });

    return *this;
}

template<typename NodeConfig>
template<VkImageTiling Tiling, VkFormat... format, bool IsPublic>
auto BaseNode::Builder<NodeConfig>::setRenderTargetDepthProperties(
  render_target_output<type_list<render_target_candidate_t<format>...>,
                       RenderTargetOutputSemantic::UNDEFINED,
                       IsPublic,
                       Tiling> &&) -> Builder&
{
    // TODO:: public depth (make possible to access depth outside of the node)

    depthFormat_ = _findSupportedFormat(std::array<VkFormat, sizeof...(format)>{ format... },
                                        device_->physicalDevice(),
                                        Tiling,
                                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    depthTiling_ = Tiling;

    return *this;
}

template<typename NodeConfig>
template<typename... RenderTargetColorOutput>
auto BaseNode::Builder<NodeConfig>::setRenderTargetColorProperties(RenderTargetColorOutput&&...)
  -> std::enable_if_t<(sizeof...(RenderTargetColorOutput) > 0), Builder&>
{
    colorOutputs_ = std::array{ std::make_tuple(_findSupportedFormat(RenderTargetColorOutput::format_candidate_array_v,
                                                                     device_->physicalDevice(),
                                                                     RenderTargetColorOutput::tiling_v,
                                                                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT),
                                                RenderTargetColorOutput::tiling_v,
                                                RenderTargetColorOutput::semantic_v)... };

    (publicSemanticBits_.set(value_cast(RenderTargetColorOutput::semantic_v), RenderTargetColorOutput::is_public_v),
     ...);

    return *this;
}

template<typename NodeConfig>
template<VkColorSpaceKHR... colorSpaceCandidate, VkPresentModeKHR... presentModeCandidate>
auto BaseNode::Builder<NodeConfig>::setSurface(
  WindowProperties&& windowProperties,
  surface_parameters<type_list<color_space_candidate_t<colorSpaceCandidate>...>,
                     type_list<present_mode_candidate_t<presentModeCandidate>...>>&&,
  VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags) -> Builder&
{
    auto&& surface = Surface{ *device_, windowProperties };

    uint32_t availableFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_->physicalDevice(), surface.handle(), &availableFormatCount, nullptr);

    if (availableFormatCount == 0)
        throw std::runtime_error("there is no available formats for surface");

    std::vector<VkSurfaceFormatKHR> availableFormats(availableFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
      device_->physicalDevice(), surface.handle(), &availableFormatCount, availableFormats.data());

    auto format = VkFormat{ VK_FORMAT_MAX_ENUM };
    auto colorSpace = VkColorSpaceKHR{ VK_COLOR_SPACE_MAX_ENUM_KHR };

    auto&& [requiredFormat, requiredTiling] = std::visit(
      [](auto&& outputs) -> std::pair<VkFormat, VkImageTiling> {
          if constexpr (!std::is_same_v<std::decay_t<decltype(outputs)>, std::monostate>) {
              auto&& [desiredFormat, desiredTiling, _] = outputs[0];
              (void)_;
              return std::make_pair(desiredFormat, desiredTiling);
          }

          return std::make_pair(VK_FORMAT_UNDEFINED, VK_IMAGE_TILING_MAX_ENUM);
      },
      colorOutputs_);

    assert(requiredFormat != VK_FORMAT_UNDEFINED && requiredTiling != VK_IMAGE_TILING_MAX_ENUM);

    for (auto&& requiredColorSpace : std::array{ colorSpaceCandidate... }) {
        for (auto&& availableFormat : availableFormats) {
            if (requiredFormat == availableFormat.format && requiredColorSpace == availableFormat.colorSpace) {
                auto formatProperties = VkFormatProperties{};
                vkGetPhysicalDeviceFormatProperties(device_->physicalDevice(), requiredFormat, &formatProperties);

                if ((formatProperties.optimalTilingFeatures & requiredTiling) == requiredTiling) {
                    colorSpace = requiredColorSpace;
                    format = requiredFormat;
                    break;
                }
            }
        }
    }

    if (format == VK_FORMAT_MAX_ENUM || colorSpace == VK_COLOR_SPACE_MAX_ENUM_KHR)
        throw std::runtime_error("can not find available pair: <surface format, surface color space>");

    uint32_t availableModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device_->physicalDevice(), surface.handle(), &availableModeCount, nullptr);

    if (availableModeCount == 0)
        throw std::runtime_error("there is no available present modes for surface");

    std::vector<VkPresentModeKHR> availablePresentModes(availableModeCount);

    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device_->physicalDevice(), surface.handle(), &availableModeCount, availablePresentModes.data());

    auto presentMode = VkPresentModeKHR{ VK_PRESENT_MODE_MAX_ENUM_KHR };

    for (auto&& candidate : std::array{ presentModeCandidate... }) {
        for (auto&& availableMode : availablePresentModes) {
            if (availableMode == candidate) {
                presentMode = availableMode;
                break;
            }
        }
    }

    if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR)
        throw std::runtime_error("can not find available present mode for surface");

    VkSwapchainCreateInfoKHR swapChainCreateInfoKHR = {};
    swapChainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfoKHR.surface = surface.handle();
    swapChainCreateInfoKHR.minImageCount =
      std::min(surface.capabilities().minImageCount + 1,
               surface.capabilities().maxImageCount > 0 ? surface.capabilities().maxImageCount
                                                        : std::numeric_limits<uint32_t>::max());
    swapChainCreateInfoKHR.imageFormat = format;
    swapChainCreateInfoKHR.imageColorSpace = colorSpace;
    swapChainCreateInfoKHR.imageExtent = VkExtent2D{ width_, height_ };
    swapChainCreateInfoKHR.imageArrayLayers = 1;
    swapChainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfoKHR.preTransform = surface.capabilities().currentTransform;
    swapChainCreateInfoKHR.compositeAlpha = vkCompositeAlphaFlags;
    swapChainCreateInfoKHR.presentMode = presentMode;
    swapChainCreateInfoKHR.clipped = VK_TRUE;
    swapChainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

    auto vkSwapChain = vulkan::Handle<VkSwapchainKHR>{ device_->handle(), vkDestroySwapchainKHR };

    if (auto result = vkCreateSwapchainKHR(device_->handle(), &swapChainCreateInfoKHR, nullptr, &vkSwapChain);
        result != VK_SUCCESS) {
        _swapChainCreationErrorHandling(result);
    }

    surfaceProps_ = std::make_tuple(std::move(surface), std::move(vkSwapChain), presentMode, colorSpace);

    return *this;
}

template<typename NodeConfig>
template<size_t inputRefCount, size_t outputRefCount>
auto BaseNode::Builder<NodeConfig>::addPass(PassType passType,
                                            std::array<uint32_t, inputRefCount> const& inputAttachmentIndices,
                                            std::array<uint32_t, outputRefCount> const& colorAttachmentIndices,
                                            uint32_t srcPassIndex,
                                            uint32_t dstPassIndex,
                                            VkPipelineStageFlags srcStageMask,
                                            VkPipelineStageFlags dstStageMask,
                                            VkAccessFlags srcAccessMask,
                                            VkAccessFlags dstAccessMask,
                                            bool writeDepth,
                                            VkDependencyFlags dependencyFlags) -> Builder&
{
    auto&& subPassDependency = VkSubpassDependency{};

    subPassDependency.srcSubpass = srcPassIndex;
    subPassDependency.dstSubpass = dstPassIndex;
    subPassDependency.srcStageMask = srcStageMask;
    subPassDependency.dstStageMask = dstStageMask;
    subPassDependency.srcAccessMask = srcAccessMask;
    subPassDependency.dstAccessMask = dstAccessMask;
    subPassDependency.dependencyFlags = dependencyFlags;

    dependencies_.push_back(subPassDependency);

    using input_refs_t =
      std::conditional_t<inputRefCount != 0, std::array<VkAttachmentReference, inputRefCount>, std::monostate>;
    using output_refs_t =
      std::conditional_t<outputRefCount != 0, std::array<VkAttachmentReference, outputRefCount>, std::monostate>;

    auto inputRefs = input_refs_t{};
    auto outputRefs = output_refs_t{};

    if constexpr (inputRefCount > 0) {
        for (size_t i = 0; i < inputRefCount; i++) {
            inputRefs[i].attachment = inputAttachmentIndices[i];
            inputRefs[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }

    if constexpr (outputRefCount > 0) {
        for (size_t i = 0; i < outputRefCount; i++) {
            outputRefs[i].attachment = colorAttachmentIndices[i];
            outputRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
    }

    renderPasses_.emplace_back(passType, inputRefs, outputRefs, writeDepth);

    return *this;
}

template<typename NodeConfig>
template<size_t outputRefCount>
auto BaseNode::Builder<NodeConfig>::addPass(PassType passType,
                                            std::array<uint32_t, outputRefCount> const& colorAttachmentIndices,
                                            uint32_t dstPassIndex,
                                            VkPipelineStageFlags srcStageMask,
                                            VkPipelineStageFlags dstStageMask,
                                            VkAccessFlags srcAccessMask,
                                            VkAccessFlags dstAccessMask,
                                            bool writeDepth,
                                            VkDependencyFlags dependencyFlags) -> Builder&
{
    return addPass(passType,
                   std::array<uint32_t, 0>{},
                   colorAttachmentIndices,
                   VK_SUBPASS_EXTERNAL,
                   dstPassIndex,
                   srcStageMask,
                   dstStageMask,
                   srcAccessMask,
                   dstAccessMask,
                   writeDepth,
                   dependencyFlags);
}

template<typename NodeConfig>
auto BaseNode::Builder<NodeConfig>::build() -> Node<NodeConfig>
{
    constexpr auto passCount = NodeConfig::pass_count;

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

    auto subPassDescriptions = std::array<VkSubpassDescription, passCount>{};
    auto depthRefs = std::array<VkAttachmentReference, passCount>{};

    for (auto passIndex = uint8_t{ 0 }; passIndex < passCount; passIndex++) {
        auto& renderPass = renderPasses_[passIndex];
        auto& subPassDesc = subPassDescriptions[passIndex];

        auto&& [passType, inputAttachmentIndices, colorAttachmentIndices, writeDepth] = renderPass;

        subPassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        std::visit(
          [&](auto& inputs) -> void {
              if constexpr (!std::is_same_v<std::decay_t<decltype(inputs)>, std::monostate>) {
                  subPassDesc.inputAttachmentCount = inputs.size();
                  subPassDesc.pInputAttachments = inputs.data();
              }
          },
          inputAttachmentIndices);

        std::visit(
          [&](auto& outputs) -> void {
              if constexpr (!std::is_same_v<std::decay_t<decltype(outputs)>, std::monostate>) {
                  subPassDesc.colorAttachmentCount = outputs.size();
                  subPassDesc.pColorAttachments = outputs.data();
              }
          },
          colorAttachmentIndices);

        if (writeDepth && depthFormat_ != VK_FORMAT_UNDEFINED) {
            auto& depthRef = depthRefs[passIndex];
            depthRef.attachment = colorAttachmentCount; // depth always goes just after color
            depthRef.layout = getDepthStencilImageLayoutByFormat(depthFormat_);

            subPassDesc.pDepthStencilAttachment = &depthRef;
        }
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
                    isSurfacePass ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

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
        attachment.finalLayout = getDepthStencilImageLayoutByFormat(depthFormat_);

        attachments.emplace_back(attachment);
    }

    renderPassCreateInfo.attachmentCount = attachments.size();
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = subPassDescriptions.size();
    renderPassCreateInfo.pSubpasses = subPassDescriptions.data();
    renderPassCreateInfo.dependencyCount = dependencies_.size();
    renderPassCreateInfo.pDependencies = dependencies_.data();

    if (auto result = vkCreateRenderPass(device_->handle(), &renderPassCreateInfo, nullptr, &vkRenderPass_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("not enough RAM memory to create render pass");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("not enough GPU memory to create render pass");
        }

        assert(false);
    }

    auto frameBufferCount = size_t{ 0 };

    if (isSurfacePass) {
        auto&& [surface, swapChain, mode, colorSpace] = *surfaceProps_;

        (void)mode;
        (void)colorSpace;

        assert(!attachments.empty());

        // for now - surface node is always single pass and with no MRT, m'kay
        auto [format, semantic] = std::visit(
          [](auto& outputs) -> std::pair<VkFormat, RenderTargetOutputSemantic> {
              if constexpr (!std::is_same_v<std::decay_t<decltype(outputs)>, std::monostate>) {
                  auto [outputFormat, _, outputSemantic] = outputs[0];
                  (void)_;

                  return std::make_pair(outputFormat, outputSemantic);
              }

              return std::make_pair(VK_FORMAT_UNDEFINED, RenderTargetOutputSemantic::DEFAULT);
          },
          colorOutputs_);

        assert(format != VK_FORMAT_UNDEFINED);

        if (depthFormat_ != VK_FORMAT_UNDEFINED) {
            assert(attachments.size() > 1);

            auto&& rt = SurfaceRenderTarget{
                *device_, static_cast<VkRenderPass>(vkRenderPass_), surface, swapChain, depthFormat_, format, semantic
            };

            frameBufferCount = rt.frameBufferCount();

            renderTarget_ = std::move(rt);
        } else {
            auto&& rt =
              SurfaceRenderTarget{ *device_, static_cast<VkRenderPass>(vkRenderPass_), surface, swapChain, format,
                                   semantic };

            frameBufferCount = rt.frameBufferCount();

            renderTarget_ = std::move(rt);
        }
    } else {
        frameBufferCount = 1;

        // and for now - no render to mips / no grab passes
        renderTarget_ = std::visit(
          [frameBufferCount, this](auto&& attachments) -> FrameBufferRenderTarget {
              if constexpr (!std::is_same_v<std::decay_t<decltype(attachments)>, std::monostate>) {
                  return _createFrameBufferRT(
                    *device_, static_cast<VkRenderPass>(vkRenderPass_), width_, height_, depthFormat_, attachments);
              }

              return FrameBufferRenderTarget{ *device_,         static_cast<VkRenderPass>(vkRenderPass_),
                                              width_,           height_,
                                              frameBufferCount, depthFormat_ };
          },
          colorOutputs_);
    }

    auto node = Node<NodeConfig>{ frameBufferCount };

    // signal semaphores
    {
        auto&& signals = node.signalSemaphores_;

        signals.reserve(frameBufferCount);

        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (auto i = size_t{ 0 }; i < frameBufferCount; i++) {
            if (auto result = vkCreateSemaphore(device_->handle(),
                                                &semaphoreCreateInfo,
                                                nullptr,
                                                &signals.emplace_back(device_->handle(), vkDestroySemaphore));
                result != VK_SUCCESS) {
                throw std::runtime_error("could not create image available semaphore.");
            }
        }
    }

    node.vkRenderPass_ = std::move(vkRenderPass_);
    node.renderTarget_ = std::move(renderTarget_);

    for (auto i = size_t{ 0 }, count = renderPasses_.size(); i < count; i++) {
        auto&& renderPass = renderPasses_[i];
        auto&& [passType, inputAttachmentIndices, colorAttachmentIndices, writeDepth] = renderPass;

        (void)inputAttachmentIndices;
        (void)colorAttachmentIndices;

        auto inputImageCount = uint32_t{ 0 };

        for (auto&& input : inputLinks_) {
            if (input.nodeIndex == std::numeric_limits<size_t>::max())
                continue;

            for (auto semanticIndex = size_t{ 0 }, semanticCount = value_cast(RenderTargetOutputSemantic::COUNT);
                 semanticIndex < semanticCount;
                 semanticIndex++) {
                if (input.semantics[semanticIndex] != RenderTargetOutputSemantic::INVALID)
                    inputImageCount++;
            }
        }

        node._createPass(i, passType, *device_, writeDepth, inputImageCount);
    }

    node.inputs_ = std::move(inputLinks_);

    node.publicSemanticBits_ = publicSemanticBits_;

    return node;
}

template<typename NodeConfig>
template<size_t candidateCount>
auto BaseNode::Builder<NodeConfig>::_findSupportedFormat(std::array<VkFormat, candidateCount> const& candidates,
                                                         VkPhysicalDevice physicalDevice,
                                                         VkImageTiling requiredTiling,
                                                         VkFormatFeatureFlags requiredFeatures) -> VkFormat
{
    for (auto&& candidate : candidates) {
        VkFormatProperties formatProperties = {};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidate, &formatProperties);

        if (requiredTiling == VK_IMAGE_TILING_LINEAR &&
            (formatProperties.linearTilingFeatures & requiredFeatures) == requiredFeatures) {
            return candidate;
        } else if (requiredTiling == VK_IMAGE_TILING_OPTIMAL &&
                   (formatProperties.optimalTilingFeatures & requiredFeatures) == requiredFeatures) {
            return candidate;
        }
    }

    throw std::runtime_error("could not find suitable format for render target");
}

template<size_t... idx>
static auto _get_output_images(
  std::index_sequence<idx...>,
  std::array<std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>, sizeof...(idx)> const& properties)
  -> std::array<std::variant<vulkan::ImagePtr, VkFormat>, sizeof...(idx)>
{
    return std::array<std::variant<vulkan::ImagePtr, VkFormat>, sizeof...(idx)>{
        FrameBufferRenderTarget::framebuffer_attachment_t{ std::get<0>(properties[idx]) }...
    };
}

template<size_t... idx>
static auto _get_output_semantics(
  std::index_sequence<idx...>,
  std::array<std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>, sizeof...(idx)> const& properties)
  -> std::array<RenderTargetOutputSemantic, sizeof...(idx)>
{
    return std::array<RenderTargetOutputSemantic, sizeof...(idx)>{ std::get<RenderTargetOutputSemantic>(
      properties[idx])... };
}

template<typename NodeConfig>
template<size_t count>
auto BaseNode::Builder<NodeConfig>::_createFrameBufferRT(
  vulkan::Device& device,
  VkRenderPass vkRenderPass,
  uint32_t width,
  uint32_t height,
  VkFormat depthStencilFormat,
  std::array<std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>, count> const& properties)
  -> FrameBufferRenderTarget
{
    return (depthStencilFormat != VK_FORMAT_UNDEFINED)
             ? FrameBufferRenderTarget{ device,
                                        vkRenderPass,
                                        width,
                                        height,
                                        depthStencilFormat,
                                        _get_output_images(std::make_index_sequence<count>{}, properties),
                                        _get_output_semantics(std::make_index_sequence<count>{}, properties) }
             : FrameBufferRenderTarget{ device,
                                        vkRenderPass,
                                        width,
                                        height,
                                        _get_output_images(std::make_index_sequence<count>{}, properties),
                                        _get_output_semantics(std::make_index_sequence<count>{}, properties) };
}

template<typename NodeConfig>
void BaseNode::Builder<NodeConfig>::_swapChainCreationErrorHandling(VkResult result)
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
}

#endif // CYCLONITE_NODEBUILDER_H
