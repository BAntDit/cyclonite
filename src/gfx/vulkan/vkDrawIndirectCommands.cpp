//
// Created by anton on 11/3/26.
//

#include "vkDrawIndirectCommands.h"
#include "gfx/buffer.h"
#include "gfx/device.h"

namespace cyclonite::gfx::vulkan {
DrawIndirectCommands::DrawIndirectCommands(core::ResourceSharedRef const& deviceRef, uint32_t initialCommandCount)
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
                          sizeof(VkDrawIndirectCommand) * initialCommandCount);

    shadowBuffer_.resize(initialCommandCount);

    auto& staging = stagingRef_.as<gfx::Buffer>();

    commandsPtr_ = reinterpret_cast<VkDrawIndirectCommand*>(staging.map());
}

void DrawIndirectCommands::addCommand(uint32_t vertexCount,
                                      uint32_t firstVertex,
                                      uint32_t instanceCount,
                                      uint32_t firstInstance)
{
    // TODO:: resize commands

    auto& command = shadowBuffer_[commandCount_++];
    command.vertexCount = vertexCount;
    command.firstVertex = firstVertex;
    command.instanceCount = instanceCount;
    command.firstInstance = firstInstance;

    state_ = DrawIndirectCommandState::UPDATE_STAGING;
}

auto DrawIndirectCommands::vertexCount(size_t commandIndex) const -> uint32_t
{
    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].vertexCount;
}

auto DrawIndirectCommands::vertexCount(size_t commandIndex) -> uint32_t&
{
    state_ = DrawIndirectCommandState::UPDATE_STAGING;

    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].vertexCount;
}

auto DrawIndirectCommands::firstVertex(size_t commandIndex) const -> uint32_t
{
    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].firstVertex;
}

auto DrawIndirectCommands::firstVertex(size_t commandIndex) -> uint32_t&
{
    state_ = DrawIndirectCommandState::UPDATE_STAGING;

    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].firstVertex;
}

auto DrawIndirectCommands::instanceCount(size_t commandIndex) const -> uint32_t
{
    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].instanceCount;
}

auto DrawIndirectCommands::instanceCount(size_t commandIndex) -> uint32_t&
{
    state_ = DrawIndirectCommandState::UPDATE_STAGING;

    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].instanceCount;
}

auto DrawIndirectCommands::firstInstance(size_t commandIndex) const -> uint32_t
{
    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].firstInstance;
}

auto DrawIndirectCommands::firstInstance(size_t commandIndex) -> uint32_t&
{
    state_ = DrawIndirectCommandState::UPDATE_STAGING;

    assert(commandIndex < shadowBuffer_.size());
    return shadowBuffer_[commandIndex].firstInstance;
}

void DrawIndirectCommands::validateStaging()
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
                memcpy(&dst, &src, sizeof(VkDrawIndirectCommand));
                memcpy(commandsPtr_ + writeIndex, &src, sizeof(VkDrawIndirectCommand));

                writeIndex++;
                commandCount_++;
            }
            copySkipped = false;
        }

        if (readIndex < shadowBuffer_.size()) {
            auto& src = shadowBuffer_[readIndex++];
            memcpy(commandsPtr_ + writeIndex++, &src, sizeof(VkDrawIndirectCommand));
            commandCount_++;
        }
    }

    state_ = DrawIndirectCommandState::UPDATE_HW_BUFFER;
}
}
