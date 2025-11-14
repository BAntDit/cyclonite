//
// Created by anton on 9/28/25.
//

#include "vkCommandList.h"
#include "gfx/commandPool.h"
#include "gfx/device.h"
#include "gfx/pipeline.h"
#include "gfx/renderPass.h"
#include "vkException.h"
#include <cassert>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
namespace {
auto getPipelineBindingPoint(PipelineType pipelineType) -> VkPipelineBindPoint
{
    auto bindingPoint = VkPipelineBindPoint{ VK_PIPELINE_BIND_POINT_MAX_ENUM };

    switch (pipelineType) {
        case PipelineType::RayTracing:
            [[fallthrough]];
        case PipelineType::GeometricShading:
            [[fallthrough]];
        case PipelineType::PrimitiveRasterization:
            bindingPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            break;
        case PipelineType::Compute:
            bindingPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
            break;
        default:
            assert(false);
    }

    return bindingPoint;
}
}

CommandList::CommandList(core::ResourceWeakRef commandPool)
  : boundRefs_{}
  , commandPool_{ commandPool }
  , vkCommandBuffer_{ VK_NULL_HANDLE }
  , usage_{}
  , state_{ CommandListState::Invalid }
{
    auto poolRef = commandPool.lock();
    assert(poolRef.valid());

    auto& pool = poolRef.as<type_traits::platform_implementation_t<gfx::CommandPool>>();

    auto allocateInfo = VkCommandBufferAllocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = pool.handle();
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    auto deviceRef = pool.device();
    assert(deviceRef.valid());

    auto& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();

    if (auto vkResult = vkAllocateCommandBuffers(device.handle(), &allocateInfo, &vkCommandBuffer_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkAllocateCommandBuffers" };
    }
}

void CommandList::begin(CommandListUsageFlagBits usage)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);

    usage_ = usage;

    auto beginInfo = VkCommandBufferBeginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = usage_.cast_to<VkCommandBufferUsageFlags>();

    if (auto vkResult = vkBeginCommandBuffer(vkCommandBuffer_, &beginInfo); vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkBeginCommandBuffe" };
    }

    state_ = CommandListState::Recording;
}

void CommandList::beginRenderPass(core::ResourceSharedRef renderPassRef)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);
    assert(renderPassRef.valid());

    boundRefs_.emplace_back(renderPassRef);

    auto& renderPass = renderPassRef.as<type_traits::platform_implementation_t<gfx::RenderPass>>();

    auto rect2D = VkRect2D{};
    rect2D.offset.x = 0;
    rect2D.offset.y = 0;
    rect2D.extent.width = renderPass.width();
    rect2D.extent.height = renderPass.height();

    auto beginInfo = VkRenderPassBeginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = renderPass.handle();
    beginInfo.framebuffer = renderPass.frameBuffer();
    beginInfo.renderArea = rect2D;

    auto clearValues = std::array<VkClearValue, config_t::max_color_attachment_count_v + 1>{};
    renderPass.getClearValues(beginInfo.clearValueCount, nullptr);
    renderPass.getClearValues(beginInfo.clearValueCount, clearValues.data());
    beginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(vkCommandBuffer_, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandList::bindPipeline(core::ResourceSharedRef pipelineRef)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);
    assert(pipelineRef.valid());

    auto& pipeline = pipelineRef.as<type_traits::platform_implementation_t<gfx::Pipeline>>();
    auto bindPoint = getPipelineBindingPoint(pipeline.type());
    vkCmdBindPipeline(vkCommandBuffer_, bindPoint, pipeline.handle());
}

void CommandList::endRenderPass()
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);
    vkCmdEndRenderPass(vkCommandBuffer_);
}

void CommandList::end()
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);
    vkEndCommandBuffer(vkCommandBuffer_);

    state_ = CommandListState::Executable;
}

}
#endif // GFX_DRIVER_VULKAN