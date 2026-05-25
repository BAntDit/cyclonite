//
// Created by anton on 5/25/26.
//

#include "renderSystem.h"
#include "gfx/device.h"
#include <array>
#include <span>

namespace cyclonite::systems {
void Renderer::init(core::ResourceSharedRef const& deviceRef, gfx::QueueSubmissionManager* queueSubmissionManager)
{
    queueSubmissionManager_ = queueSubmissionManager;

    deviceRef_ = deviceRef;
    auto& device = deviceRef_.as<gfx::Device>();

    auto allShaderStages = gfx::ShaderStageFlagBits{ gfx::ShaderStageFlags::ALL };
    auto setLayoutFlags = gfx::DescriptorSetLayoutFlagBits{ gfx::DescriptorSetLayoutFlags::UPDATE_AFTER_BIND };
    auto bindingFlags =
      gfx::BindingFlagBits{ gfx::BindingFlags::UPDATE_AFTER_BIND, gfx::BindingFlags::PARTIALLY_BOUND };

    auto bindings = std::array{ gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              0,
                                              gfx::DescriptorType::UNIFORM_BUFFER,
                                              100000,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags },
                                gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              1,
                                              gfx::DescriptorType::STORAGE_BUFFER,
                                              100000,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags },
                                gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              2,
                                              gfx::DescriptorType::SAMPLED_IMAGE,
                                              100000,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags },
                                gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS),
                                              3,
                                              gfx::DescriptorType::COMBINED_IMAGE_SAMPLER,
                                              100000,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags } };

    auto bindingSchema = device.getOrCreatePipelineBindingSchema(bindings, std::span<gfx::PushConstantRange>{});

    globalDescriptorSet_ = device.allocateDescriptorSetBySchema(
      bindingSchema, metrix::value_cast(gfx::DescriptorSpace::GLOBAL_BINDLESS), false);
}
}
