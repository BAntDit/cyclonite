//
// Created by bantdit on 1/20/19.
//

#ifndef CYCLONITE_SCENE_H
#define CYCLONITE_SCENE_H

#include <boost/uuid/uuid.hpp>
#include <string>

namespace cyclonite::core {
class Scene
{
public:
    Scene(std::string const& name, boost::uuids::uuid const& uuid);

private:
    boost::uuids::uuid uuid_;
    std::string name_;
};
}

#endif // CYCLONITE_SCENE_H
