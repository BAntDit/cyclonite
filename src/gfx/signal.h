//
// Created by anton on 9/27/25.
//

#ifndef CYCLONITE_GFX_SIGNAL_H
#define CYCLONITE_GFX_SIGNAL_H

#include "interfaces/signalInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkSignal.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Signal = interfaces::SignalInterface<vulkan::Signal>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_SIGNAL_H