//
// Created by bantdit on 3/21/20.
//

#include "uniformSystem.h"
#include "gfx/device.h"
#include "resources/resourceManager.h"
#include <glm/gtc/type_ptr.hpp>

namespace cyclonite::systems {
void UniformSystem::init(multithreading::TaskManager& taskManager,
                         resources::ResourceManager& resourceManager,
                         size_t swapChainLength)
{
}

void UniformSystem::_init(resources::ResourceManager& resourceManager, size_t swapChainLength) {}

void UniformSystem::setViewMatrix(mat4& viewMatrix) {}

void UniformSystem::setProjectionMatrix(mat4& projectionMatrix) {}

void UniformSystem::setViewProjectionMatrix(mat4& viewProjMatrix) {}

auto UniformSystem::uniforms() const -> resources::Staging const& {}

auto UniformSystem::uniforms() -> resources::Staging& {}

}
