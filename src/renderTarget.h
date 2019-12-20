//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERTARGET_H
#define CYCLONITE_RENDERTARGET_H

#include "surface.h"

namespace cyclonite {
class RenderTarget
{
public:
    enum class RenderTargetOutputSemantic
    {
        UNDEFINED = 0,
        DEFAULT = 1,
        LINEAR_HDR_COLOR = 2,
        MIN_VALUE = UNDEFINED,
        MAX_VALUE = LINEAR_HDR_COLOR,
        COUNT = MAX_VALUE + 1
    };

    template<VkFormat vkFormat, VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_MAX_ENUM_KHR>
    struct output_format_candidate_t
    {
        constexpr static VkFormat format = vkFormat;
        constexpr static VkColorSpaceKHR colorSpace = vkColorSpaceKHR;
    };

    template<typename Properties, RenderTargetOutputSemantic Semantic>
    struct output_t;

    template<RenderTargetOutputSemantic Semantic, VkFormat... format, VkColorSpaceKHR... colorSpace>
    struct output_t<easy_mp::type_list<output_format_candidate_t<format, colorSpace>...>, Semantic>
    {
        constexpr static std::array<std::pair<VkFormat, VkColorSpaceKHR>, sizeof...(format)> format_candidate_list_v = {
            std::make_pair(format, colorSpace)...
        };

        constexpr static RenderTargetOutputSemantic semantic_v = Semantic;
    };

    template<RenderTargetOutputSemantic Semantic, VkFormat... format>
    struct output_t<easy_mp::type_list<output_format_candidate_t<format>...>, Semantic>
    {
        constexpr static std::array<std::pair<VkFormat, VkColorSpaceKHR>, sizeof...(format)> format_candidate_list_v = {
            std::make_pair(format, output_format_candidate_t<format>::colorSpace)...
        };

        constexpr static RenderTargetOutputSemantic semantic_v = Semantic;
    };

public:
    template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
    class Builder
    {
    public:
        constexpr static size_t attachmentCount = std::is_same_v<DepthStencilOutputDescription, void>
                                                    ? sizeof...(ColorOutputDescriptions)
                                                    : sizeof...(ColorOutputDescriptions) + 1;

    public:
        template<size_t modeCandidateCount = 2>
        Builder(vulkan::Device const& device,
                Surface&& surface,
                std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates = {
                  VK_PRESENT_MODE_MAILBOX_KHR,
                  VK_PRESENT_MODE_FIFO_KHR });

        void setRenderPass(VkRenderPass vkRenderPass) { vkRenderPass_ = vkRenderPass; }

        auto getAttachments() const -> std::pair<std::array<VkAttachmentDescription, attachmentCount>,
                                                 std::array<VkAttachmentReference, attachmentCount>>;

        auto build() -> RenderTarget;

    private:
        template<size_t candidateCount>
        static auto _chooseSurfaceFormat(
          std::vector<VkSurfaceFormatKHR> const& availableFormats,
          std::array<std::pair<VkFormat, VkColorSpaceKHR>, candidateCount> const& candidates)
          -> std::pair<VkFormat, VkColorSpaceKHR>;

        template<size_t modeCount>
        auto _choosePresentationMode(VkPhysicalDevice physicalDevice,
                                     std::array<VkPresentModeKHR, modeCount> const& candidates) const
          -> VkPresentModeKHR;

    private:
        std::optional<Surface> surface_;
        vulkan::Handle<VkSwapchainKHR> vkSwapChain_;
        VkExtent2D extent_;
        VkRenderPass vkRenderPass_;
        VkPresentModeKHR vkPresentMode;
        VkFormat vkDepthStencilOutputFormat_;
        std::array<std::pair<VkFormat, VkColorSpaceKHR>, sizeof...(ColorOutputDescriptions)> colorOutputFormats_;
    };

public:
    // template<typename DepthStencilAttachment, typename... ColorAttachments>
    RenderTarget(VkRenderPass vkRenderPass);

    template<typename DepthStencilOutputDescription, typename ColorOutputDescription, size_t modeCandidateCount = 2>
    RenderTarget(
      vulkan::Device const& device,
      VkRenderPass vkRenderPass,
      Surface& surface,
      DepthStencilOutputDescription depthStencilOutputDescription,
      ColorOutputDescription colorOutputDescription = output_t<
        easy_mp::type_list<output_format_candidate_t<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
        RenderTargetOutputSemantic::DEFAULT>{},
      std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates = { VK_PRESENT_MODE_MAILBOX_KHR,
                                                                                        VK_PRESENT_MODE_FIFO_KHR },
      VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);

    RenderTarget(RenderTarget const&) = delete;

    RenderTarget(RenderTarget&&) = default;

    ~RenderTarget() = default;

    auto operator=(RenderTarget const&) -> RenderTarget& = delete;

    auto operator=(RenderTarget &&) -> RenderTarget& = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

    [[nodiscard]] auto colorAttachmentCount() const -> uint8_t { return colorAttachmentCount_; }

    [[nodiscard]] auto swapChainLength() const -> size_t { return swapChainLength_; }

    [[nodiscard]] auto currentChainIndex() const -> size_t { return currentChainIndex_; }

    [[nodiscard]] auto getColorAttachment(uint8_t attachmentIndex) const -> vulkan::ImageView const&;

private:
    static void _swapChainCreationErrorHandling(VkResult result);

private:
    VkExtent2D extent_;
    uint8_t colorAttachmentCount_;
    size_t swapChainLength_;
    size_t currentChainIndex_;
    std::optional<Surface> surface_;
    vulkan::Handle<VkSwapchainKHR> vkSwapChain_;
    std::vector<vulkan::Handle<VkFramebuffer>> frameBuffers_;
};

template<typename DepthStencilOutputDescription, typename ColorOutputDescription, size_t modeCandidateCount>
RenderTarget::RenderTarget(vulkan::Device const& device,
                           VkRenderPass vkRenderPass,
                           Surface& surface,
                           DepthStencilOutputDescription,
                           ColorOutputDescription,
                           std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates,
                           VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags)
  : extent_{}
  , colorAttachmentCount_{ 1 }
  , swapChainLength_{ 0 }
  , currentChainIndex_{ 0 }
  , surface_{ std::move(surface) }
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , frameBuffers_{}
{
    assert((surface_->capabilities().supportedCompositeAlpha & vkCompositeAlphaFlags) == vkCompositeAlphaFlags);

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
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, nullptr);

    swapChainLength_ = imageCount;

    std::vector<VkImage> vkImages(swapChainLength_, VK_NULL_HANDLE);

    vkGetSwapchainImagesKHR(device.handle(), static_cast<VkSwapchainKHR>(vkSwapChain_), &imageCount, vkImages.data());

    frameBuffers_.reserve(swapChainLength_);

    for (auto vkImage : vkImages) {
        frameBuffers_.emplace_back(
          device,
          vkRenderPass,
          extent_.width,
          extent_.height,
          std::array<vulkan::ImageView, 1>{ vulkan::ImageView{
            device, std::make_shared<vulkan::Image>(vkImage, extent_.width, extent_.height, surfaceFormat) } });
    }
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t modeCandidateCount>
RenderTarget::Builder<DepthStencilOutputDescription, ColorOutputDescriptions...>::Builder(
  cyclonite::vulkan::Device const& device,
  cyclonite::Surface&& surface,
  std::array<VkPresentModeKHR, modeCandidateCount> const& presentModeCandidates)
  : surface_{ std::move(surface) }
  , vkSwapChain_{ device.handle(), vkDestroySwapchainKHR }
  , extent_{}
  , vkRenderPass_{ VK_NULL_HANDLE }
  , vkPresentMode{ VK_PRESENT_MODE_MAX_ENUM_KHR }
  , vkDepthStencilOutputFormat_{ VK_FORMAT_UNDEFINED }
  , colorOutputFormats_{}
{
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
}

template<typename DepthStencilOutputDescription, typename... ColorOutputDescriptions>
template<size_t modeCount>
auto RenderTarget::Builder<DepthStencilOutputDescription, ColorOutputDescriptions...>::_choosePresentationMode(
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
template<size_t candidateCount>
auto RenderTarget::Builder<DepthStencilOutputDescription, ColorOutputDescriptions...>::_chooseSurfaceFormat(
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
}

#endif // CYCLONITE_RENDERTARGET_H
