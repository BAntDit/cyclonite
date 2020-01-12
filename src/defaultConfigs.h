//
// Created by bantdit on 9/19/19.
//

#ifndef CYCLONITE_DEFAULTCONFIGS_H
#define CYCLONITE_DEFAULTCONFIGS_H

#include "config.h"
#include "core/camera.h"
#include "core/transform.h"
#include "platform.h"
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
    using ecs_config_t =
      EcsConfig<easy_mp::type_list<core::Transform, core::PerspectiveCamera, core::OrthographicCamera>,
                easy_mp::type_list<enttx::ComponentStorage<64, 8, core::Transform>,
                                   enttx::ComponentStorage<8, 1, core::PerspectiveCamera>,
                                   enttx::ComponentStorage<8, 1, core::OrthographicCamera>>,
                easy_mp::type_list<>,
                easy_mp::value_cast(UpdateStage::COUNT)>;
};
}

#endif // CYCLONITE_DEFAULTCONFIGS_H
