//
// Created by anton on 12/4/20.
//

#ifndef CYCLONITE_BASENODE_H
#define CYCLONITE_BASENODE_H

#include "frameBufferRenderTarget.h"
#include "frameCommands.h"
#include "surfaceRenderTarget.h"
#include "uuids.h"
#include <easy-mp/enum.h>
#include <enttx/entity.h>

#include "links.h"
#include "passIterator.h"
#include "passType.h"
#include "vulkan/baseCommandBufferSet.h"
#include "vulkan/buffer.h"

namespace cyclonite::compositor {
template<VkFormat format>
using render_target_candidate_t = std::integral_constant<VkFormat, format>;

template<typename Properties,
         RenderTargetOutputSemantic Semantic = RenderTargetOutputSemantic::UNDEFINED,
         bool is_public_v = false,
         VkImageTiling Tiling = VK_IMAGE_TILING_OPTIMAL>
struct render_target_output;

template<RenderTargetOutputSemantic Semantic, VkImageTiling Tiling, VkFormat... format, bool IsPublic>
struct render_target_output<type_list<render_target_candidate_t<format>...>, Semantic, IsPublic, Tiling>
{
    constexpr static std::array<VkFormat, sizeof...(format)> format_candidate_array_v = { format... };

    constexpr static bool is_empty_v = sizeof...(format) == 0;

    constexpr static RenderTargetOutputSemantic semantic_v = Semantic;

    constexpr static bool is_public_v = IsPublic;

    constexpr static VkImageTiling tiling_v = Tiling;
};

template<VkColorSpaceKHR colorSpace>
using color_space_candidate_t = std::integral_constant<VkColorSpaceKHR, colorSpace>;

template<VkPresentModeKHR presentMode>
using present_mode_candidate_t = std::integral_constant<VkPresentModeKHR, presentMode>;

template<typename ColorSpace, typename PresentMode>
struct surface_parameters;

template<VkColorSpaceKHR... colorSpace, VkPresentModeKHR... presentMode>
struct surface_parameters<type_list<color_space_candidate_t<colorSpace>...>,
                          type_list<present_mode_candidate_t<presentMode>...>>
{
    constexpr static std::array<VkColorSpaceKHR, sizeof...(colorSpace)> color_space_candidate_array_v = {
        colorSpace...
    };

    constexpr static std::array<VkPresentModeKHR, sizeof...(presentMode)> present_mode_candidate_array_v = {
        presentMode...
    };
};

class BaseNode
{
    friend class FrameCommands;

public:
    [[nodiscard]] auto uuid() const -> boost::uuids::uuid { return uuid_; }

    auto getInputs() -> Links& { return inputs_; }

    [[nodiscard]] auto getInputs() const -> Links const& { return inputs_; }

    [[nodiscard]] auto cameraEntity() const -> enttx::Entity { return camera_; }

    auto cameraEntity() -> enttx::Entity& { return camera_; }

    template<typename RenderTargetType>
    [[nodiscard]] auto getRenderTarget() const -> RenderTargetType const&;

    template<typename RenderTargetType>
    auto getRenderTarget() -> RenderTargetType&;

    auto getRenderTargetBase() -> BaseRenderTarget&;

    [[nodiscard]] auto getRenderTargetBase() const -> BaseRenderTarget const&;

    [[nodiscard]] auto passFinishedSemaphore() const -> vulkan::Handle<VkSemaphore> const&;

    [[nodiscard]] auto commandIndex() const -> uint32_t { return commandsIndex_; }

    BaseNode(BaseNode const&) = delete;

    BaseNode(BaseNode&&) = default;

    auto operator=(BaseNode const&) -> BaseNode& = delete;

    auto operator=(BaseNode &&) -> BaseNode& = default;

    ~BaseNode() = default;

protected:
    explicit BaseNode(size_t bufferCount) noexcept;

    [[nodiscard]] auto frameCommands() const -> std::pair<uint32_t, VkCommandBuffer const*>;

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
    template<typename NodeConfig>
    class Builder;

protected:
    boost::uuids::uuid uuid_;
    uint32_t commandsIndex_;
    enttx::Entity camera_;
    Links inputs_;
    std::bitset<value_cast(RenderTargetOutputSemantic::COUNT)> publicSemanticBits_;
    vulkan::Handle<VkRenderPass> vkRenderPass_;
    render_target_t renderTarget_;
    std::vector<vulkan::Handle<VkSemaphore>> signalSemaphores_;
    std::vector<FrameCommands> frameCommands_;
};

template<typename RenderTargetType>
auto BaseNode::getRenderTarget() const -> RenderTargetType const&
{
    return std::get<RenderTargetType>(renderTarget_);
}

template<typename RenderTargetType>
auto BaseNode::getRenderTarget() -> RenderTargetType&
{
    return std::get<RenderTargetType>(renderTarget_);
}
}

#endif // CYCLONITE_BASENODE_H
