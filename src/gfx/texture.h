
#ifndef GFX_TEXTURE_H
#define GFX_TEXTURE_H

#include "interfaces/textureInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkTexture.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using Texture = interfaces::TextureInterface<vulkan::Texture>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // GFX_TEXTURE_H
