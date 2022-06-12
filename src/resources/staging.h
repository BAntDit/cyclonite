//
// Created by bantdit on 10/16/19.
//

#ifndef CYCLONITE_STAGING_H
#define CYCLONITE_STAGING_H

#include "buffers/arena.h"
#include "resource.h"
#include "resourceManager.h"
#include "vulkan/buffer.h"

namespace cyclonite::resources {
class Staging
  : public Resource
  , public buffers::Arena<Staging, ResourceManager::DynamicMemoryAllocator<std::pair<size_t, size_t>>>
{
public:
    using allocator_t = ResourceManager::DynamicMemoryAllocator<std::pair<size_t, size_t>>;

public:
    Staging(ResourceManager* resourceManager, vulkan::Device& device, VkBufferUsageFlags usageFlags, VkDeviceSize size);

    Staging(Staging const&) = delete;

    Staging(Staging&&) = default;

    ~Staging() = default;

    auto operator=(Staging const&) -> Staging& = delete;

    auto operator=(Staging &&) -> Staging& = default;

    [[nodiscard]] auto ptr() const -> void*;

    [[nodiscard]] auto handle() const -> VkBuffer { return buffer_.handle(); }

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    void handleDynamicBufferRealloc() override;

private:
    vulkan::Buffer buffer_;

private:
    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return Staging::tag; }
    static auto type_tag() -> ResourceTag& { return Staging::tag; }
};
}

#endif // CYCLONITE_STAGING_H
