//
// Created by anton on 12/4/20.
//

#ifndef CYCLONITE_NODE_H
#define CYCLONITE_NODE_H

#include "frameBufferRenderTarget.h"
#include "render/renderTargetBuilder.h"
#include "surfaceRenderTarget.h"
#include "vulkan/image.h"
#include <easy-mp/containers.h>
#include <easy-mp/type_list.h>

namespace cyclonite::compositor {
using namespace easy_mp;

class Node
{
public:
    template<VkFormat format>
    using render_target_candidate_t = std::integral_constant<VkFormat, format>;

    template<typename Properties,
             RenderTargetOutputSemantic Semantic = RenderTargetOutputSemantic::UNDEFINED,
             VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL>
    struct render_target_output;

    template<RenderTargetOutputSemantic Semantic, VkImageTiling Tiling, VkFormat... format>
    struct render_target_output<type_list<render_target_candidate_t<format>...>, Semantic, Tiling>
    {
        constexpr static std::array<VkFormat, sizeof...(format)> format_candidate_array_v = { format... };

        constexpr static bool is_empty_v = sizeof...(format) == 0;

        constexpr static RenderTargetOutputSemantic semantic_v = Semantic;

        constexpr static VkImageTiling tiling_v = Tiling;
    };

    enum class PassType
    {
        SCENE = 0,
        SCREEN = 1,
        MIN_VALUE = SCENE,
        MAX_VALUE = SCREEN,
        COUNT = MAX_VALUE + 1
    };

public:
    class Links
    {
    public:
        template<size_t linkCount>
        static auto create() -> Links;

        Links() = default;

        void set(vulkan::ImagePtr const& image, size_t index);

        [[nodiscard]] auto get(size_t index) const -> vulkan::ImagePtr const&;

    private:
        template<size_t maxSize>
        using image_io =
          to_variant_t<typename concat<type_list<std::monostate>, array_list_t<vulkan::ImagePtr, maxSize>>::type>;

        image_io<128> links_;
    };

    Node();

    auto getInputs() -> Links& { return inputs_; }

    [[nodiscard]] auto getInputs() const -> Links const& { return inputs_; }

    [[nodiscard]] auto getOutputs() const -> Links const& { return outputs_; }

private:
    template<size_t maxSize>
    using color_format_array_t = to_variant_t<
      typename concat<type_list<std::monostate>,
                      array_list_t<std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>, maxSize>>::type>;

    template<size_t maxSize>
    using attachment_ref_array_t =
      to_variant_t<typename concat<type_list<std::monostate>, array_list_t<VkAttachmentReference, maxSize>>::type>;

    template<size_t maxSize>
    using attachment_idx_array_t =
      to_variant_t<typename concat<type_list<std::monostate>, array_list_t<uint32_t, maxSize>>::type>;

public:
    class Builder
    {
    public:
        Builder(vulkan::Device& device, uint32_t width, uint32_t height);

        template<size_t inputLinkCount>
        auto createInputLinks() -> Builder&;

        template<size_t outputLinkCount>
        auto createOutputLinks() -> Builder&;

        template<size_t inputCount>
        auto setInputs(std::array<std::pair<size_t, vulkan::ImagePtr>, inputCount> const& inputs) -> Builder&;

        template<size_t candidateCount>
        auto setRenderTargetDepthProperties(VkImageTiling tiling,
                                            std::array<VkFormat, candidateCount> const& formatCandidates) -> Builder&;

        template<typename... RenderTargetColorOutput>
        auto setRenderTargetColorProperties(RenderTargetColorOutput&&... colorOutput) -> Builder&;

        template<size_t colorSpaceCandidateCount, size_t modeCandidateCount>
        auto setSurface(Surface&& surface,
                        std::array<VkColorSpaceKHR, colorSpaceCandidateCount> const& colorSpaceCandidates,
                        std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates,
                        VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags) -> Builder&;

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

        auto bindOutput(size_t outputIndex, RenderTargetOutputSemantic semantic) -> Builder&;

        void build();

    private:
        template<size_t candidateCount>
        static auto _findSupportedFormat(std::array<VkFormat, candidateCount> const& candidates,
                                         VkPhysicalDevice physicalDevice,
                                         VkImageTiling requiredTiling,
                                         VkFormatFeatureFlags requiredFeatures) -> VkFormat;

        static void _swapChainCreationErrorHandling(VkResult result);

        static auto _getDepthStencilLayoutByFormat(VkFormat format) -> VkImageLayout;

    private:
        using surface_properties_t =
          std::optional<std::tuple<Surface, vulkan::Handle<VkSwapchainKHR>, VkPresentModeKHR, VkColorSpaceKHR>>;

        using render_pass_properties_t =
          std::tuple<PassType, VkSubpassDependency, attachment_ref_array_t<32>, attachment_ref_array_t<32>, bool>;

        vulkan::Device* device_;
        uint32_t width_;
        uint32_t height_;
        Links inputLinks_;
        Links outputLinks_;
        VkFormat depthFormat_;
        VkImageTiling depthTiling_;
        color_format_array_t<32> colorOutputs_;
        surface_properties_t surfaceProps_;
        std::vector<render_pass_properties_t> renderPasses_;
    };

private:
    Links inputs_;
    Links outputs_;
    std::variant<std::monostate, SurfaceRenderTarget, FrameBufferRenderTarget> renderTarget_;
};

template<size_t linkCount>
auto Node::Links::create() -> Links
{
    Links links;
    links.links_ = std::conditional_t<linkCount != 0, std::array<vulkan::ImagePtr, linkCount>, std::monostate>{};

    return links;
}

template<size_t inputLinkCount>
auto Node::Builder::createInputLinks() -> Node::Builder&
{
    inputLinks_ = Links::create<inputLinkCount>();
    return *this;
}

template<size_t outputLinkCount>
auto Node::Builder::createOutputLinks() -> Node::Builder&
{
    outputLinks_ = Links::create<outputLinkCount>();
    return *this;
}

template<size_t inputCount>
auto Node::Builder::setInputs(std::array<std::pair<size_t, vulkan::ImagePtr>, inputCount> const& inputs)
  -> Node::Builder&
{
    for (auto&& [index, image] : inputs) {
        inputLinks_.set(image, index);
    }
    return *this;
}

template<size_t candidateCount>
auto Node::Builder::setRenderTargetDepthProperties(VkImageTiling tiling,
                                                   std::array<VkFormat, candidateCount> const& formatCandidates)
  -> Node::Builder&
{
    depthFormat_ = _findSupportedFormat(
      formatCandidates, device_->physicalDevice(), tiling, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    depthTiling_ = tiling;

    return *this;
}

template<typename... RenderTargetColorOutput>
auto Node::Builder::setRenderTargetColorProperties(RenderTargetColorOutput&&...) -> Node::Builder&
{
    colorOutputs_ = std::array{ std::make_tuple(_findSupportedFormat(RenderTargetColorOutput::format_candidate_array_v,
                                                                     device_->physicalDevice(),
                                                                     RenderTargetColorOutput::tiling_v,
                                                                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT),
                                                RenderTargetColorOutput::tiling_v,
                                                RenderTargetColorOutput::semantic_v)... };

    return *this;
}

template<size_t colorSpaceCandidateCount, size_t modeCandidateCount>
auto Node::Builder::setSurface(Surface&& surface,
                               std::array<VkColorSpaceKHR, colorSpaceCandidateCount> const& colorSpaceCandidates,
                               std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates,
                               VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags) -> Node::Builder&
{
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
          if (!std::is_same_v<std::decay_t<decltype(outputs)>, std::monostate>) {
              auto&& [format, tiling, _] = outputs[0];
              (void)_;
              return std::make_pair(format, tiling);
          }

          return std::make_pair(VK_FORMAT_UNDEFINED, VK_IMAGE_TILING_MAX_ENUM);
      },
      colorOutputs_);

    assert(requiredFormat != VK_FORMAT_UNDEFINED && requiredTiling != VK_IMAGE_TILING_MAX_ENUM);

    for (auto&& requiredColorSpace : colorSpaceCandidates) {
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

    for (auto&& candidate : presentModeCandidates) {
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

template<size_t inputRefCount, size_t outputRefCount>
auto Node::Builder::addPass(PassType passType,
                            std::array<uint32_t, inputRefCount> const& inputAttachmentIndices,
                            std::array<uint32_t, outputRefCount> const& colorAttachmentIndices,
                            uint32_t srcPassIndex,
                            uint32_t dstPassIndex,
                            VkPipelineStageFlags srcStageMask,
                            VkPipelineStageFlags dstStageMask,
                            VkAccessFlags srcAccessMask,
                            VkAccessFlags dstAccessMask,
                            bool writeDepth,
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

    using input_refs_t =
      std::conditional_t<inputRefCount != 0, std::array<VkAttachmentReference, inputRefCount>, std::monostate>;
    using output_refs_t =
      std::conditional_t<inputRefCount != 0, std::array<VkAttachmentReference, outputRefCount>, std::monostate>;

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

    renderPasses_.emplace_back(passType, subPassDependency, inputRefs, outputRefs, writeDepth);

    return *this;
}

template<size_t outputRefCount>
auto Node::Builder::addPass(PassType passType,
                            std::array<uint32_t, outputRefCount> const& colorAttachmentIndices,
                            uint32_t dstPassIndex,
                            VkPipelineStageFlags srcStageMask,
                            VkPipelineStageFlags dstStageMask,
                            VkAccessFlags srcAccessMask,
                            VkAccessFlags dstAccessMask,
                            bool writeDepth,
                            VkDependencyFlags dependencyFlags) -> Node::Builder&
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

template<size_t candidateCount>
auto Node::Builder::_findSupportedFormat(std::array<VkFormat, candidateCount> const& candidates,
                                         VkPhysicalDevice physicalDevice,
                                         VkImageTiling requiredTiling,
                                         VkFormatFeatureFlags requiredFeatures) -> VkFormat
{
    for (auto&& [candidate, _] : candidates) {
        (void)_;

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
}

#endif // CYCLONITE_NODE_H
