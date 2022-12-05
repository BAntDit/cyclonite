//
// Created by bantdit on 11/11/22.
//

#ifndef CYCLONITE_BASEGRAPHICSNODE_H
#define CYCLONITE_BASEGRAPHICSNODE_H

#include "config.h"
#include "frameBufferRenderTarget.h"
#include "frameCommands.h"
#include "links.h"
#include "node.h"
#include "passType.h"
#include "surfaceRenderTarget.h"
#include <easy-mp/type_list.h>
#include <vector>

namespace cyclonite::compositor {
using namespace easy_mp;

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

class BaseGraphicsNode : public Node
{
    friend class FrameCommands;

public:
    using render_target_t = std::variant<std::monostate, SurfaceRenderTarget, FrameBufferRenderTarget>;

    explicit BaseGraphicsNode(uint8_t bufferCount) noexcept;

    void swapBuffers(vulkan::Device& device);

    template<typename RenderTargetType>
    [[nodiscard]] auto getRenderTarget() const -> RenderTargetType const&;
    template<typename RenderTargetType>
    auto getRenderTarget() -> RenderTargetType&;

    [[nodiscard]] auto getRenderTargetBase() const -> BaseRenderTarget const&;
    auto getRenderTargetBase() -> BaseRenderTarget&;

    [[nodiscard]] auto inputs() const -> Links const& { return inputs_; }
    auto inputs() -> Links& { return inputs_; }

    [[nodiscard]] auto swapChainLength() const -> uint32_t { return swapChainLength_; }

    [[nodiscard]] auto frameBufferIndex() const -> uint32_t { return bufferIndex_; }

    [[nodiscard]] auto passFinishedSemaphore() const -> VkSemaphore;

    [[nodiscard]] auto submitInfo() const -> VkSubmitInfo const& { return submit_; }

public:
    template<NodeConfig Config>
    class Builder;

protected:
    VkSubmitInfo submit_;
    render_target_t renderTarget_;
    uint32_t frameIndex_;
    uint32_t bufferIndex_;

private:
    Links inputs_;
    uint32_t swapChainLength_; // TODO:: gets from RT

    std::vector<vulkan::Handle<VkSemaphore>> semaphores_;
    std::bitset<value_cast(RenderTargetOutputSemantic::COUNT)> publicSemanticBits_;
    std::vector<FrameCommands> frameCommands_;
    vulkan::Handle<VkRenderPass> vkRenderPass_;
};

template<typename RenderTargetType>
auto BaseGraphicsNode::getRenderTarget() const -> RenderTargetType const&
{
    return std::get<RenderTargetType>(renderTarget_);
}

template<typename RenderTargetType>
auto BaseGraphicsNode::getRenderTarget() -> RenderTargetType&
{
    return std::get<RenderTargetType>(renderTarget_);
}
}

#endif // CYCLONITE_BASEGRAPHICSNODE_H
