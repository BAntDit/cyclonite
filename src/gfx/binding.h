//
// Created by anton on 11/8/25.
//

#ifndef CYCLONITE_GFX_BINDING_H
#define CYCLONITE_GFX_BINDING_H

#include "interfaces/bindingInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkBinding.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Binding = interfaces::BindingInterface<vulkan::Binding>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_BINDING_H
