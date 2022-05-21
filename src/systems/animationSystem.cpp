//
// Created by anton on 5/21/22.
//

#include "animationSystem.h"

namespace cyclonite::systems {
void AnimationSystem::init(multithreading::TaskManager& taskManager)
{
    taskManager_ = &taskManager;
}
}