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
    Scene(std::string const& name, boost::uuids::uuid const& uuid);

private:
    boost::uuids::uuid uuid_;
    std::string name_;

    entity_manager_t* entityManager_;
    system_manager_t* systemManager_;
};
}

#endif // CYCLONITE_SCENE_H
