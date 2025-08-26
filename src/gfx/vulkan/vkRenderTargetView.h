//
// Created by anton on 8/25/25.
//

#ifndef CYCLONITE_GFX_VKRENDERTARGETVIEW_H
#define CYCLONITE_GFX_VKRENDERTARGETVIEW_H

#include "core/weakResourceRef.h"
#include "handle.h"

namespace cyclonite::gfx::vulkan {
class RenderTargetView
{
public:
    RenderTargetView(core::WeakResourceRef weakRef, uint32_t mipLevel);

    [[nodiscard]] auto texture() const -> core::WeakResourceRef { return texture_; }

private:
    core::WeakResourceRef texture_;
    Handle<VkImageView> vkImageView_;
};
}

#endif // CYCLONITE_GFX_VKRENDERTARGETVIEW_H
