//
// Created by anton on 8/30/25.
//

#ifndef CYCLONITE_GFX_RENDERWINDOWBUILDER_H
#define CYCLONITE_GFX_RENDERWINDOWBUILDER_H

#include "gfx/common.h"
#include "gfx/config.h"
#include "core/resourceSharedRef.h"
#include "core/resourceUniqueRef.h"
#include <string_view>
#include <unordered_set>

namespace cyclonite::gfx {
class RenderWindowBuilder
{
public:
    RenderWindowBuilder() = default;

    auto setDevice(core::ResourceSharedRef deviceRef) -> RenderWindowBuilder&;

    auto setTitle(std::string_view title) -> RenderWindowBuilder&;

    auto setResolution(uint32_t width, uint32_t height) -> RenderWindowBuilder&;

    template<typename... Flag>
        requires(std::is_same_v<Flag, SurfaceFlags> && ...)
    auto setSurfaceFlags(Flag... flag) -> RenderWindowBuilder&;

    auto addPresentModeCandidate(PresentMode presentMode) -> RenderWindowBuilder&;

    auto addFormatCandidate(Format format) -> RenderWindowBuilder&;

    auto addDepthStencilFormatCandidate(Format format) -> RenderWindowBuilder&;

    auto build() -> core::ResourceUniqueRef;

private:
    core::ResourceSharedRef deviceRef_;
    std::string_view title_;
    uint32_t width_;
    uint32_t height_;
    SurfaceFlagBits flags_;
    std::unordered_set<PresentMode> presentModeCandidates_;
    std::unordered_set<Format> formatCandidates_;
    std::unordered_set<Format> depthStencilFormatCandidates_;
};

template<typename... Flag>
    requires(std::is_same_v<Flag, SurfaceFlags> && ...)
auto RenderWindowBuilder::setSurfaceFlags(Flag... flag) -> RenderWindowBuilder&
{
    flags_ = SurfaceFlagBits{ flag... };
    return *this;
}
}

#endif // CYCLONITE_GFX_RENDERWINDOWBUILDER_H
