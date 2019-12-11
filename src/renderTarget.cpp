//
// Created by bantdit on 12/6/19.
//

#include "renderTarget.h"
#include "renderPass.h"

namespace cyclonite {
RenderTarget::RenderTarget(VkRenderPass vkRenderPass, Surface& surface)
  : surface_{ std::move(surface) }
{}
}
