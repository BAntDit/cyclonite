//
// Created by anton on 6/12/22.
//

#ifndef CYCLONITE_RESOURCEMANAGEMENTTESTS_H
#define CYCLONITE_RESOURCEMANAGEMENTTESTS_H

#include <gtest/gtest.h>

namespace cyclonite::resources
{
class ResourceManager;
}

class ResourceManagementTestFixture: public testing::Test
{
public:
    ResourceManagementTestFixture() = default;

protected:
    void SetUp() override;

    void TearDown() override;

protected:
    std::unique_ptr<cyclonite::resources::ResourceManager> resourceManager_;
};

#endif // CYCLONITE_RESOURCEMANAGEMENTTESTS_H
