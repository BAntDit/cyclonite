//
// Created by anton on 12/5/21.
//

#include "nodeBuilder.h"
#include "vulkan/internal/imageHelpers.h"

namespace cyclonite::compositor {
auto getDepthStencilImageLayoutByFormat(VkFormat format) -> VkImageLayout
{
    return vulkan::internal::_getDepthStencilImageLayoutByFormat(format);
}
}
