//
// Created by bantdit on 2/23/20.
//

#ifndef CYCLONITE_CAMERASYSTEM_H
#define CYCLONITE_CAMERASYSTEM_H

#include "../components/camera.h"
#include "updateStages.h"
#include "vulkan/staging.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>

namespace cyclonite::systems {
class CameraSystem : public enttx::BaseSystem<CameraSystem>
{
public:
    using tag_t = easy_mp::type_list<components::Camera>;

    CameraSystem() = default;

    ~CameraSystem() = default;

    void init(vulkan::Device& device);

private:
    std::unique_ptr<vulkan::Staging> uniforms_;
    std::unique_ptr<vulkan::Buffer> gpuUniforms_;
    vulkan::Handle<VkSemaphore> transferSemaphore_;
};
}

#endif // CYCLONITE_CAMERASYSTEM_H
