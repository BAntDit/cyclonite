//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERPASS_H
#define CYCLONITE_RENDERPASS_H

#include <vector>
#include <vulkan/vulkan.h>

namespace cyclonite {
class RenderPass
{
public:
    [[nodiscard]] auto renderQueueSubmitInfo() const -> std::vector<VkSubmitInfo> const&
    {
        return renderQueueSubmitInfo_;
    }

private:
    std::vector<VkSubmitInfo> renderQueueSubmitInfo_;
};
}

#endif // CYCLONITE_RENDERPASS_H
