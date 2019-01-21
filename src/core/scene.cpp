//
// Created by bantdit on 1/20/19.
//

#include "scene.h"

namespace cyclonite::core {
Scene::Scene(std::string const& name, boost::uuids::uuid const& uuid)
  : uuid_{ uuid }
  , name_{ name }
{}
}
