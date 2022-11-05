//
// Created by bantdit on 11/4/22.
//

#include "../src/multithreading/taskManager.h"
#include "taskManagerTest.h"

void TaskManagerTestFixture::SetUp()
{
    taskManager_ = std::make_unique<cyclonite::multithreading::TaskManager>();
}

void TaskManagerTestFixture::TearDown()
{
    taskManager_.release();
}