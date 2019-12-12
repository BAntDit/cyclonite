//
// Created by bantdit on 11/3/19.
//

#ifndef CYCLONITE_RENDERTARGET_H
#define CYCLONITE_RENDERTARGET_H

#include "surface.h"

namespace cyclonite {
class RenderTarget
{
public:
    enum class RenderTargetOutputSemantic
    {
        UNDEFINED = 0,
        DEFAULT = 1,
        LINEAR_HDR_COLOR = 2,
        MIN_VALUE = UNDEFINED,
        MAX_VALUE = LINEAR_HDR_COLOR,
        COUNT = MAX_VALUE + 1
    };

    enum class RenderTargetOutputLayout
    {
        UNDEFINED = 0,
        COLOR_ATTACHMENT = 1,
        DEPTH_ATTACHMENT = 2,
        DEPTH_STENCIL_ATTACHMENT = 3,
        MIN_VALUE = UNDEFINED,
        MAX_VALUE = DEPTH_STENCIL_ATTACHMENT,
        COUNT = MAX_VALUE + 1
    };

    template<VkFormat vkFormat, VkColorSpaceKHR vkColorSpaceKHR>
    struct output_format_candidate_t
    {
        constexpr static VkFormat format = vkFormat;
        constexpr static VkColorSpaceKHR colorSpace = vkColorSpaceKHR;
    };

    template<typename Properties, RenderTargetOutputSemantic Semantic, RenderTargetOutputLayout Layout>
    struct output_t;

    template<RenderTargetOutputSemantic Semantic,
             RenderTargetOutputLayout Layout,
             VkFormat... format,
             VkColorSpaceKHR... colorSpace>
    struct output_t<easy_mp::type_list<output_format_candidate_t<format, colorSpace>...>, Semantic, Layout>
    {
        constexpr static std::array<std::pair<VkFormat, VkColorSpaceKHR>, sizeof...(format)> format_candidate_list_v = {
            std::make_pair(format, colorSpace)...
        };

        constexpr static RenderTargetOutputSemantic semantic_v = Semantic;

        constexpr static RenderTargetOutputLayout layout_v = Layout;
    };

public:
    // template<typename DepthStencilAttachment, typename... ColorAttachments>
    RenderTarget(VkRenderPass vkRenderPass);

    // template<typename DepthStencilAttachment, typename ColorAttachment>
    RenderTarget(VkRenderPass vkRenderPass, Surface& surface);

    RenderTarget(RenderTarget const&) = delete;

    RenderTarget(RenderTarget&&) = default;

    ~RenderTarget() = default;

    auto operator=(RenderTarget const&) -> RenderTarget& = delete;

    auto operator=(RenderTarget &&) -> RenderTarget& = default;

private:
    std::optional<Surface> surface_;
};
}

#endif // CYCLONITE_RENDERTARGET_H
