//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_DEFAULTCONFIGS_H
#define CYCLONITE_DEFAULTCONFIGS_H

#include "components/camera.h"
#include "components/mesh.h"
#include "components/transform.h"
#include "components/transformStorage.h"
#include "config.h"
#include "platform.h"
#include "systems/meshSystem.h"
#include "systems/transferSystem.h"
#include "systems/transformSystem.h"
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
    using ecs_config_t = EcsConfig<
      easy_mp::type_list<components::Transform, components::Mesh, components::Camera>,
      easy_mp::type_list<components::TransformStorage<32, 1>,
                         enttx::ComponentStorage<16, 1, components::Mesh>,
                         enttx::ComponentStorage<1, 1, components::Camera>>,
      easy_mp::type_list<systems::TransformSystem, systems::MeshSystem, systems::CameraSystem, systems::TransferSystem>,
      easy_mp::value_cast(UpdateStage::COUNT)>;
};
}

#endif // CYCLONITE_DEFAULTCONFIGS_H
