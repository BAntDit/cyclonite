//
// Created by anton on 12/4/20.
//

#ifndef CYCLONITE_BASENODE_H
#define CYCLONITE_BASENODE_H

#include "frameBufferRenderTarget.h"
#include "surfaceRenderTarget.h"
#include <enttx/entity.h>

#include "links.h"
#include "passType.h"
#include "vulkan/shaderModule.h"

namespace cyclonite::compositor {

class BaseNode
{
public:
    template<VkFormat format>
    using render_target_candidate_t = std::integral_constant<VkFormat, format>;

    template<typename Properties,
             RenderTargetOutputSemantic Semantic = RenderTargetOutputSemantic::UNDEFINED,
             VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL>
    struct render_target_output;

    template<RenderTargetOutputSemantic Semantic, VkImageTiling Tiling, VkFormat... format>
    struct render_target_output<type_list<render_target_candidate_t<format>...>, Semantic, Tiling>
    {
        constexpr static std::array<VkFormat, sizeof...(format)> format_candidate_array_v = { format... };

        constexpr static bool is_empty_v = sizeof...(format) == 0;

        constexpr static RenderTargetOutputSemantic semantic_v = Semantic;

        constexpr static VkImageTiling tiling_v = Tiling;
    };

public:
    auto getInputs() -> Links& { return inputs_; }

    [[nodiscard]] auto getInputs() const -> Links const& { return inputs_; }

    [[nodiscard]] auto getOutputs() const -> Links const& { return outputs_; }

    [[nodiscard]] auto cameraEntity() const -> enttx::Entity { return camera_; }

    auto cameraEntity() -> enttx::Entity& { return camera_; }

    template<typename RenderTargetType>
    [[nodiscard]] auto getRenderTarget() const -> RenderTargetType const&;

    template<typename RenderTargetType>
    auto getRenderTarget() const -> RenderTargetType&;

    [[nodiscard]] auto passFinishedSemaphore() const -> vulkan::Handle<VkSemaphore> const&;

    [[nodiscard]] auto descriptorPool() const -> VkDescriptorPool;

    BaseNode(BaseNode const&) = delete;

    BaseNode(BaseNode&&) = default;

    auto operator=(BaseNode const&) -> BaseNode& = delete;

    auto operator=(BaseNode &&) -> BaseNode& = default;

    virtual void render(uint32_t frameIndex, VkFence frameFence) = 0;

    virtual ~BaseNode() = default;

protected:
    BaseNode() noexcept;

private:
    using render_target_t = std::variant<std::monostate, SurfaceRenderTarget, FrameBufferRenderTarget>;

    using color_attachment_properties_t = std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>;

    template<size_t maxSize>
    using color_format_array_t = to_variant_t<
      typename concat<type_list<std::monostate>, array_list_t<color_attachment_properties_t, maxSize>>::type>;

    template<size_t maxSize>
    using attachment_ref_array_t =
      to_variant_t<typename concat<type_list<std::monostate>, array_list_t<VkAttachmentReference, maxSize>>::type>;

    template<size_t maxSize>
    using attachment_idx_array_t =
      to_variant_t<typename concat<type_list<std::monostate>, array_list_t<uint32_t, maxSize>>::type>;

public:
    template<typename NodeType>
    class Builder;

protected:
    uint32_t commandsIndex_;
    enttx::Entity camera_;
    Links inputs_;
    Links outputs_;
    vulkan::Handle<VkRenderPass> vkRenderPass_;
    render_target_t renderTarget_;
    std::vector<vulkan::Handle<VkSemaphore>> signalSemaphores_;
};

template<typename RenderTargetType>
auto BaseNode::getRenderTarget() const -> RenderTargetType const&
{
    return std::get<RenderTargetType>(renderTarget_);
}

template<typename RenderTargetType>
auto BaseNode::getRenderTarget() const -> RenderTargetType&
{
    return std::get<RenderTargetType>(renderTarget_);
}
}

#endif // CYCLONITE_BASENODE_H
