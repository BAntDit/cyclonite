//
// Created by bantdit on 8/2/20.
//

#ifndef CYCLONITE_GLTFVIEWER_H
#define CYCLONITE_GLTFVIEWER_H

#include <cyclonite.h>

namespace examples {
class GLTFViewer final
  : public cyclonite::BaseApp<GLTFViewer>
  , public cyclonite::EventReceivable
{
public:
    GLTFViewer();

    auto init(cyclonite::Options const& options) -> GLTFViewer&;

    auto run() -> GLTFViewer&;

    void done();

private:
    bool shutdown_;
    std::unique_ptr<cyclonite::Root> root_;
    ecs_config_t::entity_manager_t entities_;
    ecs_config_t::system_manager_t systems_;
    enttx::Entity cameraEntity_;
};
}

#endif // CYCLONITE_GLTFVIEWER_H
