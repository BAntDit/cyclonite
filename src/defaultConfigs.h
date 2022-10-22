//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_DEFAULTCONFIGS_H
#define CYCLONITE_DEFAULTCONFIGS_H

#include "components/animator.h"
#include "components/animatorStorage.h"
#include "components/camera.h"
#include "components/mesh.h"
#include "components/meshStorage.h"
#include "components/transform.h"
#include "components/transformStorage.h"
#include "config.h"
#include "platform.h"
#include "systems/animationSystem.h"
#include "systems/cameraSystem.h"
#include "systems/meshSystem.h"
#include "systems/renderSystem.h"
#include "systems/transformSystem.h"
#include "systems/uniformSystem.h"
#include "systems/updateStages.h"
#include "vulkan/androidSurface.h"
#include "vulkan/win32Surface.h"
#include "vulkan/wlSurface.h"
#include "vulkan/xlibSurface.h"
#include <easy-mp/enum.h>
#include <enttx/componentStorage.h>

namespace cyclonite {
struct DefaultConfigs
{
    constexpr static uint8_t pass_count = 1;

    constexpr static bool is_surface_node = true;

    using ecs_config_t =
      EcsConfig<easy_mp::type_list<components::Transform, components::Mesh, components::Camera, components::Animator>,
                easy_mp::type_list<components::TransformStorage<32, 1>,
                                   components::MeshStorage<1024>,
                                   enttx::ComponentStorage<1, 1, components::Camera>,
                                   components::AnimatorStorage>,
                easy_mp::type_list<systems::AnimationSystem,
                                   systems::TransformSystem,
                                   systems::CameraSystem,
                                   systems::RenderSystem,
                                   systems::MeshSystem,
                                   systems::UniformSystem>,
                easy_mp::value_cast(UpdateStage::COUNT)>;
};
}

#endif // CYCLONITE_DEFAULTCONFIGS_H
