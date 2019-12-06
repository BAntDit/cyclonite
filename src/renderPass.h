//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERPASS_H
#define CYCLONITE_RENDERPASS_H

#include "options.h"
#include "surface.h"
#include <optional>

namespace cyclonite {
class RenderPass
{
public:
    RenderPass(vulkan::Device const& device,
               Options::WindowProperties const& windowProperties,
               VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

    [[nodiscard]] auto renderQueueSubmitInfo() const -> std::vector<VkSubmitInfo> const&
    {
        return renderQueueSubmitInfo_;
    }

    auto begin() -> std::tuple<VkFence>;

private:
    // TODO:: frame in flight count (define by swapChainLength or make possible to define manually in case render pass
    // has no window)
    std::optional<Surface> surface_;
    vulkan::Handle<VkRenderPass> vkRenderPass_;
    std::vector<vulkan::Handle<VkFence>> frameSyncFences_;
    std::vector<VkSubmitInfo> renderQueueSubmitInfo_;
    uint64_t frameNumber_;
};
}

#endif // CYCLONITE_RENDERPASS_H
