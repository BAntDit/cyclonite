
#include "meshRenderSystem.h"
#include "gfx/renderPassBuilder.h"

namespace cyclonite::systems {
void MeshRenderSystem::init(core::ResourceSharedRef const& deviceRef,
                            uint8_t frameRingSize,
                            core::ResourceSharedRef const& depthRef,
                            core::ResourceSharedRef const& normalsRef,
                            core::ResourceSharedRef const& albedoRef)
{
    deviceRef_ = deviceRef;

    frameRingSize_ = frameRingSize;

    auto& device = deviceRef_.as<gfx::Device>();

    auto& depthTexture = depthRef.as<gfx::Texture>();
    auto& albedoTexture = albedoRef.as<gfx::Texture>();

    {
        auto renderPassBuilder = cyclonite::gfx::RenderPassBuilder{};
        renderPassBuilder.setDevice(deviceRef)
          .setResolution(depthTexture.width(), depthTexture.height())
          .setDepthStencilAttachment(depthRef);

        depthPrePassRef_ = renderPassBuilder.build();
    }

    {
        auto renderPassBuilder = cyclonite::gfx::RenderPassBuilder{};
        renderPassBuilder.setDevice(deviceRef)
          .setResolution(albedoTexture.width(), albedoTexture.height())
          .setDepthStencilAttachment(depthRef)
          .addColorAttachment(normalsRef, 0)
          .addColorAttachment(albedoRef, 0);

        gBufferPassRef_ = renderPassBuilder.build();
    }
}
}
