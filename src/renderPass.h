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
    RenderPass(VkInstance vkInstance,
               vulkan::Device const& device,
               Options::WindowProperties const& windowProperties,
               VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

    [[nodiscard]] auto renderQueueSubmitInfo() const -> std::vector<VkSubmitInfo> const&
    {
        return renderQueueSubmitInfo_;
    }

private:
    std::optional<Surface> surface_;
    std::vector<VkSubmitInfo> renderQueueSubmitInfo_;
};
}

#endif // CYCLONITE_RENDERPASS_H
