//
// Created by anton on 7/13/25.
//

#ifndef CYCLONITE_RESOURCEMANAGER_H
#define CYCLONITE_RESOURCEMANAGER_H

#include "commandPool.h"
#include "core/resourceManager.h"
#include "device.h"
#include "renderPass.h"
#include "renderTargetView.h"
#include "renderWindow.h"
#include "texture.h"

namespace cyclonite::gfx {
using resource_manager_t = core::ResourceManager<gfx::Device,
                                                 gfx::RenderWindow,
                                                 gfx::RenderTargetView,
                                                 gfx::RenderPass,
                                                 gfx::Texture,
                                                 gfx::CommandPool>;
}

#endif // CYCLONITE_RESOURCEMANAGER_H
