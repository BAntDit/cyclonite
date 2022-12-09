//
// Created by bantdit on 12/9/22.
//

#ifndef CYCLONITE_GRAPHICSNODEBUILDER_H
#define CYCLONITE_GRAPHICSNODEBUILDER_H

#include "graphicsNode.h"
#include <utility>

namespace cyclonite::compositor {
template<NodeConfig Config>
class BaseGraphicsNode::Builder
{
public:
    template<typename T, typename M>
    Builder(vulkan::Device& device,
            resources::ResourceManager& resourceManager,
            T* wsBuilder,
            uint64_t M::*nameToId(std::string_view),
            uint64_t typeId);

    auto setName(std::string_view name) -> Builder&;

    auto addDependency(std::string_view name) -> Builder&;

    auto setOutputResolution(uint16_t width, uint16_t height) -> Builder&;

    template<size_t inputLinkCount>
    auto createInputLinks() -> Builder&;

    template<RenderTargetOutputSemantic... semantic>
    auto setInputs(std::string_view name) -> Builder&;

    template<VkImageTiling Tiling, VkFormat... format, bool IsPublic = false>
    auto setRenderTargetDepthProperties(render_target_output<type_list<render_target_candidate_t<format>...>,
                                                             RenderTargetOutputSemantic::UNDEFINED,
                                                             IsPublic,
                                                             Tiling> &&) -> Builder&;

    template<typename... RenderTargetColorOutput>
    auto setRenderTargetColorProperties(RenderTargetColorOutput&&...)
      -> std::enable_if_t<(sizeof...(RenderTargetColorOutput) > 0), Builder&>;

    template<VkColorSpaceKHR... colorSpaceCandidate, VkPresentModeKHR... presentModeCandidate>
    auto setSurface(WindowProperties&& windowProperties,
                    surface_parameters<type_list<color_space_candidate_t<colorSpaceCandidate>...>,
                                       type_list<present_mode_candidate_t<presentModeCandidate>...>>&&,
                    VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) -> Builder&;

    auto build() -> GraphicsNode<Config>;

private:
    template<size_t candidateCount>
    static auto _findSupportedFormat(std::array<VkFormat, candidateCount> const& candidates,
                                     VkPhysicalDevice physicalDevice,
                                     VkImageTiling requiredTiling,
                                     VkFormatFeatureFlags requiredFeatures) -> VkFormat;

private:
    using color_attachment_properties_t = std::tuple<VkFormat, VkImageTiling, RenderTargetOutputSemantic>;

    template<size_t maxSize>
    using color_format_array_t = to_variant_t<
      typename concat<type_list<std::monostate>, array_list_t<color_attachment_properties_t, maxSize>>::type>;

    vulkan::Device* device_;
    resources::ResourceManager* resourceManager_;
    std::string name_;
    std::function<uint64_t(std::string_view)> nameToNodeId_;
    std::vector<std::string> dependencies_;
    uint64_t nodeTypeId_;
    uint32_t width_;
    uint32_t height_;
    Links inputLinks_;
    VkFormat depthFormat_;
    VkImageTiling depthTiling_;
    std::bitset<value_cast(RenderTargetOutputSemantic::COUNT)> publicSemanticBits_;
    color_format_array_t<16> colorOutputs_;
};

template<NodeConfig Config>
auto BaseGraphicsNode::Builder<Config>::setName(std::string_view name) -> Builder&
{
    name = name_;
    return *this;
}

template<NodeConfig Config>
auto BaseGraphicsNode::Builder<Config>::addDependency(std::string_view name) -> Builder&
{
    dependencies_.emplace_back(name);
    return *this;
}

template<NodeConfig Config>
auto BaseGraphicsNode::Builder<Config>::setOutputResolution(uint16_t width, uint16_t height) -> Builder&
{
    width_ = width;
    height_ = height;

    return *this;
}

template<NodeConfig Config>
template<size_t inputLinkCount>
auto BaseGraphicsNode::Builder<Config>::createInputLinks() -> Builder&
{
    inputLinks_ = Links::create<inputLinkCount>(*device_);
    return *this;
}

template<NodeConfig Config>
template<RenderTargetOutputSemantic... semantic>
auto BaseGraphicsNode::Builder<Config>::setInputs(std::string_view name) -> Builder&
{
    auto nodeId = nameToNodeId_(name);
    assert(nodeId != std::numeric_limits<uint64_t>::max());

    auto it = std::find_if(inputLinks_.begin(), inputLinks_.end(), [=](auto&& link) -> bool {
        return link.nodeId == std::numeric_limits<size_t>::max();
    });

    assert(it != inputLinks_.end());

    auto setter = []<size_t... idx>(Link & link,
                                    size_t nodeId,
                                    std::index_sequence<idx...>&&,
                                    std::array<RenderTargetOutputSemantic, sizeof...(idx)> && semantics)
                    ->void
    {
        link.nodeId = nodeId;
        ((link.semantics[idx] = semantics[idx]), ...);
    };

    setter(*it, nodeId, std::make_index_sequence<sizeof...(semantic)>{}, std::array{ semantic... });

    return *this;
}

template<NodeConfig Config>
template<VkImageTiling Tiling, VkFormat... format, bool IsPublic>
auto BaseGraphicsNode::Builder<Config>::setRenderTargetDepthProperties(
  render_target_output<type_list<render_target_candidate_t<format>...>,
                       RenderTargetOutputSemantic::UNDEFINED,
                       IsPublic,
                       Tiling> &&) -> Builder&
{
    // TODO:: public depth (make possible to access depth outside of the node)

    depthFormat_ = _findSupportedFormat(std::array<VkFormat, sizeof...(format)>{ format... },
                                        device_->physicalDevice(),
                                        Tiling,
                                        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    depthTiling_ = Tiling;

    return *this;
}

template<NodeConfig Config>
template<typename... RenderTargetColorOutput> // TODO:: add concept
auto BaseGraphicsNode::Builder<Config>::setRenderTargetColorProperties(RenderTargetColorOutput&&...)
  -> std::enable_if_t<(sizeof...(RenderTargetColorOutput) > 0), Builder&>
{

    colorOutputs_ = std::array{ std::make_tuple(_findSupportedFormat(RenderTargetColorOutput::format_candidate_array_v,
                                                                     device_->physicalDevice(),
                                                                     RenderTargetColorOutput::tiling_v,
                                                                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT),
                                                RenderTargetColorOutput::tiling_v,
                                                RenderTargetColorOutput::semantic_v)... };

    (publicSemanticBits_.set(value_cast(RenderTargetColorOutput::semantic_v), RenderTargetColorOutput::is_public_v),
     ...);

    return *this;
}

template<NodeConfig Config>
template<VkColorSpaceKHR... colorSpaceCandidate, VkPresentModeKHR... presentModeCandidate>
auto BaseGraphicsNode::Builder<Config>::setSurface(
  WindowProperties&& windowProperties,
  surface_parameters<type_list<color_space_candidate_t<colorSpaceCandidate>...>,
                     type_list<present_mode_candidate_t<presentModeCandidate>...>>&&,
  VkCompositeAlphaFlagBitsKHR vkCompositeAlphaFlags) -> Builder&
{
    // TODO:: ...
    return *this;
}

template<NodeConfig Config>
template<size_t candidateCount>
auto BaseGraphicsNode::Builder<Config>::_findSupportedFormat(std::array<VkFormat, candidateCount> const& candidates,
                                                             VkPhysicalDevice physicalDevice,
                                                             VkImageTiling requiredTiling,
                                                             VkFormatFeatureFlags requiredFeatures) -> VkFormat
{
    for (auto&& candidate : candidates) {
        VkFormatProperties formatProperties = {};
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidate, &formatProperties);

        if ((requiredTiling == VK_IMAGE_TILING_LINEAR &&
             (formatProperties.linearTilingFeatures & requiredFeatures) == requiredFeatures) ||
            (requiredTiling == VK_IMAGE_TILING_OPTIMAL &&
             (formatProperties.optimalTilingFeatures & requiredFeatures) == requiredFeatures)) {
            return candidate;
        }
    }

    throw std::runtime_error("could not find suitable format for render target");
}
}

#endif // CYCLONITE_GRAPHICSNODEBUILDER_H
