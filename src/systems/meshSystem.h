//
// Created by bantdit on 2/6/20.
//

#ifndef CYCLONITE_MESHSYSTEM_H
#define CYCLONITE_MESHSYSTEM_H

#include "updateStages.h"
#include <vulkan/buffer.h>
#include <vulkan/staging.h>
#include <easy-mp/enum.h>
#include <enttx/enttx.h>

namespace cyclonite::systems {
    class MeshSystem: public enttx::BaseSystem<MeshSystem> {
    public:
    private:
        std::unique_ptr<vulkan::Staging> commandBuffer_;
        std::unique_ptr<vulkan::Buffer> gpuCommandBuffer_;
        std::unique_ptr<vulkan::Buffer> gpuIndicesBuffer_;
    };
}

#endif //CYCLONITE_MESHSYSTEM_H
