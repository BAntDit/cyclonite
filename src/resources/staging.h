//
// Created by bantdit on 10/16/19.
//

#ifndef CYCLONITE_STAGING_H
#define CYCLONITE_STAGING_H

#include "buffers/arena.h"
#include "resource.h"
#include "vulkan/buffer.h"

namespace cyclonite::resources {
class Staging
  : public Resource
  , public buffers::Arena<Staging>
{
public:
    Staging(vulkan::Device& device, VkBufferUsageFlags usageFlags, VkDeviceSize size);

    ~Staging() = default;

    [[nodiscard]] auto ptr() const -> void const*;

    [[nodiscard]] auto ptr() -> void*;

    [[nodiscard]] auto handle() const -> VkBuffer { return buffer_.handle(); }

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

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
