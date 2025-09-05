//
// Created by anton on 8/30/25.
//

#include "renderWindowBuilder.h"
#include "common.h"
#include "device.h"
#include "renderWindow.h"
#include <cassert>

#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/internal/utils.h"
#endif

namespace cyclonite::gfx {
auto RenderWindowBuilder::setDevice(core::ResourceRef deviceRef) -> RenderWindowBuilder&
{
    assert(deviceRef.valid());
    deviceRef_ = deviceRef;
    return *this;
}

auto RenderWindowBuilder::setTitle(std::string_view title) -> RenderWindowBuilder&
{
    title_ = title;
    return *this;
}

auto RenderWindowBuilder::setResolution(uint32_t width, uint32_t height) -> RenderWindowBuilder&
{
    width_ = width;
    height_ = height;
    return *this;
}

auto RenderWindowBuilder::addPresentModeCandidate(PresentMode presentMode) -> RenderWindowBuilder&
{
    presentModeCandidates_.emplace(presentMode);
    return *this;
}

auto RenderWindowBuilder::addFormatCandidate(Format format) -> RenderWindowBuilder&
{
    formatCandidates_.emplace(format);
    return *this;
}

auto RenderWindowBuilder::addDepthStencilFormatCandidate(Format format) -> RenderWindowBuilder&
{
    depthStencilFormatCandidates_.emplace(format);
    return *this;
}

auto RenderWindowBuilder::build() -> core::ResourceRef
{
    auto renderWindowRef = core::ResourceRef{};

    renderWindowRef = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>().createRenderWindow(
      width_, height_, title_, flags_);
    assert(renderWindowRef.valid());

#if defined(GFX_DRIVER_VULKAN)
    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();
    auto& renderWindow = renderWindowRef.as<type_traits::platform_implementation_t<gfx::RenderWindow>>();

    auto availablePresentModeCount = uint32_t{ 0 };
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device.physicalDevice(), renderWindow.surfaceHandle(), &availablePresentModeCount, nullptr);

    if (availablePresentModeCount == 0) {
        throw std::runtime_error("there is no available present modes for surface");
    }

    auto availablePresentModes = std::vector<VkPresentModeKHR>(availablePresentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device.physicalDevice(), renderWindow.surfaceHandle(), &availablePresentModeCount, availablePresentModes.data());

    bool presentModeFound = false;
    auto presentMode = PresentMode::Immediate;

    for (auto presentModeCandidate : presentModeCandidates_) {
        auto vkPresentMode = vulkan::internal::getPresentMode(presentModeCandidate);
        for (auto availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vkPresentMode) {
                presentMode = presentModeCandidate;
                presentModeFound = true;
                break;
            }
        }

        if (presentModeFound) {
            break;
        }
    }

    if (!presentModeFound) {
        throw std::runtime_error("could not select suitable available present mode");
    }

    auto availableFormatCount = uint32_t{ 0 };
    vkGetPhysicalDeviceSurfaceFormatsKHR(
      device.physicalDevice(), renderWindow.surfaceHandle(), &availableFormatCount, nullptr);

    if (availableFormatCount == 0) {
        throw std::runtime_error("there is no available formats for surface");
    }

    auto availableFormats = std::vector<VkSurfaceFormatKHR>(availableFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
      device.physicalDevice(), renderWindow.surfaceHandle(), &availableFormatCount, availableFormats.data());

    auto format = Format::UNDEFINED;
    for (auto formatCandidate : formatCandidates_) {
        auto vkFormat = vulkan::internal::getFormat(formatCandidate);
        for (auto [availableFormat, availableColorSpace] : availableFormats) {
            if (availableFormat == vkFormat && availableColorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                auto formatProperties = VkFormatProperties{};
                vkGetPhysicalDeviceFormatProperties(device.physicalDevice(), vkFormat, &formatProperties);

                if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0) {
                    format = formatCandidate;
                    break;
                }
            }
        }

        if (format != Format::UNDEFINED) {
            break;
        }
    }

    if (format == Format::UNDEFINED) {
        throw std::runtime_error("could not find available format for surface");
    }

    renderWindow.validateSwapchain(format, presentMode);

    auto depthStencilFormat = Format::UNDEFINED;
    for (auto depthStencilFormatCandidate : depthStencilFormatCandidates_) {
        auto vkDepthStencilFormat = vulkan::internal::getFormat(depthStencilFormatCandidate);

        auto formatProperties = VkFormatProperties{};
        vkGetPhysicalDeviceFormatProperties(device.physicalDevice(), vkDepthStencilFormat, &formatProperties);
        if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) {
            depthStencilFormat = depthStencilFormatCandidate;
            break;
        }
    }

    if (depthStencilFormat != Format::UNDEFINED) {
        renderWindow.validateDepthStencil(depthStencilFormat);
    }
#endif

    return renderWindowRef;
}
}
