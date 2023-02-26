//
// Created by anton on 2/21/23.
//

#ifndef CYCLONITE_DRAWCOMMANDS_H
#define CYCLONITE_DRAWCOMMANDS_H

#include "resources/resource.h"
#include "vulkan/buffer.h"
#include <memory>

namespace cyclonite::render {
class DrawCommands
{
private:
    resources::Resource::Id materialId_;
    resources::Resource::Id commandStagingId_;
    std::unique_ptr<vulkan::Buffer> gpuCommands_;
};
}

#endif // CYCLONITE_DRAWCOMMANDS_H
