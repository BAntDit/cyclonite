//
// Created by bantdit on 12/20/19.
//

#ifndef CYCLONITE_RENDERTARGETBUILDER_H
#define CYCLONITE_RENDERTARGETBUILDER_H

#include "../renderTarget.h"

namespace cyclonite::internal {
using namespace easy_mp;

template<VkFormat vkFormat, VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_MAX_ENUM_KHR>
struct render_target_output_candidate
{
    constexpr static VkFormat format = vkFormat;
    constexpr static VkColorSpaceKHR colorSpace = vkColorSpaceKHR;
};

template<typename Properties, RenderTargetOutputSemantic Semantic>
struct render_target_output;

template<RenderTargetOutputSemantic Semantic, VkFormat... format, VkColorSpaceKHR... colorSpace>
struct render_target_output<type_list<render_target_output_candidate<format, colorSpace>...>, Semantic>
{
    constexpr static std::array<std::pair<VkFormat, VkColorSpaceKHR>, sizeof...(format)> format_candidate_list_v = {
        std::make_pair(format, colorSpace)...
    };

    constexpr static RenderTargetOutputSemantic semantic_v = Semantic;
};

template<RenderTargetOutputSemantic Semantic, VkFormat... format>
struct render_target_output<type_list<render_target_output_candidate<format>...>, Semantic>
{
    constexpr static std::array<std::pair<VkFormat, VkColorSpaceKHR>, sizeof...(format)> format_candidate_list_v = {
        std::make_pair(format, render_target_output_candidate<format>::colorSpace)...
    };

    constexpr static RenderTargetOutputSemantic semantic_v = Semantic;
};

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
class RenderTargetBuilder
{
public:
    using attachment_list_t =
      std::conditional_t<std::is_same_v<DepthStencilOutputDescription, void>,
                         std::pair<std::array<VkAttachmentDescription, sizeof...(ColorOutputDescriptions)>,
                                   std::array<VkAttachmentReference, sizeof...(ColorOutputDescriptions)>>,
                         std::pair<std::array<VkAttachmentDescription, sizeof...(ColorOutputDescriptions) + 1>,
                                   std::array<VkAttachmentReference, sizeof...(ColorOutputDescriptions) + 1>>>;

    using render_target_t = std::conditional_t<std::is_same_v<DepthStencilOutputDescription, void>,
                                               RenderTarget<false, sizeof...(ColorOutputDescriptions)>,
                                               RenderTarget<true, sizeof...(ColorOutputDescriptions)>>;

public:
    template<size_t modeCandidateCount = 2>
    RenderTargetBuilder(vulkan::Device& device,
                        Surface&& surface,
                        std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates =
                          { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR },
                        VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

    RenderTargetBuilder(RenderTargetBuilder const&) = delete;

    RenderTargetBuilder(RenderTargetBuilder&&) = delete;

    ~RenderTargetBuilder() = default;

    auto operator=(RenderTargetBuilder const&) -> RenderTargetBuilder& = delete;

    auto operator=(RenderTargetBuilder &&) -> RenderTargetBuilder& = delete;

    auto getAttachments() const -> attachment_list_t;

    auto buildRenderPassTarget(VkRenderPass vkRenderPass) -> render_target_t;

private:
    static void _swapChainCreationErrorHandling(VkResult result);

    template<size_t candidateCount>
    static auto _chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR> const& availableFormats,
                                     std::array<std::pair<VkFormat, VkColorSpaceKHR>, candidateCount> const& candidates)
      -> std::pair<VkFormat, VkColorSpaceKHR>;

    template<size_t modeCount>
    auto _choosePresentationMode(VkPhysicalDevice physicalDevice,
                                 std::array<VkPresentModeKHR, modeCount> const& candidates) const -> VkPresentModeKHR;

    template<size_t... idx>
    auto _get_attachments(std::index_sequence<idx...>) const -> attachment_list_t;

    [[nodiscard]] auto _get_attachment(size_t idx) const -> VkAttachmentDescription;

    [[nodiscard]] auto _get_attachment_ref(size_t idx) const -> VkAttachmentReference;

private:
    vulkan::Device& device_;
    std::optional<Surface> surface_;
    vulkan::Handle<VkSwapchainKHR> vkSwapChain_;
    VkExtent2D extent_;
    VkPresentModeKHR vkPresentMode;
    VkFormat vkDepthStencilOutputFormat_;
    std::array<std::pair<VkFormat, VkColorSpaceKHR>, sizeof...(ColorOutputDescriptions)> colorOutputFormats_;
};

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t modeCandidateCount>
RenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::RenderTargetBuilder(
  cyclonite::vulkan::Device& device,
  cyclonite::Surface&& surface,
  std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates,
  VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags)
  : device_{ device }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , extent_{}
  , vkPresentMode{ VK_PRESENT_MODE_MAX_ENUM_KHR }
  , vkDepthStencilOutputFormat_{ VK_FORMAT_UNDEFINED }
  , colorOutputFormats_{}
{
    static_assert(sizeof...(ColorOutputDescriptions) > 0); // surface RT has no sense without color attachments

    extent_.width = surface_->width();
    extent_.height = surface_->height();

    uint32_t availableFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice(), surface_->handle(), &availableFormatCount, nullptr);

    assert(availableFormatCount > 0);

    std::vector<VkSurfaceFormatKHR> availableFormats(availableFormatCount);

    vkGetPhysicalDeviceSurfaceFormatsKHR(
      device.physicalDevice(), surface_->handle(), &availableFormatCount, availableFormats.data());

    colorOutputFormats_ = { _chooseSurfaceFormat(availableFormats,
                                                 ColorOutputDescriptions::format_candidate_list_v)... };

    auto presentMode = _choosePresentationMode(device.physicalDevice(), presentModeCandidates);

    assert((surface_->capabilities().supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) ==
           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    assert((surface_->capabilities().supportedCompositeAlpha & vkCompositeAlphaFlags) == vkCompositeAlphaFlags);

    auto [surfaceFormat, surfaceColorSpace] = colorOutputFormats_[0];

    VkSwapchainCreateInfoKHR swapChainCreateInfoKHR = {};
    swapChainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfoKHR.surface = surface_->handle();
    swapChainCreateInfoKHR.minImageCount =
      std::min(surface_->capabilities().minImageCount + 1,
               surface_->capabilities().maxImageCount > 0 ? surface_->capabilities().maxImageCount
                                                          : std::numeric_limits<uint32_t>::max());

    swapChainCreateInfoKHR.imageFormat = surfaceFormat;
    swapChainCreateInfoKHR.imageColorSpace = surfaceColorSpace;
    swapChainCreateInfoKHR.imageExtent = extent_;
    swapChainCreateInfoKHR.imageArrayLayers = 1;
    swapChainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainCreateInfoKHR.preTransform = surface_->capabilities().currentTransform;
    swapChainCreateInfoKHR.compositeAlpha = vkCompositeAlphaFlags;
    swapChainCreateInfoKHR.presentMode = presentMode;
    swapChainCreateInfoKHR.clipped = VK_TRUE;
    swapChainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

    if (auto result = vkCreateSwapchainKHR(device.handle(), &swapChainCreateInfoKHR, nullptr, &vkSwapChain_);
        result != VK_SUCCESS) {
        _swapChainCreationErrorHandling(result);
    }
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
auto RenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::buildRenderPassTarget(
  VkRenderPass vkRenderPass) -> render_target_t
{
    if (surface_) {
        auto [surfaceFormat, _] = colorOutputFormats_[0];

        (void)_;

        return RenderTarget(device_, vkRenderPass, *surface_, vkSwapChain_, vkDepthStencilOutputFormat_, surfaceFormat);
    } else {
        // TODO:: ...
    }
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
auto RenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::getAttachments() const
  -> attachment_list_t
{
    return _get_attachments(std::make_index_sequence<sizeof...(ColorOutputDescriptions)>());
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
void RenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::_swapChainCreationErrorHandling(
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

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t candidateCount>
auto RenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::_chooseSurfaceFormat(
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

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t modeCount>
auto RenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::_choosePresentationMode(
  VkPhysicalDevice physicalDevice,
  std::array<VkPresentModeKHR, modeCount> const& candidates) const -> VkPresentModeKHR
{
    uint32_t availableModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_->handle(), &availableModeCount, nullptr);

    assert(availableModeCount > 0);

    std::vector<VkPresentModeKHR> availablePresentModes(availableModeCount);

    vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface_->handle(), &availableModeCount, availablePresentModes.data());

    for (auto&& candidate : candidates) {
        for (auto&& availableMode : availablePresentModes) {
            if (availableMode == candidate) {
                return availableMode;
            }
        }
    }

    throw std::runtime_error("surface does support any required presentation modes");
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t... idx>
auto RenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::_get_attachments(
  std::index_sequence<idx...>) const -> attachment_list_t
{
    constexpr size_t count = sizeof...(ColorOutputDescriptions);

    if constexpr (std::is_same_v<DepthStencilOutputDescription, void>) {
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
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        return std::make_pair(
          std::array<VkAttachmentDescription, count + 1>{ depthAttachment, _get_attachment(idx)... },
          std::array<VkAttachmentReference, count + 1>{ depthAttachmentRef, _get_attachment_ref(idx + 1)... });
    }
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
auto RenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::_get_attachment_ref(
  size_t idx) const -> VkAttachmentReference
{
    VkAttachmentReference reference = {};

    reference.attachment = static_cast<uint32_t>(idx);
    reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    return reference;
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
auto RenderTargetBuilder<DepthStencilOutputDescription, ColorOutputDescriptions...>::_get_attachment(size_t idx) const
  -> VkAttachmentDescription
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
}

#endif // CYCLONITE_RENDERTARGETBUILDER_H
