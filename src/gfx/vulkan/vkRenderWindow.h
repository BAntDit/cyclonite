//
// Created by anton on 6/11/25.
//

#ifndef GFX_VK_RENDER_WINDOW_H
#define GFX_VK_RENDER_WINDOW_H

#include "core/resourceSharedRef.h"
#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include "gfx/config.h"

#if defined(GFX_DRIVER_VULKAN)

#include "vkSurfaceKHR.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <array>
#include <functional>
#include <memory>
#include <string_view>
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class RenderWindow : public core::ResourceBase
{
public:
    RenderWindow(core::ResourceManagerBase* resourceManager,
                 core::ResourceId resourceId,
                 core::ResourceSharedRef deviceRef,
                 uint32_t width,
                 uint32_t height,
                 std::string_view title,
                 SurfaceFlagBits flags);

    ~RenderWindow() = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

    [[nodiscard]] auto swapchainLength() const -> uint32_t { return swapchainLength_; }

    [[nodiscard]] auto surfaceHandle() const -> VkSurfaceKHR { return platformSurface_->handle(); }

    [[nodiscard]] auto hasDepth() const -> bool { return depthStencilFormat_ != gfx::Format::UNDEFINED; }

    [[nodiscard]] auto colorOutputFormat() const -> gfx::Format { return colorOutputFormat_; }

    [[nodiscard]] auto depthStencilFormat() const -> gfx::Format { return depthStencilFormat_; }

    [[nodiscard]] auto presentMode() const -> gfx::PresentMode { return presentMode_; }

    [[nodiscard]] auto getImageView(size_t swapchainIndex) const -> VkImageView
    {
        return static_cast<VkImageView>(imageViews_[swapchainIndex]);
    }

    [[nodiscard]] auto getDSV(size_t swapchainIndex) -> VkImageView;

    using core::ResourceBase::resourceBase;

    void validateSwapchain(Format format, PresentMode presentMode);

    void validateDepthStencil(Format format);

    [[nodiscard]] auto nextSwapchainIndex(uint64_t currentFrameIndex) -> std::pair<uint32_t, core::ResourceSharedRef>;
    
    void present();

private:
    core::ResourceSharedRef deviceRef_;
    VkExtent2D extent_;
    std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>> sdlWindowPtr_;
    std::unique_ptr<SurfaceKHR> platformSurface_;
    Handle<VkSwapchainKHR> vkSwapchain_;
    std::array<core::ResourceSharedRef, config_t::max_swapchain_length_v> depthStencilRefs_;
    std::array<Handle<VkImageView>, config_t::max_swapchain_length_v> imageViews_;
    std::array<core::ResourceSharedRef, config_t::max_swapchain_length_v> swapchainWaitSignals_;
    std::array<core::ResourceSharedRef, config_t::max_swapchain_length_v> presentationWaitSignals_;
    uint32_t swapchainLength_;
    uint32_t swapchainIndex_;
    gfx::Format colorOutputFormat_;
    gfx::Format depthStencilFormat_;
    gfx::PresentMode presentMode_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // GFX_VK_RENDER_WINDOW_H
