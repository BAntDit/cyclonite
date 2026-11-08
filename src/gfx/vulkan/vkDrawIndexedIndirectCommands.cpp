
#include "vkDrawIndexedIndirectCommands.h"
#include "gfx/buffer.h"
#include "gfx/device.h"

namespace cyclonite::gfx::vulkan {
DrawIndexedIndirectCommands::DrawIndexedIndirectCommands(core::ResourceSharedRef const& deviceRef,
                                                         uint32_t initialCommandCount)
  : deviceRef_{ deviceRef }
  , stagingRef_{}
  , shadowBuffer_{}
  , commandsPtr_{ nullptr }
  , commandCount_{ 0 }
  , state_{ DrawIndirectCommandState::UPDATE_NOTHING }
{
    auto& device = deviceRef_.as<gfx::Device>();

    stagingRef_ =
      device.createBuffer(gfx::GpuMemoryAllocationFlagBits{ gfx::GpuMemoryAllocationFlags::HOST_ACCESS_SEQUENTIAL_WRITE,
                                                            gfx::GpuMemoryAllocationFlags::PERSISTENT_MAPPED_MEMORY },
                          gfx::BufferUsageFlagBits{ gfx::BufferUsageFlags::TRANSFER_SRC },
                          sizeof(VkDrawIndexedIndirectCommand) * initialCommandCount);

    shadowBuffer_.resize(initialCommandCount);

    auto& staging = stagingRef_.as<gfx::Buffer>();

    commandsPtr_ = reinterpret_cast<VkDrawIndexedIndirectCommand*>(staging.map());
}

auto DrawIndexedIndirectCommands::indexCount(size_t commandIndex) const -> uint32_t
{
    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].indexCount;
}

auto DrawIndexedIndirectCommands::indexCount(size_t commandIndex) -> uint32_t&
{
    state_ = DrawIndirectCommandState::UPDATE_STAGING;

    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].indexCount;
}

auto DrawIndexedIndirectCommands::firstIndex(size_t commandIndex) const -> uint32_t
{
    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].firstIndex;
}

auto DrawIndexedIndirectCommands::firstIndex(size_t commandIndex) -> uint32_t&
{
    state_ = DrawIndirectCommandState::UPDATE_STAGING;

    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].firstIndex;
}

auto DrawIndexedIndirectCommands::vertexOffset(size_t commandIndex) const -> int32_t
{
    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].vertexOffset;
}

auto DrawIndexedIndirectCommands::vertexOffset(size_t commandIndex) -> int32_t&
{
    state_ = DrawIndirectCommandState::UPDATE_STAGING;

    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].vertexOffset;
}

auto DrawIndexedIndirectCommands::instanceCount(size_t commandIndex) const -> uint32_t
{
    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].instanceCount;
}

auto DrawIndexedIndirectCommands::instanceCount(size_t commandIndex) -> uint32_t&
{
    state_ = DrawIndirectCommandState::UPDATE_STAGING;

    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].instanceCount;
}

auto DrawIndexedIndirectCommands::firstInstance(size_t commandIndex) const -> uint32_t
{
    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].firstInstance;
}

auto DrawIndexedIndirectCommands::firstInstance(size_t commandIndex) -> uint32_t&
{
    state_ = DrawIndirectCommandState::UPDATE_STAGING;

    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].firstInstance;
}

void DrawIndexedIndirectCommands::addCommand(uint32_t indexCount,
                                             uint32_t firstIndex,
                                             int32_t vertexOffset,
                                             uint32_t instanceCount,
                                             uint32_t firstInstance)
{
    // TODO:: resize commands

    auto& command = shadowBuffer_[commandCount_++];
    command.indexCount = indexCount;
    command.firstIndex = firstIndex;
    command.vertexOffset = vertexOffset;
    command.instanceCount = instanceCount;
    command.firstInstance = firstInstance;

    state_ = DrawIndirectCommandState::UPDATE_STAGING;
}

void DrawIndexedIndirectCommands::validateStaging()
{
    auto readIndex = size_t{ 0 };
    auto writeIndex = size_t{ 0 };
    auto copySkipped = false;

    commandCount_ = 0;

    while (readIndex < shadowBuffer_.size()) {
        auto& command = shadowBuffer_[readIndex++];

        if (command.instanceCount == 0) {
            copySkipped = true;
            continue;
        }

        if (copySkipped) {
            auto originalReadIndex = readIndex;
            while (originalReadIndex > writeIndex && readIndex < shadowBuffer_.size()) {
                auto& dst = shadowBuffer_[writeIndex];
                auto& src = shadowBuffer_[readIndex++];
                memcpy(&dst, &src, sizeof(VkDrawIndexedIndirectCommand));
                memcpy(commandsPtr_ + writeIndex, &src, sizeof(VkDrawIndexedIndirectCommand));

                writeIndex++;
                commandCount_++;
            }
            copySkipped = false;
        }

        if (readIndex < shadowBuffer_.size()) {
            auto& src = shadowBuffer_[readIndex++];
            memcpy(commandsPtr_ + writeIndex++, &src, sizeof(VkDrawIndexedIndirectCommand));
            commandCount_++;
        }
    }

    state_ = DrawIndirectCommandState::UPDATE_HW_BUFFER;
}
}
