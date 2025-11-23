//
// Created by anton on 7/13/25.
//

#ifndef CYCLONITE_RESOURCEMANAGER_H
#define CYCLONITE_RESOURCEMANAGER_H

#include "buffer.h"
#include "commandPool.h"
#include "core/resourceManager.h"
#include "descriptorSet.h"
#include "device.h"
#include "gfx/signal.h"
#include "pipeline.h"
#include "pipelineBindingSchema.h"
#include "queueSubmission.h"
#include "renderPass.h"
#include "renderTargetView.h"
#include "renderWindow.h"
#include "sampler.h"
#include "shader.h"
#include "shaderResourceView.h"
#include "texture.h"

namespace cyclonite::gfx {
using resource_manager_t = core::ResourceManager<gfx::Device,
                                                 gfx::RenderWindow,
                                                 gfx::RenderTargetView,
                                                 gfx::RenderPass,
                                                 gfx::Texture,
                                                 gfx::Sampler,
                                                 gfx::Buffer,
                                                 gfx::Shader,
                                                 gfx::ShaderResourceView,
                                                 gfx::Signal,
                                                 gfx::CommandPool,
                                                 gfx::QueueSubmission,
                                                 gfx::PipelineBindingSchema,
                                                 gfx::Pipeline,
                                                 gfx::DescriptorSet>;
}

#endif // CYCLONITE_RESOURCEMANAGER_H
