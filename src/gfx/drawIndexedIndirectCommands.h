
#ifndef CYCLONITE_GFX_DRAW_INDEXED_INDIRECT_COMMANDS_H
#define CYCLONITE_GFX_DRAW_INDEXED_INDIRECT_COMMANDS_H

#include "interfaces/drawIndexedIndirectCommandsInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkDrawIndexedIndirectCommands.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using DrawIndexedIndirectCommands =
  interfaces::DrawIndexedIndirectCommandsInterface<vulkan::DrawIndexedIndirectCommands>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_DRAW_INDEXED_INDIRECT_COMMANDS_H
