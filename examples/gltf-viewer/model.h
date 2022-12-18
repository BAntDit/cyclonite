//
// Created by bantdit on 11/2/20.
//

#ifndef CYCLONITE_MODEL_H
#define CYCLONITE_MODEL_H

#include "viewer.h"
#include <cyclonite.h>
#include <filesystem>

namespace examples::viewer {
class Model
{
public:
    Model() noexcept;

    void init(cyclonite::Root& root,
              std::string const& path,
              std::shared_ptr<cyclonite::compositor::Workspace> const& workspace);

    void setCameraTransform(cyclonite::mat4 const& transform);

    void dispose();

private:
    std::shared_ptr<cyclonite::compositor::Workspace> workspace_;
};
}

#endif // CYCLONITE_MODEL_H
