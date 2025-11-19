//
// Created by anton on 9/28/25.
//

#include "vkCommandList.h"
#include "gfx/buffer.h"
#include "gfx/commandPool.h"
#include "gfx/descriptorSet.h"
#include "gfx/device.h"
#include "gfx/pipeline.h"
#include "gfx/pipelineBindingSchema.h"
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

void CommandList::beginRenderPass(core::ResourceSharedRef const& renderPassRef)
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

void CommandList::bindPipeline(core::ResourceSharedRef const& pipelineRef)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);
    assert(pipelineRef.valid());

    boundRefs_.emplace_back(pipelineRef);

    auto& pipeline = pipelineRef.as<type_traits::platform_implementation_t<gfx::Pipeline>>();
    auto bindPoint = getPipelineBindingPoint(pipeline.type());
    vkCmdBindPipeline(vkCommandBuffer_, bindPoint, pipeline.handle());
}

void CommandList::bindDescriptorSet(PipelineBindPoint bindPoint,
                                    core::ResourceSharedRef const& bindingSchemaRef,
                                    core::ResourceSharedRef const& descriptorSetRef,
                                    std::span<uint32_t> dynamicOffsets /* = {}*/)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);
    assert(bindingSchemaRef.valid());
    assert(descriptorSetRef.valid());

    boundRefs_.emplace_back(bindingSchemaRef);
    boundRefs_.emplace_back(descriptorSetRef);

    auto& bindingSchema = bindingSchemaRef.as<type_traits::platform_implementation_t<gfx::PipelineBindingSchema>>();
    auto& descriptorSet = descriptorSetRef.as<type_traits::platform_implementation_t<gfx::DescriptorSet>>();

    auto vkBindPoint =
      bindPoint == PipelineBindPoint::GRAPHICS ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;

    auto vkDescriptorSet = descriptorSet.handle();

    vkCmdBindDescriptorSets(vkCommandBuffer_,
                            vkBindPoint,
                            bindingSchema.handle(),
                            descriptorSet.index(),
                            1,
                            &vkDescriptorSet,
                            dynamicOffsets.size(),
                            dynamicOffsets.data());
}

void CommandList::bindIndexBuffer(core::ResourceSharedRef const& bufferRef, size_t offset, IndexType indexType)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);
    assert(bufferRef.valid());

    boundRefs_.emplace_back(bufferRef);
    auto const& buffer = bufferRef.as<type_traits::platform_implementation_t<gfx::Buffer>>();

    auto vkIndexType = (indexType == IndexType::TYPE_UINT32) ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;

    vkCmdBindIndexBuffer(vkCommandBuffer_, buffer.handle(), static_cast<VkDeviceSize>(offset), vkIndexType);
}

void CommandList::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);
    vkCmdDraw(vkCommandBuffer_, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandList::drawIndexed(uint32_t indexCount,
                              uint32_t instanceCount,
                              uint32_t firstIndex,
                              int32_t vertexOffset,
                              uint32_t firstInstance)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);
    vkCmdDrawIndexed(vkCommandBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandList::drawIndirect(core::ResourceSharedRef const& bufferRef, size_t offset, uint32_t count)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);

    assert(bufferRef.valid());
    boundRefs_.emplace_back(bufferRef);

    auto& buffer = bufferRef.as<type_traits::platform_implementation_t<gfx::Buffer>>();

    // stride has no sense cause of PVP
    vkCmdDrawIndirect(vkCommandBuffer_, buffer.handle(), static_cast<VkDeviceSize>(offset), count, 0);
}

void CommandList::drawIndexedIndirect(core::ResourceSharedRef const& bufferRef, size_t offset, uint32_t count)
{
    assert(vkCommandBuffer_ != VK_NULL_HANDLE);

    assert(bufferRef.valid());
    boundRefs_.emplace_back(bufferRef);

    auto& buffer = bufferRef.as<type_traits::platform_implementation_t<gfx::Buffer>>();

    // stride has no sense cause of PVP
    vkCmdDrawIndexedIndirect(vkCommandBuffer_, buffer.handle(), static_cast<VkDeviceSize>(offset), count, 0);
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