//
// Created by anton on 6/13/22.
//

#include "resourceManagementTests.h"
#include "../src/resources/resourceManager.h"
#include "../src/buffers/arena.h"

class TestResource:
  public cyclonite::resources::Resource
  , public cyclonite::buffers::Arena<TestResource>
{
public:
    TestResource() = default;

    ~TestResource() = default;

    [[nodiscard]] auto ptr() const -> void const* { return testBuffer_.data(); }

    auto ptr() -> void* { return testBuffer_.data(); }

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

private:
    std::array<std::byte, 2048> testBuffer_;

private:
    static cyclonite::resources::Resource::ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return TestResource::tag; }
    static auto type_tag() -> ResourceTag& { return TestResource::tag; }
};

cyclonite::resources::Resource::ResourceTag TestResource::tag{};

void ResourceManagementTestFixture::SetUp()
{
    resourceManager_ = std::make_unique<cyclonite::resources::ResourceManager>();
    resourceManager_->template registerResources(cyclonite::resources::resource_reg_info_t<TestResource, 10, 512>{});
}

void ResourceManagementTestFixture::TearDown()
{}

void resourceManagerTest(cyclonite::resources::ResourceManager& resourceManager) {
    // TODO::
}

TEST_F(ResourceManagementTestFixture, ResourceManagerTest)
{
    resourceManagerTest(*resourceManager_);
}