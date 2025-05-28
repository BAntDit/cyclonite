//
// Created by anton on 12/30/21.
//

#ifndef EX_CYCLONITE_CONFIG_H
#define EX_CYCLONITE_CONFIG_H

#include <cyclonite.h>

namespace examples::viewer {
using main_component_config_t =
  cyclonite::component_config_t<metrix::type_list<cyclonite::components::Transform,
                                                  cyclonite::components::Mesh,
                                                  cyclonite::components::Camera,
                                                  cyclonite::components::Animator>,
                                metrix::type_list<cyclonite::components::TransformStorage<32, 1>,
                                                  cyclonite::components::MeshStorage<1024>,
                                                  enttx::ComponentStorage<1, 1, cyclonite::components::Camera>,
                                                  cyclonite::components::AnimatorStorage>>;

using empty_component_config_t = cyclonite::component_config_t<metrix::type_list<>, metrix::type_list<>>;

using animation_systems_t = cyclonite::systems_config_t<metrix::value_cast(cyclonite::systems::AnimationStage::COUNT),
                                                        cyclonite::systems::AnimationSystem,
                                                        cyclonite::systems::TransformSystem>;

using g_buffer_node_systems_t = cyclonite::systems_config_t<metrix::value_cast(cyclonite::systems::UpdateStage::COUNT),
                                                            cyclonite::systems::CameraSystem,
                                                            cyclonite::systems::MeshSystem,
                                                            cyclonite::systems::UniformSystem,
                                                            cyclonite::systems::RenderSystem>;

using surface_node_systems_t = cyclonite::systems_config_t<metrix::value_cast(cyclonite::systems::UpdateStage::COUNT),
                                                           cyclonite::systems::RenderSystem>;

struct MainNodeConfig : public cyclonite::Config<main_component_config_t, animation_systems_t>
{
    constexpr static auto is_logic_node_v = true;
};

struct GBufferNodeConfig : public cyclonite::Config<main_component_config_t, g_buffer_node_systems_t>
{};

struct SurfaceNodeConfig : public cyclonite::Config<empty_component_config_t, surface_node_systems_t>
{
    constexpr static auto is_surface_node_v = true;
};

using node_type_register_t =
  cyclonite::compositor::node_type_register<cyclonite::compositor::LogicNode<MainNodeConfig>,
                                            cyclonite::compositor::GraphicsNode<GBufferNodeConfig>,
                                            cyclonite::compositor::GraphicsNode<SurfaceNodeConfig>>;
}

#endif // EX_CYCLONITE_CONFIG_H
