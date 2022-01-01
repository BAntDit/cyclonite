//
// Created by anton on 12/30/21.
//

#ifndef EX_CYCLONITE_CONFIG_H
#define EX_CYCLONITE_CONFIG_H

#include <cyclonite.h>

namespace examples::viewer {
struct MainNodeConfig : public cyclonite::DefaultConfigs
{
    constexpr static bool is_surface_node = false;
};

struct SurfaceNodeConfig : public cyclonite::DefaultConfigs
{
    using ecs_config_t =
      cyclonite::EcsConfig<easy_mp::type_list<>,
                           easy_mp::type_list<>,
                           easy_mp::type_list<cyclonite::systems::RenderSystem, cyclonite::systems::UniformSystem>,
                           easy_mp::value_cast(cyclonite::UpdateStage::COUNT)>;
};

using node_type_register_t = cyclonite::compositor::node_type_register<cyclonite::compositor::Node<MainNodeConfig>,
                                                                       cyclonite::compositor::Node<SurfaceNodeConfig>>;
}

#endif // EX_CYCLONITE_CONFIG_H
