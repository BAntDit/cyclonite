//
// Created by bantdit on 12/10/22.
//

#include "graphicsNodeBuilder.h"
#include "vulkan/internal/imageHelpers.h"

namespace cyclonite::compositor {
auto getDepthStencilImageLayoutByFormat(VkFormat format) -> VkImageLayout
{
    return vulkan::internal::_getDepthStencilImageLayoutByFormat(format);
}
}