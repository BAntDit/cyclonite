//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERTARGET_H
#define CYCLONITE_RENDERTARGET_H

#include <vulkan/vulkan.h>

namespace cyclonite {
class RenderTarget
{
public:
    RenderTarget(RenderTarget const&) = delete;

    RenderTarget(RenderTarget&&) = default;

    ~RenderTarget() = default;

    auto operator=(RenderTarget const&) -> RenderTarget& = delete;

    auto operator=(RenderTarget&&) -> RenderTarget& = default;
};
}

#endif // CYCLONITE_RENDERTARGET_H
