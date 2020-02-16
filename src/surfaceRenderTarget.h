//
// Created by bantdit on 1/7/20.
//

#ifndef CYCLONITE_SURFACERENDERTARGET_H
#define CYCLONITE_SURFACERENDERTARGET_H

#include "baseRenderTarget.h"
#include "surface.h"

namespace cyclonite {
class SurfaceRenderTarget : public BaseRenderTarget
{
public:
    SurfaceRenderTarget(vulkan::Device& device,
                        VkRenderPass vkRenderPass,
                        Surface& surface,
                        vulkan::Handle<VkSwapchainKHR>& vkSwapChain,
                        VkFormat depthStencilFormat,
                        VkClearDepthStencilValue clearDepthStencilValue,
                        VkFormat surfaceFormat,
                        VkClearColorValue clearSurfaceValue,
                        RenderTargetOutputSemantic outputSemantic);

    SurfaceRenderTarget(vulkan::Device& device,
                        VkRenderPass vkRenderPass,
                        Surface& surface,
                        vulkan::Handle<VkSwapchainKHR>& vkSwapChain,
                        VkFormat surfaceFormat,
                        VkClearColorValue clearSurfaceValue,
                        RenderTargetOutputSemantic outputSemantic);

    SurfaceRenderTarget(SurfaceRenderTarget const&) = delete;

    SurfaceRenderTarget(SurfaceRenderTarget&&) = default;

    ~SurfaceRenderTarget() = default;

    auto operator=(SurfaceRenderTarget const&) -> SurfaceRenderTarget& = delete;

    auto operator=(SurfaceRenderTarget &&) -> SurfaceRenderTarget& = default;

    [[nodiscard]] auto frameBufferAvailableSemaphore() const -> VkSemaphore
    {
        return static_cast<VkSemaphore>(imageAvailableSemaphores_[frontBufferIndex_]);
    }

    auto acquireBackBufferIndex(vulkan::Device const& device) -> size_t;

    void swapBuffers(vulkan::Device const& device, VkSemaphore passFinishedSemaphore);

private:
    std::optional<Surface> surface_;
    vulkan::Handle<VkSwapchainKHR> vkSwapChain_;
    std::vector<vulkan::Handle<VkSemaphore>> imageAvailableSemaphores_;
};
}

#endif // CYCLONITE_SURFACERENDERTARGET_H
