//
// Created by anton on 12/30/21.
//

#ifndef EX_CYCLONITE_CONFIG_H
#define EX_CYCLONITE_CONFIG_H

#include <cyclonite.h>

namespace examples::viewer {
using main_component_config_t =
  cyclonite::component_config_t<type_list<cyclonite::components::Transform,
                                          cyclonite::components::Mesh,
                                          cyclonite::components::Camera,
                                          cyclonite::components::Animator>,
                                type_list<cyclonite::components::TransformStorage<32, 1>,
                                          cyclonite::components::MeshStorage<1024>,
                                          enttx::ComponentStorage<1, 1, cyclonite::components::Camera>,
                                          cyclonite::components::AnimatorStorage>>;

using animation_systems_t = cyclonite::systems_config_t<value_cast(cyclonite::systems::AnimationStage::COUNT),
                                                        cyclonite::systems::AnimationSystem,
                                                        cyclonite::systems::TransformSystem>;

using g_buffer_node_systems_t = cyclonite::systems_config_t<value_cast(cyclonite::systems::UpdateStage::COUNT),
                                                            cyclonite::systems::CameraSystem,
                                                            cyclonite::systems::MeshSystem,
                                                            cyclonite::systems::UniformSystem,
                                                            cyclonite::systems::RenderSystem>;

struct MainNodeConfig : public cyclonite::Config<main_component_config_t, animation_systems_t>
{
    constexpr static bool is_logic_node_v = true;
};

struct GBufferNodeConfig : public cyclonite::Config<main_component_config_t, g_buffer_node_systems_t>
{};

using node_type_register_t =
  cyclonite::compositor::node_type_register<cyclonite::compositor::LogicNode<MainNodeConfig>,
                                            cyclonite::compositor::GraphicsNode<GBufferNodeConfig>>;

/* struct MainNodeConfig : public cyclonite::DefaultConfigs
{
    constexpr static bool is_surface_node = false;
};

struct SurfaceNodeConfig : public cyclonite::DefaultConfigs
{
    using ecs_config_t = cyclonite::EcsConfig<easy_mp::type_list<>,
                                              easy_mp::type_list<>,
                                              easy_mp::type_list<cyclonite::systems::RenderSystem>,
                                              easy_mp::value_cast(cyclonite::UpdateStage::COUNT)>;
};

using node_type_register_t = cyclonite::compositor::node_type_register<cyclonite::compositor::Node<MainNodeConfig>,
                                                                       cyclonite::compositor::Node<SurfaceNodeConfig>>;*/
}

#endif // EX_CYCLONITE_CONFIG_H
