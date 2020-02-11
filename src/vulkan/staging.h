//
// Created by bantdit on 10/16/19.
//

#ifndef CYCLONITE_STAGING_H
#define CYCLONITE_STAGING_H

#include "../arena.h"
#include "buffer.h"

namespace cyclonite::vulkan {
class Staging : public Arena<Staging>
{
public:
    Staging(Device& device, VkBufferUsageFlags usageFlags, VkDeviceSize size);

    Staging(Staging const&) = delete;

    Staging(Staging&&) = default;

    ~Staging() = default;

    auto operator=(Staging const&) -> Staging& = delete;

    auto operator=(Staging &&) -> Staging& = default;

    [[nodiscard]] auto ptr() const -> void*;

    [[nodiscard]] auto handle() const -> VkBuffer { return buffer_.handle(); }

private:
    Buffer buffer_;
};
}

#endif // CYCLONITE_STAGING_H
