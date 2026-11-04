//
// Created by anton on 11/3/26.
//

#include "vkDrawIndirectCommands.h"
#include "gfx/device.h"

namespace cyclonite::gfx::vulkan
{
DrawIndirectCommands::DrawIndirectCommands(core::ResourceSharedRef deviceRef, uint32_t initialCommandCount) :
    staging_{}
    , commandsPtr_{ nullptr }
    , capacity_{ initialCommandCount }
    , commandCount_{ 0 }
{
    auto& device = deviceRef.as<gfx::Device>();

    auto bufferNewRef = device.createBuffer(
          gfx::GpuMemoryAllocationFlagBits{ gfx::GpuMemoryAllocationFlags::HOST_ACCESS_SEQUENTIAL_WRITE },
          gfx::BufferUsageFlagBits{ gfx::BufferUsageFlags::TRANSFER_SRC },
          sizeof(PassConstants));
}
}

