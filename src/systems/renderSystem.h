//
// Created by anton on 5/18/26.
//

#ifndef CYCLONITE_SYSTEMS_RENDER_SYSTEM_H
#define CYCLONITE_SYSTEMS_RENDER_SYSTEM_H

#include "core/resourceSharedRef.h"
#include "gfx/queueSubmissionManager.h"

namespace cyclonite::systems {
class Renderer
{
public:
    Renderer() = default;

    void init(core::ResourceSharedRef const& device, gfx::QueueSubmissionManager* queueSubmissionManager);

private:
    core::ResourceSharedRef deviceRef_;
    gfx::QueueSubmissionManager* queueSubmissionManager_;
    core::ResourceSharedRef globalDescriptorSet_;
};
}

#endif // CYCLONITE_SYSTEMS_RENDER_SYSTEM_H
