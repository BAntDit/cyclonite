//
// Created by bantdit on 11/13/19.
//

#ifndef CYCLONITE_BASESURFACE_H
#define CYCLONITE_BASESURFACE_H

#include "handle.h"

namespace cyclonite::vulkan {
class BaseSurface
{
public:
    explicit BaseSurface(VkInstance vkInstance);

    BaseSurface(BaseSurface const&) = delete;

    BaseSurface(BaseSurface&&) = default;

    ~BaseSurface() = default;

    auto operator=(BaseSurface const&) -> BaseSurface& = delete;

    auto operator=(BaseSurface&&) -> BaseSurface& = default;

    [[nodiscard]] auto handle() const -> VkSurfaceKHR { return static_cast<VkSurfaceKHR>(vkSurfaceKHR_); }

protected:
    Handle<VkSurfaceKHR> vkSurfaceKHR_;
};
}

#endif // CYCLONITE_BASESURFACE_H
