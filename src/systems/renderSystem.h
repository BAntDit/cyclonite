//
// Created by anton on 5/18/26.
//

#ifndef CYCLONITE_SYSTEMS_RENDER_SYSTEM_H
#define CYCLONITE_SYSTEMS_RENDER_SYSTEM_H

#include "components/camera.h"
#include "components/transform.h"
#include "core/hashTable.h"
#include "core/resourceSharedRef.h"
#include "core/ringBuffer.h"
#include "gfx/queueSubmissionManager.h"

namespace cyclonite::systems {
struct PassConstantsTransferJob;
struct PassRenderJob;

class Renderer
{
    friend struct PassConstantsTransferJob;
    friend struct PassRenderJob;

    struct PassConstants
    {
        mat4 view;
        mat4 projection;
        mat4 viewProj;
    };

public:
    Renderer() = default;

    void init(core::ResourceSharedRef const& device,
              core::ResourceSharedRef const& renderWindowRef,
              core::ResourceSharedRef const& vertexShaderRef,
              core::ResourceSharedRef const& pixelShaderRef,
              core::ResourceSharedRef const& materialRef,
              gfx::QueueSubmissionManager& queueSubmissionManager);

    void setupPassConstants(components::Transform const& transform, components::Camera const& camera);

    void render();

    void reset();

private:
    [[nodiscard]] auto getPassDescriptorSet() -> core::ResourceSharedRef;

    [[nodiscard]] auto getPassConstantBuffer() -> core::ResourceSharedRef;

    [[nodiscard]] auto getPassConstantStaging() -> std::pair<core::ResourceSharedRef, PassConstants*>;

    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef renderWindowRef_;
    core::ResourceSharedRef renderPassRef_;
    core::ResourceSharedRef materialRef_;
    gfx::QueueSubmissionManager* queueSubmissionManager_;
    core::ResourceSharedRef passBindingSchemaRef_;
    core::StaticHashTable<core::ResourceSharedRef, 16, uint64_t> passDescriptorSets_;
    core::StaticHashTable<core::ResourceSharedRef, 16, uint64_t> passConstantBuffers_;
    core::StaticHashTable<std::pair<core::ResourceSharedRef, PassConstants*>, 16, uint64_t> passConstantStagings_;
    std::future<void> transferTaskFuture_;
    core::ResourceSharedRef transferSubmission_;
};
}

#endif // CYCLONITE_SYSTEMS_RENDER_SYSTEM_H
