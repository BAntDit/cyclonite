//
// Created by anton on 6/13/22.
//

#include "resourceManagementTests.h"
#include "../src/resources/resourceManager.h"
#include "../src/buffers/arena.h"

class TestResource:
  public cyclonite::resources::Resource
  , public cyclonite::buffers::Arena<
      TestResource, cyclonite::resources::ResourceManager::DynamicMemoryAllocator<std::pair<size_t, size_t>>>
{
public:
    using allocator_t = cyclonite::resources::ResourceManager::DynamicMemoryAllocator<std::pair<size_t, size_t>>;

public:
    explicit TestResource(cyclonite::resources::ResourceManager* resourceManager);

    ~TestResource() = default;

    [[nodiscard]] auto ptr() const -> void const* { return testBuffer_.data(); }

    auto ptr() -> void* { return testBuffer_.data(); }

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    void handleDynamicBufferRealloc();

private:
    std::array<std::byte, 2048> testBuffer_;

private:
    static cyclonite::resources::Resource::ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return TestResource::tag; }
    static auto type_tag() -> ResourceTag& { return TestResource::tag; }
};

cyclonite::resources::Resource::ResourceTag TestResource::tag{};

TestResource::TestResource(cyclonite::resources::ResourceManager* resourceManager) :
  cyclonite::resources::Resource(resourceManager)
  , cyclonite::buffers::Arena<TestResource, allocator_t>(
      2048, allocator_t{ resourceManager, &TestResource::type_tag() })
  , testBuffer_{}
{}

void TestResource::handleDynamicBufferRealloc()
{
    freeRanges_ = std::deque(std::move(freeRanges_), freeRanges_.get_allocator());

    /*freeRanges_ =
      std::deque<std::pair<size_t, size_t>, allocator_t>{
          allocator_t{ resourceManager(), &TestResource::type_tag() } };

    for (auto& fr : queue) {
        freeRanges_.push_back(fr);
    }*/
}

void ResourceManagementTestFixture::SetUp()
{
    resourceManager_ = std::make_unique<cyclonite::resources::ResourceManager>(128);
    resourceManager_->template registerResources(cyclonite::resources::resource_reg_info_t<TestResource, 10, 512>{});
}

void ResourceManagementTestFixture::TearDown()
{}

void resourceManagerTest(cyclonite::resources::ResourceManager& resourceManager) {
    {
        auto fullSize = resourceManager.getResourceDynamicBufferSize(TestResource::type_tag());
        auto freeSize = resourceManager.getResourceDynamicBufferFreeSize(TestResource::type_tag());

        ASSERT_EQ(fullSize, freeSize);
    }

    auto id = resourceManager.template create<TestResource>();

    {
        auto freeSize = resourceManager.getResourceDynamicBufferFreeSize(TestResource::type_tag());

        ASSERT_EQ(freeSize, 512 - 8);
    }
}

TEST_F(ResourceManagementTestFixture, ResourceManagerTest)
{
    resourceManagerTest(*resourceManager_);
}