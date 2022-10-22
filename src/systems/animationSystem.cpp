//
// Created by anton on 5/21/22.
//

#include "animationSystem.h"

namespace cyclonite::systems {
void AnimationSystem::init(resources::ResourceManager& resourceManager, multithreading::TaskManager& taskManager)
{
    resourceManager_ = &resourceManager;
    taskManager_ = &taskManager;
    start_ = std::chrono::high_resolution_clock::now();
}
}