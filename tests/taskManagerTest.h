//
// Created by bantdit on 11/4/22.
//

#ifndef CYCLONITE_TASKMANAGERTEST_H
#define CYCLONITE_TASKMANAGERTEST_H

#include <gtest/gtest.h>

namespace cyclonite::multithreading
{
class TaskManager;
}

class TaskManagerTestFixture: public testing::Test
{
public:
    TaskManagerTestFixture() = default;

protected:
    void SetUp() override;

    void TearDown() override;

protected:
    std::unique_ptr<cyclonite::multithreading::TaskManager> taskManager_;
};

#endif // CYCLONITE_TASKMANAGERTEST_H
