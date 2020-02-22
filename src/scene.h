//
// Created by bantdit on 1/12/20.
//

#ifndef CYCLONITE_SCENE_H
#define CYCLONITE_SCENE_H

#include "config.h"

namespace cyclonite {
// TODO:: main TODO for the next time - make possible to fill scene with data for rendering
template<typename EcsConfig>
class Scene
{
public:
    using scene_component_list_t = typename EcsConfig::scene_component_list_t;
    using scene_storage_list_t = typename EcsConfig::scene_storage_list_t;
    using scene_system_list_t = typename EcsConfig::scene_system_list_t;

    using entity_manager_t = typename EcsConfig::entity_manager_t;
    using system_manager_t = typename EcsConfig::system_manager_t;

private:
    entity_manager_t entities_;
    system_manager_t systems_;
};
}

#endif // CYCLONITE_SCENE_H
