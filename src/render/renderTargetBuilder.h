//
// Created by bantdit on 12/20/19.
//

#ifndef CYCLONITE_RENDERTARGETBUILDER_H
#define CYCLONITE_RENDERTARGETBUILDER_H

#include "frameBufferRenderTarget.h"
#include "surfaceRenderTarget.h"

namespace cyclonite::render {
using namespace easy_mp;

template<VkFormat vkFormat, VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_MAX_ENUM_KHR>
struct render_target_output_candidate
{
    constexpr static VkFormat format = vkFormat;
    constexpr static VkColorSpaceKHR colorSpace = vkColorSpaceKHR;
};

template<typename Properties, RenderTargetOutputSemantic Semantic = RenderTargetOutputSemantic::UNDEFINED>
struct render_target_output;

template<RenderTargetOutputSemantic Semantic, VkFormat... format, VkColorSpaceKHR... colorSpace>
struct render_target_output<type_list<render_target_output_candidate<format, colorSpace>...>, Semantic>
{
    constexpr static std::array<std::pair<VkFormat, VkColorSpaceKHR>, sizeof...(format)> format_candidate_list_v = {
        std::make_pair(format, colorSpace)...
    };

    constexpr static bool is_empty_v = sizeof...(format) == 0;

    constexpr static RenderTargetOutputSemantic semantic_v = Semantic;
};

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
class RenderTargetBuilder
{
public:
    using attachment_list_t =
      std::conditional_t<DepthStencilOutputDescription::is_empty_v,
                         std::pair<std::array<VkAttachmentDescription, sizeof...(ColorOutputDescriptions)>,
                                   std::array<VkAttachmentReference, sizeof...(ColorOutputDescriptions)>>,
                         std::pair<std::array<VkAttachmentDescription, sizeof...(ColorOutputDescriptions) + 1>,
                                   std::array<VkAttachmentReference, sizeof...(ColorOutputDescriptions) + 1>>>;

    using clear_value_list_t = std::conditional_t<DepthStencilOutputDescription::is_empty_v,
                                                  std::array<VkClearValue, sizeof...(ColorOutputDescriptions)>,
                                                  std::array<VkClearValue, sizeof...(ColorOutputDescriptions) + 1>>;

    static constexpr size_t color_attachment_count_v = sizeof...(ColorOutputDescriptions);

    static constexpr size_t depth_attachment_idx_v = sizeof...(ColorOutputDescriptions);

public:
    RenderTargetBuilder(vulkan::Device& device,
                        uint32_t width,
                        uint32_t height,
                        std::array<VkClearColorValue, sizeof...(ColorOutputDescriptions)> const& clearColorValues,
                        VkClearDepthStencilValue clearDepthStencilValue);

    RenderTargetBuilder(RenderTargetBuilder const&) = delete;

    RenderTargetBuilder(RenderTargetBuilder&&) = delete;

    ~RenderTargetBuilder() = default;

    auto operator=(RenderTargetBuilder const&) -> RenderTargetBuilder& = delete;

    auto operator=(RenderTargetBuilder &&) -> RenderTargetBuilder& = delete;

    auto getAttachments() const -> attachment_list_t;

    auto getClearValues() const -> clear_value_list_t;

private:
    template<size_t candidateCount>
    static auto _findSupportedFormat(std::array<std::pair<VkFormat, VkColorSpaceKHR>, candidateCount> const& candidates,
                                     VkPhysicalDevice physicalDevice,
                                     VkImageTiling requiredTiling,
                                     VkFormatFeatureFlags requiredFeatures) -> VkFormat;

    template<typename T>
    static auto _make_clear_value(T&& t)
      -> std::enable_if_t<std::is_same_v<std::decay_t<T>, VkClearDepthStencilValue> ||
                            std::is_same_v<std::decay_t<T>, VkClearColorValue>,
                          VkClearValue>;

    template<size_t... idx>
    auto _get_output_semantic_format_pairs(std::index_sequence<idx...>) const
      -> std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, sizeof...(idx)>;

    template<size_t... idx>
    auto _get_attachments(std::index_sequence<idx...>) const -> attachment_list_t;

    template<size_t... idx>
    [[nodiscard]] auto _get_clear_values(std::index_sequence<idx...>) const -> clear_value_list_t;

    [[nodiscard]] auto _get_attachment(size_t idx) const -> VkAttachmentDescription;

    [[nodiscard]] auto _get_attachment_ref(size_t idx) const -> VkAttachmentReference;

protected:
    vulkan::Device& device_;
    VkExtent2D extent_;
    VkFormat vkDepthStencilOutputFormat_;
    std::array<std::pair<VkFormat, VkColorSpaceKHR>, sizeof...(ColorOutputDescriptions)> colorOutputFormats_;
    std::array<RenderTargetOutputSemantic, sizeof...(ColorOutputDescriptions)> outputSemantics_;
    VkClearDepthStencilValue clearDepthStencilValue_;
    std::array<VkClearColorValue, sizeof...(ColorOutputDescriptions)> clearColorValues_;
};

template<typename DepthStencilOutputDescription, typename ColorOutputDescription>
class SurfaceRenderTargetBuilder
  : public RenderTargetBuilder<SurfaceRenderTarget, DepthStencilOutputDescription, ColorOutputDescription>
{
public:
    using base_render_target_builder_t =
      RenderTargetBuilder<SurfaceRenderTarget, DepthStencilOutputDescription, ColorOutputDescription>;

    template<size_t modeCandidateCount>
    SurfaceRenderTargetBuilder(vulkan::Device& device,
                               Surface&& surface,
                               VkClearColorValue const& clearColorValue,
                               VkClearDepthStencilValue const& clearDepthStencilValue,
                               std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates,
                               VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags);

    auto buildRenderPassTarget(VkRenderPass vkRenderPass) -> SurfaceRenderTarget;

private:
    static void _swapChainCreationErrorHandling(VkResult result);

    template<size_t candidateCount>
    static auto _chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats,
                                     std::array<std::pair<VkFormat, VkColorSpaceKHR>, candidateCount> const& candidates)
      -> std::pair<VkFormat, VkColorSpaceKHR>;

    template<size_t modeCount>
    auto _choosePresentationMode(VkPhysicalDevice physicalDevice,
                                 std::array<VkPresentModeKHR, modeCount> const& candidates) const -> VkPresentModeKHR;

private:
    Surface surface_;
    vulkan::Handle<VkSwapchainKHR> vkSwapChain_;
    VkPresentModeKHR vkPresentMode;
};

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
class FrameBufferRenderTargetBuilder
  : public RenderTargetBuilder<FrameBufferRenderTarget, DepthStencilOutputDescription, ColorOutputDescriptions...>
{
public:
    using base_render_target_builder_t =
      RenderTargetBuilder<FrameBufferRenderTarget, DepthStencilOutputDescription, ColorOutputDescriptions...>;

    FrameBufferRenderTargetBuilder(
      vulkan::Device& device,
      uint32_t swapChainLength,
      uint32_t width,
      uint32_t height,
      std::array<VkClearColorValue, sizeof...(ColorOutputDescriptions)> const& clearColorValues,
      VkClearDepthStencilValue clearDepthStencilValue);

    auto buildRenderPassTarget(VkRenderPass vkRenderPass) -> FrameBufferRenderTargetBuilder;

private:
    uint32_t swapChainLength_;
};

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::RenderTargetBuilder(
  vulkan::Device& device,
  uint32_t width,
  uint32_t height,
  std::array<VkClearColorValue, sizeof...(ColorOutputDescriptions)> const& clearColorValues,
  VkClearDepthStencilValue clearDepthStencilValue)
  : device_{ device }
  , extent_{}
  , vkDepthStencilOutputFormat_{ VK_FORMAT_UNDEFINED }
  , colorOutputFormats_{}
  , outputSemantics_{ ColorOutputDescriptions::semantic_v... }
  , clearDepthStencilValue_{ clearDepthStencilValue }
  , clearColorValues_{ clearColorValues }
{
    extent_.width = width;
    extent_.height = height;

    if constexpr (!DepthStencilOutputDescription::is_empty_v) {
        vkDepthStencilOutputFormat_ = _findSupportedFormat(DepthStencilOutputDescription::format_candidate_list_v,
                                                           device.physicalDevice(),
                                                           VK_IMAGE_TILING_OPTIMAL,
                                                           VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
FrameBufferRenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::
  FrameBufferRenderTargetBuilder(
    vulkan::Device& device,
    uint32_t swapChainLength,
    uint32_t width,
    uint32_t height,
    std::array<VkClearColorValue, sizeof...(ColorOutputDescriptions)> const& clearColorValues,
    VkClearDepthStencilValue clearDepthStencilValue)
  : base_render_target_builder_t{ device, width, height, clearColorValues, clearDepthStencilValue }
  , swapChainLength_{ swapChainLength }
{
    base_render_target_builder_t::colorOutputFormats_ = { std::make_pair(
      base_render_target_builder_t::_findSupportedFormat(ColorOutputDescriptions::format_candidate_list_v,
                                                         device.physicalDevice(),
                                                         VK_IMAGE_TILING_OPTIMAL,
                                                         VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT),
      VK_COLOR_SPACE_MAX_ENUM_KHR)... };
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
auto FrameBufferRenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::buildRenderPassTarget(
  VkRenderPass vkRenderPass) -> FrameBufferRenderTargetBuilder
{
    if constexpr (DepthStencilOutputDescription::is_empty_v) {
        return FrameBufferRenderTarget{ base_render_target_builder_t::device_,
                                        vkRenderPass,
                                        swapChainLength_,
                                        base_render_target_builder_t::extent_.width,
                                        base_render_target_builder_t::extent_.height,
                                        base_render_target_builder_t::_get_output_semantic_format_pairs(
                                          std::make_index_sequence<sizeof...(ColorOutputDescriptions)>{}),
                                        base_render_target_builder_t::clearColorValues_ };
    } else {
        return FrameBufferRenderTarget{ base_render_target_builder_t::device_,
                                        vkRenderPass,
                                        swapChainLength_,
                                        base_render_target_builder_t::extent_.width,
                                        base_render_target_builder_t::extent_.height,
                                        base_render_target_builder_t::vkDepthStencilOutputFormat_,
                                        base_render_target_builder_t::clearDepthStencilValue_,
                                        base_render_target_builder_t::_get_output_semantic_format_pairs(
                                          std::make_index_sequence<sizeof...(ColorOutputDescriptions)>{}),
                                        base_render_target_builder_t::clearColorValues_ };
    }
}

template<typename DepthStencilOutputDescription, typename ColorOutputDescription>
template<size_t modeCandidateCount>
SurfaceRenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescription>::SurfaceRenderTargetBuilder(
  vulkan::Device& device,
  Surface&& surface,
  VkClearColorValue const& clearColorValue,
  VkClearDepthStencilValue const& clearDepthStencilValue,
  std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates,
  VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags)
  : base_render_target_builder_t{ device,
                                  surface.width(),
                                  surface.height(),
                                  std::array{ clearColorValue },
                                  clearDepthStencilValue }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , vkPresentMode{ VK_PRESENT_MODE_MAX_ENUM_KHR }
{
    uint32_t availableFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice(), surface_.handle(), &availableFormatCount, nullptr);

    assert(availableFormatCount > 0);

    std::vector<VkSurfaceFormatKHR> availableFormats(availableFormatCount);

    vkGetPhysicalDeviceSurfaceFormatsKHR(
      device.physicalDevice(), surface_.handle(), &availableFormatCount, availableFormats.data());

    base_render_target_builder_t::colorOutputFormats_ = { _chooseSurfaceFormat(
      availableFormats, ColorOutputDescription::format_candidate_list_v) };

    assert((surface_.capabilities().supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ==
           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    assert((surface_.capabilities().supportedCompositeAlpha & vkCompositeAlphaFlags) == vkCompositeAlphaFlags);

    auto presentMode = _choosePresentationMode(device.physicalDevice(), presentModeCandidates);

    auto [surfaceFormat, surfaceColorSpace] = base_render_target_builder_t::colorOutputFormats_[0];

    VkSwapchainCreateInfoKHR swapChainCreateInfoKHR = {};
    swapChainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfoKHR.surface = surface_.handle();
    swapChainCreateInfoKHR.minImageCount =
      std::min(surface_.capabilities().minImageCount + 1,
               surface_.capabilities().maxImageCount > 0 ? surface_.capabilities().maxImageCount
                                                         : std::numeric_limits<uint32_t>::max());

    swapChainCreateInfoKHR.imageFormat = surfaceFormat;
    swapChainCreateInfoKHR.imageColorSpace = surfaceColorSpace;
    swapChainCreateInfoKHR.imageExtent = base_render_target_builder_t::extent_;
    swapChainCreateInfoKHR.imageArrayLayers = 1;
    swapChainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfoKHR.preTransform = surface_.capabilities().currentTransform;
    swapChainCreateInfoKHR.compositeAlpha = vkCompositeAlphaFlags;
    swapChainCreateInfoKHR.presentMode = presentMode;
    swapChainCreateInfoKHR.clipped = VK_TRUE;
    swapChainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

    if (auto result = vkCreateSwapchainKHR(device.handle(), &swapChainCreateInfoKHR, nullptr, &vkSwapChain_);
        result != VK_SUCCESS) {
        _swapChainCreationErrorHandling(result);
    }
}

template<typename DepthStencilOutputDescription, typename ColorOutputDescription>
void SurfaceRenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescription>::_swapChainCreationErrorHandling(
  VkResult result)
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

template<typename DepthStencilOutputDescription, typename ColorOutputDescription>
template<size_t candidateCount>
auto SurfaceRenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescription>::_chooseSurfaceFormat(
  std::vector<VkSurfaceFormatKHR> const& availableFormats,
  std::array<std::pair<VkFormat, VkColorSpaceKHR>, candidateCount> const& candidates)
  -> std::pair<VkFormat, VkColorSpaceKHR>
{
    for (auto&& [requiredFormat, requiredColorSpace] : candidates) {
        for (auto&& availableFormat : availableFormats) {
            if (requiredFormat == availableFormat.format && requiredColorSpace == availableFormat.colorSpace) {
                return std::make_pair(availableFormat.format, availableFormat.colorSpace);
            }
        }
    }

    throw std::runtime_error("surface does not support required formats.");
}

template<typename DepthStencilOutputDescription, typename ColorOutputDescription>
template<size_t modeCount>
auto SurfaceRenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescription>::_choosePresentationMode(
  VkPhysicalDevice physicalDevice,
  std::array<VkPresentModeKHR, modeCount> const& candidates) const -> VkPresentModeKHR
{
    uint32_t availableModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_.handle(), &availableModeCount, nullptr);

    assert(availableModeCount > 0);

    std::vector<VkPresentModeKHR> availablePresentModes(availableModeCount);

    vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface_.handle(), &availableModeCount, availablePresentModes.data());

    for (auto&& candidate : candidates) {
        for (auto&& availableMode : availablePresentModes) {
            if (availableMode == candidate) {
                return availableMode;
            }
        }
    }

    throw std::runtime_error("surface does support any required presentation modes");
}

template<typename DepthStencilOutputDescription, typename ColorOutputDescription>
auto SurfaceRenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescription>::buildRenderPassTarget(
  VkRenderPass vkRenderPass) -> SurfaceRenderTarget
{
    auto [surfaceFormat, _] = base_render_target_builder_t::colorOutputFormats_[0];

    if constexpr (DepthStencilOutputDescription::is_empty_v) {
        return SurfaceRenderTarget{ base_render_target_builder_t::device_,
                                    vkRenderPass,
                                    surface_,
                                    vkSwapChain_,
                                    surfaceFormat,
                                    base_render_target_builder_t::clearColorValues_[0],
                                    base_render_target_builder_t::outputSemantics_[0] };
    } else {
        return SurfaceRenderTarget{ base_render_target_builder_t::device_,
                                    vkRenderPass,
                                    surface_,
                                    vkSwapChain_,
                                    base_render_target_builder_t::vkDepthStencilOutputFormat_,
                                    base_render_target_builder_t::clearDepthStencilValue_,
                                    surfaceFormat,
                                    base_render_target_builder_t::clearColorValues_[0],
                                    base_render_target_builder_t::outputSemantics_[0] };
    }
}

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
auto RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::getAttachments() const
  -> attachment_list_t
{
    return _get_attachments(std::make_index_sequence<sizeof...(ColorOutputDescriptions)>());
}

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
auto RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::getClearValues() const
  -> clear_value_list_t
{
    return _get_clear_values(std::make_index_sequence<sizeof...(ColorOutputDescriptions)>{});
}

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t candidateCount>
auto RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::_findSupportedFormat(
  std::array<std::pair<VkFormat, VkColorSpaceKHR>, candidateCount> const& candidates,
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

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t... idx>
auto RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::
  _get_output_semantic_format_pairs(std::index_sequence<idx...>) const
  -> std::array<std::pair<VkFormat, RenderTargetOutputSemantic>, sizeof...(idx)>
{
    return std::array{ std::make_pair(colorOutputFormats_[idx].first, outputSemantics_[idx])... };
}

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t... idx>
auto RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::_get_attachments(
  std::index_sequence<idx...>) const -> attachment_list_t
{
    constexpr size_t count = sizeof...(ColorOutputDescriptions);

    if constexpr (DepthStencilOutputDescription::is_empty_v) {
        return std::make_pair(std::array<VkAttachmentDescription, count>{ _get_attachment(idx)... },
                              std::array<VkAttachmentReference, count>{ _get_attachment_ref(idx)... });
    } else {
        VkAttachmentDescription depthAttachment = {};

        depthAttachment.format = vkDepthStencilOutputFormat_;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = depth_attachment_idx_v;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        return std::make_pair(
          std::array<VkAttachmentDescription, count + 1>{ _get_attachment(idx)..., depthAttachment },
          std::array<VkAttachmentReference, count + 1>{ _get_attachment_ref(idx)..., depthAttachmentRef });
    }
}

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
auto RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::_get_attachment_ref(
  size_t idx) const -> VkAttachmentReference
{
    VkAttachmentReference reference = {};

    reference.attachment = static_cast<uint32_t>(idx);
    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    return reference;
}

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
auto RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::_get_attachment(
  size_t idx) const -> VkAttachmentDescription
{
    VkAttachmentDescription attachment = {};

    auto [format, _] = colorOutputFormats_[idx];

    (void)_;

    attachment.format = format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT; // multisampling support later, m'kay
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    return attachment;
}

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<typename T>
auto RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::_make_clear_value(
  T&& t) -> std::enable_if_t<std::is_same_v<std::decay_t<T>, VkClearDepthStencilValue> ||
                               std::is_same_v<std::decay_t<T>, VkClearColorValue>,
                             VkClearValue>
{
    VkClearValue clearColorValue = {};

    if constexpr (std::is_same_v<std::decay_t<T>, VkClearColorValue>) {
        clearColorValue.color = std::forward<T>(t);
    } else {
        clearColorValue.depthStencil = std::forward<T>(t);
    }

    return clearColorValue;
}

template<typename TargetType, typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t... idx>
auto RenderTargetBuilder<TargetType, DepthStencilOutputDescription, ColorOutputDescriptions...>::_get_clear_values(
  std::index_sequence<idx...>) const -> clear_value_list_t
{
    if constexpr (DepthStencilOutputDescription::is_empty_v) {
        return std::array{ _make_clear_value(clearColorValues_[idx])... };
    } else {
        return std::array{ _make_clear_value(clearColorValues_[idx])..., _make_clear_value(clearDepthStencilValue_) };
    }
}
}

#endif // CYCLONITE_RENDERTARGETBUILDER_H
