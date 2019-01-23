//
// Created by bantdit on 1/20/19.
//

#ifndef CYCLONITE_SCENEMANAGER_H
#define CYCLONITE_SCENEMANAGER_H

#include <boost/functional/hash.hpp>
#include <boost/uuid/random_generator.hpp>
#include <unordered_map>

#include "../multithreading/taskManager.h"
#include "scene.h"

namespace cyclonite::core {
template<typename EntityManager, typename SystemManager>
class SceneManager
{
public:
    using scene_t = core::Scene<SceneManager<EntityManager, SystemManager>>;

    explicit SceneManager(multithreading::TaskManager& taskManager);

    auto createScene(std::string const& name) -> scene_t;

private:
    using scene_registry_t = std::unordered_map<boost::uuids::uuid,
                                                std::tuple<std::string, EntityManager, SystemManager>,
                                                boost::hash<boost::uuids::uuid>>;

    multithreading::TaskManager& taskManager_;

    scene_registry_t sceneRegistry_;

    static boost::uuids::random_generator generator_;
};

template<typename EntityManager, typename SystemManager>
SceneManager<EntityManager, SystemManager>::SceneManager(multithreading::TaskManager& taskManager)
  : taskManager_{ taskManager }
  , sceneRegistry_{}
{}

template<typename EntityManager, typename SystemManager>
auto SceneManager<EntityManager, SystemManager>::createScene(std::string const& name) -> scene_t
{
    auto future = taskManager_.strand([&, this]() -> scene_t {
        auto [it, inserted] =
          sceneRegistry_.emplace(generator_(), std::forward_as_tuple(name, EntityManager{}, SystemManager{}));

        assert(inserted);

        (void)inserted;

        auto& [uuid, tuple] = *it;

        auto& [_name, entities, systems] = tuple;

        return scene_t{ uuid, _name, entities, systems };
    });

    return future.get();
}

template<typename EntityManager, typename SystemManager>
boost::uuids::random_generator SceneManager<EntityManager, SystemManager>::generator_{};
}

#endif // CYCLONITE_SCENEMANAGER_H
