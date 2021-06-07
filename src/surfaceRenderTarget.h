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
                        VkFormat surfaceFormat,
                        RenderTargetOutputSemantic outputSemantic);

    SurfaceRenderTarget(vulkan::Device& device,
                        VkRenderPass vkRenderPass,
                        Surface& surface,
                        vulkan::Handle<VkSwapchainKHR>& vkSwapChain,
                        VkFormat surfaceFormat,
                        RenderTargetOutputSemantic outputSemantic);

    SurfaceRenderTarget(SurfaceRenderTarget const&) = delete;

    SurfaceRenderTarget(SurfaceRenderTarget&&) = default;

    ~SurfaceRenderTarget() = default;

    auto operator=(SurfaceRenderTarget const&) -> SurfaceRenderTarget& = delete;

    auto operator=(SurfaceRenderTarget &&) -> SurfaceRenderTarget& = default;

    auto acquireBackBufferIndex(vulkan::Device const& device, uint32_t frameIndex) -> std::pair<uint32_t, VkSemaphore>;

    void swapBuffers(vulkan::Device const& device,
                     vulkan::Handle<VkSemaphore> const& signal,
                     uint32_t currentFrameImageIndex);

private:
    std::optional<Surface> surface_;
    vulkan::Handle<VkSwapchainKHR> vkSwapChain_;
    std::vector<vulkan::Handle<VkSemaphore>> imageAvailableSemaphores_;
    std::vector<uint32_t> imageIndices_;
};
}

#endif // CYCLONITE_SURFACERENDERTARGET_H
