//
// Created by bantdit on 1/20/19.
//

#ifndef CYCLONITE_SCENE_H
#define CYCLONITE_SCENE_H

#include <boost/uuid/uuid.hpp>
#include <string>

namespace cyclonite::core {
template<typename SceneManager>
class Scene
{
public:
    friend SceneManager;

    using entity_manager_t = typename SceneManager::entity_manager_t;
    using system_manager_t = typename SceneManager::system_manager_t;

private:
    Scene(boost::uuids::uuid const& uuid,
          std::string const& name,
          entity_manager_t& entities,
          system_manager_t& systems);

private:
    boost::uuids::uuid uuid_;
    std::string name_;

    entity_manager_t* entities_;
    system_manager_t* systems_;
};

template<typename SceneManager>
Scene<SceneManager>::Scene(boost::uuids::uuid const& uuid,
                           std::string const& name,
                           entity_manager_t& entities,
                           system_manager_t& systems)
  : uuid_{ uuid }
  , name_{ name }
  , entities_{ &entities }
  , systems_{ &systems }
{}
}

#endif // CYCLONITE_SCENE_H
