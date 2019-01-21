//
// Created by bantdit on 1/20/19.
//

#ifndef CYCLONITE_SCENEMANAGER_H
#define CYCLONITE_SCENEMANAGER_H

#include <unordered_map>
#include <boost/uuid/random_generator.hpp>
#include "../multithreading/taskManager.h"
#include "scene.h"

namespace cyclonite::core {
template<typename EntityManager, typename SystemManager>
class SceneManager
{
public:
    explicit SceneManager(multithreading::TaskManager& taskManager);

    auto createScene(std::string const& name) -> core::Scene&;

private:
    multithreading::TaskManager& taskManager_;
    std::unordered_map<boost::uuids::uuid, core::Scene> scenes_;
};

template<typename EntityManager, typename SystemManager>
SceneManager<EntityManager, SystemManager>::SceneManager(multithreading::TaskManager& taskManager) :
    taskManager_{taskManager}
{
}

template<typename EntityManager, typename SystemManager>
auto SceneManager<EntityManager, SystemManager>::createScene(std::string const& name) -> core::Scene&
{
    auto uuid = taskManager_.strand([]() -> boost::uuids::uuid {
        static boost::uuids::random_generator generator;
        return generator();
    }).get();
}
}

#endif // CYCLONITE_SCENEMANAGER_H
