//
// Created by anton on 8/10/26.
//

#include "meshRenderSystem.h"
#include "gfx/binding.h"
#include "gfx/common.h"
#include "gfx/device.h"
#include "gfx/renderPassBuilder.h"

namespace cyclonite::systems {
void MeshRenderSystem::init(core::ResourceSharedRef const& deviceRef,
                            uint32_t width,
                            uint32_t height,
                            core::ResourceSharedRef const& depthStencilTargetRef,
                            core::ResourceSharedRef const& normalsTargetRef)
{
    deviceRef_ = deviceRef;

    auto shaderStages = gfx::ShaderStageFlagBits{ gfx::ShaderStageFlags::VERTEX };
    auto setLayoutFlags = gfx::DescriptorSetLayoutFlagBits{ gfx::DescriptorSetLayoutFlags::UPDATE_AFTER_BIND };
    auto bindingFlags = gfx::BindingFlagBits{ gfx::BindingFlags::UPDATE_AFTER_BIND };

    auto bindings = std::array{ gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::PER_PASS),
                                              0,
                                              gfx::DescriptorType::UNIFORM_BUFFER,
                                              1,
                                              shaderStages,
                                              setLayoutFlags,
                                              bindingFlags } };

    auto& device = deviceRef_.as<gfx::Device>();
    passBindingSchemaRef_ = device.getOrCreatePipelineBindingSchema(bindings, std::span<gfx::PushConstantRange>{});

    auto renderPassBuilder = gfx::RenderPassBuilder{};
    renderPassBuilder.setDevice(deviceRef_);
    renderPassBuilder.setResolution(width, height);
    renderPassBuilder.setDepthStencilAttachment(depthStencilTargetRef);
    renderPassBuilder.addColorAttachment(normalsTargetRef, 0);

    gBufferRenderPassRef_ = core::ResourceSharedRef{ renderPassBuilder.build() };

    // TODO:: make possible to pass resource manager into system
    // TODO:: add space resource
}
}
