//
// Created by bantdit on 11/8/20.
//

#include "model.h"
#include "gltf/reader.h"
#include "resources/buffer.h"
#include "resources/geometry.h"

namespace examples::viewer {
using namespace cyclonite;

Model::Model() noexcept {}

void Model::init(cyclonite::Root& root, std::string const& path) {}

void Model::setCameraTransform(mat4 const& transform) {}

void Model::dispose() {}
}
