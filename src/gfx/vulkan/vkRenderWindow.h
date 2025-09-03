//
// Created by anton on 6/11/25.
//

#ifndef GFX_VK_RENDER_WINDOW_H
#define GFX_VK_RENDER_WINDOW_H

#include "core/resourceRef.h"
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
                 core::ResourceRef deviceRef,
                 uint32_t width,
                 uint32_t height,
                 std::string_view title,
                 SurfaceFlagBits flags);

    ~RenderWindow() = default;

    [[nodiscard]] auto width() const -> uint32_t { return extent_.width; }

    [[nodiscard]] auto height() const -> uint32_t { return extent_.height; }

    [[nodiscard]] auto surfaceHandle() const -> VkSurfaceKHR { return platformSurface_->handle(); }

    using core::ResourceBase::resourceBase;

    void validateSwapchain(VkFormat format, VkPresentModeKHR presentMode);

    void validateDepthStencil(VkFormat format);

private:
    core::ResourceRef deviceRef_;
    VkExtent2D extent_;
    std::unique_ptr<SDL_Window, std::function<void(SDL_Window*)>> sdlWindowPtr_;
    std::unique_ptr<SurfaceKHR> platformSurface_;
    Handle<VkSwapchainKHR> vkSwapchain_;
    std::array<core::ResourceRef, compile_time_config_t::max_swapchain_length_v> depthStencilRefs_;
    uint32_t swapchainLength_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // GFX_VK_RENDER_WINDOW_H
